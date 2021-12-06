/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _LOONGARCH_CPU_H_
#define _LOONGARCH_CPU_H_

#define _LOCORE

//add
/* Exception types decoded by machdep exception decoder */
#define EXC_BAD         1       /* Undecodeable */
#define EXC_INT         0       /* HW interrupt */
#define EXC_BPT         2       /* Breakpoint */
#define EXC_TRC         3       /* Trace (not all arches) */
#define EXC_WTCH        4       /* Watch (not all arches) */
#define EXC_RES 5

//
// Location of exception vectors.
//
#define TLB_MISS_EXC_VEC  0x0
#define GEN_EXC_VEC       0x0

#define COPY_SIGCODE    // copy sigcode above user stack in exec
#define zero	$r0	/* wired zero */
#define ra	$r1	/* return address */
#define gp	$r2	/* global pointer - caller saved for PIC */
#define sp	$r3	/* stack pointer */
#define v0	$r4	/* return value - caller saved */
#define v1	$r5
#define a0	$r4	/* argument registers */
#define a1	$r5
#define a2	$r6
#define a3	$r7
#define a4	$r8	/* arg reg 64 bit; caller saved in 32 bit */
#define a5	$r9
#define a6	$r10
#define a7	$r11
#define t0	$r12	/* caller saved */
#define t1	$r13
#define t2	$r14
#define t3	$r15
#define t4	$r16	/* callee saved */
#define t5	$r17
#define t6	$r18
#define t7	$r19
#define t8	$r20	/* caller saved */
#define tp	$r21	/* TLS */
#define fp	$r22	/* frame pointer */
#define s0	$r23	/* callee saved */
#define s1	$r24
#define s2	$r25
#define s3	$r26
#define s4	$r27
#define s5	$r28
#define s6	$r29
#define s7	$r30
#define s8	$r31	/* callee saved */

//
// Location of the saved registers relative to ZERO.
// Usage is p->p_regs[XX].
//
#define ZERO   0
#define RA_NUM     1
#define GP_NUM     2
#define SP_NUM     3
#define A0_NUM     4
#define A1_NUM     5
#define A2_NUM     6
#define A3_NUM     7
#define A4_NUM     8
#define A5_NUM     9
#define A6_NUM     10
#define A7_NUM     11
#define T0_NUM     12
#define T1_NUM     13
#define T2_NUM     14
#define T3_NUM     15
#define T4_NUM     16
#define T5_NUM     17
#define T6_NUM     18
#define T7_NUM     19
#define T8_NUM     20
#define TP_NUM     21
#define FP_NUM     22
#define S0_NUM     23
#define S1_NUM     24
#define S2_NUM     25
#define S3_NUM     26
#define S4_NUM     27
#define S5_NUM     28
#define S6_NUM     29
#define S7_NUM     30
#define S8_NUM     31
#define BASE_NUM 32

#define LOONGARCH_CSR_CRMD      0
#define LOONGARCH_CSR_PRMD      1
#define LOONGARCH_CSR_EXCFG     4
#define LOONGARCH_CSR_EXST      5
#define LOONGARCH_CSR_EPC       6
#define LOONGARCH_CSR_BADV      7
#define LOONGARCH_CSR_BADINST   8
#define LOONGARCH_CSR_EBASE     0xc     /* Exception entry base address */
#define LOONGARCH_CSR_CPUNUM    0x20    /* CPU core number */

#define BOOTCORE_ID      0

#define LOONGSON_IOCSR_IPI_STATUS  0x1000
#define LOONGSON_IOCSR_IPI_EN      0x1004
#define LOONGSON_IOCSR_IPI_SET     0x1008
#define LOONGSON_IOCSR_IPI_CLEAR   0x100c
#define LOONGSON_CSR_MAIL_BUF0     0x1020
#define LOONGSON_CSR_MAIL_BUF1     0x1028
#define LOONGSON_CSR_MAIL_BUF2     0x1030
#define LOONGSON_CSR_MAIL_BUF3     0x1038

/* Bit Domains for CFG registers */
#define LOONGARCH_CPUCFG4          0x4
#define LOONGARCH_CPUCFG5          0x5

/* Kscratch registers */
#define LOONGARCH_CSR_KS0          0x30
#define LOONGARCH_CSR_KS1          0x31

#define LOONGARCH_CSR_TVAL         0x42    /* Timer value */
#define LOONGARCH_CSR_CNTC         0x43    /* Timer offset */
#define LOONGARCH_CSR_TINTCLR      0x44    /* Timer interrupt clear */

#define loongarch_csr_readq(val, reg)           \
do {                                            \
        UINT64 __res;                           \
        /* csrrd rd, csr_num */                 \
        __asm__ __volatile__(                   \
                "csrrd  %0, %1 \n\t"            \
                :"=r"(__res)                    \
                :"i"(reg)                       \
                :                               \
                );                              \
        (val) = __res;                          \
} while(0)

#define loongarch_csr_writeq(val, reg)          \
do {                                            \
        UINT64 __val = val;                     \
        /* csrwr rd, csr_num */                 \
        __asm__ __volatile__(                   \
                "csrwr  %0, %1 \n\t"            \
                : "=r"(__val)                   \
                : "i"(reg), "r"(__val)          \
                :                               \
                );                              \
} while(0)

#define LoongArchGetCpucfg(val, reg)            \
do {                                            \
        UINT64 __res;                           \
        /* cpucfg rd, rj */                   \
        __asm__ __volatile__(                   \
                "cpucfg  %0, %1 \n\t"           \
                :"=r"(__res)                    \
                :"r"(reg)                       \
                :                               \
                );                              \
        val = (UINT32)__res;                    \
} while(0)

#endif
