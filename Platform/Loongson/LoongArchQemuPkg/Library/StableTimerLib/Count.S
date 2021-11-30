#------------------------------------------------------------------------------
#
# Count for LoongArch
#
# Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
#------------------------------------------------------------------------------

#include "Library/Cpu.h"
#include "TimerLibInternal.h"
#include "LoongArchAsmMacro.h"

ASM_FUNC(CpuSetIP)
    csrrd   t0, LOONGARCH_CSR_EXCFG 
    or      t0, t0, a0
    csrwr   t0, LOONGARCH_CSR_EXCFG
    jirl    zero, ra,0

