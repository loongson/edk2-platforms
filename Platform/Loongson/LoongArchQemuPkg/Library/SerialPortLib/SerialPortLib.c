/** @file
  UART Serial Port library functions

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Library/Cpu.h>
#include <Library/IoLib.h>
#include <Library/SerialPortLib.h>

//---------------------------------------------
// UART Register Offsets
//---------------------------------------------
#define BAUD_LOW_OFFSET         0x00
#define BAUD_HIGH_OFFSET        0x01
#define IER_OFFSET              0x01
#define LCR_SHADOW_OFFSET       0x01
#define FCR_SHADOW_OFFSET       0x02
#define IR_CONTROL_OFFSET       0x02
#define FCR_OFFSET              0x02
#define EIR_OFFSET              0x02
#define BSR_OFFSET              0x03
#define LCR_OFFSET              0x03
#define MCR_OFFSET              0x04
#define LSR_OFFSET              0x05
#define MSR_OFFSET              0x06

/* character format control register */
#define	CFCR_DLAB	0x80	/* divisor latch */
#define	CFCR_SBREAK	0x40	/* send break */
#define	CFCR_PZERO	0x30	/* zero parity */
#define	CFCR_PONE	0x20	/* one parity */
#define	CFCR_PEVEN	0x10	/* even parity */
#define	CFCR_PODD	0x00	/* odd parity */
#define	CFCR_PENAB	0x08	/* parity enable */
#define	CFCR_STOPB	0x04	/* 2 stop bits */
#define	CFCR_8BITS	0x03	/* 8 data bits */
#define	CFCR_7BITS	0x02	/* 7 data bits */
#define	CFCR_6BITS	0x01	/* 6 data bits */
#define	CFCR_5BITS	0x00	/* 5 data bits */
/* modem control register */
#define	MCR_LOOPBACK	0x10	/* loopback */
#define	MCR_IENABLE	0x08	/* output 2 = int enable */
#define	MCR_DRS		0x04	/* output 1 = xxx */
#define	MCR_RTS		0x02	/* enable RTS */
#define	MCR_DTR		0x01	/* enable DTR */

/* line status register */
#define	LSR_RCV_FIFO	0x80	/* error in receive fifo */
#define	LSR_TSRE	0x40	/* transmitter empty */
#define	LSR_TXRDY	0x20	/* transmitter ready */
#define	LSR_BI		0x10	/* break detected */
#define	LSR_FE		0x08	/* framing error */
#define	LSR_PE		0x04	/* parity error */
#define	LSR_OE		0x02	/* overrun error */
#define	LSR_RXRDY	0x01	/* receiver ready */
#define	LSR_RCV_MASK	0x1f

/* 16550 UART register offsets and bitfields */
#define R_UART_RXBUF          0
#define R_UART_TXBUF          0
#define R_UART_BAUD_LOW       0
#define R_UART_BAUD_HIGH      1
#define R_UART_FCR            2
#define B_UART_FCR_FIFOE      BIT0
#define B_UART_FCR_FIFO64     BIT5
#define R_UART_LCR            3
#define B_UART_LCR_DLAB       BIT7
#define R_UART_MCR            4
#define B_UART_MCR_DTRC       BIT0
#define B_UART_MCR_RTS        BIT1
#define R_UART_LSR            5
#define B_UART_LSR_RXRDY      BIT0
#define B_UART_LSR_TXRDY      BIT5
#define B_UART_LSR_TEMT       BIT6
#define R_UART_MSR            6
#define B_UART_MSR_CTS        BIT4
#define B_UART_MSR_DSR        BIT5
#define B_UART_MSR_RI         BIT6
#define B_UART_MSR_DCD        BIT7

//---------------------------------------------
// UART Settings
//---------------------------------------------
#define PHYS_TO_CACHED(x)       ((x))
#define PHYS_TO_UNCACHED(x)     ((x))

UINTN   gUartBase = PHYS_TO_UNCACHED(0x1fe001e0);
UINTN   gBps      = 115200;

/**
  Initialize the serial device hardware.

  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfuly initialized, then return RETURN_SUCCESS.
  If the serial device could not be initialized, then return RETURN_DEVICE_ERROR.

  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serail device could not be initialized.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINTN  TimeOut;
  //
  // wait for Tx fifo to completely drain */
  //
  TimeOut = 1000000;
  while (!(MmioRead8 ((UINTN) gUartBase + LSR_OFFSET) & LSR_TSRE)) {
    if (--TimeOut == 0)
      break;
  }
  //
  // Set communications format
  //
  MmioWrite8 ((UINTN) (gUartBase + LCR_OFFSET), CFCR_DLAB);

  //
  // Configure baud rate
  //


  MmioWrite8 ((UINTN) (gUartBase + LCR_OFFSET), CFCR_8BITS);
  MmioWrite8 ((UINTN) (gUartBase + MCR_OFFSET), MCR_IENABLE | MCR_DTR | MCR_RTS);
  //
  // if enable interrupt the kernel of lemote will error in STR mode during wake up phase.
  //
  //MmioWrite8 ((UINTN) (gUartBase + IER_OFFSET), CFCR_8BITS);

  return RETURN_SUCCESS;
}

/**
  Write data from buffer to serial device.

  Writes NumberOfBytes data bytes from Buffer to the serial device.
  The number of bytes actually written to the serial device is returned.
  If the return value is less than NumberOfBytes, then the write operation failed.

  If Buffer is NULL, then ASSERT().

  If NumberOfBytes is zero, then return 0.

  @param  Buffer           Pointer to the data buffer to be written.
  @param  NumberOfBytes    Number of bytes to written to the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes written to the serial device.
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
UartCtlWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes,
  IN UINTN     CtlAddr
)
{
  UINTN  Result;
  UINT8  Data;

  if (Buffer == NULL) {
    return 0;
  }

  Result = NumberOfBytes;

  while (NumberOfBytes--) {
    //
    // Wait for the serail port to be ready.
    //
    do {
      Data = MmioRead8 (CtlAddr + LSR_OFFSET);
    } while ((Data & LSR_TXRDY) == 0);
    MmioWrite8 (CtlAddr, *Buffer++);
  }

  return Result;
}

UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  return UartCtlWrite(Buffer,NumberOfBytes,gUartBase);
}
/**
  Reads data from a serial device into a buffer.

  @param  Buffer           Pointer to the data buffer to store the data read from the serial device.
  @param  NumberOfBytes    Number of bytes to read from the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes read from the serial device.
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
UartCtlRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes,
  IN  UINTN     CtlAddr
)
{
  UINTN  Result;
  UINT8  Data;

  if (NULL == Buffer) {
    return 0;
  }

  Result = NumberOfBytes;

  while (NumberOfBytes--) {
    //
    // Wait for the serail port to be ready.
    //
    do {
      Data = MmioRead8 (CtlAddr + LSR_OFFSET);
    } while ((Data & LSR_RXRDY) == 0);

    *Buffer++ = MmioRead8 (CtlAddr);
  }

  return Result;
}

UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  return UartCtlRead(Buffer,NumberOfBytes,gUartBase);
}
/**
  Polls a serial device to see if there is any data waiting to be read.

  Polls aserial device to see if there is any data waiting to be read.
  If there is data waiting to be read from the serial device, then TRUE is returned.
  If there is no data waiting to be read from the serial device, then FALSE is returned.

  @retval TRUE             Data is waiting to be read from the serial device.
  @retval FALSE            There is no data waiting to be read from the serial device.

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  UINT8  Data;

  //
  // Read the serial port status.
  //
  Data = MmioRead8 ((UINTN) gUartBase + LSR_OFFSET);

  return (BOOLEAN) ((Data & LSR_RXRDY) != 0);
}

UINTN
GetSerialRegisterBase (
  VOID
  )
{
  return gUartBase;
}
/**
  Read an 8-bit register.
  @param  Base    The base address register of UART device.
  @param  Offset  The offset of the register to read.

  @return The value read from the 16550 register.

**/
UINT8
SerialPortReadRegister (
  UINTN  Base,
  UINTN  Offset
  )
{
    return MmioRead8 (Base + Offset);
}

/**
  Write an 8-bit register.
  @param  Base    The base address register of UART device.
  @param  Offset  The offset of the register to write.
  @param  Value   The value to write to the register specified by Offset.

  @return The value written to the 16550 register.

**/
UINT8
SerialPortWriteRegister (
  UINTN  Base,
  UINTN  Offset,
  UINT8  Value
  )
{
    return MmioWrite8 (Base + Offset, Value);
}



/**
  Sets the control bits on a serial device.

  @param Control                Sets the bits of Control that are settable.

  @retval RETURN_SUCCESS        The new control bits were set on the serial device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortSetControl (
  IN UINT32 Control
  )
{
  UINTN SerialRegisterBase;
  UINT8 Mcr;

  //
  // First determine the parameter is invalid.
  //
  if ((Control & (~(EFI_SERIAL_REQUEST_TO_SEND | EFI_SERIAL_DATA_TERMINAL_READY |
                    EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE))) != 0) {
    return RETURN_UNSUPPORTED;
  }

  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return RETURN_UNSUPPORTED;
  }

  //
  // Read the Modem Control Register.
  //
  Mcr = SerialPortReadRegister (SerialRegisterBase, R_UART_MCR);
  Mcr &= (~(B_UART_MCR_DTRC | B_UART_MCR_RTS));

  if ((Control & EFI_SERIAL_DATA_TERMINAL_READY) == EFI_SERIAL_DATA_TERMINAL_READY) {
    Mcr |= B_UART_MCR_DTRC;
  }

  if ((Control & EFI_SERIAL_REQUEST_TO_SEND) == EFI_SERIAL_REQUEST_TO_SEND) {
    Mcr |= B_UART_MCR_RTS;
  }

  //
  // Write the Modem Control Register.
  //
  SerialPortWriteRegister (SerialRegisterBase, R_UART_MCR, Mcr);

  return RETURN_SUCCESS;
}

/**
  Retrieve the status of the control bits on a serial device.

  @param Control                A pointer to return the current control signals from the serial device.

  @retval RETURN_SUCCESS        The control bits were read from the serial device.
  @retval RETURN_UNSUPPORTED    The serial device does not support this operation.
  @retval RETURN_DEVICE_ERROR   The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
SerialPortGetControl (
  OUT UINT32 *Control
  )
{
  UINTN SerialRegisterBase;
  UINT8 Msr;
  UINT8 Mcr;
  UINT8 Lsr;

  SerialRegisterBase = GetSerialRegisterBase ();
  if (SerialRegisterBase ==0) {
    return RETURN_UNSUPPORTED;
  }

  *Control = 0;

  //
  // Read the Modem Status Register.
  //
  Msr = SerialPortReadRegister (SerialRegisterBase, R_UART_MSR);

  if ((Msr & B_UART_MSR_CTS) == B_UART_MSR_CTS) {
    *Control |= EFI_SERIAL_CLEAR_TO_SEND;
  }

  if ((Msr & B_UART_MSR_DSR) == B_UART_MSR_DSR) {
    *Control |= EFI_SERIAL_DATA_SET_READY;
  }

  if ((Msr & B_UART_MSR_RI) == B_UART_MSR_RI) {
    *Control |= EFI_SERIAL_RING_INDICATE;
  }

  if ((Msr & B_UART_MSR_DCD) == B_UART_MSR_DCD) {
    *Control |= EFI_SERIAL_CARRIER_DETECT;
  }

  //
  // Read the Modem Control Register.
  //
  Mcr = SerialPortReadRegister (SerialRegisterBase, R_UART_MCR);

  if ((Mcr & B_UART_MCR_DTRC) == B_UART_MCR_DTRC) {
    *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
  }

  if ((Mcr & B_UART_MCR_RTS) == B_UART_MCR_RTS) {
    *Control |= EFI_SERIAL_REQUEST_TO_SEND;
  }

  if (PcdGetBool (PcdSerialUseHardwareFlowControl)) {
    *Control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
  }

  //
  // Read the Line Status Register.
  //
  Lsr = SerialPortReadRegister (SerialRegisterBase, R_UART_LSR);

  if ((Lsr & (B_UART_LSR_TEMT | B_UART_LSR_TXRDY)) == (B_UART_LSR_TEMT | B_UART_LSR_TXRDY)) {
    *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
  }

  if ((Lsr & B_UART_LSR_RXRDY) == 0) {
    *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
  }

  return RETURN_SUCCESS;
}

/**
  Sets the baud rate, receive FIFO depth, transmit/receice time out, parity,
  data bits, and stop bits on a serial device.

  @param BaudRate           The requested baud rate. A BaudRate value of 0 will use the
                            device's default interface speed.
                            On output, the value actually set.
  @param ReveiveFifoDepth   The requested depth of the FIFO on the receive side of the
                            serial interface. A ReceiveFifoDepth value of 0 will use
                            the device's default FIFO depth.
                            On output, the value actually set.
  @param Timeout            The requested time out for a single character in microseconds.
                            This timeout applies to both the transmit and receive side of the
                            interface. A Timeout value of 0 will use the device's default time
                            out value.
                            On output, the value actually set.
  @param Parity             The type of parity to use on this serial device. A Parity value of
                            DefaultParity will use the device's default parity value.
                            On output, the value actually set.
  @param DataBits           The number of data bits to use on the serial device. A DataBits
                            vaule of 0 will use the device's default data bit setting.
                            On output, the value actually set.
  @param StopBits           The number of stop bits to use on this serial device. A StopBits
                            value of DefaultStopBits will use the device's default number of
                            stop bits.
                            On output, the value actually set.

  @retval RETURN_SUCCESS            The new attributes were set on the serial device.
  @retval RETURN_UNSUPPORTED        The serial device does not support this operation.
  @retval RETURN_INVALID_PARAMETER  One or more of the attributes has an unsupported value.
  @retval RETURN_DEVICE_ERROR       The serial device is not functioning correctly.

**/
RETURN_STATUS
EFIAPI
UartCtlConfig (
  IN OUT UINT64             *BaudRate,
  IN OUT UINT32             *ReceiveFifoDepth,
  IN OUT UINT32             *Timeout,
  IN OUT EFI_PARITY_TYPE    *Parity,
  IN OUT UINT8              *DataBits,
  IN OUT EFI_STOP_BITS_TYPE *StopBits,
  IN UINTN                   CtlAddr
  )
{
  UINTN     SerialRegisterBase;
  UINT8     Lcr;
  UINT8     LcrData;
  UINT8     LcrParity;
  UINT8     LcrStop;


  SerialRegisterBase = CtlAddr;
  if (SerialRegisterBase ==0) {
    return RETURN_UNSUPPORTED;
  }

  //
  // Check for default settings and fill in actual values.
  //
  if (*BaudRate == 0) {
    *BaudRate = PcdGet32 (PcdSerialBaudRate);
  }

  if (*DataBits == 0) {
    LcrData = (UINT8) (PcdGet8 (PcdSerialLineControl) & 0x3);
    *DataBits = LcrData + 5;
  } else {
    if ((*DataBits < 5) || (*DataBits > 8)) {
      return RETURN_INVALID_PARAMETER;
    }
    //
    // Map 5..8 to 0..3
    //
    LcrData = (UINT8) (*DataBits - (UINT8) 5);
  }

  if (*Parity == DefaultParity) {
    LcrParity = (UINT8) ((PcdGet8 (PcdSerialLineControl) >> 3) & 0x7);
    switch (LcrParity) {
      case 0:
        *Parity = NoParity;
        break;

      case 3:
        *Parity = EvenParity;
        break;

      case 1:
        *Parity = OddParity;
        break;

      case 7:
        *Parity = SpaceParity;
        break;

      case 5:
        *Parity = MarkParity;
        break;

      default:
        break;
    }
  } else {
    switch (*Parity) {
      case NoParity:
        LcrParity = 0;
        break;

      case EvenParity:
        LcrParity = 3;
        break;

      case OddParity:
        LcrParity = 1;
        break;

      case SpaceParity:
        LcrParity = 7;
        break;

      case MarkParity:
        LcrParity = 5;
        break;

      default:
        return RETURN_INVALID_PARAMETER;
    }
  }

  if (*StopBits == DefaultStopBits) {
    LcrStop = (UINT8) ((PcdGet8 (PcdSerialLineControl) >> 2) & 0x1);
    switch (LcrStop) {
      case 0:
        *StopBits = OneStopBit;
        break;

      case 1:
        if (*DataBits == 5) {
          *StopBits = OneFiveStopBits;
        } else {
          *StopBits = TwoStopBits;
        }
        break;

      default:
        break;
    }
  } else {
    switch (*StopBits) {
      case OneStopBit:
        LcrStop = 0;
        break;

      case OneFiveStopBits:
      case TwoStopBits:
        LcrStop = 1;
        break;

      default:
        return RETURN_INVALID_PARAMETER;
    }
  }
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, B_UART_LCR_DLAB);

  //
  // Clear DLAB and configure Data Bits, Parity, and Stop Bits.
  // Strip reserved bits from line control value
  //
  Lcr = (UINT8) ((LcrParity << 3) | (LcrStop << 2) | LcrData);
  SerialPortWriteRegister (SerialRegisterBase, R_UART_LCR, (UINT8) (Lcr & 0x3F));

  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
SerialPortSetAttributes (
  IN OUT UINT64             *BaudRate,
  IN OUT UINT32             *ReceiveFifoDepth,
  IN OUT UINT32             *Timeout,
  IN OUT EFI_PARITY_TYPE    *Parity,
  IN OUT UINT8              *DataBits,
  IN OUT EFI_STOP_BITS_TYPE *StopBits
  )
{
  UINTN     SerialRegisterBase;

  SerialRegisterBase = GetSerialRegisterBase ();

  return  UartCtlConfig (&gBps,ReceiveFifoDepth,Timeout,Parity,DataBits,StopBits,\
      SerialRegisterBase);
}
