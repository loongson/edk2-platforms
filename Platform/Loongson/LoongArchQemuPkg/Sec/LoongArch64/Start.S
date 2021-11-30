#------------------------------------------------------------------------------
#
# Start for LoongArch
#
# Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
#------------------------------------------------------------------------------

#include <Library/Cpu.h>
#include "LoongArchAsmMacro.h"

    .text
    .globl  _ModuleEntryPoint
_ModuleEntryPoint:

    /* configure reset ebase */
    li.d    t0, 0x1c000000
    csrwr   t0, LOONGARCH_CSR_EBASE

    /*disable interrupt*/
    li.d    t0, (1 << 2)
    csrxchg zero, t0, LOONGARCH_CSR_CRMD 

    /* read physical cpu number id */
    csrrd   t0, LOONGARCH_CSR_CPUNUM
    andi    t0, t0, 0x3ff
    li.d    a0, BOOTCORE_ID  //0
    bne     t0, a0, slave_main

call_centry:
    /*call C function make sure parameter true*/
    li.d    a1, FixedPcdGet64(PcdSecPeiTempRamBase) + FixedPcdGet32(PcdSecPeiTempRamSize)  # stack base
    li.d    a0, FixedPcdGet64(PcdFlashPeiFvBase) # PEI Fv base
    move    sp, a1
    addi.d  sp, sp, -0x8
    bl      SecCoreStartupWithStack

slave_main:
    # clear mailbox
    li.d      t1, LOONGSON_CSR_MAIL_BUF0
    iocsrwr.d zero, t1

    # enable IPI interrupt
    li.d      t0, (1 << 12)
    csrxchg   t0, t0, LOONGARCH_CSR_EXCFG

1:
    # wait for wakeup
    idle 0
    nop
    iocsrrd.w t0, t1
    beqz      t0, 1b

    # read and clear ipi interrupt
    li.d      t1, LOONGSON_IOCSR_IPI_STATUS
    iocsrrd.w t0, t1
    li.d      t1, LOONGSON_IOCSR_IPI_CLEAR
    iocsrwr.w t0, t1

    # disable IPI interrupt
    li.d      t0, (1 << 12)
    csrxchg   zero, t0, LOONGARCH_CSR_EXCFG

    # read mail buf and jump to specified entry
    li.d      t1, LOONGSON_CSR_MAIL_BUF0
    iocsrrd.d t0, t1
    or	      ra, t0, zero
    jirl      zero, ra, 0x0
