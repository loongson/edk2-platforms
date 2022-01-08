/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <string.h>
#include "Library/Cpu.h"
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include "CpuDxe.h"
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/UefiLib.h>
#include <Guid/DebugImageInfoTable.h>



EFI_EXCEPTION_CALLBACK  gExceptionHandlers[MAX_LOONGARCH_EXCEPTION + 1];
EFI_EXCEPTION_CALLBACK  gDebuggerExceptionHandlers[MAX_LOONGARCH_EXCEPTION + 1];

/**
  This function registers and enables the handler specified by InterruptHandler for a processor
  interrupt or exception type specified by InterruptType. If InterruptHandler is NULL, then the
  handler for the processor interrupt or exception type specified by InterruptType is uninstalled.
  The installed handler is called once for each processor interrupt or exception.

  @param  InterruptType    A pointer to the processor's current interrupt state. Set to TRUE if interrupts
                           are enabled and FALSE if interrupts are disabled.
  @param  InterruptHandler A pointer to a function of type EFI_CPU_INTERRUPT_HANDLER that is called
                           when a processor interrupt occurs. If this parameter is NULL, then the handler
                           will be uninstalled.

  @retval EFI_SUCCESS           The handler for the processor interrupt was successfully installed or uninstalled.
  @retval EFI_ALREADY_STARTED   InterruptHandler is not NULL, and a handler for InterruptType was
                                previously installed.
  @retval EFI_INVALID_PARAMETER InterruptHandler is NULL, and a handler for InterruptType was not
                                previously installed.
  @retval EFI_UNSUPPORTED       The interrupt specified by InterruptType is not supported.

**/
EFI_STATUS
RegisterInterruptHandler (
  IN EFI_EXCEPTION_TYPE             InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER      InterruptHandler
  )
{
  if (InterruptType > MAX_LOONGARCH_EXCEPTION) {
    return EFI_UNSUPPORTED;
  }

  if ((InterruptHandler != NULL) 
    && (gExceptionHandlers[InterruptType] != NULL))
  {
    return EFI_ALREADY_STARTED;
  }

  gExceptionHandlers[InterruptType] = InterruptHandler;

  return EFI_SUCCESS;
}

/*
 * Get Exception Type
 */
INT32
EFIAPI
GetExceptionType (IN EFI_SYSTEM_CONTEXT frame)
{
  INT32 tmp;

  tmp = frame.SystemContextLoongArch64->ESTAT & CSR_ESTAT_EXC;
  tmp = tmp >> CSR_ESTAT_EXC_SHIFT;
  switch (tmp) {
  case EXC_INT:
    return (EXC_INT);
  default:
    return (EXC_BAD);
  }
}

/*
 *To Invoke the Handler Function
 */
STATIC VOID
EFIAPI
InterruptHandler (
  IN     INT32           ExceptionType,
  IN OUT EFI_SYSTEM_CONTEXT           SystemContext
  )
{
  INT32 Pending;
  /*irq [13-0] NMI IPI TI PCOV hw IP10-IP2 soft IP1-IP0*/
  Pending = ((SystemContext.SystemContextLoongArch64->ESTAT) & 
             (SystemContext.SystemContextLoongArch64->ECFG) & 0x1fff);
  if (Pending & (1 << 11/*TI*/)) {
      gExceptionHandlers[ExceptionType] (ExceptionType, SystemContext);
  } else {
      DEBUG ((EFI_D_INFO, "Pending: 0x%0x, ExceptionType: 0x%0x\n", Pending, ExceptionType));
  }
}

/**
  Use the EFI Debug Image Table to lookup the FaultAddress and find which PE/COFF image
  it came from. As long as the PE/COFF image contains a debug directory entry a
  string can be returned. For ELF and Mach-O images the string points to the Mach-O or ELF
  image. Microsoft tools contain a pointer to the PDB file that contains the debug information.

  @param  FaultAddress         Address to find PE/COFF image for.
  @param  ImageBase            Return load address of found image
  @param  PeCoffSizeOfHeaders  Return the size of the PE/COFF header for the image that was found

  @retval NULL                 FaultAddress not in a loaded PE/COFF image.
  @retval                      Path and file name of PE/COFF image.

**/
CHAR8 *
GetImageName (
  IN  UINTN  FaultAddress,
  OUT UINTN  *ImageBase,
  OUT UINTN  *PeCoffSizeOfHeaders
  )
{
  EFI_STATUS                          Status;
  EFI_DEBUG_IMAGE_INFO_TABLE_HEADER   *DebugTableHeader;
  EFI_DEBUG_IMAGE_INFO                *DebugTable;
  UINTN                               Entry;
  CHAR8                               *Address;

  Status = EfiGetSystemConfigurationTable (&gEfiDebugImageInfoTableGuid, (VOID **)&DebugTableHeader);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  DebugTable = DebugTableHeader->EfiDebugImageInfoTable;
  if (DebugTable == NULL) {
    return NULL;
  }

  Address = (CHAR8 *)(UINTN)FaultAddress;
  for (Entry = 0; Entry < DebugTableHeader->TableSize; Entry++, DebugTable++) {
    if (DebugTable->NormalImage != NULL) {
      if ((DebugTable->NormalImage->ImageInfoType == EFI_DEBUG_IMAGE_INFO_TYPE_NORMAL) &&
          (DebugTable->NormalImage->LoadedImageProtocolInstance != NULL)) {
        if ((Address >= (CHAR8 *)DebugTable->NormalImage->LoadedImageProtocolInstance->ImageBase) &&
            (Address <= ((CHAR8 *)DebugTable->NormalImage->LoadedImageProtocolInstance->ImageBase + DebugTable->NormalImage->LoadedImageProtocolInstance->ImageSize))) {
          *ImageBase = (UINTN)DebugTable->NormalImage->LoadedImageProtocolInstance->ImageBase;
          *PeCoffSizeOfHeaders = PeCoffGetSizeOfHeaders ((VOID *)(UINTN)*ImageBase);
          return PeCoffLoaderGetPdbPointer (DebugTable->NormalImage->LoadedImageProtocolInstance->ImageBase);
        }
      }
   }
  }

  return NULL;
}

STATIC
CONST CHAR8 *
BaseName (
  IN  CONST CHAR8 *FullName
  )
{
  CONST CHAR8 *Str;

  Str = FullName + AsciiStrLen (FullName);

  while (--Str > FullName) {
    if (*Str == '/' || *Str == '\\') {
      return Str + 1;
    }
  }
  return Str;
}

VOID
EFIAPI
mException (
IN OUT EFI_SYSTEM_CONTEXT           SystemContext
)
{
  INT32           ExceptionType;
  ExceptionType = GetExceptionType (SystemContext);
  CHAR8  *Pdb;
  UINTN  ImageBase, epc;
  UINTN  PeCoffSizeOfHeader;


  if (ExceptionType == EXC_INT) { // Timer
    /* 
     * handle interrupt exception
     */
    InterruptHandler (ExceptionType, SystemContext);
  } else { // Others
    epc = SystemContext.SystemContextLoongArch64->ERA;

    DEBUG ((DEBUG_ERROR, "CRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->CRMD));
    DEBUG ((DEBUG_ERROR, "PRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->PRMD));
    DEBUG ((DEBUG_ERROR, "ECFG  0x%llx\n",  SystemContext.SystemContextLoongArch64->ECFG));
    DEBUG ((DEBUG_ERROR, "ESTAT   0x%llx\n",  SystemContext.SystemContextLoongArch64->ESTAT));
    DEBUG ((DEBUG_ERROR, "ERA    0x%llx\n",  SystemContext.SystemContextLoongArch64->ERA));
    DEBUG ((DEBUG_ERROR, "BADV    0x%llx\n",  SystemContext.SystemContextLoongArch64->BADV));
    DEBUG ((DEBUG_ERROR, "BADI 0x%llx\n",  SystemContext.SystemContextLoongArch64->BADI));


    Pdb = GetImageName (epc, &ImageBase, &PeCoffSizeOfHeader);
    if (Pdb != NULL) {
      DEBUG ((DEBUG_ERROR, "PC 0x%012lx (0x%012lx+0x%08x) [ 0] %a\n",
        epc, ImageBase,
        epc - ImageBase, BaseName (Pdb)));
    } else {
      DEBUG ((DEBUG_ERROR, "PC 0x%012lx\n", epc));
    }


    while (1);
  }
}

EFI_STATUS
InitializeExceptions (
  IN EFI_CPU_ARCH_PROTOCOL    *Cpu
  )
{
  EFI_STATUS           Status;
  BOOLEAN              IrqEnabled;
  EFI_PHYSICAL_ADDRESS Address;

  ZeroMem (gExceptionHandlers, sizeof (*gExceptionHandlers));

  //
  // Disable interrupts
  //
  Cpu->GetInterruptState (Cpu, &IrqEnabled);
  Cpu->DisableInterrupt (Cpu);

  //
  // EFI does not use the FIQ, but a debugger might so we must disable
  // as we take over the exception vectors.
  //
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                 EfiRuntimeServicesData,
                 1,
                 &Address
                 );
  if (EFI_ERROR (Status)) {
         return Status;
  }

  DEBUG ((EFI_D_INFO, "set ebase\n"));
  DEBUG ((EFI_D_INFO, "LoongArchException address: 0x%x\n", LoongArchException));
  DEBUG ((EFI_D_INFO, "LoongArchExceptionEnd address: 0x%x\n", LoongArchExceptionEnd));
  CopyMem ((char *)Address, LoongArchException, (LoongArchExceptionEnd - LoongArchException));
  InvalidateInstructionCacheRange ((char *)Address, (LoongArchExceptionEnd - LoongArchException));

  SetEbase (Address);

  DEBUG ((EFI_D_INFO, "InitializeExceptions, IrqEnabled = %x\n", IrqEnabled));
  if (IrqEnabled) {
    //
    // Restore interrupt state
    //
    Status = Cpu->EnableInterrupt (Cpu);
  }

  return Status;
}
