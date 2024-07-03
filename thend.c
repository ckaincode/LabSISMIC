#include <msp430.h> 
#include <stdio.h>

#define BR32 32768

unsigned int med[1000];
unsigned int conversao = 0;;
char flag = 0;
char flagconv = 0;
char stringuart[30];

void debounce(long x){
    volatile long i;
    for(i=0; i < x; i++);
}

void TA0config(){
    TA0CTL = TASSEL_2 | MC__UP;
    TA0CCTL1 = OUTMOD_7;
    TA0CCR0 = 2098;
    TA0CCR1 = TA0CCR0/4;
}

void USCI_A0_config(){
    UCA0CTL1 = UCSWRST;
    UCA0CTL0 = 0;
    UCA0BRW = 6;
    UCA0MCTL |= UCBRF_13 | UCOS16;
    P3SEL |= BIT3;
    P3SEL |= BIT4;
    UCA0CTL1 = UCSSEL_2;
}

void configadc(){
    volatile unsigned char *pt1;
    volatile unsigned int cont;
    ADC12CTL0 &= ~ADC12ENC; //Desabilitar para configuração
    ADC12CTL0 = ADC12ON; //ligar adc
    ADC12CTL1 = ADC12CONSEQ_3 | // Multichannel com rep
                ADC12SHS_1 |    //Acionamento pelo Timer A0.1
                ADC12CSTARTADD_0 | //MEM0
                ADC12SSEL_3; //SMCLK
    ADC12CTL2 = ADC12RES_2; //12 Bits bits
    pt1 = &ADC12MCTL0;
    for(cont = 0; cont <= 15; cont++){
        *pt1 = ADC12SREF_0 | ADC12INCH_1;
        ++pt1;
    }
    ADC12MCTL15 |= ADC12EOS;
    ADC12IE |= ADC12IE15;
    P6SEL |= BIT1; //Dedicada
    ADC12CTL0 |= ADC12ENC; //Habilitar
}

void uart_char(char c){
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = c;
}

void uart_str(char *stringuart){
    while (*stringuart != '\0'){
        uart_char(*stringuart++);
    }
}

void gpioconfig(){
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;
}

#pragma vector = ADC12_VECTOR
__interrupt void isr_media(){
    volatile unsigned int *pt;
    volatile unsigned int cont = 0;
    volatile unsigned int media = 0;
    pt = &ADC12MEM0;
    for(cont = 0; cont < 16; cont++){
        media += *pt;
        ++pt;
    }
    med[conversao] = media >> 4;
    conversao++;
    if(conversao == 999){
        flagconv = 1;
    }
    flag = 1;
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	char stringuart[30];
	TA0config();
	USCI_A0_config();
	gpioconfig();
	configadc();
	while(1){
	    uart_str("\r\nAperte a chava S2 para inicio\r\n");
	    while((P1IN&BIT1) == BIT1);
	    debounce(1000);
	    __enable_interrupt();
	    while(flagconv == 0){
	        if(flag == 1){
	            snprintf(stringuart,sizeof(stringuart),"%d: %d\r\n",conversao,med[conversao]);
	            uart_str(stringuart);
	            flag = 0;
	        }
	    }
	    __disable_interrupt();
	    uart_str("Pronto!\r\n");
	}
	
	return 0;
}
