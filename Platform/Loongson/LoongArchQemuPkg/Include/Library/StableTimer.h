/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _STABLETIMER_H_
#define _STABLETIMER_H_

/* timer : start with TM */
#define LISA_CSR_TMCFG			0x41
#define LISA_CSR_TMCFG_EN		(1ULL << 0)
#define LISA_CSR_TMCFG_PERIOD		(1ULL << 1)
#define LISA_CSR_TMCFG_TIMEVAL		(0x3fffffffffffULL << 2)

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
