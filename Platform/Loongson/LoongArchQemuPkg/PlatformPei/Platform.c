/** @file
  Platform PEI driver

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Mem - Memory
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
#include <Library/MmuLib.h>
#include <Guid/FdtHob.h>
#include <libfdt.h>
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

/**
  Create Reserved type memory range hand off block.

  @param  MemoryBase    memory base address.
  @param  MemoryLimit  memory length.

  @return  VOID
**/
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
/**
  Create system type  memory range hand off block.

  @param  MemoryBase    memory base address.
  @param  MemoryLimit  memory length.

  @return  VOID
**/
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

/**
  Create  memory range hand off block.

  @param  MemoryBase    memory base address.
  @param  MemoryLimit  memory length.

  @return  VOID
**/
VOID
AddMemoryRangeHob (
  EFI_PHYSICAL_ADDRESS        MemoryBase,
  EFI_PHYSICAL_ADDRESS        MemoryLimit
  )
{
  AddMemoryBaseSizeHob (MemoryBase, (UINT64) (MemoryLimit - MemoryBase));
}
/**
  Create  memory type information hand off block.

  @param  VOID

  @return  VOID
**/
VOID
MemMapInitialization (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "==%a==\n", __func__));
  //
  // Create Memory Type Information HOB
  //
  BuildGuidDataHob (
    &gEfiMemoryTypeInformationGuid,
    mDefaultMemoryTypeInformation,
    sizeof (mDefaultMemoryTypeInformation)
    );
}

/**
  Misc Initialization.

  @param  VOID

  @return  VOID
**/
VOID
MiscInitialization (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "==%a==\n", __func__));
  //
  // Creat CPU HOBs.
  //
  BuildCpuHob (PcdGet8 (PcdPrePiCpuMemorySize), PcdGet8 (PcdPrePiCpuIoSize));
}
/**
  add fdt hand off block.

  @param  VOID

  @return  VOID
**/
VOID
AddFdtHob (VOID)
{
  VOID    *Base;
  VOID    *NewBase;
  UINTN   FdtSize;
  UINTN   FdtPages;
  UINT64  *FdtHobData;

  Base = (VOID*)(UINTN)PcdGet64 (PcdDeviceTreeBase);
  ASSERT (Base != NULL);

  FdtSize = fdt_totalsize (Base) + PcdGet32 (PcdDeviceTreePadding);
  FdtPages = EFI_SIZE_TO_PAGES (FdtSize);
  NewBase = AllocatePages (FdtPages);
  ASSERT (NewBase != NULL);
  fdt_open_into (Base, NewBase, EFI_PAGES_TO_SIZE (FdtPages));

  FdtHobData = BuildGuidHob (&gFdtHobGuid, sizeof *FdtHobData);
  ASSERT (FdtHobData != NULL);
  *FdtHobData = (UINTN)NewBase;

}

/**
  Fetch the size of system memory from QEMU.

  @param  VOID

  @return  VOID
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
  PcdStatus =  PcdSet64S (PcdRamSize, RamSize);
  ASSERT_RETURN_ERROR (PcdStatus);
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

  DEBUG ((DEBUG_INFO, "Platform PEIM Loaded\n"));

  Status = PeiServicesInstallPpi (&mPpiListBootMode);
  ASSERT_EFI_ERROR (Status);

  SystemMemorySizeInitialization ();
  PublishPeiMemory ();
  PeiFvInitialization ();
  InitializeRamRegions ();
  MemMapInitialization ();
  MiscInitialization ();
  AddFdtHob ();
  ConfigureMmu ();

  return EFI_SUCCESS;
}
