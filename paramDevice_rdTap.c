void write_device_file() {
	int8 crc;



	/* write the config structure */
	crc = EEPROMDataWrite(DEVICE_ADDRESS,(void *)&device,sizeof(device));
	/* write the CRC that was calculated on the structure */
	write_eeprom(DEVICE_CRC_ADDRESS,crc);

//	fprintf(STREAM_WORLD,"# write_device_file crc=%u\r\n",crc);
}

/* load device configuration from program memory */
void write_default_device_file(void) {
	int8 i;

	/* green LED for 1.5 seconds */
	timers.led_on_green=150;

	fprintf(STREAM_WORLD,"# disabling all devices\r\n");

	/* disable all devices */
	for ( i=0 ; i<DEV_MAX_N ; i++ ) {
		device[i].type=DEV_TYPE_DISABLED;	
	}

	/* UAF Fairway */
	/* dcswc_module_latching_contactor */
	device[0].type=DEV_TYPE_I2C_READ_8;
	device[0].typeWorld=DEVICE_TYPE_WORLD_RSDATA;
	device[0].transmitEvery=1;
	device[0].networkAddress=0x3e;
	device[0].serialNumber=('A'<<16) + 4790; 
	device[0].startRegister=0;
	device[0].nRegisters=3*2; /* two byte registers */

	/* dcswc_module_voltage_current_counter */
	device[1].type=DEV_TYPE_I2C_READ_8;
	device[1].typeWorld=DEVICE_TYPE_WORLD_RSDATA;
	device[1].transmitEvery=1;
	device[1].networkAddress=0x36;
	device[1].serialNumber=('A'<<16) + 4791; 
	device[1].startRegister=0;
	device[1].nRegisters=16;

	/* dcswc_module_voltage_current_counter */
	device[2].type=DEV_TYPE_I2C_READ_8;
	device[2].typeWorld=DEVICE_TYPE_WORLD_RSDATA;
	device[2].transmitEvery=1;
	device[2].networkAddress=0x38;
	device[2].serialNumber=('A'<<16) + 4792; 
	device[2].startRegister=0;
	device[2].nRegisters=16;


	fprintf(STREAM_WORLD,"# write_device_file()\r\n");

	write_device_file();
}

void read_device_file() {
	int8 crc;

	crc = EEPROMDataRead(DEVICE_ADDRESS,(void *)&device, sizeof(device)); 

	fprintf(STREAM_WORLD,"# read_device_file crc=%u\r\n",crc);

	if ( crc != read_eeprom(DEVICE_CRC_ADDRESS) ) {
		fprintf(STREAM_WORLD,"# read_device_file CRC mis-match. Writing new default!\r\n");
		write_default_device_file();
	}
}

void devicesDump(void) {
	int8 i;

	fprintf(STREAM_WORLD,"# dumping all enabled devices:\r\n");
	for ( i=0 ; i<16 ; i++ ) {
		restart_wdt();

		/* skip disabled devices */
		if ( DEV_TYPE_DISABLED == device[i].type ) 
			continue;

		fprintf(STREAM_WORLD,"device[%u]\r\n",i);
		fprintf(STREAM_WORLD,"\ttype=%u\r\n",device[i].type);
		fprintf(STREAM_WORLD,"\ttransmitEvery=%u\r\n",device[i].transmitEvery);
		fprintf(STREAM_WORLD,"\tnetworkAddress=%u\r\n",device[i].networkAddress);
		fprintf(STREAM_WORLD,"\tserialNumber=0x%02x%02x%02x%02x\r\n",
			make8(device[i].serialNumber,3),
			make8(device[i].serialNumber,2),
			make8(device[i].serialNumber,1),
			make8(device[i].serialNumber,0)
		);
		fprintf(STREAM_WORLD,"\tstartRegister=%lu\r\n",device[i].startRegister);
		fprintf(STREAM_WORLD,"\tnRegisters=%u\r\n",device[i].nRegisters);
	}
	fprintf(STREAM_WORLD,"# done\r\n");
}
