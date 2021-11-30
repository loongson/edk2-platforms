/** @file
  Platform PEI driver

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


//
// The package level header files this module uses
//
#include <PiPei.h>

//
// The Library classes this module consumes
//
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include <Library/ResourcePublicationLib.h>
#include <Guid/MemoryTypeInformation.h>
#include <Library/QemuFwCfgLib.h>

#include <Ppi/MasterBootMode.h>

#include "Platform.h"

/* TODO */
EFI_MEMORY_TYPE_INFORMATION mDefaultMemoryTypeInformation[] = {
  { EfiReservedMemoryType,  0x004 },
  { EfiRuntimeServicesData, 0x024 },
  { EfiRuntimeServicesCode, 0x030 },
  { EfiBootServicesCode,    0x180 },
  { EfiBootServicesData,    0xF00 },
  { EfiMaxMemoryType,       0x000 }
};

//
// Module globals
//
CONST EFI_PEI_PPI_DESCRIPTOR  mPpiListBootMode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiMasterBootModePpiGuid,
  NULL
};

VOID
AddReservedMemoryBaseSizeHob (
  EFI_PHYSICAL_ADDRESS        MemoryBase,
  UINT64                      MemorySize
  )
{
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
      EFI_RESOURCE_ATTRIBUTE_PRESENT     |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED,
    MemoryBase,
    MemorySize
    );
}

VOID
AddMemoryBaseSizeHob (
  EFI_PHYSICAL_ADDRESS        MemoryBase,
  UINT64                      MemorySize
  )
{
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
      EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED,
    MemoryBase,
    MemorySize
    );
}


VOID
AddMemoryRangeHob (
  EFI_PHYSICAL_ADDRESS        MemoryBase,
  EFI_PHYSICAL_ADDRESS        MemoryLimit
  )
{
  AddMemoryBaseSizeHob (MemoryBase, (UINT64)(MemoryLimit - MemoryBase));
}


VOID
MemMapInitialization (
  VOID
  )
{
  DEBUG((EFI_D_INFO, "==%a==\n", __func__));
  //
  // Create Memory Type Information HOB
  //
  BuildGuidDataHob (
    &gEfiMemoryTypeInformationGuid,
    mDefaultMemoryTypeInformation,
    sizeof(mDefaultMemoryTypeInformation)
    );
}


VOID
MiscInitialization (
  VOID
  )
{
  DEBUG((EFI_D_INFO, "==%a==\n", __func__));
  //
  // Creat CPU HOBs.
  //
  BuildCpuHob (PcdGet8 (PcdPrePiCpuMemorySize), PcdGet8 (PcdPrePiCpuIoSize));
}


/**
  Fetch the number of boot CPUs from QEMU.
**/
VOID
CpuCountInitialization (
  VOID
  )
{
  UINT16        ProcessorCount;
  UINT16        MaxProcessorCount;
  RETURN_STATUS PcdStatus;

  QemuFwCfgSelectItem (QemuFwCfgItemSmpCpuCount);
  ProcessorCount = QemuFwCfgRead16 ();
  //ProcessorCount = 1;
  QemuFwCfgSelectItem (QemuFwCfgItemMaximumCpuCount);
  MaxProcessorCount = QemuFwCfgRead16 ();
  //MaxProcessorCount = 1;

  DEBUG ((DEBUG_INFO, "%a: QEMU reports %d processor(s)\n", __FUNCTION__,
    ProcessorCount));
  DEBUG ((DEBUG_INFO, "%a: QEMU support maximum %d processor(s)\n", __FUNCTION__,
    MaxProcessorCount));
  //
  // If the fw_cfg key or fw_cfg entirely is unavailable, no change to PCD.
  //
  if ((ProcessorCount == 0) || (MaxProcessorCount == 0)) {
    return;
  }

  if (ProcessorCount > MaxProcessorCount) {
    ProcessorCount = MaxProcessorCount;
  }

  //
  // Otherwise, set them to PCD.
  //
  PcdStatus = PcdSet16S(PcdLocalCpuCount, ProcessorCount);
  ASSERT_RETURN_ERROR (PcdStatus);
  PcdStatus = PcdSet16S(PcdMaximumCpuCount, MaxProcessorCount);
  ASSERT_RETURN_ERROR (PcdStatus);

}

/**
  Fetch the size of system memory from QEMU.
**/
VOID
SystemMemorySizeInitialization (
  VOID
  )
{
  UINT64        RamSize;
  RETURN_STATUS PcdStatus;

  QemuFwCfgSelectItem (QemuFwCfgItemRamSize);
  RamSize= QemuFwCfgRead64 ();
  DEBUG ((DEBUG_INFO, "%a: QEMU reports %dM system memory\n", __FUNCTION__,
    RamSize/1024/1024));

  //
  // If the fw_cfg key or fw_cfg entirely is unavailable, no change to PCD.
  //
  if (RamSize == 0) {
    return;
  }

  //
  // Otherwise, set RamSize to PCD.
  //
  PcdStatus =  PcdSet64S(PcdRamSize, RamSize);
  ASSERT_RETURN_ERROR (PcdStatus);
}

/**
  Fetch the numa info from QEMU.
**/
VOID
NumaDataInitialization (
  VOID
  )
{
  UINT16 MaxCpus,i,NrCpusPerNode = 0;
  UINT64 NumaNodes;
  UINT64 NumaCfgSize ;
  UINT64 *NumaCfgBuf ;

  MaxCpus = PcdGet16 (PcdMaximumCpuCount);

  QemuFwCfgSelectItem (QemuFwCfgItemNumaData);
  NumaNodes = QemuFwCfgRead64 ();
  //NumaNodes = 0;
  if(NumaNodes ==  0)
    return;

  PcdSet64S(PcdNumaNodes, NumaNodes);
  NumaCfgSize = (1 + MaxCpus + NumaNodes) * sizeof(*NumaCfgBuf);
  NumaCfgBuf = AllocateZeroPool(NumaCfgSize);
  QemuFwCfgSelectItem (QemuFwCfgItemNumaData);
  QemuFwCfgReadBytes (NumaCfgSize, NumaCfgBuf);

  //ZeroMem (NumaCfgBuf, NumaCfgSize);
  for(i = 0;i < MaxCpus ;i++){
    if(NumaCfgBuf[1+i] == 0)
      NrCpusPerNode++;
  }
  PcdSet16S(PcdCpusPerNode, NrCpusPerNode);

  DEBUG ((EFI_D_INFO, "max_cpus %d,nb_numa_nodes 0x%lx,NumaCfgSize 0x%lx\n",
    MaxCpus,NumaNodes,NumaCfgSize));
}

/**
  Perform Platform PEI initialization.

  @param  FileHandle      Handle of the file being invoked.
  @param  PeiServices     Describes the list of possible PEI Services.

  @return EFI_SUCCESS     The PEIM initialized successfully.

**/
EFI_STATUS
EFIAPI
InitializePlatform (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS Status;

  DEBUG ((EFI_D_INFO, "Platform PEIM Loaded\n"));

  Status = PeiServicesInstallPpi (&mPpiListBootMode);
  ASSERT_EFI_ERROR (Status);

  CpuCountInitialization ();
  SystemMemorySizeInitialization ();
  NumaDataInitialization ();
  PublishPeiMemory ();
  PeiFvInitialization ();
  InitializeRamRegions ();
  MemMapInitialization ();
  MiscInitialization ();

  return EFI_SUCCESS;
}
