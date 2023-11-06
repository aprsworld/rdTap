# rdTap

## Introduction

`rdTap` is a system for remotely monitoring and controlling. It runs on 
APRS World's DCSWC hardware platform. The DCSWC has a small onboard PIC18F25K80
microcontroller that runs rdTap. There are five option slots that accomodate
additional modules. The modules can be smart and communicate via I2C or they 
can be dump and just have a single control and a single status line.

## Communications

### RS-485 Port

The DCSWC board's port P4 provides an RS-485 port for communicating with off-board devices. The rdTap software operates as a Modbus RTU master on this port.

### Async port

There is an async UART on the controller that is exposed via the TTL level port P3 on the DCSWC board. And it is jumper connectable to an onboard Lantronix XPort ethernet embedded device server.

This port runs at 57600 / 8N1.

#### XPort

The onboard XPort provides a 10/100 megabit ethernet connection. It is enabled
on the DCSWC board installing the three jumpbers near it. Communications can be
limited to outgoing only or incomming only by installing the corresponding jumpers. The XPort can be completely disabled and non-powered by removing all three jumpers.

[XPort documentation](https://www.lantronix.com/products/xport/#product-resources) is available from Lantronix

## Hardware

### Programming the DCSWC board

The DCSWC is programmed via the J1 header which is an ICSP port. Pins are shared
between this port and the `SER_TO_NET` and `SER_FROM_NET` signals used for
async communications. These signals are present on the P3 header and on the 
jumpers to the onboard XPORT for rev3 and later hardware. To program, the three
jumpers enabling the XPORT should be removed and any cables connected to P3
should be disconnected. 
