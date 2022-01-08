/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include "Uefi.h"
#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/QemuFwCfgLib.h>
#include "Bpi.h"

VOID
BpiChecksum (
  IN UINT8      *Buffer,
  IN UINTN      Size
  )
{
  UINTN ChecksumOffset;

  //
  // CheckSum's offset in EXT_LIST is the same as in EXT_LIST's container (Buffer).
  //
  ChecksumOffset = OFFSET_OF (EXT_LIST, CheckSum);

  //
  // set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8 (Buffer, Size);
}

VOID
EFIAPI
AddToList (
  IN BootParamsInterface *Bpi,
  OUT EXT_LIST *Header
  )
{
  EXT_LIST *LastHeader;

  if (Bpi->ExtList == NULL) {
    Bpi->ExtList = Header;
  } else {
    for (LastHeader = Bpi->ExtList; LastHeader->next; LastHeader = LastHeader->next) {
    }
    LastHeader->next = Header;
    BpiChecksum ((UINT8 *)LastHeader, LastHeader->Length);
  }
  Header->next = NULL;
  BpiChecksum ((UINT8 *)Header, Header->Length);
}

#ifdef DEBUG_BPI
VOID
EFIAPI
ShowList (
  IN BootParamsInterface *Bpi
  )
{
  EXT_LIST *LastHeader;

  if (Bpi->ExtList == NULL) {
    DEBUG ((EFI_D_INFO, "List is null\n"));
  } else {
    for (LastHeader = Bpi->ExtList; LastHeader; LastHeader = LastHeader->next) {
      DEBUG ((EFI_D_INFO, "Header: 0x%llx, Legth: 0x%x, CheckSum: 0x%x\n", LastHeader, LastHeader->Length, 
            LastHeader->CheckSum));
    }
  }
}
#endif

VOID
EFIAPI
InitMemMap (
  IN OUT BootParamsInterface *Bpi,
  OUT MEM_MAP *mMemMap
  )
{
  UINT64   RamSize;
  UINT8    Data[8] = {'M', 'E', 'M'};
  INTN      i = 0;
  EFI_STATUS           Status;
  FIRMWARE_CONFIG_ITEM FwCfgItem;
  UINTN                FwCfgSize;
  EFI_LA_MEMMAP_ENTRY  LaMemMapEntry;
  EFI_LA_MEMMAP_ENTRY  *StartEntry;
  EFI_LA_MEMMAP_ENTRY  *pEntry;
  UINTN                Processed;

  SetMem (&mMemMap->Header, sizeof (EXT_LIST), 0);

  CopyMem (&mMemMap->Header.Signature, Data, sizeof (EXT_LIST));
  mMemMap->Header.Revision = 0;
  mMemMap->Header.Length = sizeof (MEM_MAP);

  RamSize = PcdGet64 (PcdRamSize); //in Byte

  ASSERT (RamSize != 0);
  DEBUG ((EFI_D_INFO, "RamSize %lld Byte\n", RamSize));

  /*
   * 1. The lowest 2M region cannot record in MemMap, cause Linux ram region should begin with usable ram.
   *    MAP_ENTRY (i, SYSTEM_RAM_RESERVED, 0x0, 0x200000);  // 0x0-0x200000
   */

  /* 2. Available SYSTEM_RAM area. */
  MAP_ENTRY (i, SYSTEM_RAM, 0x200000, 0xf000000 - 0x200000);  // 0x200000~0xf000000

  /* 3. Reserved low memory highest 16M. */
  MAP_ENTRY (i, SYSTEM_RAM_RESERVED, 0xf000000, 0x1000000);  // 0xf000000~0x10000000
  /*
   * 0x90000000 -- 0xC0000000 is used by uefi bios
   */
  //MAP_ENTRY (i, SYSTEM_RAM, 0x90000000, 0x30000000);


  /* Read memmap from QEMU when RamSize is larger than 1G */
  if (RamSize > SIZE_1GB) {
    Status = QemuFwCfgFindFile ("etc/memmap", &FwCfgItem, &FwCfgSize);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "fw read etc/la_memmap error Status %d \n", Status));
    }
    if (FwCfgSize % sizeof LaMemMapEntry != 0) {
      DEBUG ((EFI_D_ERROR, "no LaMemMapEntry FwCfgSize:%d\n", FwCfgSize));
    }

    QemuFwCfgSelectItem (FwCfgItem);
    StartEntry = AllocateZeroPool (FwCfgSize);
    QemuFwCfgReadBytes (FwCfgSize, StartEntry);
    for (Processed = 0; Processed < (FwCfgSize / sizeof LaMemMapEntry); Processed++) {
      pEntry = StartEntry + Processed;
      DEBUG ((
        EFI_D_INFO,
        "%a:%d Base=0x%Lx Length=0x%Lx Type=%u sizeof (LaMemMapEntry):%d, Processed:%d, FwCfgSize:%d\n",
        __FUNCTION__,
        __LINE__,
        pEntry->BaseAddr,
        pEntry->Length,
        pEntry->Type,
        sizeof LaMemMapEntry,
        Processed,
        FwCfgSize
        ));
      MAP_ENTRY (i, pEntry->Type, pEntry->BaseAddr, pEntry->Length);
    }
  }

  mMemMap->MapCount = i;
  AddToList (Bpi, &mMemMap->Header);
#ifdef DEBUG_BPI
  INTN j;
  for (j = 0; j < i; j++) {
    DEBUG ((EFI_D_INFO, "%d: type: 0x%x, start: 0x%llx, size: 0x%llx\n", j, mMemMap->Map[j].MemType,
           mMemMap->Map[j].MemStart, mMemMap->Map[j].MemSize));
  }
#endif
}

VOID
FindBits (
  IN UINT64 Mask,
  OUT UINT8 *Pos,
  OUT UINT8 *Size
  )
{
  UINT8 First;
  UINT8 Len;

  First = 0;
  Len = 0;

  if (Mask) {
    while (!(Mask & 0x1)) {
      Mask = Mask >> 1;
      First++;
    }

    while (Mask & 0x1) {
      Mask = Mask >> 1;
      Len++;
    }
  }
  *Pos = First;
  *Size = Len;
}

EFI_STATUS
SetupGraphicsFromGop (
  OUT SCREEN_INFO           *Si,
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
  )
{
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_STATUS                           Status;
  UINTN                                Size;

  Status = Gop->QueryMode (Gop, Gop->Mode->Mode, &Size, &Info);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* We found a GOP */

  /* EFI framebuffer */
  Si->orig_video_isVGA = 0x70;

  Si->orig_x = 0;
  Si->orig_y = 0;
  Si->orig_video_page = 0;
  Si->orig_video_mode = 0;
  Si->orig_video_cols = 0;
  Si->orig_video_lines = 0;
  Si->orig_video_ega_bx = 0;
  Si->orig_video_points = 0;

  Si->lfb_base = (UINT32) Gop->Mode->FrameBufferBase;
  Si->lfb_size = (UINT32) Gop->Mode->FrameBufferSize;
  Si->lfb_width = (UINT16) Info->HorizontalResolution;
  Si->lfb_height = (UINT16) Info->VerticalResolution;
  Si->pages = 1;
  Si->vesapm_seg = 0;
  Si->vesapm_off = 0;

  if (Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor) {
    Si->lfb_depth = 32;
    Si->red_size = 8;
    Si->red_pos = 0;
    Si->green_size = 8;
    Si->green_pos = 8;
    Si->blue_size = 8;
    Si->blue_pos = 16;
    Si->rsvd_size = 8;
    Si->rsvd_pos = 24;
    Si->lfb_linelength = (UINT16) (Info->PixelsPerScanLine * 4);
  } else if (Info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
    Si->lfb_depth = 32;
    Si->red_size = 8;
    Si->red_pos = 16;
    Si->green_size = 8;
    Si->green_pos = 8;
    Si->blue_size = 8;
    Si->blue_pos = 0;
    Si->rsvd_size = 8;
    Si->rsvd_pos = 24;
    Si->lfb_linelength = (UINT16) (Info->PixelsPerScanLine * 4);
  } else if (Info->PixelFormat == PixelBitMask) {
    FindBits (Info->PixelInformation.RedMask,
      &Si->red_pos, &Si->red_size);
    FindBits (Info->PixelInformation.GreenMask,
      &Si->green_pos, &Si->green_size);
    FindBits (Info->PixelInformation.BlueMask,
      &Si->blue_pos, &Si->blue_size);
    FindBits (Info->PixelInformation.ReservedMask,
      &Si->rsvd_pos, &Si->rsvd_size);
    Si->lfb_depth = Si->red_size + Si->green_size +
      Si->blue_size + Si->rsvd_size;
    Si->lfb_linelength = (UINT16) ((Info->PixelsPerScanLine * Si->lfb_depth) / 8);
  } else {
    Si->lfb_depth = 4;
    Si->red_size = 0;
    Si->red_pos = 0;
    Si->green_size = 0;
    Si->green_pos = 0;
    Si->blue_size = 0;
    Si->blue_pos = 0;
    Si->rsvd_size = 0;
    Si->rsvd_pos = 0;
    Si->lfb_linelength = Si->lfb_width / 2;
  }

  return Status;
}

EFI_STATUS
SetupGraphics (
  IN OUT SCREEN_INFO *si
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           HandleCount;
  UINTN                           Index;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *Gop;

  ZeroMem ((VOID*)si, sizeof (SCREEN_INFO));

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; Index++) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiGraphicsOutputProtocolGuid,
                      (VOID*) &Gop
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }

      Status = SetupGraphicsFromGop (si, Gop);
      if (!EFI_ERROR (Status)) {
        FreePool (HandleBuffer);
        return EFI_SUCCESS;
      }
    }
    FreePool (HandleBuffer);
  }
  return EFI_NOT_FOUND;
}

VOID
EFIAPI
InitScreenInfo (
  IN BootParamsInterface *Bpi,
  OUT SInfo *si
  )
{

//  UINTN   Addr;
  UINT8   Data[8] = {'S', 'I', 'N', 'F', 'O'};

  SetMem (&si->Header, sizeof (EXT_LIST), 0);

  CopyMem (&si->Header.Signature, Data, sizeof (UINT64));
  si->Header.Revision = 0;
  si->Header.Length = sizeof (SInfo);

  SetupGraphics (&si->si);
  AddToList (Bpi, &si->Header);
}

VOID
EFIAPI
InitializeBpi (
  IN BootParamsInterface *Bpi,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  DEBUG ((EFI_D_INFO, "New BPI addr: 0x%llx\n", (UINTN)Bpi));
  UINT8 Data[8] = {'B', 'P', 'I', '0', '1', '0', '0', '1'};
  CopyMem (&Bpi->Signature, Data, sizeof (UINT64));
  Bpi->SystemTable = SystemTable;
  Bpi->ExtList = NULL;
}

EFI_STATUS
EFIAPI
SetBootParams (VOID)
{
  EFI_STATUS                 Status;
  UINTN                      Size;
  VOID                       *BufferAddress;
  BootParamsInterface *Bpi;
  MEM_MAP                    *mMemMap;
  SInfo                      *si;
  EFI_SYSTEM_TABLE           *SystemTable;

  SystemTable = gST;
  Size  = sizeof (BootParamsInterface);
  Size += sizeof (MEM_MAP);
  Size += sizeof (SInfo);

  Status = SystemTable->BootServices->AllocatePool (
                                        EfiRuntimeServicesData,
                                        Size,
                                        &BufferAddress
                                        );
  if (EFI_ERROR (Status)) {
   return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (BufferAddress, Size);
  Bpi = (BootParamsInterface *) BufferAddress;
  InitializeBpi (Bpi, SystemTable);

  mMemMap = (MEM_MAP *) (Bpi + 1);
  InitMemMap (Bpi, mMemMap);

  si = (SInfo*) (mMemMap + 1);
  InitScreenInfo (Bpi, si);

#ifdef DEBUG_BPI
  ShowList (Bpi);
#endif
  Status = SystemTable->BootServices->InstallConfigurationTable (
                                        &gEfiLoongsonBootparamsTableGuid,
                                        Bpi
                                        );

  return Status;
}
