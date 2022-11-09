/** @file
   LoongArch Qemu Platform macro definition.

   Copyright (c) 2022, Loongson Limited. All rights reserved.

   SPDX-License-Identifier: BSD-2-Clause-Patent

 **/

#ifndef LOONGARCH_QEMU_PLATFORM_H_
#define LOONGARCH_QEMU_PLATFORM_H_

/* Acpi pm device */
#define LS7A_PCH_REG_BASE             0x10000000UL
#define LS7A_ACPI_REG_BASE            (LS7A_PCH_REG_BASE  + 0x000D0000)
#define LS7A_PM_CNT_BLK               (0x14) /* 2 bytes */
#define LS7A_GPE0_RESET_REG           (0x30) /* 4 bytes */

#define ACPI_BITMASK_SLEEP_TYPE       0x1C00
#define ACPI_BITMASK_SLEEP_ENABLE     0x2000

//---------------------------------------------
// UART Register Offsets
//---------------------------------------------
#define BAUD_LOW_OFFSET               0x00
#define BAUD_HIGH_OFFSET              0x01
#define IER_OFFSET                    0x01
#define LCR_SHADOW_OFFSET             0x01
#define FCR_SHADOW_OFFSET             0x02
#define IR_CONTROL_OFFSET             0x02
#define FCR_OFFSET                    0x02
#define EIR_OFFSET                    0x02
#define BSR_OFFSET                    0x03
#define LCR_OFFSET                    0x03
#define MCR_OFFSET                    0x04
#define LSR_OFFSET                    0x05
#define MSR_OFFSET                    0x06

/* character format control register */
#define CFCR_DLAB                     0x80  /* divisor latch */
#define CFCR_SBREAK                   0x40  /* send break */
#define CFCR_PZERO                    0x30  /* zero parity */
#define CFCR_PONE                     0x20  /* one parity */
#define CFCR_PEVEN                    0x10  /* even parity */
#define CFCR_PODD                     0x00  /* odd parity */
#define CFCR_PENAB                    0x08  /* parity enable */
#define CFCR_STOPB                    0x04  /* 2 stop bits */
#define CFCR_8BITS                    0x03  /* 8 data bits */
#define CFCR_7BITS                    0x02  /* 7 data bits */
#define CFCR_6BITS                    0x01  /* 6 data bits */
#define CFCR_5BITS                    0x00  /* 5 data bits */
/* modem control register */
#define MCR_LOOPBACK                  0x10  /* loopback */
#define MCR_IENABLE                   0x08  /* output 2 = int enable */
#define MCR_DRS                       0x04  /* output 1 = xxx */
#define MCR_RTS                       0x02  /* enable RTS */
#define MCR_DTR                       0x01  /* enable DTR */

/* line status register */
#define LSR_RCV_FIFO                  0x80  /* error in receive fifo */
#define LSR_TSRE                      0x40  /* transmitter empty */
#define LSR_TXRDY                     0x20  /* transmitter ready */
#define LSR_BI                        0x10  /* break detected */
#define LSR_FE                        0x08  /* framing error */
#define LSR_PE                        0x04  /* parity error */
#define LSR_OE                        0x02  /* overrun error */
#define LSR_RXRDY                     0x01  /* receiver ready */
#define LSR_RCV_MASK                  0x1f

/* 16550 UART register offsets and bitfields */
#define R_UART_RXBUF                  0
#define R_UART_TXBUF                  0
#define R_UART_BAUD_LOW               0
#define R_UART_BAUD_HIGH              1
#define R_UART_FCR                    2
#define B_UART_FCR_FIFOE              BIT0
#define B_UART_FCR_FIFO64             BIT5
#define R_UART_LCR                    3
#define B_UART_LCR_DLAB               BIT7
#define R_UART_MCR                    4
#define B_UART_MCR_DTRC               BIT0
#define B_UART_MCR_RTS                BIT1
#define R_UART_LSR                    5
#define B_UART_LSR_RXRDY              BIT0
#define B_UART_LSR_TXRDY              BIT5
#define B_UART_LSR_TEMT               BIT6
#define R_UART_MSR                    6
#define B_UART_MSR_CTS                BIT4
#define B_UART_MSR_DSR                BIT5
#define B_UART_MSR_RI                 BIT6
#define B_UART_MSR_DCD                BIT7
#define UART_BASE_ADDRESS             (0x1fe001e0)
#define UART_BPS                      (115200)
#define UART_WAIT_TIMOUT              (1000000)
#endif // LOONGARCH_QEMU_PLATFORM_H_
