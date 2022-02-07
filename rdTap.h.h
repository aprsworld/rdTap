#include <18F24J11.h>
#device ADC=10
#device *=16

/* 
leave page 0 alone for bootloader (0x000 to 0x3ff) 
leave page 1 alone for config storage (0x400 to 0x7ff)
*/
#build(reset=0x800,interrupt=0x808)
#org 0x0000,0x07ff {}

#define PARAM_CRC_ADDRESS  0x400
#define PARAM_ADDRESS      PARAM_CRC_ADDRESS+2

/* 
leave last 1K of code space alone for device storage 
*/
#define DEVICE_CRC_ADDRESS  0x3800
#define DEVICE_ADDRESS      DEVICE_CRC_ADDRESS+2
#org DEVICE_CRC_ADDRESS,DEVICE_CRC_ADDRESS+1023 {}



#include <stdlib.h>
/* fuses set in bootloader ... bootload using DS30 loader at 57600 */
#use delay(clock=8000000, restart_wdt)

#pin_select U2TX=PIN_C0
#pin_select U2RX=PIN_C1

/* uart1 is for modbus and set in modbus_int_uart */
#use rs232(UART2,stream=world, baud=57600, errors)	


#use standard_io(A)
#use standard_io(B)
#use standard_io(C)

/* status LEDs */
#define LED_GREEN                  PIN_B4
#define LED_RED                    PIN_B5
#define TP1                        PIN_B3
#define TP2                        PIN_B2
#define TP3                        PIN_B0


/* for communicating with world (802.15.4 or ethernet) */
#define XBEE_SLEEP                 PIN_C2
#define WORLD_TX                   PIN_C0
#define WORLD_RX                   PIN_C1
#define WORLD_RTS_IN               PIN_C4
#define WORLD_CTS_OUT              PIN_C5

/* for communicating with device on our RS-232 or RS-485 net */
#define NET_TX                     PIN_C6
#define NET_RX                     PIN_C7
#define RS485_NOT_RE               PIN_A0
#define RS485_DE                   PIN_A1

/* device types for struct_device */
#define DEV_TYPE_DISABLED     0
#define DEV_TYPE_MODBUS_1     1
#define DEV_TYPE_MODBUS_2     2
#define DEV_TYPE_MODBUS_3     3
#define DEV_TYPE_MODBUS_4     4
#define DEV_TYPE_MODBUS_5     5
#define DEV_TYPE_MODBUS_6     6
#define DEV_TYPE_MODBUS_16    7


/* serial port speeds */
#define DEV_SERIAL_9600  0
#define DEV_SERIAL_19200 1


#define DEV_MAX_N 64
