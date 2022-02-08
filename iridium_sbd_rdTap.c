typedef struct {
	/* "Mobile Originated" message from us to network */
	int8 mo_state;
	int8 mo_buff[272];
	int16 mo_length;

	/* modem talking to us for status, AT commands, etc */
	/*
	mr_state:
		0 => receiving data
		1 => message received ('\r' encountered), null terminated, and ready to be processed
	*/
	int8 mr_state; 
	int8 mr_buff[64];
	int8 mr_length;

	/* "Mobile Terminated" messages from network to us */

} struct_iridium_sbd;

struct_iridium_sbd sbd={0};

/* if we have characters to be received, this function reads them */
void iridium_getc(void) {
	int8 c;

	if ( 1 == sbd.mr_state ) {
		/* we have received a message. bail out and let message be processed */
		return;
	}

	/* TODO: check for overflow? */
	
	/* read character from UART */
	c = uart_getc();

	if ( 0 == sbd.mr_state ) {
		/* receiving data */

		if ( '\n' == c ) {
			/* except when receiving binary data, we don't care about '\n' */
			return;
		}

		if ( '\r' == c ) {
			/* replace '\r' with '\0' */
			sbd.mr_buff[sbd.mr_length]='\0';
			
			/* mark that we have a message ready */
			sbd.mr_state=1;

			/* done */
			return;
		}

		/* put in response buffer if not full */
		if ( sbd.mr_length < ( sizeof(sbd.mr_buff) - 1 ) ) {
			sbd.mr_buff[sbd.mr_length]=c;
			sbd.mr_length++;
		} else {
			/* buffer is full ... do nothing and wait for message to be terminated */
		}
	}
}

void iridium_mr_clear(void) {
	sbd.mr_state=0;
	sbd.mr_length=0;
}

void iridium_send_mo(void) {
	int16 l;
	int16 checksum;

	if ( 0 == sbd.mo_state ) {
		/* nothing to do */
		return;

	} else if ( 1 == sbd.mo_state ) {
		/* send 'AT' to say hi to the modem */
		iridium_mr_clear();
		printf(uart_putc,"AT\r");
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 2 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_state ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();				
		}	

	} else if ( 3 == sbd.mo_state ) {
		/* send 'AT&K0' to turn off flow control */
		iridium_mr_clear();
		printf(uart_putc,"AT&K0\r");
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 4 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_state ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}	

	} else if ( 5 == sbd.mo_state ) {
		/* TODO do we want to clear buffer first? */

		/* send 'AT+SBDWB=[<SBD Message Length>]' to tell modem how long our MO */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDWB=%lu\r",sbd.mo_length);
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 6 == sbd.mo_state ) {
		/* receive response 'READY' */

		if ( 1 == sbd.mr_state ) {
			if ( 'R'==sbd.mr_buff[0] 
                 && 'E'==sbd.mr_buff[1] 
                 && 'A'==sbd.mr_buff[2]
			     && 'D'==sbd.mr_buff[3]
			     && 'Y'==sbd.mr_buff[4]
			    ) {
				sbd.mo_state++;
			}
		
			/* clear mr because either we got READY or we got a bad response */
			iridium_mr_clear();				
		}	

	} else if ( 7 == sbd.mo_state ) {
		/* send binary message and 2 byte checksum */
		iridium_mr_clear();
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
		/* TODO set response timeout */
		
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

		if ( 1 == sbd.mr_state ) {
			if ( '0'==sbd.mr_buff[0] ) {
				/* TODO BUG ... what about non-zero response */
				sbd.mo_state++;
			} else {
				/* didn't get '0' ... clear mr message */
				iridium_mr_clear();				
			}
		}

	} else if ( 9 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_state ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}

			iridium_mr_clear();				
		}

	} else if ( 10 == sbd.mo_state ) {
		/* send 'AT+SBDIX' to initiate Extended SBD Session */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDIX\r");
		sbd.mo_state++;
		/* TODO set response timeout */

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

		if ( 1 == sbd.mr_state ) {
			/* TODO BUG ignore for now .... if we got a response, then go on to next */
			sbd.mo_state++;
		}

	} else if ( 12 == sbd.mo_state ) {
		/* send 'AT+SBDD=0' to clear MO buffer */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDD=0\r");
		sbd.mo_state++;
		/* TODO set response timeout */
		/* TODO ... be careful about starting over without buffer being cleared */
	} else if ( 13 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_state ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}
		
			iridium_mr_clear();				
		}

	} else if ( 14 == sbd.mo_state ) {
		/* done sending */
		/* download MT if needed? */
		/* turn off modem if desired? */

		/* go back to waiting */
		iridium_mr_clear();	
		sbd.mo_state=0;
	}



}
