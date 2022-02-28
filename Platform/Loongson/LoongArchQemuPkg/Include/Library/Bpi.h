/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - BPI or Bpi    - Boot Parameter Interface
    - MEM or Mem    - Memory
    - info or INFO  - Information
    - SINFO    - Screen information
**/
#ifndef BPI_H_
#define BPI_H_

#include <IndustryStandard/LinuxBzimage.h>
#define SYSTEM_RAM              1
#define SYSTEM_RAM_RESERVED     2
#define ACPI_TABLE              3
#define ACPI_NVS                4
#define SYSTEM_PMEM             5

#define MAX_MEM_MAP             128

#define DEBUG_BPI

#define MAP_ENTRY(Entry, Type, Start, Size)  \
   MemMap->Map[(Entry)].MemType = (Type),   \
   MemMap->Map[(Entry)].MemStart = (Start), \
   MemMap->Map[(Entry)].MemSize = (Size),   \
   Entry++

#pragma pack(1)
typedef struct _extension_list_hdr{
  UINT64  Signature;
  UINT32  Length;
  UINT8   Revision;
  UINT8   CheckSum;
  struct  _extension_list_hdr *next;
}EXT_LIST;

typedef struct _BootParams_Interface {
  UINT64           Signature;   //{'B', 'P', 'I', '0', '1', '0', '0', '1'}
  EFI_SYSTEM_TABLE *SystemTable;
  EXT_LIST         *ExtList;
}BootParamsInterface;

typedef struct _MEMMAP_ {
  UINT32 MemType;
  UINT64 MemStart;
  UINT64 MemSize;
}MEMMAP;

typedef struct _MEM_MAP_ {
  EXT_LIST Header;         //  {'M', 'E', 'M'}
  UINT8    MapCount;
  MEMMAP   Map[MAX_MEM_MAP];
} MEM_MAP;

typedef struct {
  EXT_LIST Header;          // {SI}
  struct screen_info si;
}SINFO;

typedef struct {
  UINT64  BaseAddr;
  UINT64  Length;
  UINT32  Type;
  UINT32  Reserved;
} LOONGARCH_MEMMAP_ENTRY;

#pragma pack()

typedef struct screen_info SCREEN_INFO;

#endif

