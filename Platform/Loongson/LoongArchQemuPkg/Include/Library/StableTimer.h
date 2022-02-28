/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Csr    - Cpu Status Register
    - Calc   - Calculation
    - Freq   - frequency
**/

#ifndef STABLE_TIMER_H_
#define STABLE_TIMER_H_
#include "Library/Cpu.h"

/**
  Gets the timer count value.

  @param[] VOID 

  @retval  timer count value.
**/
UINTN
EFIAPI
CsrReadTime (
  VOID
  );

/**
  Calculate the timer frequency.

  @param[] VOID 

  @retval  Timer frequency.
**/
UINT32
EFIAPI
CalcConstFreq (
  VOID
  );
#endif
