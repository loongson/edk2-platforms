/** @file
   LoongArch ASM macro definition.

   Copyright (c) 2021, Loongson Limited. All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#ifndef LOONGARCH_ASM_MACRO_H_
#define LOONGARCH_ASM_MACRO_H_

#include <Base.h>

#define _ASM_FUNC(Name, Section)    \
  .global   Name                  ; \
  .section  #Section, "ax"        ; \
  .type     Name, %function       ; \
  Name:

#define ASM_FUNC(Name)            _ASM_FUNC(ASM_PFX(Name), .text. ## Name)

#endif // __LOONGARCH_ASM_MACRO_H__
