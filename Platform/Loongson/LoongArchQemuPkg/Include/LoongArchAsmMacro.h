/** @file
   LoongArch ASM macro definition.

   Copyright (c) 2021, Loongson Limited. All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#ifndef __LOONGARCH_ASM_MACRO_H__
#define __LOONGARCH_ASM_MACRO_H__

#include <Base.h>

/*
 * Register definition for loongarch
 */
#define zero    $r0     /* wired zero */
#define ra      $r1     /* return address */
#define gp      $r2     /* global pointer - caller saved for PIC */
#define sp      $r3     /* stack pointer */
#define v0      $r4     /* return value - caller saved */
#define v1      $r5
#define a0      $r4     /* argument registers */
#define a1      $r5
#define a2      $r6
#define a3      $r7
#define a4      $r8     /* arg reg 64 bit; caller saved in 32 bit */
#define a5      $r9
#define a6      $r10
#define a7      $r11
#define t0      $r12
#define t1      $r13
#define t2      $r14
#define t3      $r15
#define t4      $r16
#define t5      $r17
#define t6      $r18
#define t7      $r19
#define t8      $r20
#define tp      $r21
#define fp      $r22
#define s0      $r23
#define s1      $r24
#define s2      $r25
#define s3      $r26
#define s4      $r27
#define s5      $r28
#define s6      $r29
#define s7      $r30
#define s8      $r31    /* callee saved */

#define _ASM_FUNC(Name, Section)    \
  .global   Name                  ; \
  .section  #Section, "ax"        ; \
  .type     Name, %function       ; \
  Name:

#define ASM_FUNC(Name)            _ASM_FUNC(ASM_PFX(Name), .text. ## Name)

#endif // __LOONGARCH_ASM_MACRO_H__
