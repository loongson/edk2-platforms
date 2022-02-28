/** @file
  Memory Detection for Virtual Machines.

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
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/ResourcePublicationLib.h>

#include "Platform.h"

/**
  Publish PEI core memory

  @return EFI_SUCCESS     The PEIM initialized successfully.

**/
EFI_STATUS
PublishPeiMemory (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT64 Base;
  UINT64      Size;
  UINT64      RamTop;

  //
  // Determine the range of memory to use during PEI
  //
  Base = PcdGet64 (PcdSecPeiTempRamBase) + PcdGet32 (PcdSecPeiTempRamSize);
  RamTop = PcdGet64 (PcdUefiRamTop);
  Size = RamTop - Base;


  //
  // Publish this memory to the PEI Core
  //
  Status = PublishSystemMemory (Base, Size);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "Publish Memory Initialize done.\n"));
  return Status;
}

/**
  Peform Memory Detection
  Publish system RAM and reserve memory regions
**/
VOID
InitializeRamRegions (
  VOID
  )
{
  UINT64 Base;
  UINT64 End;


  //
  // DDR memory space address range
  // 0x00000000 - 0x10000000  lower 256M memory space
  // 0x90000000 - BASE_4GB    if there is
  // BASE_4GB   - 
  Base = PcdGet64 (PcdRamRegionsBottom);
  End  = Base + PcdGet64 (PcdRamSize) - 0x10000000;

  //
  // Create memory HOBs.
  // Put memory below 4G address space at the first memory HOB
  //
  DEBUG ((EFI_D_INFO, "%a: MemoryBase=%llx, MemoryEnd=%llx\n", __FUNCTION__, Base, End));

  if (End > BASE_4GB) {
    AddMemoryRangeHob (Base, BASE_4GB);
    AddMemoryRangeHob (BASE_4GB, End);
  } else {

    AddMemoryRangeHob (Base, End);
  }
  AddMemoryRangeHob (0x0, 0x10000000);

  //
  // Lock the scope of the cache.
  //
  BuildMemoryAllocationHob (
    PcdGet64 (PcdSecPeiTempRamBase),
    PcdGet32 (PcdSecPeiTempRamSize),
    EfiACPIMemoryNVS
    );

  //
  // SEC stores its table of GUIDed section handlers here.
  //
  BuildMemoryAllocationHob (
    PcdGet64 (PcdGuidedExtractHandlerTableAddress),
    PcdGet32 (PcdGuidedExtractHandlerTableSize),
    EfiACPIMemoryNVS
    );
}
