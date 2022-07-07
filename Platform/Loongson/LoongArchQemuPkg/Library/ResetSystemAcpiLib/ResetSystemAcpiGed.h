/** @file

  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>

typedef struct {
  UINT64  SleepControlRegAddr;
  UINT64  SleepStatusRegAddr;
  UINT64  ResetRegAddr;
  UINT8   ResetValue;
} POWER_MANAGER;

extern POWER_MANAGER PowerManager;

EFI_STATUS
GetPowerManagerByParseAcpiInfo (VOID);
