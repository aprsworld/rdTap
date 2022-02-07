void write_device_file() {
	int8 crc;



	/* write the config structure */
	crc = EEPROMDataWrite(DEVICE_ADDRESS,(void *)&device,sizeof(device));
	/* write the CRC that was calculated on the structure */
	write_program_eeprom(DEVICE_CRC_ADDRESS,crc);

//	fprintf(world,"# write_device_file crc=%u\r\n",crc);
}

/* load device configuration from program memory */
void write_default_device_file(void) {
	int8 i;

	/* green LED for 1.5 seconds */
	timers.led_on_green=150;

	/* disable all devices */
	for ( i=0 ; i<DEV_MAX_N ; i++ ) {
		device[i].type=DEV_TYPE_DISABLED;	
	}

//	fprintf(world,"# writing default devices\r\n");


//	fprintf(world,"# writing default devices for Fremont\r\n");


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
