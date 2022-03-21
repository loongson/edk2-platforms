/** @file
  Defines data structure that is the volume header found.These data is intent
  to decouple FVB driver with FV header.

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

//
// The package level header files this module uses
//
#include <Pi/PiFirmwareVolume.h>

//
// The protocols, PPI and GUID definitions for this module
//
#include <Guid/SystemNvDataGuid.h>
//
// The Library classes this module consumes
//
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>

typedef struct {
  UINT64                        FvLength;
  EFI_FIRMWARE_VOLUME_HEADER    FvbInfo;
  //
  // EFI_FV_BLOCK_MAP_ENTRY    ExtraBlockMap[n];//n=0
  //
  EFI_FV_BLOCK_MAP_ENTRY        End[1];
} EFI_FVB_MEDIA_INFO;

EFI_FVB_MEDIA_INFO  mPlatformFvbMediaInfo[] = {
  //
  // System NvStorage FVB
  //
  {
    FixedPcdGet32 (PcdAllVarSize),
    {
      {
        0,
      },  // ZeroVector[16]
      EFI_SYSTEM_NV_DATA_FV_GUID,
      FixedPcdGet32 (PcdAllVarSize),
      EFI_FVH_SIGNATURE,
      EFI_FVB2_MEMORY_MAPPED |
      EFI_FVB2_READ_ENABLED_CAP |
      EFI_FVB2_READ_STATUS |
      EFI_FVB2_WRITE_ENABLED_CAP |
      EFI_FVB2_WRITE_STATUS |
      EFI_FVB2_ERASE_POLARITY |
      EFI_FVB2_ALIGNMENT_16,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,  // CheckSum
      0,  // ExtHeaderOffset
      {
        0,
      },  // Reserved[1]
      2,  // Revision
      {
        {
          (FixedPcdGet32 (PcdAllVarSize))/
          FixedPcdGet32 (PcdFlashBlockSize),
          FixedPcdGet32 (PcdFlashBlockSize),
        }
      } // BlockMap[1]
    },
    {
      {
        0,
        0
      }
    }  // End[1]
  }
};

EFI_STATUS
GetFvbInfo (
  IN  UINT64                      FvLength,
  OUT EFI_FIRMWARE_VOLUME_HEADER  **FvbInfo
  )
{
  STATIC BOOLEAN  Checksummed = FALSE;
  UINTN           Index;

  if (!Checksummed) {
    for (Index = 0;
         Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB_MEDIA_INFO);
         Index += 1)
    {
      UINT16  Checksum;
      mPlatformFvbMediaInfo[Index].FvbInfo.Checksum = 0;
      Checksum                                      = CalculateCheckSum16 (
                                                        (UINT16 *)&mPlatformFvbMediaInfo[Index].FvbInfo,
                                                        mPlatformFvbMediaInfo[Index].FvbInfo.HeaderLength
                                                        );
      mPlatformFvbMediaInfo[Index].FvbInfo.Checksum = Checksum;
    }
    Checksummed = TRUE;
  }

  for (Index = 0;
       Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB_MEDIA_INFO);
       Index += 1)
  {
    if (mPlatformFvbMediaInfo[Index].FvLength == FvLength) {
      *FvbInfo = &mPlatformFvbMediaInfo[Index].FvbInfo;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
