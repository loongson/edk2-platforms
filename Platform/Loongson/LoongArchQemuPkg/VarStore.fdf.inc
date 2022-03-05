## @file
#
#  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

$(VARIABLE_OFFSET)|$(VARIABLE_SIZE)
#NV_VARIABLE_STORE
DATA = {
  #jmp to 0x1c060000
  0x01, 0x0c, 0x38, 0x14, 0x21, 0x00, 0x80, 0x03,
  0x21, 0x00, 0x00, 0x4c, 0x00, 0x10, 0x14, 0x00,
  # FileSystemGuid: gEfiSystemNvDataFvGuid =
  #   { 0xFFF12B8D, 0x7696, 0x4C8B,
  #     { 0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50 }}
  0x8D, 0x2B, 0xF1, 0xFF, 0x96, 0x76, 0x8B, 0x4C,
  0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50,
  # FvLength: 0x60000
  0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
  # Signature "_FVH"       # Attributes
  0x5f, 0x46, 0x56, 0x48, 0x36, 0x0c, 0x04, 0x00,
  # HeaderLength # CheckSum # ExtHeaderOffset #Reserved #Revision
  0x48, 0x00, 0x8F, 0x6B, 0x00, 0x00, 0x00, 0x02,
  # Blockmap[0]: 0x10 Blocks * 0x1000 Bytes / Block
  0x60, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  # Blockmap[1]: End
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  ## This is the VARIABLE_STORE_HEADER
  # gEfiVariableGuid = {0xddcf3616, 0x3275, 0x4164, { 0x98, 0xb6, 0xfe, 0x85, 0x70, 0x7f, 0xfe, 0x7d}}
  0x16, 0x36, 0xcf, 0xdd, 0x75, 0x32, 0x64, 0x41,
  0x98, 0xb6, 0xfe, 0x85, 0x70, 0x7f, 0xfe, 0x7d,
  # Size: 0x10000 (gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize) -
  #         0x48 (size of EFI_FIRMWARE_VOLUME_HEADER) = 0xffb8
  # This can speed up the Variable Dispatch a bit.
  0xb8, 0xff, 0x00, 0x00,
  # FORMATTED: 0x5A #HEALTHY: 0xFE #Reserved: UINT16 #Reserved1: UINT32
  0x5a, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}
$(RESERVE1_OFFSET)|$(RESERVE1_SIZE)
#NV_RESERVE1_STORE

$(RESERVE2_OFFSET)|$(RESERVE2_SIZE)
#NV_RESERVE2_STORE

$(SPARE_OFFSET)|$(SPARE_SIZE)
#NV_FTW_SPARE

$(FWTWORKING_OFFSET)|$(FWTWORKING_SIZE)
#NV_FTW_WORKING
DATA = {
  # EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER->Signature = gEdkiiWorkingBlockSignatureGuid =
  #  { 0x9e58292b, 0x7c68, 0x497d, { 0xa0, 0xce, 0x65,  0x0, 0xfd, 0x9f, 0x1b, 0x95 }}
  0x2b, 0x29, 0x58, 0x9e, 0x68, 0x7c, 0x7d, 0x49,
  0xa0, 0xce, 0x65,  0x0, 0xfd, 0x9f, 0x1b, 0x95,
  # Crc:UINT32            #WorkingBlockValid:1, WorkingBlockInvalid:1, Reserved
  0x07, 0x7f, 0x44, 0x88, 0xfe, 0xff, 0xff, 0xff,
  # WriteQueueSize: UINT64 #Size:0x8000 - 0x20(FTW_WORKING_HEADER) = 0x7FE0
  0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}

$(EVENT_LOG_OFFSET)|$(EVENT_LOG_SIZE)
#NV_EVENT_LOG