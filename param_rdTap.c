#inline
char xor_crc(char oldcrc, char data) {
	return oldcrc ^ data;
}

char EEPROMDataRead( int16 address, int8 *data, int16 count ) {
	char crc=0;

	while ( count-- != 0 ) {
		*data = read_eeprom( address++ );
		crc = xor_crc(crc,*data);
		data++;
	}
	return crc;
}

char EEPROMDataWrite( int16 address, int8 *data, int16 count ) {
	char crc=0;

	while ( count-- != 0 ) {
		restart_wdt();
		crc = xor_crc(crc,*data);
		write_eeprom( address++, *data++ );
	}

	return crc;
}

void write_param_file() {
	int8 crc;

	/* write the config structure */
	crc = EEPROMDataWrite(PARAM_ADDRESS,(void *)&config,sizeof(config));
	/* write the CRC was calculated on the structure */
	write_eeprom(PARAM_CRC_ADDRESS,crc);

}

void write_default_param_file() {
	/* green LED for 1.5 seconds */
	timers.led_on_green=150;
//	fprintf(world,"# writing default parameters\r\n");

	config.serial_prefix='A';
	config.serial_number=4812;
		/* 
			A4797 Fairway UAF (SBD in slot 5 closest to power connector)
			A4800 Fairway UVic (SBD in slot 1)
			A4810 Uvic Spare Fairway panel (SBD in slot 1)
			A4820 JJJ Bench (SBD in slot 1)
			A4756 A test unit (SBD in slot 1)
		*/

	/* data to internet on STREAM_WORLD port */
	config.live_interval=10; 

	/* Iridium SBD requires dcswc_module_rockblock which provides an I2C UART */
	config.sbd_config=1;   /* physical slot for module. Used for power control */
	config.sbd_every=90;   /* send SBD message every this number of live_interval */
                           /* with 10 second live interval, 4320 => SBD every 12 hours */
                           /* with 10 second live interval, 1080 => SBD every 3 hours */	
						   /* with 10 second live interval, 360  => SBD every 1 hour */			
                           /* with 10 second live interval, 90   => SBD every 15 minutes */
                           /* values less than 15 minutes don't work well with the SBD backoff algorithm */
				

	/* write them so next time we use from EEPROM */
	write_param_file();

}


void read_param_file() {
	int8 crc;

	crc = EEPROMDataRead(PARAM_ADDRESS,(void *)&config, sizeof(config)); 

	if ( crc != read_eeprom(PARAM_CRC_ADDRESS) ) {
#if DEBUG_ASCII
		fprintf(STREAM_WORLD,"# read_param_file CRC mis-match. Writing new default!\r\n");
#endif
		write_default_param_file();
	}
}


