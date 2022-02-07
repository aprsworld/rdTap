#define DEBUG_ASCII 0

#include "rsTap.h"
#include "worldDeviceTypes.h"

typedef struct {
	int8  type;
	int16 typeWorld;      /* see worldDeviceTypes.h for defines */
	int8  transmitEvery;
	int8  networkAddress;
	int32 serialNumber;
	int16 startRegister;
	int8  nRegisters;
	int8  serialSpeed;   /* see rsTap.h for defines */
} struct_device;


typedef struct {
	int8  now_poll;
	int8  world_timeout;
	int8  factory_unlocked;

	int8  led_on_green;
	int8  led_on_red;
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

#include "param.c"
#include "paramDevice.c"
#include "modbus_int_uart.c"
#include "interrupt.c"
#include "live.c"
#include "queryHandler.c"



void init() {
	setup_oscillator(OSC_8MHZ | OSC_INTRC);
	setup_adc(ADC_OFF);
	setup_adc_ports(NO_ANALOGS);

//	setup_timer_2(T2_DIV_BY_4,49,1); // set 1 millisecond period with 8 MHz oscillator

	setup_timer_4(T4_DIV_BY_16,77,16); 
	enable_interrupts(INT_TIMER4);	

	/* global structures */
	timers.now_poll=1;
	timers.world_timeout=255;
	timers.factory_unlocked=0;
	timers.led_on_green=0;
	timers.led_on_red=0;

	query_reset();

	/* receive data from serial ports */
	enable_interrupts(INT_RDA2);


}





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


/* this is started after the bootloader is done loading or times out */
void main(void) {
	/* normal device startup */
	init();
	enable_interrupts(GLOBAL);


	write_default_param_file();
	write_default_device_file();

	read_param_file();
	read_device_file();

	fprintf(world,"# rsTap.c (planetPrecision) %s (%c%lu)\r\n",__DATE__,config.serial_prefix,config.serial_number);

	modbus_init();


	/* blink LED's quickly */
	timers.led_on_red=10;
	delay_ms(100);
	timers.led_on_green=10;
	delay_ms(100);
	timers.led_on_red=0;
	timers.led_on_green=0;

	output_low(TP2);
	output_high(TP3);

	/* main loop */
	for ( ; ; ) {
		restart_wdt();

		if ( timers.now_poll ) {
			timers.now_poll=0;
			deviceQuery();
		}

		if ( query.buff_ready ) {
			query_process();
			query_reset();

		}
	}
}

