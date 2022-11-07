## @file
#
#  Copyright (c) 2022 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

DEFINE BLOCK_SIZE                 = 0x1000

############################################################################
# fd total
DEFINE FD_BASE_ADDRESS            = 0x1c000000
DEFINE FD_BLOCKS                  = 0x400
DEFINE FD_SIZE                    = 0x400000

############################################################################
#flash code layout
#Set Sec base address and size in flash
DEFINE SECFV_OFFSET               = 0x00000000
DEFINE SECFV_SIZE                 = 0x00010000

#Set Pei base address and size in flash
DEFINE PEIFV_OFFSET               = 0x00010000
DEFINE PEIFV_SIZE                 = 0x00040000

#Set Dxe base address and size in flash
DEFINE DXEFV_OFFSET               = 0x00050000
DEFINE DXEFV_SIZE                 = 0x00350000

#Set Var base address and size in flash
DEFINE VARIABLE_OFFSET            = 0x003a0000
DEFINE VAR_ALL_SIZE               = 0x60000

############################################################################
#Set Var Flash layout
DEFINE VARIABLE_SIZE              = 0x00010000
DEFINE VAR_ALL_BLOCKS             = 0x60

DEFINE RESERVE1_OFFSET            = $(VARIABLE_OFFSET) + $(VARIABLE_SIZE)
DEFINE RESERVE1_SIZE              = 0xB000

DEFINE RESERVE2_OFFSET            = $(RESERVE1_OFFSET) + $(RESERVE1_SIZE)
DEFINE RESERVE2_SIZE              = 0x14000

DEFINE SPARE_OFFSET               = $(RESERVE2_OFFSET) + $(RESERVE2_SIZE)
DEFINE SPARE_SIZE                 = 0x20000

DEFINE FWTWORKING_OFFSET          = $(SPARE_OFFSET) + $(SPARE_SIZE)
DEFINE FWTWORKING_SIZE            = 0x8000

DEFINE EVENT_LOG_OFFSET           = $(FWTWORKING_OFFSET) + $(FWTWORKING_SIZE)
DEFINE EVENT_LOG_SIZE             = 0x8000

# Set Variable
SET gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase64 = 0
SET gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableSize = $(VARIABLE_SIZE)

# Set FtwSpare
SET gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareSize = $(SPARE_SIZE)

# Set FtwWorking
SET gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingSize = $(FWTWORKING_SIZE)
