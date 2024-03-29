/* int_timer2 in modbus_int_uart.c */

#int_timer4
void isr_10ms(void) {
	static int8 sc=0;
	static int16 li=0;

	/* once per second */
	sc++;
	if ( 100==sc ) {
		sc=0;

		li++;
		if ( li >= config.live_interval ) {
			li=0;
			timers.now_poll=1;
		}


		if ( sbd.mo_sbdix_wait > 0 ) {
			sbd.mo_sbdix_wait--;
		}
	}

	if ( timers.world_timeout < 255 ) {
		timers.world_timeout++;
	}

	/* LEDs */
	if ( 0==timers.led_on_green ) {
		output_low(LED_GREEN);
	} else {
		output_high(LED_GREEN);
		timers.led_on_green--;
	}

}


/* int_rda in modbus_int_uart.c */

#int_rda2
void isr_world(void) {
	static int8 pre[5];



	if ( query.buff_ready ) {
		/* throw out data received while processing previous query */
		fgetc(STREAM_WORLD);
	}

	/* timeout after 100 milliseconds */
	if ( timers.world_timeout > 10 ) {
		query.buff_pos=0;
//		output_toggle(LED_RED);
	}
	timers.world_timeout=0;

	if ( 0 == query.buff_pos ) {
		pre[0]=pre[1];
		pre[1]=pre[2];
		pre[2]=pre[3];
		pre[3]=pre[4];
		pre[4]=fgetc(STREAM_WORLD);

		/* packet addressed to us */
		if ( '#'==pre[0] && config.serial_prefix==pre[1] && config.serial_number==make16(pre[2],pre[3]) ) {
			output_high(CTRL_1);
			query.buff[0]=pre[0];
			query.buff[1]=pre[1];
			query.buff[2]=pre[2];
			query.buff[3]=pre[3];
			query.buff[4]=pre[4];
			query.buff_pos=5;
		}
	} else {
		query.buff[query.buff_pos++]=fgetc(STREAM_WORLD);

		if ( query.buff_pos == query.buff[4] ) {
			output_toggle(CTRL_1);
			query.buff_ready=1;
		}
	}
}
