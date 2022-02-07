#inline
int8 xor_crc(int8 oldcrc, int8 data) {
	return oldcrc ^ data;
}

int8 EEPROMDataRead(int16 start, int8 *data, int16 count ) {
	int8 crc=0;
	int16 address=0;

	for ( address=0 ; address<count ; address++ ) {
		restart_wdt();
		*data = read_program_eeprom( start + address*2 );
		crc = xor_crc(crc,*data);
		data++;
	}
	return crc;
}

int8 EEPROMDataWrite(int16 start, int8 *data, int16 count ) {
	int8 crc=0;
	int16 address=0;

	for ( address=0 ; address<count ; address++ ) {
		restart_wdt();
		crc = xor_crc(crc,*data);
		write_program_eeprom( start + address*2, *data );
		data++;
	}

	return crc;
}

void write_param_file() {
	int8 crc;


	/* erase 512 byte chunks of flash before write */
	erase_program_eeprom(PARAM_CRC_ADDRESS);
	erase_program_eeprom(PARAM_CRC_ADDRESS+512);


	/* write the config structure */
	crc = EEPROMDataWrite(PARAM_ADDRESS,(void *)&config,sizeof(config));
	/* write the CRC that was calculated on the structure */
	write_program_eeprom(PARAM_CRC_ADDRESS,crc);
}

void write_default_param_file() {
	/* red LED for 1.5 seconds */
	timers.led_on_red=150;
//	fprintf(world,"# writing default parameters\r\n");

//	config.serial_prefix='Z';
//	config.serial_number=9875;
	config.serial_prefix='A';
	config.serial_number=4001;

//	config.live_interval=60;
	config.live_interval=10;

	/* write them so next time we use from EEPROM */
	write_param_file();

}


void read_param_file() {
	int8 crc;

	crc = EEPROMDataRead(PARAM_ADDRESS,(void *)&config, sizeof(config)); 

	if ( crc != read_program_eeprom(PARAM_CRC_ADDRESS) ) {
		write_default_param_file();
	}
}


