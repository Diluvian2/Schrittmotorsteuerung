#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void ADC_Init(void)
{
  // die Versorgungsspannung AVcc als Referenz wählen:
  //ADMUX = (0<<REFS1) | (0<<REFS0);    
  // oder interne Referenzspannung als Referenz für den ADC wählen:
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
		// Kanal waehlen, ohne andere Bits zu beeinflußen
		ADMUX = (ADMUX & ~(0x1F)) | (3 & 0x1F);
		ADCSRA |= (1<<ADSC);					// eine Wandlung "single conversion"
		last_val = ADCW;
		return ADCW;                    // 16bit ADC auslesen und zurückgeben
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

void setAdValuetoTimerTOP() {
	unsigned char sreg;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Set TCNT0 to AD Value */
	
	TC1H = ADCW>>8;
	OCR1C = ADCW;
	/* Restore global interrupt flag */
	SREG = sreg;
}


int main(void)
{
// 	Change CLOCK
// 		CLKPR  = (1<<CLKPCE);
// 		CLKPR = (0<<CLKPS1) | (0<<CLKPS1) | (0<<CLKPS2) | (0<<CLKPS3);

	DDRA = 0x0F; //Stepper Motor ouputs PA0 - PA3	
	DDRB = (1<<DDB1);
	// Timer 0 ist für die Motor-Schritte zuständig    
	TCCR1A = (1<<COM1A0);
	TCCR1B = (1<<CS12) ;
	//Timer-TOP-Registers
	TC1H = 0x06;
	OCR1C  = 0x1;
	

	TIMSK |=  (1<<OCIE1A)  ;  //Interrupt enable	

	ADC_Init();  
	// Interrupts global freigeben  
	TC1H = 0;
	TCNT1 = 0;	

	PORTA = (1<<PORTA0)	;
	while(1)
	{	
		setAdValuetoTimerTOP();		
	}
}
