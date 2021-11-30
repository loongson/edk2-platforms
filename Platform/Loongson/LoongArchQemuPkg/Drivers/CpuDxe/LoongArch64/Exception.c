/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <string.h>
#include "CpuDxe.h"

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

  if ((InterruptHandler != NULL) && (gExceptionHandlers[InterruptType] != NULL)) {
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
GetExceptionType(IN EFI_SYSTEM_CONTEXT frame)
{
  INT32 tmp;
#define CSR_ECODE_SHIFT 16
#define CSR_ECODE 0x3f0000
  tmp = (frame.SystemContextLoongArch64->ESTAT & CSR_ECODE) >> CSR_ECODE_SHIFT;
  switch(tmp) {
  case EXC_INT:
    return (EXC_INT);
  default:
    return(EXC_BAD);
  }
}

/*
 *To Invoke the Handler Function
 */
VOID
EFIAPI
CommonCExceptionHandler (
  IN     INT32           ExceptionType,
  IN OUT EFI_SYSTEM_CONTEXT           SystemContext
  )
{
  INT32 Pending;
  /*irq [13-0] NMI IPI TI PCOV hw IP10-IP2 soft IP1-IP0*/
  Pending = ((SystemContext.SystemContextLoongArch64->ESTAT) & (SystemContext.SystemContextLoongArch64->ECFG) & 0x1fff);
  if(Pending & (1 << 11/*TI*/))
  {
      gExceptionHandlers[ExceptionType] (ExceptionType, SystemContext);
  } else {
      DEBUG ((EFI_D_INFO, "Pending: 0x%0x, ExceptionType: 0x%0x\n", Pending, ExceptionType));
  }
}

int exc_times = 0;
VOID
EFIAPI
mException(
IN OUT EFI_SYSTEM_CONTEXT           SystemContext
)
{
  INT32           ExceptionType;
  ExceptionType = GetExceptionType(SystemContext);

	exc_times++;
  if(ExceptionType == 0) { // Timer
	if (exc_times == 1000) {
			DEBUG ((DEBUG_ERROR, "\nSystemContext.SystemContextLoongArch64 address 0x%llx\n",SystemContext.SystemContextLoongArch64));
			DEBUG ((DEBUG_ERROR, "CRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->CRMD));
			DEBUG ((DEBUG_ERROR, "PRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->PRMD));
			DEBUG ((DEBUG_ERROR, "ECFG  0x%llx\n",  SystemContext.SystemContextLoongArch64->ECFG));
			DEBUG ((DEBUG_ERROR, "ESTAT   0x%llx\n",  SystemContext.SystemContextLoongArch64->ESTAT));
			DEBUG ((DEBUG_ERROR, "ERA    0x%llx\n",  SystemContext.SystemContextLoongArch64->ERA));
			DEBUG ((DEBUG_ERROR, "BADV    0x%llx\n",  SystemContext.SystemContextLoongArch64->BADV));
			DEBUG ((DEBUG_ERROR, "BADI 0x%llx\n",  SystemContext.SystemContextLoongArch64->BADI));
		exc_times = 0;
	}
    CommonCExceptionHandler(ExceptionType, SystemContext);
  } else { // Others
    DEBUG ((DEBUG_ERROR, "\nSystemContext.SystemContextLoongArch64 address 0x%llx\n",SystemContext.SystemContextLoongArch64));
    DEBUG ((DEBUG_ERROR, "CRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->CRMD));
    DEBUG ((DEBUG_ERROR, "PRMD   0x%llx\n",  SystemContext.SystemContextLoongArch64->PRMD));
    DEBUG ((DEBUG_ERROR, "ECFG  0x%llx\n",  SystemContext.SystemContextLoongArch64->ECFG));
    DEBUG ((DEBUG_ERROR, "ESTAT   0x%llx\n",  SystemContext.SystemContextLoongArch64->ESTAT));
    DEBUG ((DEBUG_ERROR, "ERA    0x%llx\n",  SystemContext.SystemContextLoongArch64->ERA));
    DEBUG ((DEBUG_ERROR, "BADV    0x%llx\n",  SystemContext.SystemContextLoongArch64->BADV));
    DEBUG ((DEBUG_ERROR, "BADI 0x%llx\n",  SystemContext.SystemContextLoongArch64->BADI));
    DEBUG ((DEBUG_ERROR, "RA 0x%llx\n",  SystemContext.SystemContextLoongArch64->ERA));

    while(1);
  }
}

EFI_STATUS
InitializeExceptions (
  IN EFI_CPU_ARCH_PROTOCOL    *Cpu
  )
{
  EFI_STATUS           Status;
  BOOLEAN              IrqEnabled;

  Status = EFI_SUCCESS;
  ZeroMem (gExceptionHandlers,sizeof(*gExceptionHandlers));

  //
  // Disable interrupts
  //
  Cpu->GetInterruptState (Cpu, &IrqEnabled);
  Cpu->DisableInterrupt (Cpu);

  //
  // EFI does not use the FIQ, but a debugger might so we must disable
  // as we take over the exception vectors.
  //

  // We do not copy the Exception Table at PcdGet32(PcdCpuVectorBaseAddress). We just set Vector
  // Base Address to point into CpuDxe code.

  SetEbase();
  SetTLBEbase();

  InvalidateInstructionCacheRange((char *)GEN_EXC_VEC,(LoongArchExceptionEnd - LoongArchException) );
  DEBUG ((EFI_D_INFO, "set ebase\n"));
  DEBUG ((EFI_D_INFO, "LoongArchException address: 0x%x\n", LoongArchException));
  DEBUG ((EFI_D_INFO, "LoongArchExceptionEnd address: 0x%x\n", LoongArchExceptionEnd));
  CopyMem((char *)GEN_EXC_VEC,LoongArchException, (LoongArchExceptionEnd - LoongArchException));
  CopyMem((char *)TLB_MISS_EXC_VEC,LoongArchException, (LoongArchExceptionEnd - LoongArchException));
  /*ls3a5000 no BEV*/
  //puSetStatusRegister(0, SR_BOOT_EXC_VEC);
  InvalidateInstructionCacheRange((char *)GEN_EXC_VEC,(LoongArchExceptionEnd - LoongArchException) );
// InvalidateInstructionCacheRange((char *)TLB_MISS_EXC_VEC,(LoongArchExceptionEnd - LoongArchException) );
  DEBUG ((EFI_D_INFO, "InitializeExceptions,IrqEnabled = %x\n",IrqEnabled));
  if (IrqEnabled) {
    //
    // Restore interrupt state
    //
    Status = Cpu->EnableInterrupt (Cpu);
  }

  return Status;
}
