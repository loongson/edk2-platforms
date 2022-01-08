/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef STABLE_TIMER_H_
#define STABLE_TIMER_H_
#include "Library/Cpu.h"

UINTN
EFIAPI
CsrReadTime (
  VOID
  );

UINT32
EFIAPI
CalcConstFreq (
  VOID
  );
#endif
