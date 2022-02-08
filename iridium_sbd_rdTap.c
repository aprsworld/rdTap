typedef struct {
	/* "Mobile Originated" message from us to network */
	int8 mo_state;
	int8 mo_buff[272];
	int16 mo_length;

	/* modem talking to us for status, AT commands, etc */
	int8 mr_state;
	int8 mr_buff[64];
	int8 mr_length;

	/* "Mobile Terminated" messages from network to us */

} struct_iridium_sbd;

struct_iridium_sbd sbd={0};

/* if we have characters to be received, this function reads them */
void iridium_getc(void) {

}

void iridium_send_mo(void) {
	int16 l;
	int16 checksum;

	if ( 0 == sbd.mo_state ) {
		/* nothing to do */
		return;
	} else if ( 1 == sbd.mo_state ) {
		/* send 'AT' to say hi to the modem */
		printf(uart_putc,"AT\r");
		sbd.mo_state++;

	} else if ( 2 == sbd.mo_state ) {
		/* receive response 'OK' */

	} else if ( 3 == sbd.mo_state ) {
		/* send 'AT&K0' to turn off flow control */
		printf(uart_putc,"AT&K0\r");
		sbd.mo_state++;

	} else if ( 4 == sbd.mo_state ) {
		/* receive response 'OK' */

	} else if ( 5 == sbd.mo_state ) {
		/* send 'AT+SBDWB=[<SBD Message Length>]' to tell modem how long our MO */
		printf(uart_putc,"AT+SBDWB=%lu\r",sbd.mo_length);
		sbd.mo_state++;

	} else if ( 6 == sbd.mo_state ) {
		/* receive response 'READY' */

	} else if ( 7 == sbd.mo_state ) {
		/* send binary message and 2 byte checksum */

		checksum=0;
		/* send data bytes and update checksum */
		for ( l=0 ; l<sbd.mo_length ; l++ ) {
			uart_putc(sbd.mo_buff[l]);
			checksum += sbd.mo_buff[l];
		}

		/* send checksum bytes */
		uart_putc(make8(checksum,1));
		uart_putc(make8(checksum,0));

		sbd.mo_state++;
		
	} else if ( 8 == sbd.mo_state ) {
		/* 
		receive response <Status> which means 
		
		0 => SBD message successfully written to the ISU.

		1 => SBD message write timeout. An insufficient number of bytes were transferred 
		to ISU during the transfer period of 60 seconds.

		2 => SBD message checksum sent from DTE does not match the checksum calculated at 
		the ISU.

		3 => SBD message size is not correct. The maximum mobile originated SBD message 
		length is 340 bytes. The minimum mobile originated SBD message length is 1 byte.
		*/

		/* status must be 0 */

	} else if ( 9 == sbd.mo_state ) {
		/* receive response 'OK' */

	} else if ( 10 == sbd.mo_state ) {
		/* send 'AT+SBDIX' to initiate Extended SBD Session */
		printf(uart_putc,"AT+SBDIX\r");
		sbd.mo_state++;

	} else if ( 11 == sbd.mo_state ) {
		/*
		receive response:
			 +SBDIX: <MO status>, <MOMSN>, <MT status>, <MTMSN>, <MT length>, <MT queued>
		which means

		<MO status> - Any returned number with a value of 0 to 2 indicates your message 
		has been successfully transmitted. Any number above 2 indicates that the message 
		has not been successfully transmitted.

		<MOMSN> - This number denotes the MO message number and cycles between 0 and 65535.

		<MT status>
			0 => No messages waiting to be received.
			1 => New message successfully received.
			2 => Error during mailbox check / message reception.

		<MTMSN> - This number denotes the MT message number and cycles between 0 and 65535.

		<MT length> - The size (in bytes) of the MT message.

		<MT queued> - The number of MT messages in the queue waiting to be downloaded.
		*/

	} else if ( 12 == sbd.mo_state ) {
		/* send 'AT+SBDD=0' to clear MO buffer */
		printf(uart_putc,"AT+SBDD=0\r");
		sbd.mo_state++;

	} else if ( 13 == sbd.mo_state ) {
		/* receive response 'OK' */

	} else if ( 14 == sbd.mo_state ) {
		/* done sending */
		/* download MT if needed */
		/* turn on modem if desired */
	}



}

