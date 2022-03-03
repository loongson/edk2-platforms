/** @file

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Tlb or TLB     - Translation Lookaside Buffer
    - CSR            - Cpu State Register
    - PGDL           - Page Global Directory Low
    - PGDH           - Page Global Directory High
    - TLBIDX         - TLB Index
    - TLBREHI        - TLB Refill Entry High
    - PWCTL          - Page Walk Control
    - STLB           - Singular Page Size TLB
    - PS             - Page Size
**/
#ifndef MMU_H_
#define MMU_H_
/*page size 4k*/
#define DEFAULT_PAGE_SIZE     0x0c
#define LOONGARCH_CSR_PGDL    0x19 /* Page table base address when VA[47] = 0 */
#define LOONGARCH_CSR_PGDH    0x1a /* Page table base address when VA[47] = 1 */
#define LOONGARCH_CSR_TLBIDX  0x10 /* TLB Index, EHINV, PageSize, NP */
#define LOONGARCH_CSR_TLBREHI 0x8e /* TLB refill entryhi */
#define LOONGARCH_CSR_PWCTL0  0x1c /* PWCtl0 */
#define LOONGARCH_CSR_PWCTL1  0x1d /* PWCtl1 */
#define LOONGARCH_CSR_STLBPGSIZE  0x1e
#define  CSR_TLBIDX_SIZE_MASK    0x3f000000
#define  CSR_TLBIDX_PS_SHIFT    24
#define  CSR_TLBIDX_SIZE    CSR_TLBIDX_PS_SHIFT

/*
  Set Cpu Status Register STLB Page Size.

  @param val  Page Size.

  @retval  VOID
 */
#define WRITE_CSR_STLB_PAGE_SIZE(val)  LOONGARCH_CSR_WRITEQ((val), LOONGARCH_CSR_STLBPGSIZE)
/*
  Set Cpu Status Register Page Size.

  @param size  Page Size.

  @retval  VOID
 */
#define WRITE_CSR_PAGE_SIZE(size)  LOONGARCH_CSR_XCHGQ((size) << CSR_TLBIDX_SIZE, CSR_TLBIDX_SIZE_MASK, LOONGARCH_CSR_TLBIDX)
/*
  Set Cpu Status Register TLBREFILL Page Size.

  @param size  Page Size.

  @retval  VOID
 */
#define WRITE_CSR_TLBREFILL_PAGE_SIZE(size)  LOONGARCH_CSR_XCHGQ((size) << CSR_TLBREHI_PS_SHIFT, CSR_TLBREHI_PS, LOONGARCH_CSR_TLBREHI)
/*
  Set Cpu Status Register TLBREFILL Base Address.

  @param BaseAddress the code base address of TLB refills .

  @retval  VOID
 */
#define SET_REFILL_TLBBASE(BaseAddress) LOONGARCH_CSR_WRITEQ((BaseAddress), LOONGARCH_CSR_TLBREBASE);
/*
  Get Cpu Status Register Page Size.

  @param  val  Gets the page size.

  @retval  VOID
 */
#define READ_CSR_PAGE_SIZE(val)                               \
{                                                             \
  LOONGARCH_CSR_READQ ((val), LOONGARCH_CSR_TLBIDX);          \
  (val) = ((val) & CSR_TLBIDX_SIZE_MASK) >> CSR_TLBIDX_SIZE;  \
}


#define  CSR_TLBREHI_PS_SHIFT    0
#define  CSR_TLBREHI_PS      ((UINTN)(0x3f) << CSR_TLBREHI_PS_SHIFT)

#define EFI_MEMORY_CACHETYPE_MASK     (EFI_MEMORY_UC  | \
                                       EFI_MEMORY_WC  | \
                                       EFI_MEMORY_WT  | \
                                       EFI_MEMORY_WB  | \
                                       EFI_MEMORY_UCE   \
                                       )


typedef struct {
  EFI_PHYSICAL_ADDRESS PhysicalBase;
  EFI_VIRTUAL_ADDRESS  VirtualBase;
  UINTN                Length;
  UINTN                Attributes;
} MEMORY_REGION_DESCRIPTOR;

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS (128)

extern CHAR8 HandleTlbRefill[], HandleTlbRefillEnd[];
#endif
