#include <msp430.h>
#include <math.h>
#define BR32 32768 //32 hertz com smCLK
#define PASSOVOLT 41 //PWM com SMCLK para o servo
#define BR50 20971  //50hz
#define ABERTA 380
#define FECHADA 44
#define TRUE 1
#define FALSE 0
#define DBC 1000
#define VRX 1
#define VRY 0

unsigned char flag = 0;
unsigned int vrx,vry = 0;
float decx,decy = 0;

void TA0config(){
    TA0CTL = TASSEL_2 | MC__UP;
    TA0CCTL1 = OUTMOD_7;
    TA0CCR0 = BR32;
    TA0CCR1 = TA0CCR0/2;
}

void debounce(long x){
    volatile long i;
    for(i = 0; i < x; i++);
}

void GPIOconfig(){
    P6DIR &= ~BIT3;
    P6OUT |= BIT3;
    P6REN |= BIT3;
}

void configadc(){
    volatile unsigned char *pt;
    volatile unsigned int cont;
    ADC12CTL0 &= ADC12ENC; //Desabilitar para configuração
    ADC12CTL0 = ADC12ON; //ligar adc
    ADC12CTL1 = ADC12CONSEQ_3 | // Multichannel com rep
                ADC12SHS_1 |    //Acionamento pelo Timer A0.1
                ADC12CSTARTADD_0 | //MEM0
                ADC12SSEL_3; //SMCLK
    ADC12CTL2 = ADC12RES_2; //12 Bits bits
    pt = &ADC12MCTL0;
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            *pt = ADC12SREF_0 | ADC12INCH_1;
        }
        else{
            *pt = ADC12SREF_0 | ADC12INCH_2;
        }
        ++pt;
    }
    ADC12MCTL7 |= ADC12EOS;
    ADC12IE |= ADC12IE7;
    P6SEL |= BIT1|BIT2; //Dedicada
    ADC12CTL0 |= ADC12ENC; //Habilitar
}

void servoconfig(void){
    TA2CTL = TASSEL__SMCLK | MC__UP;
    TA2CCTL2 = OUTMOD_7;
    TA2CCR0 = BR50;
    TA2CCR2 = 2164;

    P2SEL |= BIT5;
    P2DIR |= BIT5;
}

int sw_mon(){
    static int psw = ABERTA;
    if((P6IN&BIT3) == 0){
        if(psw == ABERTA){
            debounce(DBC);
            psw = FECHADA;
            return TRUE;
        }
    }
    else{
        if(psw == FECHADA){
            debounce(DBC);
            psw = ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

#pragma vector = ADC12_VECTOR
__interrupt void isr_media(){
    volatile unsigned int *pt;
    unsigned int cont,somaX,somaY = 0;
    pt = &ADC12MEM0;
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            somaX += *pt;
        }
        else{
            somaY += *pt;
        }
        ++pt;
    }
    vrx = somaX >>2;
    vry = somaY >>2;
    flag = 1;
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    char canal = VRX;
    TA0config();
    servoconfig();
    debounce(20000);
	GPIOconfig();
	configadc();
	__enable_interrupt();
	while(TRUE){
	    if(flag == 1){
	        flag = 0;
	        if(canal == VRX){
	            TA2CCR2 = 524 + round((0.33*vrx));
	        }
	        else{
	            TA2CCR2 =  524 + round((4095 - vry)*0.33);
	        }
	        if(sw_mon() == TRUE) canal ^= 1;
	    }
	}
	return 0;
}
