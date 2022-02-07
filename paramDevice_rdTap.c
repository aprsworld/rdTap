void write_device_file() {
	int8 crc;

	/* erase 512 byte chunks of flash before write */
	erase_program_eeprom(DEVICE_CRC_ADDRESS);
	erase_program_eeprom(DEVICE_CRC_ADDRESS+512);


	/* write the config structure */
	crc = EEPROMDataWrite(DEVICE_ADDRESS,(void *)&device,sizeof(device));
	/* write the CRC that was calculated on the structure */
	write_program_eeprom(DEVICE_CRC_ADDRESS,crc);

//	fprintf(world,"# write_device_file crc=%u\r\n",crc);
}

/* load device configuration from program memory */
void write_default_device_file(void) {
	int8 i;

	/* red LED for 1.5 seconds */
	timers.led_on_red=150;

	/* disable all devices */
	for ( i=0 ; i<DEV_MAX_N ; i++ ) {
		device[i].type=DEV_TYPE_DISABLED;	
	}

//	fprintf(world,"# writing default devices\r\n");


//	fprintf(world,"# writing default devices for Fremont\r\n");


#if 1
/* Warner McClure - New Meter 20170517
RS485Tap
Serial: A4001

XRW2G
Serial: A3747
ID: 24

AccuEnergy AcuDC 243 
Serial: DF14050358
ID: 100 
*/

	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 3747; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* AcuDC 243-60V-A1-C Basic Registers */
	device[1].type=DEV_TYPE_MODBUS_3;
	device[1].typeWorld=DEVICE_TYPE_WORLD_RSDATA_FLOAT_LITTLE;
	device[1].transmitEvery=1;
	device[1].networkAddress=100;
	device[1].serialNumber=0; 
	device[1].startRegister=512;
	device[1].nRegisters=10;

	/* AcuDC 243-60V-A1-C Energy Registers */
	device[2].type=DEV_TYPE_MODBUS_3;
	device[2].typeWorld=DEVICE_TYPE_WORLD_RSDATA;
	device[2].transmitEvery=1;
	device[2].networkAddress=100;
	device[2].serialNumber=1;
	device[2].startRegister=768;
	device[2].nRegisters=8;

#endif

#if 0
/*
JRC New System
RS485Tap
Serial: A4750

XRW2G
Serial: A5137
ID: 24

Wattnode
Serial: 402031
ID: 1 
*/
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=402031;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* XRW2G modbus all data (no status) */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=24;
		device[1].serialNumber=4259840 + 5137; /* ord('A')<<16 (=4259840) + serial */
		device[1].startRegister=0;
		device[1].nRegisters=46;
#endif


#if 0
/*
Kean Wind
RS485Tap
Serial: A4707

XRW2G
Serial: A4708
ID: 24

Wattnode
Serial: 241831
ID: 1 
*/
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=241831;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* XRW2G modbus all data (no status) */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=24;
		device[1].serialNumber=4259840 + 4708; /* ord('A')<<16 (=4259840) + serial */
		device[1].startRegister=0;
		device[1].nRegisters=46;
#endif

#if 0
		/* Lucon */
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=149767;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* XRW2G modbus all data (no status) */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=24;
		device[1].serialNumber=4259840 + 4689; /* ord('A')<<16 (=4259840) + serial */
		device[1].startRegister=0;
		device[1].nRegisters=46;
#endif

#if 0
	/* bearing glacier camp */
	/* TS Dump */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].serialSpeed=DEV_SERIAL_9600;
	device[0].typeWorld=DEVICE_TYPE_WORLD_TS_KWH;
	device[0].transmitEvery=1;
	device[0].networkAddress=1;
	device[0].serialNumber=6113282;
	device[0].startRegister=57388;
	device[0].nRegisters=3;

	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].serialSpeed=DEV_SERIAL_9600;
	device[1].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=6113282;
	device[1].startRegister=8;
	device[1].nRegisters=21;

#endif


#if 0
	/* Castle Danger */
	/* Adam-4117 Modbus analog current values */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].serialSpeed=DEV_SERIAL_9600;
	device[0].typeWorld=ADAM_4XXX_ANALOG_CURRENT;
	device[0].transmitEvery=1;
	device[0].networkAddress=50;
	device[0].serialNumber=0;
	device[0].startRegister=0;
	device[0].nRegisters=8;

	/* TS MPPT Solar - calibration */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].serialSpeed=DEV_SERIAL_9600;
	device[1].typeWorld=DEVICE_TYPE_WORLD_TSMPPT_SCALING ;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=12320866;
	device[1].startRegister=0;
	device[1].nRegisters=4;

	/* TS MPPT Solar - data */
	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].serialSpeed=DEV_SERIAL_9600;
	device[2].typeWorld=DEVICE_TYPE_WORLD_TSMPPT_FILTERED_ADC;
	device[2].transmitEvery=1;
	device[2].networkAddress=1;
	device[2].serialNumber=12320866;
	device[2].startRegister=24;
	device[2].nRegisters=38;

	/* Midnite Classic Solar */
	device[3].type=DEV_TYPE_MODBUS_3;
	device[3].serialSpeed=DEV_SERIAL_19200;
	device[3].typeWorld=DEVICE_TYPE_WORLD_CLASSIC_BASIC;
	device[3].transmitEvery=1;
	device[3].networkAddress=10;
	device[3].serialNumber=0;
	device[3].startRegister=4113;
	device[3].nRegisters=31;

	/* Classic reason for resting */
	device[4].type=DEV_TYPE_MODBUS_3;
	device[4].serialSpeed=DEV_SERIAL_19200;
	device[4].typeWorld=DEVICE_TYPE_WORLD_CLASSIC_RESTING;
	device[4].transmitEvery=1;
	device[4].networkAddress=10;
	device[4].serialNumber=0;
	device[4].startRegister=4274;
	device[4].nRegisters=3;

	/* TS Dump */
	device[5].type=DEV_TYPE_MODBUS_4;
	device[5].serialSpeed=DEV_SERIAL_9600;
	device[5].typeWorld=DEVICE_TYPE_WORLD_TS_KWH;
	device[5].transmitEvery=1;
	device[5].networkAddress=2;
	device[5].serialNumber=9020293;
	device[5].startRegister=57388;
	device[5].nRegisters=3;

	device[6].type=DEV_TYPE_MODBUS_4;
	device[6].serialSpeed=DEV_SERIAL_9600;
	device[6].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[6].transmitEvery=1;
	device[6].networkAddress=2;
	device[6].serialNumber=9020293;
	device[6].startRegister=8;
	device[6].nRegisters=21;

	/* watt node modbus basic floating point */
	device[7].type=DEV_TYPE_MODBUS_4;
	device[7].serialSpeed=DEV_SERIAL_9600;
	device[7].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[7].transmitEvery=1;
	device[7].networkAddress=4;
	device[7].serialNumber=27401;
	device[7].startRegister=1000;
	device[7].nRegisters=34;

	/* watt node modbus energy floating point */
	device[8].type=DEV_TYPE_MODBUS_4;
	device[8].serialSpeed=DEV_SERIAL_9600;
	device[8].typeWorld=DEVICE_TYPE_WORLD_WNC_ADVANCED_ENERGY;
	device[8].transmitEvery=1;
	device[8].networkAddress=4;
	device[8].serialNumber=27401;
	device[8].startRegister=1100;
	device[8].nRegisters=38;

	/* XRW2G modbus all data (no status) */
	device[9].type=DEV_TYPE_MODBUS_4;
	device[9].serialSpeed=DEV_SERIAL_9600;
	device[9].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[9].transmitEvery=1;
	device[9].networkAddress=24;
	device[9].serialNumber=4259840 + 4048; /* ord('A')<<16 (=4259840) + serial */
	device[9].startRegister=0;
	device[9].nRegisters=46;


#endif

#if 0
		/* Wattnode Test */
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=104238;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

#endif

#if 0
		/* EDGE */
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=186964;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* XRW2G modbus all data (no status) */
		device[2].type=DEV_TYPE_MODBUS_4;
		device[2].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[2].transmitEvery=1;
		device[2].networkAddress=24;
		device[2].serialNumber=4259840 + 4080; /* ord('A')<<16 (=4259840) + serial */
		device[2].startRegister=0;
		device[2].nRegisters=46;
#endif

#if 0
		/* Peter Carse */
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=134078;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* watt node modbus basic floating point */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=2;
		device[1].serialNumber=134082;
		device[1].startRegister=1000;
		device[1].nRegisters=34;

		/* XRW2G modbus all data (no status) */
		device[2].type=DEV_TYPE_MODBUS_4;
		device[2].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[2].transmitEvery=1;
		device[2].networkAddress=24;
		device[2].serialNumber=4259840 + 4252; /* ord('A')<<16 (=4259840) + serial */
		device[2].startRegister=0;
		device[2].nRegisters=46;
#endif

#if 0
		/* AVIS VI */
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=67337;
		device[0].startRegister=1000;
		device[0].nRegisters=34;

		/* watt node modbus energy floating point */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_ADVANCED_ENERGY;
		device[1].transmitEvery=1;
		device[1].networkAddress=1;
		device[1].serialNumber=67337;
		device[1].startRegister=1100;
		device[1].nRegisters=38;


		/* XRW2G modbus all data (no status) */
		device[2].type=DEV_TYPE_MODBUS_4;
		device[2].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[2].transmitEvery=1;
		device[2].networkAddress=24;
		device[2].serialNumber=4259840 + 4048; /* ord('A')<<16 (=4259840) + serial */
		device[2].startRegister=0;
		device[2].nRegisters=46;
#endif

#if 0
		/* Veris E51C3 Meter */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_VERISE50_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=0x4E36C14C;
		device[0].startRegister=256;
		device[0].nRegisters=54;
#endif

#if 0
	/* Naval Support Activity */
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 4058; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* PS2Tap */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_PS2TAP_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=27;
	device[1].serialNumber=4259840 + 4037; /* ord('A')<<16 (=4259840) + serial */
	device[1].startRegister=0;
	device[1].nRegisters=19;

#endif

#if 0
	/* Ferrisburgh School - Upgraded*/
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 2660; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* watt node modbus basic floating point - bergey */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=40802;
	device[1].startRegister=1000;
	device[1].nRegisters=34;

	/* watt node modbus basic floating point - PV*/
	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[2].transmitEvery=1;
	device[2].networkAddress=2;
	device[2].serialNumber=52257;
	device[2].startRegister=1000;
	device[2].nRegisters=34;

#endif


#if 0
	/* Warner McClure */
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 3747; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* M850MPDV19PE */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[1].transmitEvery=1;
	device[1].networkAddress=100;
	device[1].serialNumber=35383; 
	device[1].startRegister=0;
	device[1].nRegisters=20;

#endif



#if 0
	/* Cuttler Cranberry / SGES - 600 VAC w/ 240 VAC step-down transformer*/
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 3470; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* watt node modbus basic floating point */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=65201;
	device[1].startRegister=1000;
	device[1].nRegisters=34;
#endif


#if 0
	/* Cuttler Cranberry / SGES - 480 VAC three phase*/
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 3455; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* watt node modbus basic floating point */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=67901;
	device[1].startRegister=1000;
	device[1].nRegisters=34;
#endif

#if 0
	/* VP supply */
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=4259840 + 3457; /* ord('A')<<16 (=4259840) + serial */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* watt node modbus basic floating point */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=65211;
	device[1].startRegister=1000;
	device[1].nRegisters=34;

	/* watt node modbus basic floating point */
	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[2].transmitEvery=1;
	device[2].networkAddress=2;
	device[2].serialNumber=65220;
	device[2].startRegister=1000;
	device[2].nRegisters=34;

	/* modbus shunt monitor, all data */
	device[3].type=DEV_TYPE_MODBUS_4;
	device[3].typeWorld=DEVICE_TYPE_WORLD_SHUNTMONITOR_BASIC;
	device[3].transmitEvery=1;
	device[3].networkAddress=30;
	device[3].serialNumber=4259840 + 3509; /* ord('A')<<16 (=4259840) + serial */
	device[3].startRegister=0;
	device[3].nRegisters=32;

	/* M850MPDV19PE */
	device[4].type=DEV_TYPE_MODBUS_4;
	device[4].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[4].transmitEvery=1;
	device[4].networkAddress=100;
	device[4].serialNumber=100; 
	device[4].startRegister=0;
	device[4].nRegisters=20;

#endif

#if 0		
		/* XRW2G modbus all data (no status) */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=24;
		device[1].serialNumber=4259840 + 3044; /* ord('A')<<16 (=4259840) + serial */
		device[1].startRegister=0;
		device[1].nRegisters=46;
#endif


#if 0
	/* watt node modbus basic floating point */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=4;
	device[0].serialNumber=27401;
	device[0].startRegister=1000;
	device[0].nRegisters=34;

	/* relay driver  */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_RD1_RAM;
	device[1].transmitEvery=1;
	device[1].networkAddress=5;
	device[1].serialNumber=9070114;
	device[1].startRegister=0;
	device[1].nRegisters=18;

	device[2].type=DEV_TYPE_MODBUS_1;
	device[2].typeWorld=DEVICE_TYPE_WORLD_RD1_COILS;
	device[2].transmitEvery=1;
	device[2].networkAddress=9;
	device[2].serialNumber=9070114;
	device[2].startRegister=0;
	device[2].nRegisters=4;

	/* XRW2G modbus all data (no status) */
	device[3].type=DEV_TYPE_MODBUS_4;
	device[3].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[3].transmitEvery=1;
	device[3].networkAddress=24;
	device[3].serialNumber=('A'<<16) + 2281;
	device[3].startRegister=0;
	device[3].nRegisters=46;

	/* tristar hot water */
	device[4].type=DEV_TYPE_MODBUS_4;
	device[4].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[4].transmitEvery=1;
	device[4].networkAddress=2;
	device[4].serialNumber=9020293;
	device[4].startRegister=8;
	device[4].nRegisters=21;
#endif

#if 0
	} else {
		/* watt node modbus basic floating point */
		device[0].type=DEV_TYPE_MODBUS_4;
		device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
		device[0].transmitEvery=1;
		device[0].networkAddress=1;
		device[0].serialNumber=40802;
		device[0].startRegister=1000;
		device[0].nRegisters=34;
		
		/* XRW2G modbus all data (no status) */
		device[1].type=DEV_TYPE_MODBUS_4;
		device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
		device[1].transmitEvery=1;
		device[1].networkAddress=24;
		device[1].serialNumber=0x410a64; /* A2660 ord('A')<<16 + 2660 */
		device[1].startRegister=0;
		device[1].nRegisters=46;
	}
#endif

#if 0
	/* outback systems */
	/* watt node modbus basic floating point */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=1;
	device[0].serialNumber=0;
	device[0].startRegister=1000;
	device[0].nRegisters=34;
	
	/* Adam-4117 Modbus analog current values */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=ADAM_4XXX_ANALOG_CURRENT;
	device[1].transmitEvery=1;
	device[1].networkAddress=2;
	device[1].serialNumber=0;
	device[1].startRegister=0;
	device[1].nRegisters=8;
#endif



#if 0
	/* alpha */
	/* Adam-4117 Modbus analog current values */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=ADAM_4XXX_ANALOG_CURRENT;
	device[0].transmitEvery=1;
	device[0].networkAddress=2;
	device[0].serialNumber=0;
	device[0].startRegister=0;
	device[0].nRegisters=8;

	/* XRW2G modbus all data (no status) */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=24;
	device[1].serialNumber=0x410a66; /* A2660 ord('A')<<16 + 2662 */
	device[1].startRegister=0;
	device[1].nRegisters=46;

		
	/* M850MPDV19PE */
	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[2].transmitEvery=1;
	device[2].networkAddress=100;
	device[2].serialNumber=100; 
	device[2].startRegister=0;
	device[2].nRegisters=20;

	/* M850MPDV19PE */
	device[3].type=DEV_TYPE_MODBUS_4;
	device[3].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[3].transmitEvery=1;
	device[3].networkAddress=101;
	device[3].serialNumber=101; 
	device[3].startRegister=0;
	device[3].nRegisters=20;

	/* M850MPDV19PE */
	device[4].type=DEV_TYPE_MODBUS_4;
	device[4].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[4].transmitEvery=1;
	device[4].networkAddress=102;
	device[4].serialNumber=102; 
	device[4].startRegister=0;
	device[4].nRegisters=20;

	/* M850MPDV19PE */
	device[5].type=DEV_TYPE_MODBUS_4;
	device[5].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[5].transmitEvery=1;
	device[5].networkAddress=103;
	device[5].serialNumber=103; 
	device[5].startRegister=0;
	device[5].nRegisters=20;

	/* M850MPDV19PE */
	device[6].type=DEV_TYPE_MODBUS_4;
	device[6].typeWorld=DEVICE_TYPE_WORLD_M850MPD_CUSTOM_1;
	device[6].transmitEvery=1;
	device[6].networkAddress=104;
	device[6].serialNumber=104; 
	device[6].startRegister=0;
	device[6].nRegisters=20;


	/* tristar A */
	device[7].type=DEV_TYPE_MODBUS_4;
	device[7].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[7].transmitEvery=1;
	device[7].networkAddress=120;
	device[7].serialNumber=11040024;
	device[7].startRegister=8;
	device[7].nRegisters=21;

	/* tristar B */
	device[8].type=DEV_TYPE_MODBUS_4;
	device[8].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[8].transmitEvery=1;
	device[8].networkAddress=121;
	device[8].serialNumber=11040021;
	device[8].startRegister=8;
	device[8].nRegisters=21;

	/* tristar C */
	device[9].type=DEV_TYPE_MODBUS_4;
	device[9].typeWorld=DEVICE_TYPE_WORLD_TS_RAM;
	device[9].transmitEvery=1;
	device[9].networkAddress=122;
	device[9].serialNumber=11040022;
	device[9].startRegister=8;
	device[9].nRegisters=21;



#endif

#if 0
	/* JRC */
	/* watt node modbus basic floating point */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=1;
	device[0].serialNumber=42187;
	device[0].startRegister=1000;
	device[0].nRegisters=34;

	/* XRW2G modbus all data (no status) */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=24;
	device[1].serialNumber=0x410a62; /* A2662 ord('A')<<16 + 2662 */
	device[1].startRegister=0;
	device[1].nRegisters=46;
#endif

#if 0
	/* Anderson Electric / Naval Academy */
	/* XRW2G modbus all data (no status) */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=24;
	device[0].serialNumber=0x410ae0; /* A2662 ord('A')<<16 + 2662 */
	device[0].startRegister=0;
	device[0].nRegisters=46;

	/* Satcon 50KW inverter ... whole bunch of stuff */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_0;
	device[1].transmitEvery=1;
	device[1].networkAddress=1;
	device[1].serialNumber=0; 
	device[1].startRegister=9;
	device[1].nRegisters=40;

	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_1;
	device[2].transmitEvery=1;
	device[2].networkAddress=1;
	device[2].serialNumber=0; 
	device[2].startRegister=50;
	device[2].nRegisters=40;

	device[3].type=DEV_TYPE_MODBUS_4;
	device[3].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_2;
	device[3].transmitEvery=1;
	device[3].networkAddress=1;
	device[3].serialNumber=0; 
	device[3].startRegister=90;
	device[3].nRegisters=40;

	device[4].type=DEV_TYPE_MODBUS_4;
	device[4].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_3;
	device[4].transmitEvery=1;
	device[4].networkAddress=1;
	device[4].serialNumber=0; 
	device[4].startRegister=131;
	device[4].nRegisters=8;

	device[5].type=DEV_TYPE_MODBUS_4;
	device[5].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_4;
	device[5].transmitEvery=1;
	device[5].networkAddress=1;
	device[5].serialNumber=0; 
	device[5].startRegister=175;
	device[5].nRegisters=2;

	device[6].type=DEV_TYPE_MODBUS_4;
	device[6].typeWorld=DEVICE_TYPE_WORLD_SATCON_PVS50_5;
	device[6].transmitEvery=1;
	device[6].networkAddress=1;
	device[6].serialNumber=0; 
	device[6].startRegister=275;
	device[6].nRegisters=6;
#endif

#if 0
	/* Wausau East High School / SGES - 3x 480 VAC three phase*/
	/* watt node modbus basic floating point */
	device[0].type=DEV_TYPE_MODBUS_4;
	device[0].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[0].transmitEvery=1;
	device[0].networkAddress=1;
	device[0].serialNumber=78529;
	device[0].startRegister=1000;
	device[0].nRegisters=34;

	/* watt node modbus basic floating point */
	device[1].type=DEV_TYPE_MODBUS_4;
	device[1].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[1].transmitEvery=1;
	device[1].networkAddress=2;
	device[1].serialNumber=78536;
	device[1].startRegister=1000;
	device[1].nRegisters=34;

	/* watt node modbus basic floating point */
	device[2].type=DEV_TYPE_MODBUS_4;
	device[2].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[2].transmitEvery=1;
	device[2].networkAddress=3;
	device[2].serialNumber=80272;
	device[2].startRegister=1000;
	device[2].nRegisters=34;

	/* dummy watt nodee to get a delay */
	device[3].type=DEV_TYPE_MODBUS_4;
	device[3].typeWorld=DEVICE_TYPE_WORLD_WNC_BASIC;
	device[3].transmitEvery=1;
	device[3].networkAddress=4;
	device[3].serialNumber=0;
	device[3].startRegister=0;
	device[3].nRegisters=1;

	/* XRW2G modbus all data (no status) */
	device[4].type=DEV_TYPE_MODBUS_4;
	device[4].typeWorld=DEVICE_TYPE_WORLD_XRW2G_BASIC;
	device[4].transmitEvery=1;
	device[4].networkAddress=24;
	device[4].serialNumber=4259840 + 4012; /* ord('A')<<16 (=4259840) + serial */
	device[4].startRegister=0;
	device[4].nRegisters=46;
#endif

	write_device_file();
}

void read_device_file() {
	int8 crc;

	crc = EEPROMDataRead(DEVICE_ADDRESS,(void *)&device, sizeof(device)); 

//	fprintf(world,"# read_device_file crc=%u\r\n",crc);

	if ( crc != read_program_eeprom(DEVICE_CRC_ADDRESS) ) {
		write_default_device_file();
	}
}

void devicesDump(void) {
	int8 i;

	fprintf(world,"# dumping all enabled devices:\r\n");
	for ( i=0 ; i<16 ; i++ ) {
		restart_wdt();

		/* skip disabled devices */
		if ( DEV_TYPE_DISABLED == device[i].type ) 
			continue;

		fprintf(world,"device[%u]\r\n",i);
		fprintf(world,"\ttype=%u\r\n",device[i].type);
		fprintf(world,"\ttransmitEvery=%u\r\n",device[i].transmitEvery);
		fprintf(world,"\tnetworkAddress=%u\r\n",device[i].networkAddress);
		fprintf(world,"\tserialNumber=0x%02x%02x%02x%02x\r\n",
			make8(device[i].serialNumber,3),
			make8(device[i].serialNumber,2),
			make8(device[i].serialNumber,1),
			make8(device[i].serialNumber,0)
		);
		fprintf(world,"\tstartRegister=%lu\r\n",device[i].startRegister);
		fprintf(world,"\tnRegisters=%u\r\n",device[i].nRegisters);
	}
	fprintf(world,"# done\r\n");
}
