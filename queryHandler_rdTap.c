/* this will do an arbitrary query to some other device on the network */
void query_other(void) {
	int8 parseType;

//	fprintf(world,"# query_other()\r\n");
//	fprintf(world,"# query function=%lu\r\n",query.function);

	switch ( query.function ) {
		case DEV_TYPE_DISABLED:
			return;
		case DEV_TYPE_MODBUS_1: /* not very well tested */
			query.resultException=modbus_read_coils(query.network_address, query.start_address, query.n_words);
			parseType=1;
			break;
		case DEV_TYPE_MODBUS_2: /* not very well tested */
			query.resultException=modbus_read_discrete_input(query.network_address, query.start_address, query.n_words);
			parseType=1;
			break;
		case DEV_TYPE_MODBUS_3: /* tested */
			query.resultException=modbus_read_holding_registers(query.network_address, query.start_address, query.n_words);
			parseType=1;
			break;
		case DEV_TYPE_MODBUS_4: /* tested */
			query.resultException=modbus_read_input_registers(query.network_address, query.start_address, query.n_words);
			parseType=1;
			break;
		case DEV_TYPE_MODBUS_5: /* tested */
			query.resultException=modbus_write_single_coil(query.network_address, query.start_address, query.buff[query.data_start_offset]);
			parseType=2;
			break;
		case DEV_TYPE_MODBUS_6: /* tested */
			query.resultException=modbus_write_single_register(query.network_address, query.start_address, 
				make16(query.buff[query.data_start_offset],query.buff[query.data_start_offset+1]));
			parseType=2;
			return;
		case DEV_TYPE_MODBUS_16: /* tested ... endian issues lead to having to send data in a (backwards?) order */
			query.resultException=modbus_write_multiple_registers_flip(query.network_address, query.start_address, 
				query.n_words,&query.buff+query.data_start_offset
			);

			parseType=2;
			return;
	}	

	/* Modbus query performed above. */
	/* no error, copy data to buffer to send response */
	if ( 1==parseType && 0==query.resultException ) {
		query.resultLength=modbus_rx.len-1;
		memcpy(&query.buff,&modbus_rx.data[1],modbus_rx.len-1); 
	} else if ( 2==parseType && 0==query.resultException ) {
		/* return the value of the coil we wrote */
		query.resultLength=2;
		memcpy(&query.buff,&modbus_rx.data[3],2); 
	}
}


/* 
try to write the specified register
if successful, return 0, otherwise return a modbus exception
*/
exception query_self_write_register(int16 address, int16 value) {
	int8 dev, offset;
	int16 last;

#if DEBUG_ASCII
	fprintf(world,"# wr address=%lu value=%lu\r\n",address,value);
#endif

	/* if we have been unlocked, then we can modify serial number */
	if ( timers.factory_unlocked ) {
		if ( 1000 == address ) {
			config.serial_prefix=value;
			return 0;
		} else if ( 1001 == address ) {
			config.serial_number=value;
			return 0;
		}
	}

	if ( address < 1000 ) {
		dev=address>>3;
		offset=address&0b111;

		if ( dev > DEV_MAX_N )
			return ILLEGAL_DATA_ADDRESS;

		switch ( offset ) {
			case 0: device[dev].type=make8(value,0); break;
			case 1: device[dev].typeWorld=value; break;
			case 2: device[dev].transmitEvery=make8(value,0); break;
			case 3: device[dev].networkAddress=make8(value,0); break;
			/* first pass get the high word of serial number */
			case 4: last=value; break;
			/* second pass, combine last value and current value */
			case 5: device[dev].serialNumber=make32(last,value);
			case 6: device[dev].startRegister=value; break;
			case 7: device[dev].nRegisters=make8(value,0); break;
		}
	} else {
		/* publicly writeable addresses */

		switch ( address ) {
			case 1100: config.live_interval=value; break;

			case 1997:
				reset_cpu();
			case 1998:
				if ( 1998 != value ) return ILLEGAL_DATA_VALUE;
				write_default_param_file();
				write_default_device_file();
#if DEBUG_ASCII
	fprintf(world,"# writing default files\r\n");
#endif
				break;
			case 1999:
				/* write config to EEPROM */
				if ( 1 != value ) return ILLEGAL_DATA_VALUE;
				write_param_file();
				write_device_file();

#if DEBUG_ASCII
	fprintf(world,"# config.live_interval=%lu\r\n"config.live_interval);
#endif

				break;

			case 19999:
				/* unlock factory programming registers when we get 1802 in passcode register */
				if ( 1802 != value ) {
					timers.factory_unlocked=0;
					return ILLEGAL_DATA_VALUE;
				}
				timers.factory_unlocked=1;
				/* green LED for 2 seconds */
				timers.led_on_green=200;
				timers.led_on_red=0;
				break;
#if DEBUG_ASCII
	fprintf(world,"# factory unlocked\r\n");
#endif

			default:
				return ILLEGAL_DATA_ADDRESS;
		}
	}

	/* must not have triggered an exception */
	return 0;
}


/* address to read, put 16-bit results in query.buff[n] and query.buff[n+1] */
exception query_self_read_register(int16 address, int8 n) {
	int16 result;
	int8 dev;
	int8 offset;


	if ( address < 1000 ) {
		dev=address>>3;
		offset=address&0b111;

		if ( dev > DEV_MAX_N )
			return ILLEGAL_DATA_ADDRESS;

		switch ( offset ) {
			case 0: result=device[dev].type; break;
			case 1: result=device[dev].typeWorld; break;
			case 2: result=device[dev].transmitEvery; break;
			case 3: result=device[dev].networkAddress; break;
			case 4: result=make16(make8(device[dev].serialNumber,3),make8(device[dev].serialNumber,2)); break;
			case 5: result=make16(make8(device[dev].serialNumber,1),make8(device[dev].serialNumber,0)); break;
			case 6: result=device[dev].startRegister; break;
			case 7: result=device[dev].nRegisters; break;
		}
	} else {
		switch ( address ) {
			case 1000: result=config.serial_prefix; break;
			case 1001: result=config.serial_number; break;
			case 1002: result='R'; break;
			case 1003: result=0; break;
			case 1004: result='4'; break;
			case 1005: result=0; break;
	
			case 1100: result=config.live_interval; break;
	
			default: return ILLEGAL_DATA_ADDRESS;
		}
	}

//	fprintf(world,"# query_self_read_register(address=%lu, n=%u) result=%lu\r\n",address,n,result);

	query.buff[n]  =make8(result,1);
	query.buff[n+1]=make8(result,0);
	query.resultLength += 2;

	return 0;
}

exception query_self_read_registers(int16 address, int8 nRegisters) {
	int8 i;
	exception e=0;

	for ( i=0 ; i<nRegisters && 0==e ; i++ ) {
		e=query_self_read_register(address+i,i*2);
	}

	return e;
}




void query_self(void) {
//	fprintf(world,"# query_self()\r\n");

	query.resultLength=0;

	switch ( query.function ) {
		case DEV_TYPE_MODBUS_3:
		case DEV_TYPE_MODBUS_4:
			/* read registers */
			query.resultException=query_self_read_registers(query.start_address, query.n_words);
			break;
		case DEV_TYPE_MODBUS_6:
			/* write single register */
			query.resultException=query_self_write_register(query.start_address, 
				make16(query.buff[query.data_start_offset],query.buff[query.data_start_offset+1]));
			break;
		case DEV_TYPE_MODBUS_16:
			/* write multiple registers */
			break;
		default:
			query.resultException=ILLEGAL_FUNCTION;
			query.resultLength=0;
	}
}

/*
Remote query (from WorldData to us) syntax
'#'                 0  STX
RECV'R ID PREFIX    1  First character (A-Z) for gateway serial number
RECV'R ID MSB       2  gateway serial number
RECV'R ID LSB       3  
PACKET LENGTH       4  
PACKET TYPE         5  type of packet we are receiving (19)
QUERY ID MSB        6  32 bit query id that we must answer with
QUERY ID            7
QUERY ID            8
QUERY ID LSB        9
QUERY DEV SN MSB    10 32 bit serial number of device to query
QUERY DEV SN        11
QUERY DEV SN        12
QUERY DEV SN LSB    13
Q NETWORK ADDR MSB  14 Network address of device to query, if SN not specified
Q NETWORK ADDR LSB  15
QUERY FUNCTION MSB  16 type of query to perform
QUERY FUNCTION LSB  17
QUERY ADDRESS MSB   18 start address for query
QUERY ADDRESS LSB   19
QUERY N WORDS       20 number of 16-bit words in query data
QUERY DATA[0] MSB   [] first word of query data
QUERY DATA[0] LSB   [+1]
CRC MSB       length-1 CRC of data
CRC LSB         length

Parse first four bytes for match to our serial number. If matched, receive rest 
of packet and do checksum.

Send query to network one of two ways:
if ( 0 != query_dev_sn ) then scan device table to determine network id
else use network id

if ( 0==network id || our_serial_number==query_dev_sn ) then use local registers
else perform network query
*/

void query_reset(void) {
	query.buff_pos=0;
	
	/* !buff_ready causes serial receive interrupt to throw out data ... so clear last */
	query.buff_ready=0;
}

/* 
response to remote query (from us to WorldData)
'#'                 0  STX
SERIAL PREFIX       1  First character (A-Z) for serial number
SERIAL MSB          2  high byte of sending station ID
SERIAL LSB          3  low byte of sending station ID
PACKET LENGTH       4  always 0xff
PACKET TYPE         5  type of packet we are sending (20)
PACKET LENGTH MSB   6  number of byte for packet including STX through CRC
PACKET LENGTH LSB   7
QUERY ID MSB        8  32 bit query id that we are answering
QUERY ID            9
QUERY ID            10
QUERY ID LSB        11
QUERY EXCEPTION     12 exception encountered in performing query
QUERY DATA[0] MSB [13] first word of query data
QUERY DATA[0] LSB [14]
CRC MSB       length-1 CRC of data
CRC LSB         length
*/
void query_response(void) {
	int8 buff[13];
	int16 l;
	int8 i;

	buff[0]='#';
	buff[1]=config.serial_prefix;
	buff[2]=make8(config.serial_number,1);
	buff[3]=make8(config.serial_number,0);
	buff[4]=0xff;
	buff[5]=20;
	/* packet length */
	l=sizeof(buff)+query.resultLength+2;
	buff[6]=make8(l,1);
	buff[7]=make8(l,0);

	buff[8]=make8(query.query_id,3);
	buff[9]=make8(query.query_id,2);
	buff[10]=make8(query.query_id,1);
	buff[11]=make8(query.query_id,0);
	buff[12]=query.resultException;

	/* compute CRC on header (skip STX) and result data */
	l=crc_chk_pass(0xFFFF,buff+1,sizeof(buff)-1);
	l=crc_chk_pass(l,query.buff,query.resultLength);

	/* send buff, result, CRC */
	for ( i=0 ; i<sizeof(buff) ; i++ ) {
		fputc(buff[i],world);
	}	
	for ( i=0 ; i<query.resultLength ; i++ ) {
		fputc(query.buff[i],world);
	}
	fputc(make8(l,1),world);
	fputc(make8(l,0),world);

#if 0
	fprintf(world,"# query result (query.resultLength=%u query.resultException=%u):\r\n",query.resultLength,query.resultException);
	/* query.resultLength is in bytes  .... result is in 16-bit words */
	for ( i=0 ; i<query.resultLength ; i+=2 ) {
		fprintf(world,"# query.buff[%u]=%lu (0x%04lX)\r\n",
			i/2,
			make16(query.buff[i],query.buff[i+1]),
			make16(query.buff[i],query.buff[i+1])
		);
		query.buff[i]=0xff;
		query.buff[i+1]=0xff;
	}
#endif
}

/* we received a query addressed to us */
void query_process(void) {
	int32 our_serial;
//	int8 i;
	int16 lCRC;

//	fprintf(world,"# in query_process()\r\n");

	query.packet_length=query.buff[4];
	query.crc=make16(query.buff[query.packet_length-2],query.buff[query.packet_length-1]);	
	lCRC = crc_chk_pass(0xFFFF,query.buff+1,query.packet_length-3);

	if ( lCRC != query.crc ) {
//		fprintf(world,"# CRC 0x%04lX != 0x%04lX (LOCAL)\r\n",query.crc,lCRC);
		return;
	}


	if ( 19 != query.buff[5] ) {
//		fprintf(world,"# not query type 19 ... don't know how to handle!\r\n");
		return;
	}

	
	query.query_id=make32(query.buff[6],query.buff[7],query.buff[8],query.buff[9]);
	query.device_serial=make32(query.buff[10],query.buff[11],query.buff[12],query.buff[13]);
	query.network_address=make16(query.buff[14],query.buff[15]);
	query.function=make16(query.buff[16],query.buff[17]);
	query.start_address=make16(query.buff[18],query.buff[19]);
	query.n_words=query.buff[20];
	query.data_start_offset=21;
	

#if 0
	fprintf(world,"# length=%u (query.buff_pos=%u)\r\n",query.packet_length,query.buff_pos);
	fprintf(world,"# query_id=0x%02X %02X %02X %02X\r\n",
		make8(query.query_id,3),
		make8(query.query_id,2),
		make8(query.query_id,1),
		make8(query.query_id,0)
	);
	fprintf(world,"# device serial=0x%02X %02X %02X %02X\r\n",
		make8(query.device_serial,3),
		make8(query.device_serial,2),
		make8(query.device_serial,1),
		make8(query.device_serial,0)
	);
	fprintf(world,"# network address=%lu\r\n",query.network_address);
	fprintf(world,"# function=%lu\r\n",query.function);
	fprintf(world,"# start address=%lu\r\n",query.start_address);
	fprintf(world,"# n_words=%u\r\n",query.n_words);

	/* n words can be the number of query words, in which case we won't have any data besides the CRC */
	for ( i=0 ; i<query.n_words && i<query.packet_length-2 ; i++ ) {
		fprintf(world,"# word[%u]=%lu\r\n",i,make16(query.buff[21+i*2],query.buff[22+i*2]));
	}

	fprintf(world,"# rCRC=%lu\r\n",query.crc);
#endif

#if 0
	fprintf(world,"# dump whole packet:\r\n");
	for ( i=0 ; i<query.packet_length ; i++ ) {
		fprintf(world,"# buff[%u]=0x%02X",i,query.buff[i]);
		if ( i==query.data_start_offset )
			fprintf(world," (start)");
		fprintf(world,"\r\n");
	}
#endif

	query.resultLength=0;
	/* check to see who handles this query */
	our_serial=make32(0,config.serial_prefix,config.serial_number);

#if DEBUG_ASCII
	fprintf(world,"#   our serial=%c%lu\r\n",config.serial_prefix,config.serial_number);
	fprintf(world,"# query serial=%c%lu\r\n",make8(query.device_serial,2),(int16) query.device_serial);
#endif

	if ( our_serial == query.device_serial ) {
		query_self();
	} else {
		query_other();
		
	}	

	query_response();

}
