#------------------------------------------------------------------------------
#
# LoongArch for LoongArch
#
# Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
#-----------------------------------------------------------------------------

#ifndef _KERNEL
#define _KERNEL
#endif

#include "Library/Cpu.h"
#include "LoongArchAsmMacro.h"

#  Query the page table.
#
#  @param  VOID
#
#  @retval  VOID
ASM_FUNC(HandleTlbRefill)
  csrwr T0, LOONGARCH_CSR_TLBRSAVE
  csrrd T0, LOONGARCH_CSR_PGD
  lddir T0, T0, 3   #Put pud BaseAddress into T0
  lddir T0, T0, 2   #Put pmd BaseAddress into T0
  lddir T0, T0, 1   #Put pte BaseAddress into T0
  ldpte T0, 0
  ldpte T0, 1
  tlbfill
  csrrd T0, LOONGARCH_CSR_TLBRSAVE
  ertn
.globl  HandleTlbRefillEnd
HandleTlbRefillEnd:
