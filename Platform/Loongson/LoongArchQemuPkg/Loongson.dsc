## @file
#
#  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
###############################################################################
[Defines]
  PLATFORM_NAME                  = LoongArchQemu
  PLATFORMPKG_NAME               = LoongArchQemu
  PLATFORM_GUID                  = 7926ea52-b0dc-4ee8-ac63-341eebd84ed4
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/$(PLATFORM_NAME)
  SUPPORTED_ARCHITECTURES        = LOONGARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = Platform/Loongson/LoongArchQemuPkg/Loongson.fdf
  TTY_TERMINAL                   = FALSE

############################################################################
#
# Defines for default states.  These can be changed on the command line.
# -D FLAG=VALUE
############################################################################
[BuildOptions]
  GCC:RELEASE_*_*_CC_FLAGS       = -DSPEEDUP

  #
  # Disable deprecated APIs.
  #
  GCC:*_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES


[BuildOptions.LOONGARCH64.EDKII.SEC]
  *_*_*_CC_FLAGS                 =

[BuildOptions.common.EDKII.DXE_CORE,BuildOptions.common.EDKII.DXE_DRIVER,BuildOptions.common.EDKII.UEFI_DRIVER,BuildOptions.common.EDKII.UEFI_APPLICATION]
  GCC:*_*_*_DLINK_FLAGS = -z common-page-size=0x1000

[BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER]
  GCC:*_*_LOONGARCH64_DLINK_FLAGS = -z common-page-size=0x10000

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses.common]
  PcdLib                           | MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  TimerLib                         | Platform/Loongson/LoongArchQemuPkg/Library/StableTimerLib/TimerLib.inf
  PrintLib                         | MdePkg/Library/BasePrintLib/BasePrintLib.inf
  BaseMemoryLib                    | MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf


  BaseLib                          | MdePkg/Library/BaseLib/BaseLib.inf
  PerformanceLib                   | MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PeCoffLib                        | MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  CacheMaintenanceLib              | MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  UefiDecompressLib                | MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  PeCoffGetEntryPointLib           | MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  IoLib                            | MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SerialPortLib                    | Platform/Loongson/LoongArchQemuPkg/Library/SerialPortLib/SerialPortLib.inf
  DebugPrintErrorLevelLib          | MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  FdtLib                           | EmbeddedPkg/Library/FdtLib/FdtLib.inf
  PeCoffExtraActionLib             | MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  DebugAgentLib                    | MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf

  DebugLib                         | MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf

  PeiServicesLib                   | MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
[LibraryClasses.common.SEC]
  ReportStatusCodeLib              | MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  HobLib                           | MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib              | MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf

[LibraryClasses.common.PEI_CORE]
  HobLib                           | MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesTablePointerLib       | Platform/Loongson/LoongArchQemuPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  MemoryAllocationLib              | MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiCoreEntryPoint                | MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  ReportStatusCodeLib              | MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  OemHookStatusCodeLib             | MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib           | MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  QemuFwCfgLib                     | Platform/Loongson/LoongArchQemuPkg/Library/QemuFwCfgLib/QemuFwCfgLib.inf
  MmuLib                           | Platform/Loongson/LoongArchQemuPkg/Library/MmuLib/MmuBaseLibPei.inf

[LibraryClasses.common.PEIM]
  HobLib                           | MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesTablePointerLib       | Platform/Loongson/LoongArchQemuPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  MemoryAllocationLib              | MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeimEntryPoint                   | MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  ReportStatusCodeLib              | MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf
  OemHookStatusCodeLib             | MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib           | MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeiResourcePublicationLib        | MdePkg/Library/PeiResourcePublicationLib/PeiResourcePublicationLib.inf
  ExtractGuidedSectionLib          | MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  PcdLib                           | MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  QemuFwCfgS3Lib                   | OvmfPkg/Library/QemuFwCfgS3Lib/PeiQemuFwCfgS3LibFwCfg.inf
  QemuFwCfgLib                     | Platform/Loongson/LoongArchQemuPkg/Library/QemuFwCfgLib/QemuFwCfgLib.inf

  MmuLib                       | Platform/Loongson/LoongArchQemuPkg/Library/MmuLib/MmuBaseLibPei.inf


################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform.
#
################################################################################
[PcdsFixedAtBuild]
## BaseLib ##
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength               | 1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength                 | 1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength                  | 1000000

  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel                     | 0x8000004F
  # DEBUG_INIT      0x00000001  // Initialization
  # DEBUG_WARN      0x00000002  // Warnings
  # DEBUG_LOAD      0x00000004  // Load events
  # DEBUG_FS        0x00000008  // EFI File system
  # DEBUG_POOL      0x00000010  // Alloc & Free (pool)
  # DEBUG_PAGE      0x00000020  // Alloc & Free (page)
  # DEBUG_INFO      0x00000040  // Informational debug messages
  # DEBUG_DISPATCH  0x00000080  // PEI/DXE/SMM Dispatchers
  # DEBUG_VARIABLE  0x00000100  // Variable
  # DEBUG_BM        0x00000400  // Boot Manager
  # DEBUG_BLKIO     0x00001000  // BlkIo Driver
  # DEBUG_NET       0x00004000  // Network Io Driver
  # DEBUG_UNDI      0x00010000  // UNDI Driver
  # DEBUG_LOADFILE  0x00020000  // LoadFile
  # DEBUG_EVENT     0x00080000  // Event messages
  # DEBUG_GCD       0x00100000  // Global Coherency Database changes
  # DEBUG_CACHE     0x00200000  // Memory range cachability changes
  # DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may
  # DEBUG_ERROR     0x80000000  // Error

!if $(TARGET) == RELEASE
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask                        | 0x21
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask                        | 0x2f
!endif
  # DEBUG_ASSERT_ENABLED       0x01
  # DEBUG_PRINT_ENABLED        0x02
  # DEBUG_CODE_ENABLED         0x04
  # CLEAR_MEMORY_ENABLED       0x08
  # ASSERT_BREAKPOINT_ENABLED  0x10
  # ASSERT_DEADLOOP_ENABLED    0x20

#######################################################################################
  gLoongArchQemuPkgTokenSpaceGuid.PcdSecPeiTempRamBase                 | 0x10000
  gLoongArchQemuPkgTokenSpaceGuid.PcdSecPeiTempRamSize                 | 0x10000
  gLoongArchQemuPkgTokenSpaceGuid.PcdDeviceTreeBase                    | 0x1c400000
  #
  # minimal memory for uefi bios should be 512M
  # 0x00000000 - 0x10000000
  # 0x90000000 - 0xA0000000
  #
  gLoongArchQemuPkgTokenSpaceGuid.PcdUefiRamTop                        | 0x10000000

[Components]

  #
  # SEC Phase modules
  #
  Platform/Loongson/LoongArchQemuPkg/Sec/SecMain.inf

  #
  # PEI Phase modules
  #
  MdeModulePkg/Core/Pei/PeiMain.inf
  MdeModulePkg/Universal/PCD/Pei/Pcd.inf  {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }
  MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  MdeModulePkg/Core/DxeIplPeim/DxeIpl.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }

  Platform/Loongson/LoongArchQemuPkg/PlatformPei/PlatformPei.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  }

