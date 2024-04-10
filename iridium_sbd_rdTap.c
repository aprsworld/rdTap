#define SBD_DEBUG 1

typedef struct {
	/* +SBDIX data from most recent */
	/* result from modem */
	int8  sbdix_response[42];
	/* parsed into */
	int8  sbdix_mo_status;
	int16 sbdix_mo_msn;
	int8  sbdix_mt_status;
	int16 sbdix_mt_msn;
	int16 sbdix_mt_length;
	int8  sbdix_mt_queued;


	/* "Mobile Originated" message from us to network */
	int8  mo_state;
	int8  mo_buff[272];
	int16 mo_length;
	int8  mo_try;
	int8  mo_sbdix_wait;

	/* "Mobile Terminated" messages from network to us */
	int8  mt_ready;
	int8  mt_state;
	int8  mt_buff[256];
	int16 mt_length;
	int16 mt_checksum;
	
	/* ring alert */
	int8  ring_flag;
	int8  ring_state;

	/* modem talking to us for status, AT commands, etc */
	int8 mr_ready;          /* 1=> message ready for consumption */
	int8 mr_buff[64];
	int8 mr_length;
	int8 mr_disable;
} struct_iridium_sbd;

struct_iridium_sbd sbd={0};

int8 a_to_uint8(int8 *s) {
   int8 value=0;

   /* swallow white space before number */
   while (isspace(*s)) {
      s++;
   }

   /* read number while we have digit */
   while (isdigit(*s)) {
      value *= 10;
      value += (*s-'0');
      s++;
   }

   return value;
}

int16 a_to_uint16(int8 *s) {
   int16 value=0;

   /* swallow white space before number */
   while (isspace(*s)) {
      s++;
   }

   /* read number while we have digit */
   while (isdigit(*s)) {
      value *= 10;
      value += (*s-'0');
      s++;
   }

   return value;
}

void iridium_sbdix_parse(void) {
	int8 *p;

	/*
	 * Quick check of potential validity of response:
	 * a) valid response would need to be at least 24 characters
	 * +SBDIX: 0, 25, 0, 0, 0, 0	
	 * 012345678901234567890123456789
	 *           1         2
	 * b) mo_status always starts at character 8
	 * */
	if ( strlen(sbd.sbdix_response) < 24 || ! isdigit(sbd.sbdix_response[8] ) ) {
		/* if not a digit or too short, then we give up */
		return;
	}
	sbd.sbdix_mo_status=a_to_uint8(sbd.sbdix_response+8);

	/* mo_msn (with preceeding space) starts after comma */
	p = strchr(sbd.sbdix_response+9,',');
	if ( 0 == p ) {
		/* not found */
		return;
	}
	p++; /* swallow the ',' */
	sbd.sbdix_mo_msn=a_to_uint16(p);

	/* mt_status (with preceeding space) starts after comma */
	p = strchr(p+1,',');
	if ( 0 == p ) {
		/* not found */
		return;
	}
	p++; /* swallow the ',' */
	sbd.sbdix_mt_status=a_to_uint8(p);

	/* mt_msn (with preceeding space) starts after comma */
	p = strchr(p+1,',');
	if ( 0 == p ) {
		/* not found */
		return;
	}
	p++; /* swallow the ',' */
	sbd.sbdix_mt_msn=a_to_uint16(p);

	/* mt_length (with preceeding space) starts after comma */
	p = strchr(p+1,',');
	if ( 0 == p ) {
		/* not found */
		return;
	}
	p++; /* swallow the ',' */
	sbd.sbdix_mt_length=a_to_uint16(p);

	/* mt_queued (with preceeding space) starts after comma */
	p = strchr(p+1,',');
	if ( 0 == p ) {
		/* not found */
		return;
	}
	p++; /* swallow the ',' */
	sbd.sbdix_mt_queued=a_to_uint8(p);

#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_sbdix_parse():\r\n");
	fprintf(STREAM_WORLD,"#   sbdix_mo_status   = %d\n",sbd.sbdix_mo_status);
	fprintf(STREAM_WORLD,"#   sbdix_mo_msn      = %lu\n",sbd.sbdix_mo_msn);
	fprintf(STREAM_WORLD,"#   sbdix_mt_status   = %d\n",sbd.sbdix_mt_status);
	fprintf(STREAM_WORLD,"#   sbdix_mt_msn      = %lu\n",sbd.sbdix_mt_msn);
	fprintf(STREAM_WORLD,"#   sbdix_mt_length   = %lu\n",sbd.sbdix_mt_length);
	fprintf(STREAM_WORLD,"#   sbdix_mt_queued   = %d\n",sbd.sbdix_mt_queued);
#endif

}

#if 0
void _iridium_on(void) {
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

void _iridium_off(void) {
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
#endif

void iridium_mr_clear(void) {
	sbd.mr_ready=0;
	sbd.mr_length=0;
}

void iridium_mo_clear(void) {
	sbd.mo_length=0;
	sbd.mo_state=0;
}

void iridium_mt_clear(void) {
	sbd.mt_length=0;
	sbd.mt_state=0;
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
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_ringing() sbd.ring_state=%u\r\n",sbd.ring_state);
#endif

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
		/* parse CRIS response. But in reality, with an SBD only device it can only be SBD message that causes us to ring */
	} 

}

void iridium_mo_send(void) {
	int16 l;
	int16 checksum;

#if DEBUG_SBD
//	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u sbd.mo_try=%u sbd.mo_sbdix_wait=%u\r\n",sbd.mo_state,sbd.mo_try,sbd.mo_sbdix_wait);
#endif


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
#if DEBUG_SBD
		fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u start\r\n",sbd.mo_state);
#endif
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
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u start\r\n",sbd.mo_state);
#endif
		/*
		receive response:
			 +SBDIX: <MO status>, <MOMSN>, <MT status>, <MTMSN>, <MT length>, <MT queued>
		which means

		<MO status> - Any returned number with a value of 0 to 2 indicates your message 
		has been successfully transmitted. Any number above 2 indicates that the message 
		has not been successfully transmitted.

		Per rock7 website (https://docs.rockblock.rock7.com/reference/sbdix):
		0	MO message, if any, transferred successfully.
		1	MO message, if any, transferred successfully, but the MT message in the queue was too big to be transferred.
		2	MO message, if any, transferred successfully, but the requested Location Update was not accepted.
		3 .. 4	Reserved, but indicate MO session success if used.
		5 .. 8	Reserved, but indicate MO session failure if used.
		10	GSS reported that the call did not complete in the allowed time.
		11	MO message queue at the GSS is full.
		12	MO message has too many segments.
		13	GSS reported that the session did not complete.
		14	Invalid segment size.
		15	Access is denied.
		16	ISU has been locked and may not make SBD calls (see +CULK command).
		17	Gateway not responding (local session timeout).
		18	Connection lost (RF drop).
		19	Link failure (A protocol error caused termination of the call).
		20 .. 31	Reserved, but indicate failure if used.
		32	No network service, unable to initiate call.
		33	Antenna fault, unable to initiate call.
		34	Radio is disabled, unable to initiate call (see *Rn command).
		35	ISU is busy, unable to initiate call.
		36	Try later, must wait 3 minutes since last registration.
		37	SBD service is temporarily disabled.
		38	Try later, traffic management period (see +SBDLOE command)
		39 .. 63	Reserved, but indicate failure if used.
		64	Band violation (attempt to transmit outside permitted frequency band).
		65	PLL lock failure; hardware error during attempted transmit.

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
				/* copy +SBDIX result to seperate buffer so it can be further processed for message downloading */
				if ( '\0' == sbd.sbdix_response[0] ) {
					strncpy(sbd.sbdix_response,sbd.mr_buff,sizeof(sbd.sbdix_response)-1);
					sbd.sbdix_response[sizeof(sbd.sbdix_response)-1]='\0';
				}


				/* so we got an +SBDIX response. If first byte ([8]) is '0', '1', '2' and second byte ([9]) is ',' 
				we are okay to proceed and clear buffer. If it is anything else, we need to wait and try again */
				/* TODO: white space location / pading may not be consistent. Use atoi style parsing to determine MO_STATUS reliably */
				if ( ' '==sbd.mr_buff[7] && ( sbd.mr_buff[8] >= '0' && sbd.mr_buff[8] <= '2'  && ',' == sbd.mr_buff[9]) ) {
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
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u sbd.mo_try=%u start\r\n",sbd.mo_state,sbd.mo_try);
#endif

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
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u sbd.mo_try=%u finish\r\n",sbd.mo_state,sbd.mo_try);
#endif
	} else if ( 15 == sbd.mo_state ) {

		/* send 'AT+SBDD=0' to clear MO buffer */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDD0\r");
		sbd.mo_state++;
		/* TODO set response timeout */
		/* TODO ... be careful about starting over without buffer being cleared */
	} else if ( 16 == sbd.mo_state ) {
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u start\r\n",sbd.mo_state);
#endif
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
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u start\r\n",sbd.mo_state);
#endif

		/* receive response 'OK' */
		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mo_state++;
			}
		
			iridium_mr_clear();				
		}
	} else if ( 18 == sbd.mo_state ) {
#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mo_send() sbd.mo_state=%u start\r\n",sbd.mo_state);
#endif

		/* done sending */
		/* download MT if needed? */
		/* turn off modem */
//		iridium_off();

		/* go back to waiting */
		iridium_mr_clear();
		iridium_mo_clear();	
	}
}

void iridium_mt_receive(void) {
	int8 c;
	static int16 l;
	static int16 checksum;

#if DEBUG_SBD
	fprintf(STREAM_WORLD,"# iridium_mt_receive() sbd.mt_state=%u\r\n",sbd.mt_state);
#endif


	if ( sbd.mt_state <= 1 ) {
		/* send 'ATE0' to turn off modem echo for subsequent commands */
		iridium_mr_clear();
		printf(uart_putc,"ATE0\r");
		sbd.mt_state = 2;
		/* TODO set response timeout */

	} else if ( 2 == sbd.mt_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mt_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();				
		}	

	} else if ( 3 == sbd.mt_state ) {
		/* send 'AT&K0' to turn off flow control */
		iridium_mr_clear();
		printf(uart_putc,"AT&K0\r");
		sbd.mt_state++;
		/* TODO set response timeout */

	} else if ( 4 == sbd.mt_state ) {
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mt_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}	
	} else if ( 5 == sbd.mt_state ) {
		/* send 'AT+SBDMTA=1' to turn on ring alerts */
		iridium_mr_clear();
		printf(uart_putc,"AT+SBDMTA=1\r");
		sbd.mt_state++;
		/* TODO set response timeout */

	} else if ( 6 == sbd.mt_state ) {
		/* receive response 'OK' */
		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mt_state++;
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}
	} else if ( 7 == sbd.mt_state ) {
		/* TODO do we want to clear buffer first? */

		/* send 'AT+SBDRB' to tell modem to send us our MT */
		iridium_mr_clear();
		sbd.mr_disable=1; /* switch iridum character receiver to binary */
		printf(uart_putc,"AT+SBDRB\r");
		sbd.mt_state++;
		/* TODO set response timeout */

	} else if ( 8 == sbd.mt_state ) {
		/* receive first byte of length */
		if ( ! uart_kbhit() ) {
			/* no character available */
			return;
		}
	
		sbd.mt_length=make16(uart_getc(),0);
		sbd.mt_state++;
	} else if ( 9 == sbd.mt_state ) {
		/* receive second byte of length */
		if ( ! uart_kbhit() ) {
			/* no character available */
			return;
		}
	
		sbd.mt_length += uart_getc();
		sbd.mt_state++;

		checksum=0;
		l=0;
	} else if ( 10 == sbd.mt_state ) {
		/* receive MT length of characters and calculate checksum */
		if ( ! uart_kbhit() ) {
			/* no character available */
			return;
		}

		if ( 0 == sbd.mt_length ) {
			/* zero byte message skips this state */
			sbd.mt_state++;
			return;
		}

		c = uart_getc();
		sbd.mt_buff[l]=c;
		checksum += c;
		l++;

		if ( l == sbd.mt_length ) {
			/* received right number of characters */
			sbd.mt_state++;
		}

	} else if ( 11 == sbd.mt_state ) {
		if ( ! uart_kbhit() ) {
			/* no character available */
			return;
		}

		/* high byte of checksum */
		l=make16(uart_getc(),0);
		sbd.mt_state++;
	} else if ( 12 == sbd.mt_state ) {
		if ( ! uart_kbhit() ) {
			/* no character available */
			return;
		}

		/* low byte of checksum */
		l += uart_getc();
		sbd.mt_state++;

		sbd.mr_disable=0;
	} else if ( 13 == sbd.mt_state ) {
		/* compare local and remote checksum */
#if DEBUG_SBD
		fprintf(STREAM_WORLD,"# iridium_mt_receive() checksum l=%lu r=%lu\r\n",checksum,l);
#endif

		if ( checksum == l ) {
			/* checksums matched, we have a good message! */
			sbd.mt_ready=1;

#if DEBUG_SBD
			fprintf(STREAM_WORLD,"# iridium_mt_receive() mt_ready=1, here is our message:\r\n");
			for ( l=0 ; l<sbd.mt_length ; l++ ) {
				fprintf(STREAM_WORLD,"# mt_buff[%lu]=%c\r\n",l,sbd.mt_buff[l]);
			}
#endif
		}

		/* even if we have a bad checksum, we advance to next state and watch for OK */
		sbd.mt_state++;

	} else if ( 14 == sbd.mt_state ) {
		/* 
		Iridium reference manual says:
		"There are no response codes generated by the ISU for this command".
		But it does, in fact, give an "OK" (0x0D 0x0A 0x4F 0x4B 0x0D 0x0A) 
		about 2.7ms after finishing send the SBD data 
		*/
		/* receive response 'OK' */

		if ( 1 == sbd.mr_ready ) {
			if ( 'O'==sbd.mr_buff[0] && 'K'==sbd.mr_buff[1] ) {
				sbd.mt_state++;

#if DEBUG_SBD
				fprintf(STREAM_WORLD,"# iridium_mt_receive() got final OK\r\n");
#endif
			} 
			/* clear mr because either we got OK or we got a bad response */
			iridium_mr_clear();	
		}	
	} else if ( 15 == sbd.mt_state ) {
		/* message has processed */
		sbd.sbdix_mt_status=0;
		sbd.mt_state=0;
	}
}



#if 0
			/* if not using RING ALERT through the UART, we can do this */
			/* check if RING ALERT is active via the !CTS pin connected to RING ALERT line on the SBD modem */
			if ( bit_test(uart_read(UART_MSR),4) ) {
				sbd.ring_flag=1;
			}
#endif