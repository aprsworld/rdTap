#include <18F25K80.h>
#device ADC=12
#device *=16

#fuses INTRC_IO
#fuses NOFCMEN
#fuses NOIESO
#fuses NOPUT
#fuses BROWNOUT
#fuses BORV30
#fuses WDT512
#fuses NOMCLR
#fuses STVREN
#fuses SOSC_DIG
#fuses NOXINST
#fuses NODEBUG
#fuses NOPROTECT


#byte ANCON0=GETENV("SFR:ancon0")
#byte ANCON1=GETENV("SFR:ancon1")



#include <stdlib.h>
#use delay(clock=8000000, restart_wdt)

/* 
Parameters and devices are stored in 1024 byte EEPROM
*/
#define PARAM_CRC_ADDRESS  0x0000
#define DEVICE_CRC_ADDRESS 0x0002

#define PARAM_ADDRESS      0x0004 /* parameters go from 4 to 127 ... 123 bytes max */ 
#define DEVICE_ADDRESS     0x0080 /* devices go from 128 to 1024 ... 896 bytes max */


/* uart1 is for modbus and set in modbus_int_uart */
#use rs232(UART2,stream=world, baud=57600, errors)	


#use standard_io(A)
#use standard_io(B)
#use standard_io(C)

/* I2C on hardware */
#use i2c(master, sda=PIN_C4, scl=PIN_C3, FAST)



#define LED_GREEN       PIN_B5

#define SER_TO_NET      PIN_B6
#define SER_FROM_NET    PIN_B7
#define RS485_DE        PIN_C5


#define CTRL_0          PIN_A0
#define CTRL_1          PIN_A1
#define CTRL_2          PIN_A2
#define CTRL_3          PIN_A3
#define CTRL_4          PIN_A5

#define STAT_0          PIN_A7
#define STAT_1          PIN_A6
#define STAT_2          PIN_C0
#define STAT_3          PIN_C1
#define STAT_4          PIN_C2

#define I2C_SCL         PIN_C3
#define I2C_SDA         PIN_C4

#define CS_ADC0         PIN_B0
#define SPI_DIN         PIN_B1
#define SPI_DOUT        PIN_B2
#define SPI_CLK         PIN_B3
#define CS_ADC1         PIN_B4

/* U4 - first ADC */
#define ADC_CH_VDIV_0   0
#define ADC_CH_IMON_0   1
#define ADC_CH_VDIV_1   2
#define ADC_CH_IMON_1   3
#define ADC_CH_VDIV_2   4
#define ADC_CH_IMON_2   5
#define ADC_CH_VDIV_3   6
#define ADC_CH_IMON_3   7

/* U6 - second ADC */
#define ADC_CH_VDIV_4   8
#define ADC_CH_IMON_4   9
#define ADC_CH_VDIV_5   10
#define ADC_CH_TP_1     11
#define ADC_CH_VDIV_6   12
#define ADC_CH_TP_2     13
#define ADC_CH_VDIV_7   14
#define ADC_CH_TP_3     15





/* device types for struct_device */
#define DEV_TYPE_DISABLED       0

/* modbus slave on RS-485 bus */
#define DEV_TYPE_MODBUS_1       1
#define DEV_TYPE_MODBUS_2       2
#define DEV_TYPE_MODBUS_3       3
#define DEV_TYPE_MODBUS_4       4
#define DEV_TYPE_MODBUS_5       5
#define DEV_TYPE_MODBUS_6       6
#define DEV_TYPE_MODBUS_16      7

/* I2C slave on I2C bus */
#define DEV_TYPE_I2C_READ_8     16
#define DEV_TYPE_I2C_READ_16    17
#define DEV_TYPE_I2C_READ_24    18
#define DEV_TYPE_I2C_READ_32    19
#define DEV_TYPE_I2C_WRITE_8    20
#define DEV_TYPE_I2C_WRITE_16   21
#define DEV_TYPE_I2C_WRITE_24   22
#define DEV_TYPE_I2C_WRITE_32   23

/* local registers on this device */
#define DEV_TYPE_LOCAL_READ_16  32
#define DEV_TYPE_LOCAL_WRITE_16 32



/* serial port speeds */
#define DEV_SERIAL_9600  0
#define DEV_SERIAL_19200 1


#define DEV_MAX_N 64
