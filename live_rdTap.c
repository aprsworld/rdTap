/* pass reg_crc value of 0xFFFF to reset */
int16 crc_chk_pass(int16 reg_crc, int8 *data, int8 length) {
	int8 j;
	
	while ( length-- ) {
		reg_crc ^= *data++;

		for ( j=0 ; j<8 ; j++ ) {
			if ( reg_crc & 0x01 ) {
				reg_crc=(reg_crc>>1) ^ 0xA001;
			} else {
				reg_crc=reg_crc>>1;
			}
		}	
	}
	
	return reg_crc;
}

void live_send(void) {
	int16 lCRC;
	int8 i;
	int8 buff[17];
	int16 completeLength;

	completeLength=sizeof(buff) + qbuff.rResultLength + 2;

	buff[0]='#';
	buff[1]=config.serial_prefix;
	buff[2]=make8(config.serial_number,1);
	buff[3]=make8(config.serial_number,0); 
	buff[4]=255; /* tell packet length to be read from 6 and 7 */
	buff[5]=18; /* packet type */
	buff[6]=make8(completeLength,1);
	buff[7]=make8(completeLength,0);

	buff[8]=make8(qbuff.measurementNumber,1);
	buff[9]=make8(qbuff.measurementNumber,0);

	/* device info */
	/* WorldData device type identifier (16-bit) */
	i=qbuff.deviceNumber;
	buff[10]=make8(device[i].typeWorld,1);
	buff[11]=make8(device[i].typeWorld,0);
	/* manufacturers serial number */
	buff[12]=make8(device[i].serialNumber,3);
	buff[13]=make8(device[i].serialNumber,2);
	buff[14]=make8(device[i].serialNumber,1);
	buff[15]=make8(device[i].serialNumber,0);
	/* status of the data read */
	buff[16]=qbuff.rException;

	/* compute CRC on header and result data */
	lCRC=crc_chk_pass(0xFFFF,buff+1,sizeof(buff)-1);
	lCRC=crc_chk_pass(lCRC,qbuff.rResult,qbuff.rResultLength);

	/* send buff, qbuff.rResult, CRC */
	for ( i=0 ; i<sizeof(buff) ; i++ ) {
		fputc(buff[i],world);
	}	
	for ( i=0 ; i<qbuff.rResultLength ; i++ ) {
		fputc(qbuff.rResult[i],world);
	}
	fputc(make8(lCRC,1),world);
	fputc(make8(lCRC,0),world);
}

