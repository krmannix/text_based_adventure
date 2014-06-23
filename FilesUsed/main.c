#include "msp430g2553.h"

 /* declarations of functions defined later */
 void init_wdt(void);
#define ADC_INPUT_BIT_MASK 0x10
#define ADC_INCH INCH_4
#define ADC_INPUT_BIT_MASK2 0x02
#define ADC_INCH2 INCH_1

#define TA1_BIT 0x20 // P1.5
#define TA_DIR P1DIR
#define TA_OUT P1OUT
#define TA_SEL P1SEL
// declarations for UART calls
#include "uart_out.h"
#include "states.h" // include the states
// include declarations for snprintf from the library & generic string stuff
#include <string.h>
#include <stdio.h>

#define BUTTON 0x08

volatile signed short state = 0;
volatile unsigned short latest_result = 0;
volatile unsigned short latest_result2 = 0;
volatile unsigned short last_result=0;
volatile unsigned short choice_reset = 0;
volatile short battle = 0;
volatile short left = 0;
volatile short collectFlag = 0;
volatile int motorCounter = 500;
volatile int motorCountdown = 5000;

// a buffer to construct short text output strings
#define CBUFLEN 20
char cbuffer[CBUFLEN]; // buffer for output of characters

void set_up_first() {
	int r = ((rand()%3)*-1) - 1;
	if (r == -3) {
		state = -3;
		first_1a();
	} else if (r == -2) {
		state = -2;
		first_1b();
	} else if (r == -1) {
		state = -1;
		first_1c();
	} else {
		first_1c();
	}
	 IE1 |= WDTIE;
}

void init_adc(){
  ADC10CTL1= ADC_INCH	//input channel 4
 			  +SHS_0 //use ADC10SC bit to trigger sampling
 			  +ADC10DIV_4 // ADC10 clock/5
 			  +ADC10SSEL_0 // Clock Source=ADC10OSC
 			  +CONSEQ_0; // single channel, single conversion
 			  ;
  ADC10AE0=ADC_INPUT_BIT_MASK; // enable A4 analog input
  ADC10CTL0= SREF_0	//reference voltages are Vss and Vcc
 	          +ADC10SHT_3 //64 ADC10 Clocks for sample and hold time (slowest)
 	          +ADC10ON	//turn on ADC10
 	          +ENC		//enable (but not yet start) conversions
 	          +ADC10IE  //enable interrupts
 	          ;
}

void adc_for_light() { // Set the adc for the photoresistor
	ADC10CTL1= ADC_INCH2	//input channel 1
	 			  +SHS_0 //use ADC10SC bit to trigger sampling
	 			  +ADC10DIV_4 // ADC10 clock/5
	 			  +ADC10SSEL_0 // Clock Source=ADC10OSC
	 			  +CONSEQ_0; // single channel, single conversion
	 			  ;
	  ADC10AE0=ADC_INPUT_BIT_MASK2; // enable A1 analog input
	  ADC10CTL0= SREF_0	//reference voltages are Vss and Vcc
	 	          +ADC10SHT_3 //64 ADC10 Clocks for sample and hold time (slowest)
	 	          +ADC10ON	//turn on ADC10
	 	          +ENC		//enable (but not yet start) conversions
	 	          +ADC10IE  //enable interrupts
	 	          ;
	  ADC10CTL0 |= ADC10SC;  // trigger a conversion

}

void wait_for_light() {
	adc_for_light();
	while (latest_result > 300) { // Some arbitrary number
		ADC10CTL0 |= ADC10SC;  // trigger a conversion
	}
	u_print_string("You finally illuminate the cave with a light.");
}

void interrupt adc_handler(){

	latest_result=ADC10MEM;   // store the answer
}
ISR_VECTOR(adc_handler, ".int05")


void rumbleMotor(void) {
	motorCounter = 1000;
	TA_SEL|=TA1_BIT;				// connect timer 1 output to pin 2
	TA_DIR|=TA1_BIT;
	while (--motorCounter > 0) {}
	TA_SEL&=~TA1_BIT;				// connect timer 1 output to pin 2
	TA_DIR&=~TA1_BIT;
	motorCounter = 1000;
}

void choose_state(){
	IE1 &= ~WDTIE;
	//////////// STAGE ADVANCEMENT /////////////////////////
	if (state == -3) {
		//latest_result = 100;
		if (left == 1) {
			state = 2;
			collectFlag = stage_2();
		} else {
			state = 3;
			collectFlag = stage_3();
		}
	} else if (state == -2) {
		//latest_result = 200;
		if (left == 1) {
			state = 4;
			collectFlag = stage_4();
		} else {
			state = -1;
			collectFlag = first_1c();
		}
	} else if (state == -1) {
		//latest_result = 300;
		if (left == 1) {
			state = 6;
			collectFlag = stage_6();
		} else {
			state = 7;
			collectFlag = stage_7();
		}
	} else if (state == 2) {
		if (left == 1) {
			state = 8;
			collectFlag = stage_8();
		} else {
			state = -1;
			collectFlag = first_1c();
		}
	} else if (state == 3) {
		if (left == 1) {
			state = -2;
			collectFlag = first_1b();
		} else {
			state = 4;
			collectFlag = stage_4();
		}
	} else if (state == 4) {
		if (left == 1) {
			state = 8;
			collectFlag = stage_8();
		} else {
			state = 9;
			wait_for_light();
			collectFlag = stage_9a();
		}
	} else if (state == 6) {
		if (left == 1) {
			state = 4;
			collectFlag = stage_4();
		} else {
			state = 9;
			collectFlag = stage_9b();
		}
	} else if (state == 7) {
		if (left == 1) {
			state = 4;
			collectFlag = stage_4();
		} else {
			state = 10;
			collectFlag = stage_10();
		}
	} else if (state == 8) {
		if (left == 1) {
			state = 11;
			collectFlag = stage_11();
		} else {
			state = 12;
			collectFlag = stage_12();
		}
	} else if (state == 9) {
		if (left == 1) {
			state = 12;
			collectFlag = stage_12();
		} else {
			state = 13;
			collectFlag = stage_13();
		}
	} else if (state == 10) {
		if (left == 1) {
			state = 13;
			collectFlag = stage_13();
		} else {
			state = 9;
			collectFlag = stage_9b();
		}
	} else if (state == 11) {
		if (left == 1) {
			state = 14;
			collectFlag = stage_14();
		}
	} else if (state == 12) {
		if (left == 1) {
			state = 11;
			collectFlag = stage_11();
		} else {
			state = 14;
			collectFlag = stage_14();
		}
	} else if (state == 13) {
		if (left == 1) {
			state = 14;
			collectFlag = stage_14();
		} else {
			state = 12;
			collectFlag = stage_12();
		}
	} else if (state == 14) {
		if (left == 1) {

		} else {

		}
	}

	IE1 |= WDTIE;
}


interrupt void WDT_interval_handler(){

/*
	if (motorCountdown-- == 0) {
		latest_result2++;
		adc_for_light();
		ADC10CTL0 |= ADC10SC;
		motorCountdown = 10000;
	}*/


	ADC10CTL0 |= ADC10SC;  // trigger a conversion
	// Make sure users reset to make a choice
	if (latest_result > 20 && latest_result < 1000) {
				choice_reset = 1;
	}
	if (choice_reset == 1) {
		//choice_reset = 0;
		if (latest_result < 20) {
			left = 1;
			choose_state();
		} else if (latest_result > 1000) {
			left = 0;
			choose_state();
		}



	}

}
ISR_VECTOR(WDT_interval_handler, ".int10")


void main(){

	WDTCTL =(WDTPW + WDTTMSEL + WDTCNTCL + 0 +1);

	BCSCTL1 = CALBC1_8MHZ;			// 8Mhz calibration for clock
  	DCOCTL  = CALDCO_8MHZ;

  	init_USCI_UART(); // initialize the UART
  	// Setup printing

  	// Set up timer
  	TACTL = TASSEL_2+ID_3;			// clock source = SMCLK
  										// clock divider=8
  										// (clock still off)

  		TACCTL0=0;						// Nothing on CCR0
  		TACCTL1=OUTMOD_7;				// reset/set mode
  		TACCR0 = 999;					// period-1 in CCR0
  		TACCR1 =  900;                  // duty cycle in CCR1


  		TACTL |= MC_1;



  	init_adc();
    // Set up first state
    srand(time(NULL));
    _bis_SR_register(GIE); //enable interrupts so we can print.
    set_up_first();
    while(1) {}
  	_bis_SR_register(LPM0_bits); //powerdown CPU

}
