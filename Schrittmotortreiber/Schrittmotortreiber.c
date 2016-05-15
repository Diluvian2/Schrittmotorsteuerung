// Raphael Klapczynski

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void ADC_Init(void)
{
  ADCSRA = (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADATE);     // Prescaler and Free Running Mode
  ADCSRB = (0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
  ADMUX = (ADMUX & ~(0x1F)) | (3 & 0x1F); //AD Channel select
  ADCSRA |= (1<<ADEN);                    //activate ADC 
  ADCSRA |= (1<<ADSC);                    //one inintial ADC-conversion 
}

void setTimerFromADC() {	
	//Write 10-Bit Timer register with 16-Bit AD-Value ADCW
	TC1H = ADCW>>8;
	OCR1C = ADCW;
}


int main(void)
{

	DDRB = (1<<DDB1); //Set Port B1 to OUTPUT
	PORTA = (1<<PORTA0)	; //Activate Stepper-Driver-IC
	
	// PWM Timer Setup
	TCCR1A = (1<<COM1A0);
	TCCR1B = (1<<CS11) | (1<<CS10); //Prescaler = CK/8
	//Timer-TOP-Registers (initial values, irrelevant for later use)
	TC1H = 0x06;
	OCR1C  = 0x1;

	ADC_Init();  
	
	// clear Timer (not necessary in this case)
	TC1H = 0;
	TCNT1 = 0;	

	
	while(1)
	{	
		setTimerFromADC();		
	}
}
