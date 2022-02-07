#define DEBUG_ASCII 0

#include "rdTap.h"
#include "worldDeviceTypes.h"

typedef struct {
	int8  type;           /* device query type */
	int16 typeWorld;      /* see worldDeviceTypes.h for defines */
	int8  transmitEvery;
	int8  networkAddress; /* modbus or I2C address */
	int32 serialNumber;   /* APRS World or other serial number */
	int16 startRegister;
	int8  nRegisters;
	int8  serialSpeed;    /* see rsTap.h for defines */
} struct_device;


typedef struct {
	int8  now_poll;
	int8  world_timeout;
	int8  factory_unlocked;

	int8  led_on_green;
} struct_timer;


typedef struct {
	int8  rException;
	int8  rResult[256];
	int8  rResultLength;
	int8  deviceNumber;
	int16 measurementNumber;
} struct_qbuff;

typedef struct {
	int8 buff[256];
	int8 buff_pos;
	int8 buff_ready;

	int8  packet_length;
	int32 query_id;
	int32 device_serial;
	int16 network_address;
	int16 function;
	int16 start_address;
	int8  n_words;
	int8  data_start_offset;
	int16 crc;

	int8  resultException;
	int8  resultLength;
} struct_query;

typedef struct {
	int8  serial_prefix;
	int16 serial_number;
	
	int16 live_interval;
} struct_config;

/* global structures */
struct_device device[DEV_MAX_N];
struct_timer  timers;
struct_qbuff  qbuff;
struct_query  query;
struct_config config;

#include "mcp3208_rdTap.c"
#include "uart_sc16is740_rdTap.c"
#include "param_rdTap.c"
#include "paramDevice_rdTap.c"
#include "modbus_int_uart_rdTap.c"
#include "interrupt_rdTap.c"
#include "live_rdTap.c"
#include "queryHandler_rdTap.c"


void deviceQuery(void) {
	static int16 measurementNumber=0;
	static int8  nCycles[DEV_MAX_N];
	int8 n;


//	fprintf(world,"# querying all enabled devices:\r\n");
	for ( n=0 ; n<DEV_MAX_N ; n++ ) {
		restart_wdt();
		timers.led_on_green=100;

		/* skip disabled devices */
		if ( DEV_TYPE_DISABLED == device[n].type ) 
			continue;

		nCycles[n]++;
		if ( nCycles[n] < device[n].transmitEvery ) {
			continue;
		} else {
			nCycles[n]=0;
		}


		/* set serial port speed */
		if ( DEV_SERIAL_19200 == device[n].serialSpeed ) {
			set_uart_speed(19200,MODBUS_SERIAL);
		} else {
			set_uart_speed(9600,MODBUS_SERIAL);
		}


		qbuff.deviceNumber=n;
		qbuff.measurementNumber=measurementNumber;

		if ( DEV_TYPE_MODBUS_3 == device[n].type || DEV_TYPE_MODBUS_4 == device[n].type ) {
			/* Modbus read input or holding registers */
			if ( DEV_TYPE_MODBUS_4 == device[n].type ) {
				qbuff.rException=modbus_read_input_registers(device[n].networkAddress, device[n].startRegister, device[n].nRegisters);
			} else {
				qbuff.rException=modbus_read_holding_registers(device[n].networkAddress, device[n].startRegister, device[n].nRegisters);
			}

			/* no error, copy data to buffer to send */
			if ( 0 == qbuff.rException ) {
				/* prepare qbuff with data to live send */
				qbuff.rResultLength=modbus_rx.len-1;
				/* copy Modbus results to live buffer */
				memcpy(&qbuff.rResult,&modbus_rx.data[1],modbus_rx.len-1); 

				live_send();
			}
		} else if ( DEV_TYPE_MODBUS_1==device[n].type || DEV_TYPE_MODBUS_2==device[n].type ) {
			/* Modbus read coil or discrete input */
			qbuff.rException=modbus_read_coils(device[n].networkAddress, device[n].startRegister, device[n].nRegisters);

			/* no error, copy data to buffer to send */
			if ( 0 == qbuff.rException ) {
				/* prepare qbuff with data to live send */
				qbuff.rResultLength=modbus_rx.len-1;
				/* copy Modbus results to live buffer */
				memcpy(&qbuff.rResult,&modbus_rx.data[1],modbus_rx.len-1); 

				live_send();
			}
		}
	}

	measurementNumber++;
	timers.led_on_green=0;
}

void init() {
	setup_oscillator(OSC_8MHZ | OSC_INTRC); 
	setup_adc_ports(NO_ANALOGS);
	setup_wdt(WDT_ON);

	/* 
	Manually set ANCON0 to 0xff and ANCON1 to 0x1f for all digital
	Otherwise set high bit of ANCON1 for VbGen enable, then remaining bits are AN12 ... AN8
	ANCON1 AN7 ... AN0
	set bit to make input digital
	*/
	/* AN7 AN6 AN5 AN4 AN3 AN2 AN1 AN0 */
	ANCON0=0xff;
	/* VbGen x x 12 11 10 9 8 */
	ANCON1=0x1f;

	setup_ccp1(CCP_OFF);
	setup_ccp2(CCP_OFF);
	setup_ccp3(CCP_OFF);
	setup_ccp4(CCP_OFF);
	setup_ccp5(CCP_OFF);

	output_low(RS485_DE); /* shut off RS-485 transmitter */

	setup_timer_4(T4_DIV_BY_16,77,16); 
	enable_interrupts(INT_TIMER4);	

	/* global structures initialized to 0, set something else below if needed */
	timers.now_poll=1;
	timers.world_timeout=255;
	timers.factory_unlocked=0;
	timers.led_on_green=0;

	query_reset();

	/* receive data from serial ports */
	enable_interrupts(INT_RDA2);

	/* initialize MCP3208 external ADCs */
	mcp3208_init();


	/* initialize SCI UART @ 19200 */
	uart_init(6); /* 2=>57600 (tested, works) 6=>19200 */


	delay_ms(14);




}

void main(void) {
	int8 i;

	/* normal device startup */
	init();

	for ( i=0 ; i<10 ; i++ ) {
		restart_wdt();

		output_high(LED_GREEN);
		delay_ms(100);
		output_low(LED_GREEN);
		delay_ms(100);
	}

	fprintf(STREAM_WORLD,"# rdTap %s (%c%lu)\r\n",
		__DATE__,
		config.serial_prefix,
		config.serial_number
	);


	fprintf(STREAM_WORLD,"# restart cause: ");
	switch ( restart_cause ) {
		case WDT_TIMEOUT:       fprintf(STREAM_WORLD,"WDT TIMEOUT"); break;
		case MCLR_FROM_SLEEP:   fprintf(STREAM_WORLD,"MCLR FROM SLEEP"); break;
		case MCLR_FROM_RUN:     fprintf(STREAM_WORLD,"MCLR FROM RUN"); break;
		case NORMAL_POWER_UP:   fprintf(STREAM_WORLD,"NORMAL POWER UP"); break;
		case BROWNOUT_RESTART:  fprintf(STREAM_WORLD,"BROWNOUT RESTART"); break;
		case WDT_FROM_SLEEP:    fprintf(STREAM_WORLD,"WDT FROM SLEEP"); break;
		case RESET_INSTRUCTION: fprintf(STREAM_WORLD,"RESET INSTRUCTION"); break;
		default:                fprintf(STREAM_WORLD,"UNKNOWN!");
	}
	fprintf(STREAM_WORLD,"\r\n");


//	fprintf(STREAM_WORLD,"# enable_interrupts(GLOBAL)\r\n");
	enable_interrupts(GLOBAL);

//	fprintf(STREAM_WORLD,"# write default param file\r\n");
	write_default_param_file();

//	fprintf(STREAM_WORLD,"# write default device file\r\n");
	write_default_device_file();

//	fprintf(STREAM_WORLD,"# read paramaters\r\n");
	read_param_file();
	read_device_file();

//	fprintf(STREAM_WORLD,"# modbus_init()\r\n");
	modbus_init();


	devicesDump();

	fprintf(STREAM_WORLD,"# starting main() loop\r\n");

	/* main loop */
	for ( ; ; ) {
		restart_wdt();

		if ( timers.now_poll ) {
			timers.now_poll=0;
			timers.led_on_green=50;
			deviceQuery();
		}

		if ( query.buff_ready ) {
			query_process();
			query_reset();

		}

		fprintf(STREAM_WORLD,"*\r\n");
	}
}

