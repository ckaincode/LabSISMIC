#include <msp430.h>
#include <math.h>
#include <stdio.h>

#define BR32 32768 //32 hertz com smCLK
#define PASSOVOLT 41 //PWM com SMCLK para o servo
#define BR10K 105
#define BR50 20971  //50hz
#define BR100K 11
#define ABERTA 380
#define FECHADA 44
#define TRUE 1
#define FALSE 0
#define DBC 1000
#define VRX 1
#define VRY 0
#define BL BIT3
#define RS BIT0
#define EN BIT2

typedef unsigned char uint8;


unsigned char flag = 0;
int vrx,vry = 0;
int tensx,tensy = 0;
int decx,decy = 0;
int maxxI,maxyI = 0;
int minxI,minyI = 0;
int maxxD,maxyD = 0;
int minxD,minyD = 0;

void debounce(long x){
    volatile long i;
    for(i = 0; i < x; i++);
}


void setUSCIB0master(){
    UCB0CTL1 = UCSWRST;
    UCB0CTL0 = UCMST | UCSYNC | UCMODE_3; // Mestre/ Síncrono / I2C
    UCB0BRW = BR100K; // Divisor de Freq
    UCB0CTL1 = UCSSEL_3; // SMCLK
    //Configuração dos pinos (3.0 e 3.1)
    P3SEL |= BIT1 | BIT0; // Selecionar pino para função USCI
    P3REN |= BIT1 | BIT0;
    P3OUT |= BIT1 | BIT0;
}

void lcdbyte(char byte, char rs){
    LCDWriteNB(((byte>>4)&0x0F),rs);
    LCDWriteNB((byte&0x0F),rs);
}

void i2c_write(char dado){
    UCB0CTL1 |= UCTR | UCTXSTT;
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = dado;
    while((UCB0CTL1 & UCTXSTT) == UCTXSTT);
    if((UCB0IFG & UCNACKIFG) == UCNACKIFG){
        P1OUT |= BIT0;
        while(1);
    }
    UCB0CTL1 |= UCTXSTP;
    while((UCB0CTL1 & UCTXSTP) == UCTXSTP);
}


void LCDWriteNB(uint8 nibble, uint8 ischar){
    nibble <<= 4;
    i2c_write(nibble | ischar | BL);
    i2c_write(nibble | ischar | BL | EN);
    i2c_write(nibble | ischar | BL);
}

void lcd_aux(char dado){
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = ((dado<<4)&0XF0) | BL;
    debounce(50);
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = ((dado<<4)&0XF0) | BL | EN;
    debounce(50);
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = ((dado<<4)&0XF0) | BL;
}

void lcd_inic(){
    UCB0CTL1 |= UCTR | UCTXSTT;
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = 0;
    while((UCB0CTL1 & UCTXSTT) == UCTXSTT);
    if((UCB0IFG & UCNACKIFG) == UCNACKIFG){
        P1OUT |= BIT0;
        while(1);
    }
    lcd_aux(0);
    debounce(20000);
    lcd_aux(3);
    debounce(10000);
    lcd_aux(3);
    debounce(10000);
    lcd_aux(3);
    debounce(10000);
    lcd_aux(2);


    //Entrou em modo de 4 bits
    lcd_aux(2); lcd_aux(8);
    lcd_aux(0); lcd_aux(8);
    lcd_aux(0); lcd_aux(1);
    lcd_aux(0); lcd_aux(6);
    lcd_aux(0); lcd_aux(0xC);

    while((UCB0IFG & UCTXIFG) == 0);
    UCB0CTL1 |= UCTXSTP;
    while((UCB0CTL1 & UCTXSTP) == UCTXSTP);
    debounce(50);


}

void lcdWrite(char *str){
    int cont = 0;
    while (!(*str == '\0')){
        if ((cont == 16) || (*str == '\n')){
            lcdbyte(0xC0, 0);
            cont = 0;
            str++;
            continue;
        }
        lcdbyte(*str, 1);
        str++;
        cont++;
    }
}


void TA0config(){
    TA0CTL = TASSEL_2 | MC__UP;
    TA0CCTL1 = OUTMOD_7;
    TA0CCR0 = BR32;
    TA0CCR1 = TA0CCR0/2;
}

void GPIOconfig(){
    P6DIR &= ~BIT3;
    P6OUT |= BIT3;
    P6REN |= BIT3;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
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
    TA2CCR2 = 687;

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
    int maxx,maxy = 0;
    int minx,miny = 4095;
    pt = &ADC12MEM0;
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            somaX += *pt;
            maxx = (*pt > maxx) ? *pt : maxx;
            minx = (*pt < minx) ? *pt : minx;
        }
        else{
            somaY += *pt;
            maxy = (*pt > maxy) ? *pt : maxy;
            miny = (*pt < miny) ? *pt : miny;
        }
        ++pt;
    }
    vrx = somaX >>2;
    vry = somaY >>2;
    maxxI = (int)(0.0008*maxx);
    maxyI = (int)(0.0008*maxy);
    minxI = (int)(0.0008*minx);
    minyI = (int)(0.0008*miny);
    maxxD = (int)(100*((0.0008*maxx) - maxxI));
    maxyD = (int)(100*((0.0008*maxy) - maxyI));
    minxD = (int)(100*((0.0008*minx) - minxI));
    minyD = (int)(100*((0.0008*miny) - minyI));
    tensx = (int)(0.0008*vrx);
    tensy = (int)(0.0008*vry);
    decx = (int)(1000*((0.0008*vrx) - tensx));
    decy = (int)(1000*((0.0008*vry) - tensy));
    flag = 1;
}


int main(void)
{
    __disable_interrupt();
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    char canal = VRX;
    char stringlcd[40];
    GPIOconfig();
    setUSCIB0master();
    UCB0I2CSA = 0x3F;
    i2c_write(0);
    lcd_inic();
    TA0config();
    servoconfig();
	configadc();
	__enable_interrupt();
	while(TRUE){
	    if(flag == 1){
	        flag = 0;
            lcdbyte(1,0);
            debounce(1200);
            lcdbyte(3,0);
            debounce(1200);
	        if(canal == VRX){
                if(vrx < 10){
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%d0V   000%d\nMn=%d,%d0  Mx=%d,%d0",tensx,decx,vrx,minxI,minxD,maxxI,maxxD);
                }
                else if(vrx < 100){
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%d0V   00%d\nMn=%d,%d0  Mx=%d,%d0",tensx,decx,vrx,minxI,minxD,maxxI,maxxD);

                }
                else if(vrx < 1000){
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%dV   0%d\nMn=%d,%d  Mx=%d,%d",tensx,decx,vrx,minxI,minxD,maxxI,maxxD);
                }
                else {
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%dV   %d\nMn=%d,%d  Mx=%d,%d",tensx,decx,vrx,minxI,minxD,maxxI,maxxD);
                }
                lcdWrite(stringlcd);
	            TA2CCR2 = 524 + round((0.33*vrx));
	        }
	        else{
                if(vry < 10){
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%d0V   000%d\nMn=%d,%d0  Mx=%d,%d0",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                }
                else if(vry < 100){
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%d0V   00%d\nMn=%d,%d0  Mx=%d,%d0",tensy,decy,vry,minyI,minyD,maxyI,maxyD);

                }
                else if(vry < 1000){
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%dV   0%d\nMn=%d,%d  Mx=%d,%d",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                }
                else {
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%dV   %d\nMn=%d,%d  Mx=%d,%d",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                }
                lcdWrite(stringlcd);
	            TA2CCR2 =  524 + round((4095 - vry)*0.33);
	        }
	        debounce(1000);
	        if(sw_mon() == TRUE) canal ^= 1;
	    }
	}
	return 0;
}
