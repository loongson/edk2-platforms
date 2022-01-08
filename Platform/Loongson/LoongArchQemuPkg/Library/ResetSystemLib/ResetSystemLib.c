/** @file
  Base Reset System Library Shutdown API implementation for LoongArch.

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeLib.h>
#include <LoongArchQemuPlatform.h>

UINTN
LoongArchQemuAcpiBase (VOID)
{
  VOID *Address = (VOID*) LS7A_ACPI_REG_BASE;

  if (EfiGoneVirtual ()) {
    /**The RTC controller address and the ResetSystem controller address are in the same page.
       The function KVMToolRTCMapMemory has placed the entire page address in the memory mapping table,
       do not add additional.
     * */
    EfiConvertPointer (0, &Address);
    DEBUG ((DEBUG_INFO, "%a: virtual -> 0x%x\n", __FUNCTION__, Address));
  } else {
    DEBUG ((DEBUG_INFO, "%a: physical -> 0x%x\n", __FUNCTION__, Address));
  }

  return (UINTN) Address;
}

VOID
LoongArchQemuReset (VOID)
{

  UINTN Address;

  DEBUG ((DEBUG_INFO, "%a: LoongArchQemu reset via acpi\n", __FUNCTION__));

  Address = LoongArchQemuAcpiBase ();
  MmioWrite8 (Address + LS7A_GPE0_RESET_REG, 1);
  CpuDeadLoop ();
}

VOID
LoongArchQemuShutdown (VOID)
{
  UINTN  Address;

  //
  // sleep with S5
  //
  Address = LoongArchQemuAcpiBase ();
  MmioWrite16 (Address + LS7A_PM_CNT_BLK, ACPI_BITMASK_SLEEP_ENABLE);
  CpuDeadLoop ();
}

/**
 This function causes a system-wide reset (cold reset), in which
 all circuitry within the system returns to its initial state. This type of reset
 is asynchronous to system operation and operates without regard to
 cycle boundaries.

 If this function returns, it means that the system does not support cold reset.
**/
VOID
EFIAPI ResetCold (VOID)
{
  LoongArchQemuReset ();
}

/**
 This function causes a system-wide initialization (warm reset), in which all processors
 are set to their initial state. Pending cycles are not corrupted.

 If this function returns, it means that the system does not support warm reset.
**/
VOID
EFIAPI ResetWarm (VOID)
{
  LoongArchQemuReset ();
}

/**
 This function causes a systemwide reset. The exact type of the reset is
 defined by the EFI_GUID that follows the Null-terminated Unicode string passed
 into ResetData. If the platform does not recognize the EFI_GUID in ResetData
 the platform must pick a supported reset type to perform.The platform may
 optionally log the parameters from any non-normal reset that occurs.

 @param[in]  DataSize   The size, in bytes, of ResetData.
 @param[in]  ResetData  The data buffer starts with a Null-terminated string,
                        followed by the EFI_GUID.
 **/
VOID
EFIAPI
ResetPlatformSpecific (
  IN UINTN   DataSize,
  IN VOID    *ResetData
  )
{
  LoongArchQemuReset ();
}

/**
 This function causes the system to enter a power state equivalent
 to the ACPI G2/S5 or G3 states.

 If this function returns, it means that the system does not support shutdown reset.
**/
VOID
EFIAPI ResetShutdown (VOID)
{
  LoongArchQemuShutdown ();
}


/**
  This function causes the system to enter S3 and then wake up immediately.

  If this function returns, it means that the system does not support S3 feature.
 **/
VOID
EFIAPI
EnterS3WithImmediateWake (
  VOID
  )
{
  // not implemented
}
