/** @file

  Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <PiPei.h>
#include <Library/BaseLib.h>        // CpuDeadLoop()
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ResetSystemLib.h> // ResetCold()
#include <Library/UefiRuntimeLib.h> // EfiConvertPointer()
#include <Library/IoLib.h>
#include <Library/QemuFwCfgLib.h>
#include "ResetSystemAcpiGed.h"

POWER_MANAGER PowerManager;

VOID *
GetFwCfgData(
CONST CHAR8           *Name
)
{
  FIRMWARE_CONFIG_ITEM FwCfgItem;
  EFI_STATUS           Status;
  UINTN                FwCfgSize;
  VOID                 *Data;

  Status = QemuFwCfgFindFile (Name, &FwCfgItem, &FwCfgSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a %d read  %s error Status %d \n", __func__, __LINE__, Name, Status));
    return NULL;
  }

  Data = AllocatePool (FwCfgSize);
  if (Data == NULL) {
    return NULL;
  }

  QemuFwCfgSelectItem (FwCfgItem);
  QemuFwCfgReadBytes (FwCfgSize, Data);

  return Data;
}
/**
  Find the power manager related info from ACPI table


  @retval RETURN_SUCCESS     Successfully find out all the required information.
  @retval RETURN_NOT_FOUND   Failed to find the required info.

**/
EFI_STATUS
GetPowerManagerByParseAcpiInfo (VOID)
{
  EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE    *Fadt = NULL;
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *Rsdp = NULL;
  EFI_ACPI_DESCRIPTION_HEADER                  *Xsdt = NULL;
  EFI_ACPI_DESCRIPTION_HEADER                  *Rsdt = NULL;
  VOID                                         *AcpiTables = NULL;
  UINT32                                       *Entry32 = NULL;
  UINTN                                         Entry32Num;
  UINT32                                       *Signature = NULL;
  UINTN                                         Idx;

  Rsdp = GetFwCfgData ("etc/acpi/rsdp");
  if (Rsdp == NULL) {
    DEBUG ((DEBUG_ERROR, "%a %d read etc/acpi/rsdp error \n", __func__, __LINE__));
    return RETURN_NOT_FOUND;
  }

  AcpiTables = GetFwCfgData ("etc/acpi/tables");
  if (AcpiTables == NULL) {
    DEBUG ((DEBUG_ERROR, "%a %d read etc/acpi/tables error \n", __func__, __LINE__));
    FreePool (Rsdp);
    return RETURN_NOT_FOUND;
  }

  Rsdt = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)AcpiTables +  Rsdp->RsdtAddress);
  Entry32    = (UINT32 *)(Rsdt + 1);
  Entry32Num = (Rsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) >> 2;
  for (Idx = 0; Idx < Entry32Num; Idx++) {
    Signature = (UINT32 *)((UINTN)Entry32[Idx] + (UINTN)AcpiTables);
    if (*Signature == EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
      Fadt = (EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE *)Signature;
      DEBUG ((DEBUG_INFO, "Found Fadt in Rsdt\n"));
      goto Done;
    }
  }


  Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)((UINTN)AcpiTables +  Rsdp->XsdtAddress);
  Entry32    = (UINT32 *)(Xsdt + 1);
  Entry32Num = (Xsdt->Length - sizeof (EFI_ACPI_DESCRIPTION_HEADER)) >> 2;
  for (Idx = 0; Idx < Entry32Num; Idx++) {
    Signature = (UINT32 *)((UINTN)Entry32[Idx] + (UINTN)AcpiTables);
    if (*Signature == EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
      Fadt = (EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE *)Signature;
      DEBUG ((DEBUG_INFO, "Found Fadt in Xsdt\n"));
      goto Done;
    }
  }  


  FreePool (Rsdp);
  FreePool (AcpiTables);
  DEBUG ((DEBUG_ERROR, " Fadt Not Found\n"));
  return RETURN_NOT_FOUND;

Done:

  PowerManager.ResetRegAddr        = Fadt->ResetReg.Address;
  PowerManager.ResetValue          = Fadt->ResetValue;
  PowerManager.SleepControlRegAddr = Fadt->SleepControlReg.Address;
  PowerManager.SleepStatusRegAddr  = Fadt->SleepStatusReg.Address;

  FreePool (Rsdp);
  FreePool (AcpiTables);

  return RETURN_SUCCESS;
}

static VOID
AcpiGedReset (
  VOID
  )
{
  MmioWrite8 (
    (UINTN)PowerManager.ResetRegAddr,
    PowerManager.ResetValue
    );

  CpuDeadLoop ();
}

static VOID
AcpiGedShutdown (
  VOID
  )
{
  MmioWrite8 (
    (UINTN)PowerManager.SleepControlRegAddr,
    (1 << 5) /* enable bit */ |
    (5 << 2) /* typ == S5  */
    );

  CpuDeadLoop ();
}

VOID EFIAPI
ResetCold (
  VOID
  )
{
  AcpiGedReset ();
}

VOID EFIAPI
ResetWarm (
  VOID
  )
{
  AcpiGedReset ();
}

VOID
EFIAPI
ResetPlatformSpecific (
  IN UINTN  DataSize,
  IN VOID   *ResetData
  )
{
  AcpiGedReset ();
}

VOID
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE  ResetType,
  IN EFI_STATUS      ResetStatus,
  IN UINTN           DataSize,
  IN VOID            *ResetData OPTIONAL
  )
{
  AcpiGedReset ();
}

VOID EFIAPI
ResetShutdown (
  VOID
  )
{
  AcpiGedShutdown ();
}
