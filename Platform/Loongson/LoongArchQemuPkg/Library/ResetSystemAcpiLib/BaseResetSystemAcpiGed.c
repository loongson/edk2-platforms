/** @file

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/DebugLib.h>
#include "ResetSystemAcpiGed.h"

EFI_STATUS
ResetSystemLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  
  Status = GetPowerManagerByParseAcpiInfo ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d\n",  __FUNCTION__, __LINE__));
  }

  ASSERT (PowerManager.SleepControlRegAddr);
  ASSERT (PowerManager.SleepStatusRegAddr);
  ASSERT (PowerManager.ResetRegAddr);

  return Status;
}
