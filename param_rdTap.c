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
	config.serial_number=4800;
		/* 
			A4797 Fairway UAF (SBD in slot 5 closest to power connector)
			A4800 Fairway UVic (SBD in slot 1)
			A48?? Uvic Spare Fairway panel (SBD in slot 1)
		*/

	config.live_interval=60;
//	config.live_interval=10;

	config.sbd_config=1; /* in slot 1 */
	config.sbd_every=600; /* every 5 minutes */

	/* write them so next time we use from EEPROM */
	write_param_file();

}


void read_param_file() {
	int8 crc;

	crc = EEPROMDataRead(PARAM_ADDRESS,(void *)&config, sizeof(config)); 

	if ( crc != read_eeprom(PARAM_CRC_ADDRESS) ) {
		fprintf(STREAM_WORLD,"# read_param_file CRC mis-match. Writing new default!\r\n");
		write_default_param_file();
	}
}


