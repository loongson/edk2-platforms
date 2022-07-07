/** @file

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeLib.h> // EfiConvertPointer()
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "ResetSystemAcpiGed.h"

EFI_STATUS
SetMemoryAttributesRunTime (
  UINTN Address
  )
{
  EFI_STATUS  Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  Descriptor;

  Address &= ~EFI_PAGE_MASK;

  Status = gDS->GetMemorySpaceDescriptor (Address, &Descriptor);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a: GetMemorySpaceDescriptor failed\n", __FUNCTION__));
    return Status;
  }

  if (Descriptor.GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
    Status = gDS->AddMemorySpace (
                  EfiGcdMemoryTypeMemoryMappedIo,
                  Address,
                  EFI_PAGE_SIZE,
                  EFI_MEMORY_UC | EFI_MEMORY_RUNTIME
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "%a: AddMemorySpace failed\n", __FUNCTION__));
      return Status;
    }

    Status = gDS->SetMemorySpaceAttributes (
                  Address,
                  EFI_PAGE_SIZE,
                  EFI_MEMORY_RUNTIME
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "%a:%d SetMemorySpaceAttributes failed\n", __FUNCTION__, __LINE__));
      return Status;
    }
  } else if (!(Descriptor.Attributes & EFI_MEMORY_RUNTIME)) {
    Status = gDS->SetMemorySpaceAttributes (
                  Address,
                  EFI_PAGE_SIZE,
                  Descriptor.Attributes | EFI_MEMORY_RUNTIME
                  );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "%a:%d SetMemorySpaceAttributes failed\n", __FUNCTION__, __LINE__));
      return Status;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PowerManagerInit (VOID)
{
  EFI_STATUS                       Status;

  Status = GetPowerManagerByParseAcpiInfo ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a: sleepControl %llx\n", __FUNCTION__, PowerManager.SleepControlRegAddr));
  ASSERT (PowerManager.SleepControlRegAddr);
  Status =  SetMemoryAttributesRunTime (PowerManager.SleepControlRegAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d\n",  __FUNCTION__, __LINE__));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a: sleepStatus %llx\n", __FUNCTION__, PowerManager.SleepStatusRegAddr));
  ASSERT (PowerManager.SleepStatusRegAddr);
  Status =  SetMemoryAttributesRunTime (PowerManager.SleepStatusRegAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d\n",  __FUNCTION__, __LINE__));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "%a: ResetReg %llx\n", __FUNCTION__, PowerManager.ResetRegAddr));
  ASSERT (PowerManager.ResetRegAddr);
  Status =  SetMemoryAttributesRunTime (PowerManager.ResetRegAddr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d\n",  __FUNCTION__, __LINE__));
    return Status;
  }

  return Status;
}

VOID
EFIAPI
ResetSystemLibAddressChangeEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EfiConvertPointer (0, (VOID **)&PowerManager.SleepControlRegAddr);
  EfiConvertPointer (0, (VOID **)&PowerManager.SleepStatusRegAddr);
  EfiConvertPointer (0, (VOID **)&PowerManager.ResetRegAddr);
}


EFI_STATUS
EFIAPI
ResetSystemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   ResetSystemVirtualNotifyEvent;
  
  Status = PowerManagerInit ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d\n",  __FUNCTION__, __LINE__));
    return Status;
  }
  //
  // Register SetVirtualAddressMap () notify function
  //
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                  TPL_NOTIFY,
                  ResetSystemLibAddressChangeEvent,
                  NULL,
                  &ResetSystemVirtualNotifyEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
