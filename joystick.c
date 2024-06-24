#include <msp430.h> 
#define TRUE 1
#define PASSO 40
#define BR100 10485 //100Hz
#define BR50 20971  //50hz

void debounce(long x){
    volatile long i;
    for(i = 0; i < x; i++);
}

void configadc(){
    ADC12CTL0 &= ~ADC12ENC; //Desabilitar para configuração
    ADC12CTL0 = ADC12ON; //ligar adc
    ADC12CTL1 = ADC12CONSEQ_3 | // Multichannel com rep
                ADC12SHS_1 |    //Acionamento pelo Timer A0.1
                ADC12CSTARTADD_0 | //MEM0
                ADC12SSEL_3; //SMCLK
    ADC12CTL2 = 0; //8 bits
    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_1; //Referência interna desligada, CH1
    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_2 | ADC12EOS;
    P6SEL |= BIT1|BIT2; //Dedicada
    ADC12CTL0 |= ADC12ENC; //Habilitar
}

void TA0config(){
    TA0CTL = TASSEL_2 | MC__UP;
    TA0CCTL1 = OUTMOD_6;
    TA0CCR0 = BR50;
    TA0CCR1 = TA0CCR0/2;
}

void config_leds(){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P1SEL |= BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
    P4SEL |= BIT7;
}

void TBconfig(){
    TB0CTL = TASSEL_2 | MC__UP;
    TB0CCTL1 = OUTMOD_6;
    TB0CCTL2 = OUTMOD_6;
    TB0CCR0 = BR100;
    TB0CCR1 = TB0CCR0/2;
    TB0CCR2 = TB0CCR0/2;
    P4SEL |= BIT3;
    P4DIR |= BIT3;
    PMAPKEYID = 0x02D52;
    P4MAP3 = PM_TB0CCR1A;
    P4MAP7 = PM_TB0CCR2A;
}

/*#pragma vector = ADC12_VECTOR
__interrupt void pwmswitch(){
    TB0CCR1 = PASSO*ADC12MEM0;
    TB0CCR2 = PASSO*ADC12MEM1;
}*/

int main(void)
{
    volatile unsigned char adc0,adc1;
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	config_leds();
	TA0config();
	TBconfig();
	configadc();
	while(TRUE){
	    ADC12CTL0 |= ADC12SC;
	    ADC12CTL0 &= ~ADC12SC;
	    while((ADC12IFG&ADC12IFG1) == 0);
	    TB0CCR1 = PASSO*ADC12MEM0;
	    TB0CCR2 = PASSO*ADC12MEM1;
	    adc0 = ADC12MEM0;
	    adc1 = ADC12MEM1;
	}
	return 0;
}
