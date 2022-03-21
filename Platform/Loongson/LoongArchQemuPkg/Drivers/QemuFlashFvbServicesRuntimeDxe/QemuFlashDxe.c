/** @file
  LoongArch support for QEMU system firmware flash device: functions specific to the
  runtime DXE driver build.

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Library/UefiRuntimeLib.h>

#include "QemuFlash.h"

VOID
QemuFlashConvertPointers (
  VOID
  )
{
  EfiConvertPointer (0x0, (VOID **)&mFlashBase);
}
