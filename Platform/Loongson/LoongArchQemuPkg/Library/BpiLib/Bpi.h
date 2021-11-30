/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#ifndef _BPI_H_
#define _BPI_H_

#include <IndustryStandard/LinuxBzimage.h>
#define SYSTEM_RAM              1
#define SYSTEM_RAM_RESERVED     2
#define ACPI_TABLE              3
#define ACPI_NVS                4
#define SYSTEM_PMEM             5

#define MAX_MEM_MAP             128

#pragma pack(1)
typedef struct _extension_list_hdr{
  UINT64  Signature;
  UINT32  Length;
  UINT8   Revision;
  UINT8   CheckSum;
  struct  _extension_list_hdr *next;
}EXT_LIST;

struct BootParamsInterface {
  UINT64           Signature;   //{'B', 'P', 'I', '_', '0', '_', '1'}
  EFI_SYSTEM_TABLE *SystemTable;
  EXT_LIST         *ExtList;
};

typedef struct {
  EXT_LIST Header;         //  {'M', 'E', 'M'}
  UINT8    MapCount;
  struct   MemMap {
    UINT32 MemType;
    UINT64 MemStart;
    UINT64 MemSize;
  }Map[MAX_MEM_MAP];
} MEM_MAP;

typedef struct {
  EXT_LIST Header;          // {SI}
  struct screen_info si;
}SInfo;

typedef struct {
  UINT64                BaseAddr;
  UINT64                Length;
  UINT32		Type;
  UINT32		Reserved;
} EFI_LA_MEMMAP_ENTRY;

#pragma pack()

#endif

