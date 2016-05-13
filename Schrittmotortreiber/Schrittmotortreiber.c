/*
 * GccApplication1.c
 *
 * Created: 24.09.2015 19:14:46
 *  Author: Rapha
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void ADC_Init(void)
{
  // die Versorgungsspannung AVcc als Referenz w�hlen:
  //ADMUX = (0<<REFS1) | (0<<REFS0);    
  // oder interne Referenzspannung als Referenz f�r den ADC w�hlen:
  // ADMUX = (1<<REFS1) | (1<<REFS0);
 
  // Bit ADFR ("free running") in ADCSRA steht beim Einschalten
  // schon auf 0, also single conversion
  ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADATE);     // Frequenzvorteiler und Free Running Mode Adate
  ADCSRB = (0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
  ADMUX = (ADMUX & ~(0x1F)) | (3 & 0x1F);
  ADCSRA |= (1<<ADEN);                  // ADC aktivieren
 
  /* nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
     also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
 
  ADCSRA |= (1<<ADSC);                  // eine ADC-Wandlung 
}

/* ADC Einzelmessung */
uint16_t ADC_Read( uint8_t channel)
{
	static uint16_t last_val;
	
	if( ADCSRA & (1<<ADSC) )
	{	//Konvertierung noch nicht abgeschlossen
		return last_val;
	}
	else
	{
		// Kanal waehlen, ohne andere Bits zu beeinflu�en
		ADMUX = (ADMUX & ~(0x1F)) | (3 & 0x1F);
		ADCSRA |= (1<<ADSC);					// eine Wandlung "single conversion"
		last_val = ADCW;
		return ADCW;                    // 16bit ADC auslesen und zur�ckgeben
	}
}

void delay_us(int ms)
{
	while (ms--) {
		_delay_us(1);  // one millisecond
	}
}

void turn_motor(int16_t *steptime, int16_t *adcval, int16_t *zaehler) {
	static uint16_t deltav_zaehler = 0;
	static uint16_t deltav_zmax = 1500;
	static uint8_t STEP_STATE = 0;
	static uint16_t powerduration_steps = 0;
	powerduration_steps =  *steptime>>1;
	
	if(*zaehler >= powerduration_steps)
	{
		PORTA = 0x01;
	}
	
		
	if(*zaehler >= *steptime)
	{
		*zaehler = 0;
		PORTA = 0b00000011;			
	}
	
  	if(deltav_zaehler > deltav_zmax)
  	{
		if(*adcval > *steptime)
			{
				*steptime += 1;
				deltav_zaehler = 0;
			}
			else if(*adcval < *steptime)
			{
				*steptime -= 1;
				deltav_zaehler = 0;
			}
}
	deltav_zaehler++;
}



int main(void)
{
	DDRA = 0x0F; //Stepper Motor ouputs PA0 - PA3
	
	// Timer 0 ist f�r die Motor-Schritte zust�ndig
	// Timer 0 initialisieren, CTC, Prescaler 64	    
	TCCR0A = (1<<WGM00);
	TCCR0B = (1<<CS00) | (1<<CS01); // CLK:8 = 1MHz
	OCR0A  = 0xAA;       //25us	
	//OCR0B  = 0x00;       //25us	
	// Timer 1 ist f�r die Schrittgeschwindigkeits�nderung zust�ndig
//	TCCR1A = (1<<COM1A1);
//	TCCR1B = (1<<CS13) | (1<<CS11) | (1<<CS10); // CLK:64 = 125kHz	
//	OCR1C  = 128;       //128 = 1,024ms
	TIMSK |=  (1<<OCIE0A)  ;  //Interrupt enable	
	
/*	TCNT1 = 10;		    //80us offset to timer 0	*/

	ADC_Init();  
	// Interrupts global freigeben  
	
	sei();
	//TCNT0H = 0;
	TCNT0L = 0;
	
	while(1)
	{	
		//OCR0B = 20;
		
		
	}
}

ISR(TIMER0_COMPA_vect) {
    static int8_t repetition = 0;

	if(repetition)
	{
		PORTA = 0x01;
		repetition--;
	}
	else
	{		
		PORTA = 0b00000011;
		repetition++;
	}
	OCR0A = ADCW>>2;
}