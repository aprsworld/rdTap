#define DEBUG_ASCII     0
#define DEBUG_SBD       0
#define MCP3208_ENABLED 0
#define STARTUP_BLINKS  150


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
	int16 sbd_cycle;
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

	/* Iridium SBD or other limited bandwidth transmission */
	/* 0 is no SBD, 1...5 is SBD modem with power switch on CONTROL pin 1...5 */
	int8  sbd_config;

	/*	
	every n'th live transmission we transmit the SBD data
	Interval should be at least 10 minutes to allow for full retry cycle	
	on cycle before transmit, we clear previous buffer and turn on modem 
	*/
	int16 sbd_every;

} struct_config;

/* global structures */
struct_device device[DEV_MAX_N];
struct_timer  timers;
struct_qbuff  qbuff;
struct_query  query;
struct_config config;

#include "i2c_access_rdTap.c"
#include "mcp3208_rdTap.c"
#include "uart_sc16is740_rdTap.c"
#include "param_rdTap.c"
#include "paramDevice_rdTap.c"
#include "modbus_int_uart_rdTap.c"
#include "iridium_sbd_rdTap.c"
#include "interrupt_rdTap.c"
#include "live_rdTap.c"
#include "queryHandler_rdTap.c"



void deviceQuery(void) {
	static int16 measurementNumber=0;
	static int8  nCycles[DEV_MAX_N];
	int8 n;
	int16 l;
#if DEBUG_ASCII
	int8 i;
#endif

	/* check if next cycle will be an SBD transmission. */
	if ( 1 == timers.sbd_cycle ) {
		/* next cycle will be an Iridium transmit cycle, so turn on modem and clear outgoing buffer */
#if DEBUG_SBD
		fprintf(STREAM_WORLD,"# deviceQuery() next cycle will be SBD transmit cycle.\r\n");
#endif
//		iridium_on();
		iridium_mo_clear();
	}

	/* check if we are going to be doing a SBD transmision */
	if ( 0 == timers.sbd_cycle ) {
#if DEBUG_SBD
		fprintf(STREAM_WORLD,"# deviceQuery() this cycle is an SBD transmit cycle.\r\n");
#endif
		/* turn on modem in case it isn't already on */
//		iridium_on();
		/* clear MO buffer */
		iridium_mo_clear();

		/* build header */
		sbd.mo_buff[sbd.mo_length++]=make8(measurementNumber,1);
		sbd.mo_buff[sbd.mo_length++]=make8(measurementNumber,0);
	}

#if DEBUG_ASCII
		fprintf(STREAM_WORLD,"# deviceQuery() querying all enabled devices:\r\n");
#endif

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



		qbuff.deviceNumber=n;
		qbuff.measurementNumber=measurementNumber;


		if ( device[n].type <= DEV_TYPE_MODBUS_MAX ) {
			/* modbus device type */
#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# deviceQuery() MODBUS device\r\n");
#endif
			/* set serial port speed */
			if ( DEV_SERIAL_19200 == device[n].serialSpeed ) {
				set_uart_speed(19200,MODBUS_SERIAL);
			} else {
				set_uart_speed(9600,MODBUS_SERIAL);
			}

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
		} else if ( device[n].type <= DEV_TYPE_I2C_MAX ) {
			/* I2C device */

#if DEBUG_ASCII
			restart_wdt();
			fprintf(STREAM_WORLD,"# deviceQuery() I2C device\r\n");
			fprintf(STREAM_WORLD,"device[%u]\r\n",n);
			fprintf(STREAM_WORLD,"\ttype=%u\r\n",device[n].type);
			fprintf(STREAM_WORLD,"\ttransmitEvery=%u\r\n",device[n].transmitEvery);
			fprintf(STREAM_WORLD,"\tnetworkAddress=0x%02x\r\n",device[n].networkAddress);
			fprintf(STREAM_WORLD,"\tserialNumber=0x%02x%02x%02x%02x\r\n",
				make8(device[n].serialNumber,3),
				make8(device[n].serialNumber,2),
				make8(device[n].serialNumber,1),
				make8(device[n].serialNumber,0)
			);
			fprintf(STREAM_WORLD,"\tstartRegister=%lu\r\n",device[n].startRegister);
			fprintf(STREAM_WORLD,"\tnRegisters=%u\r\n",device[n].nRegisters);
#endif

			if ( DEV_TYPE_I2C_READ_8 == device[n].type ) {
				/* start a read at start address then just read a byte at a time. nRegisters is bytes */
//fprintf(STREAM_WORLD,"z");
				i2c_buff_read(device[n].networkAddress, device[n].startRegister, qbuff.rResult, device[n].nRegisters);

				qbuff.rException=0;
				qbuff.rResultLength=device[n].nRegisters;
//fprintf(STREAM_WORLD,"Z");
#if DEBUG_ASCII
				/* debug dump */
				restart_wdt();
				for ( i=0 ; i<device[n].nRegisters ; i++ ) {
					fprintf(STREAM_WORLD,"# reg addr[0x%02x]=0x%02x (%u)\r\n",i+device[n].startRegister,qbuff.rResult[i],qbuff.rResult[i]);
				}
#endif

				live_send();

			}

		} else {
			/* local */
#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# deviceQuery() local (not implemented) device\r\n");
#endif
		}

		/* Add data to SBD if this is an SBD cycle and we have valid data */
		if ( 0 == timers.sbd_cycle && 0 == qbuff.rException ) {
#if DEBUG_SBD
			fprintf(STREAM_WORLD,"# deviceQuery() adding to sbd.mo_buff since qbuff.rException=0\r\n");
#endif
			/* sub-header, 2 bytes, device number, and data length */
			sbd.mo_buff[sbd.mo_length++]=n;
			sbd.mo_buff[sbd.mo_length++]=qbuff.rResultLength;
		
			/* sub-message */
			for ( l=0 ; l<qbuff.rResultLength ; l++ ) {
				sbd.mo_buff[sbd.mo_length++]=qBuff.rResult[l];
			}
		}

	}

	/* if we have data to send, then we sent it */
	if ( 0 == timers.sbd_cycle && sbd.mo_length > 0 ) {
#if DEBUG_SBD
		fprintf(STREAM_WORLD,"# deviceQuery() setting sbd.mo_state=1\r\n");
#endif
		sbd.mo_state=1;
	}

	measurementNumber++;

	if ( 0 == timers.sbd_cycle ) {
		timers.sbd_cycle=(config.sbd_every-1);
	} else {
		timers.sbd_cycle--;
	}
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
	timers.sbd_cycle=1; /* next packet will be an SBD packet */
	timers.world_timeout=255;
	timers.factory_unlocked=0;
	timers.led_on_green=0;

	query_reset();

	/* receive data from serial ports */
	enable_interrupts(INT_RDA2);

#if MCP3208_ENABLED
	/* initialize MCP3208 external ADCs */
	mcp3208_init();
#endif


	if ( config.sbd_config ) {
#if DEBUG_ASCII
		fprintf(STREAM_WORLD,"# init() initializing I2C Uart for SBD @ 19200\r\n");
#endif
		/* initialize I2C UART for Iridium @ 19200 */
		uart_init(6); /* 2=>57600 (tested, works) 6=>19200 */
	}

	delay_ms(14);
}

void main(void) {
	int8 i;

	/* normal device startup */
	init();

#if DEBUG_ASCII
	fprintf(STREAM_WORLD,"# main() startup blinking wait\r\n");
#endif
	for ( i=0 ; i<STARTUP_BLINKS ; i++ ) {
		restart_wdt();

		output_high(LED_GREEN);
		delay_ms(100);
		output_low(LED_GREEN);
		delay_ms(100);
#if DEBUG_ASCII
		fprintf(STREAM_WORLD,"# main() blink=%u\r\n",i);
#endif
	}

#if DEBUG_ASCII
	fprintf(STREAM_WORLD,"# main() startup blinking done\r\n");

	fprintf(STREAM_WORLD,"# rdTap %s %s\r\n",__DATE__,__TIME__);

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
#endif

	restart_wdt();
	enable_interrupts(GLOBAL);

	write_default_param_file();
	write_default_device_file();

	read_param_file();
	read_device_file();

	restart_wdt();
	modbus_init();

#if DEBUG_ASCII
	devicesDump();
#endif

	iridium_mr_clear();

#if DEBUG_ASCII
	fprintf(STREAM_WORLD,"# main() starting for ( ; ; ) loop\r\n");
#endif


	/* main loop */
	for ( ; ; ) {
		restart_wdt();

		if ( config.sbd_config ) {
			/* iridium enabled */

			/* act on flag set by SBDRING in UART character processor or RING ALERT line */
			if ( sbd.ring_flag ) {
#if DEBUG_SBD
				fprintf(STREAM_WORLD,"# main() sbd.ring_flag=1\r\n");
#endif
				/* our easiest way to get the ringing message is to send another message. This also
				would help capture the results from a query */
				timers.sbd_cycle=1; /* next packet will be an SBD packet */

				sbd.ring_flag=0;
			}

			if ( '\0' != sbd.sbdix_response[0] && 0 == sbd.mo_state ) {
				/* 
				if there is something in our +SBDIX response we should check if we have a message or need to 
				get a message. But wait until MO sending is done (state machine idle).
				*/

#if DEBUG_SBD
				fprintf(STREAM_WORLD,"# sbd.sbdix_response='%s'\r\n",sbd.sbdix_response);
#endif

 				iridium_sbdix_parse();

				/* clear the unparsed buffer so we don't get back here */
				sbd.sbdix_response[0]='\0';

				if ( sbd.sbdix_mt_queued > 0 ) {
					/* our easiest way to get the next message is to send another message. This also
					would help capture the results from a query */
					timers.sbd_cycle=1; /* next packet will be an SBD packet */
					
#if DEBUG_SBD
					fprintf(STREAM_WORLD,"# sbd.sbdix_mt_queued > 0 (is %u)\r\n",sbd.sbdix_mt_queued);
#endif
				}
			}

			/* read character into MR if we don't have an unprocessed message and there is a character available */
			if ( 0==sbd.mr_ready && ! sbd.mr_disable && uart_kbhit() ) {
				iridium_getc();
			}

			/* download message from SBD modem */
			if ( 1 == sbd.sbdix_mt_status ) {
#if DEBUG_SBD
				fprintf(STREAM_WORLD,"# sbd.sbdix_mt_status=1\r\n");
#endif
				iridium_mt_receive();
			}

			/* send messages to SBD modem */
			if ( 0 != sbd.mo_state && 0 == sbd.mt_state ) {
				iridium_mo_send();
			}

			if ( 1 == sbd.mt_ready ) {
#if DEBUG_SBD
				fprintf(STREAM_WORLD,"# sbd.mt_ready=1\r\n");
#endif
				/* TODO: send to message parser */
				sbd.mt_ready=0;

				/* do another SBDIX if we have sbd.mt_queued > 0 */
				if ( sbd.sbdix_mt_queued > 0 ) {
					/* our easiest way to get the next message is to send another message. This also
					would help capture the results from a query */
					timers.sbd_cycle=1; /* next packet will be an SBD packet */
				}
			} 
		}


		if ( timers.now_poll ) {
#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# main() timers.now_poll\r\n");
#endif
			timers.now_poll=0;
			timers.led_on_green=200;
			deviceQuery();

#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# main() timers.now_poll done\r\n");
#endif
		}

		/* queries are messages send to us that we respond to */
		if ( query.buff_ready ) {
#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# main() starting query_process()\r\n");
#endif

			query_process();

#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# main() starting query_reset()\r\n");
#endif
	
			query_reset();

#if DEBUG_ASCII
			fprintf(STREAM_WORLD,"# main() done with query.buff_ready()\r\n");
#endif
		}
	}
}

