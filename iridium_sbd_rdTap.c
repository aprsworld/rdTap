#define SBD_DEBUG 1

typedef struct {
	/* +SBDIX data from most recent */
	int8 sbdix_mo_status;
	int8 sbdix_mo_msn;
	int8 sbdix_mt_status;
	int8 sbdix_mt_msn;
	int8 sbdix_mt_length;
	int8 sbdix_mt_queued;

	/* "Mobile Originated" message from us to network */
	int8 mo_state;
	int8 mo_buff[272];
	int16 mo_length;
	int8 mo_try;
	int8 mo_sbdix_wait;

	/* "Mobile Terminated" messages from network to us */
	int8 mt_state;
	
	/* ring alert */
	int8 ring_flag;
	int8 ring_state;

	/* modem talking to us for status, AT commands, etc */
	int8 mr_ready;          /* 1=> message ready for consumption */
	int8 mr_buff[64];
	int8 mr_length;

} struct_iridium_sbd;

struct_iridium_sbd sbd={0};


void iridium_on(void) {
	/* turn power switch on */
	switch ( config.sbd_config ) {
		case 1: output_high(CTRL_0); break;
		case 2: output_high(CTRL_1); break;
		case 3: output_high(CTRL_2); break;
		case 4: output_high(CTRL_3); break;
		case 5: output_high(CTRL_4); break;
	}
	
	/* set UART NRTS to turn on Iridium modem ON/OFF input */
	uart_write(UART_MCR, 0b00000000);

}

void iridium_off(void) {
	switch ( config.sbd_config ) {
		case 1: output_low(CTRL_0); break;
		case 2: output_low(CTRL_1); break;
		case 3: output_low(CTRL_2); break;
		case 4: output_low(CTRL_3); break;
		case 5: output_low(CTRL_4); break;
	}

	/* set UART NRTS to turn on Iridium modem ON/OFF input */
	uart_write(UART_MCR, 0b00000010);
}

void iridium_mr_clear(void) {
	sbd.mr_ready=0;
	sbd.mr_length=0;
}

void iridium_mo_clear(void) {
	sbd.mo_length=0;
	sbd.mo_state=0;
}

/* if we have characters to be received, this function reads them */
void iridium_getc(void) {
	int8 c;


	/* read character from UART */
	c = uart_getc();

	if ( '\r' == c ) {
		/* except when receiving binary data, we don't care about '\r' */
		return;
	}

	/* modem always appears to send '\r' '\n' */
	

	if ( 0 == sbd.mr_ready ) {
		/* receiving data */

		if ( '\n' == c && 0==sbd.mr_length ) {
			/* responses usually appear to be \r\nOK\r\n ... this skips the empty message */
			return;
		}

		
		/* check for 'SBDRING' message and deal with it accordingly */
		if ( 'S'==sbd.mr_buff[0] && 'B'==sbd.mr_buff[1] && 'D'==sbd.mr_buff[2] 
			&& 'R'==sbd.mr_buff[3] && 'I'==sbd.mr_buff[4] && 'N'==sbd.mr_buff[5] && 'G'==c ) {
			sbd.ring_flag=1;
			iridium_mr_clear();
			return;
		 }

		if ( '\n' == c ) {
			/* replace '\n' with '\0' */
			sbd.mr_buff[sbd.mr_length]='\0';
			
			/* mark that we have a message ready */
			sbd.mr_ready=1;

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

void iridium_ringing(void) {
	if ( 0 == sbd.ring_state ) {
		/* nothing to do */
		return;
	}else if ( 1 == sbd.ring_state ) {
		/* send 'ATE0' to turn off modem echo for subsequent commands */
		iridium_mr_clear();
		printf(uart_putc,"ATE0\r");
		sbd.ring_state++;
		/* TODO set response timeout */

	} else if ( 2 == sbd.ring_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.ring_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();				
		}	

	} else if ( 3 == sbd.ring_state ) {
		/* send 'AT&K0' to turn off flow control */
		iridium_mr_clear();
		printf(uart_putc,"AT&K0\r");
		sbd.ring_state++;
		/* TODO set response timeout */

	} else if ( 4 == sbd.ring_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.ring_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}	
	} else if ( 5 == sbd.ring_state ) {
		/* send 'AT+SBDMTA=1' to turn on ring alerts */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDMTA=1\r");
		sbd.ring_state++;
		/* TODO set response timeout */

	} else if ( 6 == sbd.ring_state ) {
		/* receive response 'OK' */
		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.ring_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}
	} else if ( 7 == sbd.ring_state ) {
		/* send 'AT+CRIS' to determine why we rang */
		iridium_mr_clear();
		printf(uart_putc,"AT+CRIS\r");
		sbd.ring_state++;
		/* TODO set response timeout */

	} else if ( 8 == sbd.ring_state ) {
		/* parse CRIS response */
	} 

}

void iridium_mo_send(void) {
	int16 l;
	int16 checksum;

	if ( 0 == sbd.mo_state ) {
		/* nothing to do */
		return;

	} else if ( 1 == sbd.mo_state ) {
		/* send 'ATE0' to turn off modem echo for subsequent commands */
		iridium_mr_clear();
		printf(uart_putc,"ATE0\r");
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 2 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
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

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}	
	} else if ( 5 == sbd.mo_state ) {
		/* send 'AT+SBDMTA=1' to turn on ring alerts */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDMTA=1\r");
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 6 == sbd.mo_state ) {
		/* receive response 'OK' */
		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}
	} else if ( 7 == sbd.mo_state ) {
		/* TODO do we want to clear buffer first? */

		/* send 'AT+SBDWB=[<SBD Message Length>]' to tell modem how long our MO */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDWB=%lu\r",sbd.mo_length);
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 8 == sbd.mo_state ) {
		/* receive response 'READY' */

		if ( 1 == sbd.mr_ready ) {
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

	} else if ( 9 == sbd.mo_state ) {
		/* send binary message and 2 byte checksum */
		iridium_mr_clear();
		checksum=0;
		/* send data bytes and update checksum */
		for ( l=0 ; l<sbd.mo_length ; l++ ) {
			uart_putc(sbd.mo_buff[l]);
			delay_us(500);
			checksum += sbd.mo_buff[l];
		}

		/* send checksum bytes */
		uart_putc(make8(checksum,1));
		uart_putc(make8(checksum,0));

		sbd.mo_state++;
		/* TODO set response timeout */
		
	} else if ( 10 == sbd.mo_state ) {
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

		if ( 1 == sbd.mr_ready ) {
			if ( '0'==sbd.mr_buff[0] ) {
				/* TODO BUG ... what about non-zero response */
				sbd.mo_state++;
			} else {
				/* didn't get '0' ... clear mr message */
				iridium_mr_clear();				
			}
		}

	} else if ( 11 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}

			iridium_mr_clear();				
		}

	} else if ( 12 == sbd.mo_state ) {
		/* send 'AT+SBDIX' to initiate Extended SBD Session */

		if ( sbd.mo_try > 0 && 0 != sbd.mo_sbdix_wait ) {
			/* external function decrements sbd.mo_sbdix_wait once per second */
			return;
		}

		iridium_mr_clear();
		printf(uart_putc,"AT+SBDIX\r");
		sbd.mo_state++;
		/* TODO set response timeout */

	} else if ( 13 == sbd.mo_state ) {
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

		+SBDIX:32,22, 2, 0, 0, 0
        012345678901234567890123
        0         1         2
		*/

		if ( 1 == sbd.mr_ready ) {
			if ( '+'==sbd.mr_buff[0] && 'S'==sbd.mr_buff[1] &&
			     'B'==sbd.mr_buff[2] && 'D'==sbd.mr_buff[3] &&
			     'I'==sbd.mr_buff[4] && 'X'==sbd.mr_buff[5] 
			) {

				/* so we got an +SBDIX response. If it is 0, 1, 2 we are okay to proceed and clear
				buffer. If it is anything else, we need to wait and try again */
				if ( ' '==sbd.mr_buff[7] && ( sbd.mr_buff[8] >= '0' && sbd.mr_buff[8] <= '2' ) ) {
					sbd.mo_state++;
					sbd.mo_try=0;
				} else {
					/* didn't get a 0 or 1 or 2 ... but still go on to next state for OK */
					sbd.mo_state++;
					sbd.mo_try++;
				}
			}

			iridium_mr_clear();				
		}
	} else if ( 14 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}
		
			iridium_mr_clear();				
		}

		if ( 0 != sbd.mo_try ) {
			/* need to try SBDIX again */
			sbd.mo_state=12;
			/* back off */
			if ( 1 == sbd.mo_try || 2 == sbd.mo_try ) {
				sbd.mo_sbdix_wait=3;
			} else if ( 3 == sbd.mo_try || 4 == sbd.mo_try ) {
				sbd.mo_sbdix_wait=21;
			} else if ( 5 == sbd.mo_try ) {
				sbd.mo_sbdix_wait=251;
			} else {
				/* give up and clear buffer */
				sbd.mo_state=15;
			}
		}
	} else if ( 15 == sbd.mo_state ) {

		/* send 'AT+SBDD=0' to clear MO buffer */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDD0\r");
		sbd.mo_state++;
		/* TODO set response timeout */
		/* TODO ... be careful about starting over without buffer being cleared */
	} else if ( 16 == sbd.mo_state ) {
		if ( 1 == sbd.mr_ready ) {
			if ( '0'==sbd.mr_buff[0] ) {
				/* TODO BUG ... what about non-zero response */
				sbd.mo_state++;
			} else {
				/* didn't get '0' ... clear mr message */
				iridium_mr_clear();				
			}
		}
	} else if ( 17 == sbd.mo_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}
		
			iridium_mr_clear();				
		}

	} else if ( 18 == sbd.mo_state ) {
		/* done sending */
		/* download MT if needed? */
		/* turn off modem */
		iridium_off();

		/* go back to waiting */
		iridium_mr_clear();
		iridium_mo_clear();	
	}
}

