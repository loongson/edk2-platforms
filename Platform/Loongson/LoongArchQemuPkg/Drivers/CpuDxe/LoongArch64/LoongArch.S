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
#include "CpuDxe.h"
#include "LoongArchAsmMacro.h"

#define RSIZE   8       /* 64 bit mode register size */
#define RLOGSIZE 3


    .data
/*  Register area outgoing */

/*  return value and jump buffer used when exiting program returns. */
    .global retvalue
retvalue:
    .word   0
    .globl  go_return_jump
    .common go_return_jump, 12*8

/*
 *  Main exception handler. Not really a leaf routine but not a normal
 *  function either. Save away the entire cpu state end enter exception mode.
 */
ASM_FUNC(Exception_handler)

    // store registers in space
    nop

    csrrd   t0, 0x30
    nop
    addi.d  sp, sp, -1024
    st.d   ra, sp, RA_NUM * RSIZE
    st.d   gp, sp, GP_NUM * RSIZE
    st.d   a0, sp, A0_NUM * RSIZE
    st.d   a1, sp, A1_NUM * RSIZE
    st.d   a2, sp, A2_NUM * RSIZE
    st.d   a3, sp, A3_NUM * RSIZE
    st.d   a4, sp, A4_NUM * RSIZE
    st.d   a5, sp, A5_NUM * RSIZE
    st.d   a6, sp, A6_NUM * RSIZE
    st.d   a7, sp, A7_NUM * RSIZE
    st.d   t0, sp, T0_NUM * RSIZE
    st.d   t1, sp, T1_NUM * RSIZE
    st.d   t2, sp, T2_NUM * RSIZE
    st.d   t3, sp, T3_NUM * RSIZE
    st.d   t4, sp, T4_NUM * RSIZE
    st.d   t5, sp, T5_NUM * RSIZE
    st.d   t6, sp, T6_NUM * RSIZE
    st.d   t7, sp, T7_NUM * RSIZE
    st.d   t8, sp, T8_NUM * RSIZE
    st.d   tp, sp, TP_NUM * RSIZE
    st.d   fp, sp, FP_NUM * RSIZE
    st.d   s0, sp, S0_NUM * RSIZE
    st.d   s1, sp, S1_NUM * RSIZE
    st.d   s2, sp, S2_NUM * RSIZE
    st.d   s3, sp, S3_NUM * RSIZE
    st.d   s4, sp, S4_NUM * RSIZE
    st.d   s5, sp, S5_NUM * RSIZE
    st.d   s6, sp, S6_NUM * RSIZE
    st.d   s7, sp, S7_NUM * RSIZE
    st.d   s8, sp, S8_NUM * RSIZE
    csrrd   t1, LOONGARCH_CSR_CRMD
    st.d   t1, sp, (LOONGARCH_CSR_CRMD + BASE_NUM)  * RSIZE
    csrrd   t1, LOONGARCH_CSR_PRMD
    st.d   t1, sp, (LOONGARCH_CSR_PRMD + BASE_NUM)  * RSIZE
    csrrd   t1, LOONGARCH_CSR_EXCFG
    st.d   t1, sp, (LOONGARCH_CSR_EXCFG + BASE_NUM) * RSIZE
    csrrd   t1, LOONGARCH_CSR_EXST
    st.d   t1, sp, (LOONGARCH_CSR_EXST + BASE_NUM)  * RSIZE
    csrrd   t1, LOONGARCH_CSR_EPC
    st.d   t1, sp, (LOONGARCH_CSR_EPC+ BASE_NUM)    * RSIZE
    csrrd   t1, LOONGARCH_CSR_BADV
    st.d   t1, sp, (LOONGARCH_CSR_BADV + BASE_NUM)  * RSIZE
    csrrd   t1, LOONGARCH_CSR_BADINST
    st.d   t1, sp, (LOONGARCH_CSR_BADINST + BASE_NUM)  * RSIZE

    or      a0, sp, zero
    bl      mException
    /*disable interrupt*/
    li.d     t0, (1 << 2)
    csrxchg zero, t0, LOONGARCH_CSR_CRMD

    ld.d    t1, sp, (LOONGARCH_CSR_PRMD + BASE_NUM) * RSIZE
    csrwr   t1, LOONGARCH_CSR_PRMD
    ld.d    t1, sp, (LOONGARCH_CSR_EXCFG + BASE_NUM) * RSIZE
    csrwr   t1, LOONGARCH_CSR_EXCFG
    ld.d    t1, sp, (LOONGARCH_CSR_EPC + BASE_NUM) * RSIZE
    csrwr   t1, LOONGARCH_CSR_EPC
    ld.d    ra, sp, RA_NUM * RSIZE
    ld.d    gp, sp, GP_NUM * RSIZE
    ld.d    a0, sp, A0_NUM * RSIZE
    ld.d    a1, sp, A1_NUM * RSIZE
    ld.d    a2, sp, A2_NUM * RSIZE
    ld.d    a3, sp, A3_NUM * RSIZE
    ld.d    a4, sp, A4_NUM * RSIZE
    ld.d    a5, sp, A5_NUM * RSIZE
    ld.d    a6, sp, A6_NUM * RSIZE
    ld.d    a7, sp, A7_NUM * RSIZE
    ld.d    t0, sp, T0_NUM * RSIZE
    ld.d    t1, sp, T1_NUM * RSIZE
    ld.d    t2, sp, T2_NUM * RSIZE
    ld.d    t3, sp, T3_NUM * RSIZE
    ld.d    t4, sp, T4_NUM * RSIZE
    ld.d    t5, sp, T5_NUM * RSIZE
    ld.d    t6, sp, T6_NUM * RSIZE
    ld.d    t7, sp, T7_NUM * RSIZE
    ld.d    t8, sp, T8_NUM * RSIZE
    ld.d    tp, sp, TP_NUM * RSIZE
    ld.d    fp, sp, FP_NUM * RSIZE
    ld.d    s0, sp, S0_NUM * RSIZE
    ld.d    s1, sp, S1_NUM * RSIZE
    ld.d    s2, sp, S2_NUM * RSIZE
    ld.d    s3, sp, S3_NUM * RSIZE
    ld.d    s4, sp, S4_NUM * RSIZE
    ld.d    s5, sp, S5_NUM * RSIZE
    ld.d    s6, sp, S6_NUM * RSIZE
    ld.d    s7, sp, S7_NUM * RSIZE
    ld.d    s8, sp, S8_NUM * RSIZE
    addi.d  sp, sp, 1024
    ertn

/*
 *  Exception trampoline copied down to RAM after initialization.
 */
ASM_FUNC(LoongArchException)
    csrwr   t0, 0x30
    la      t0, Exception_handler
    jirl    zero, t0, 0
.globl  LoongArchExceptionEnd
LoongArchExceptionEnd:


ASM_FUNC(SetTLBEbase)
    li.d    t0, 0x0
    csrwr   t0, 0x88
    jirl    zero, ra, 0

ASM_FUNC(SetEbase)
    /*clear Vint cofigure*/
    csrrd   t0, 0x4
    li.d     t1, ~0x70000
    and     t0, t0, t1
    csrwr   t0, 0x4
    /*set ebase*/
    li.d     t0, 0x0
    csrwr   t0, 0xc
    jirl    zero, ra, 0
