/** @file
  UART Serial Port library functions

  Copyright (c) 2021 Loongson Technology Corporation Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Glossary:
    - Bps - Bit Per Second
    - CTL - Control
    - Config - Configure
**/

#include <Base.h>
#include <Library/Cpu.h>
#include <Library/IoLib.h>
#include <Library/SerialPortLib.h>
#include <LoongArchQemuPlatform.h>

UINTN   gUartBase = PHYS_TO_UNCACHED(UART_BASE_ADDRESS);
UINTN   gBps      = UART_BPS;

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
  TimeOut = UART_WAIT_TIMOUT;
  while (!(MmioRead8 ((UINTN) gUartBase + LSR_OFFSET) & LSR_TSRE)) {
    if (--TimeOut == 0) {
      break;
    }
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

  If Buffer is NULL, then ASSERT ().

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
/**
  Writes data to serial port.

  @param  Buffer           Pointer to the data buffer to store the data writed to serial port.
  @param  NumberOfBytes    Number of bytes to write to the serial port.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes write the serial port.
                           If this value is less than NumberOfBytes, then the write operation failed.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  return UartCtlWrite (Buffer, NumberOfBytes, gUartBase);
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
/**
  Read data from serial port.

  @param  Buffer           Pointer to the data buffer to store the data read from serial port.
  @param  NumberOfBytes    Number of bytes to read from the serial port.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes read from the serial port.
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  return UartCtlRead (Buffer, NumberOfBytes, gUartBase);
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
/**
  To get serial register base address.

  @param  VOID

  @return  serial register base address.
**/
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
                    EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE))) != 0)
  {
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
    if ((*DataBits < 5)
      || (*DataBits > 8))
    {
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
/**
  Set the serial port Attributes.

  @param  VOID

  @return  serial register base address.
**/
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

  return  UartCtlConfig (&gBps, ReceiveFifoDepth, Timeout, Parity, DataBits, StopBits,
            SerialRegisterBase);
}
