#include <msp430.h>
#include <math.h>
#include <stdio.h>

#define BR32 32768 //32 hertz com smCLK
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
int minxD,minyD = 0;
int maxxI,maxyI = 0;
int minxI,minyI = 0;
int maxxD,maxyD = 0;
char minxDstr[20];


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

void lcdWrite(char *strlcd){
    int cont = 0;
    while (!(*strlcd == '\0')){
        if ((cont == 16) || (*strlcd == '\n')){
            lcdbyte(0xC0, 0);
            cont = 0;
            strlcd++;
            continue;
        }
        lcdbyte(*strlcd, 1);
        strlcd++;
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
    for(cont = 0; cont <= 7; cont++){
        if(cont % 2 == 0){
            *pt1 = ADC12SREF_0 | ADC12INCH_1;
        }
        else{
            *pt1 = ADC12SREF_0 | ADC12INCH_2;
        }
        ++pt1;
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

void USCI_A1_config(){
    UCA1CTL1 = UCSWRST;
    UCA1CTL1 = 0;
    UCA1BRW = 6;
    UCA1MCTL |= UCBRF_13 | UCOS16;
    P4SEL |= BIT4;
    UCA1CTL1 = UCSSEL_2;
}

void bt_char(char c){
    while((UCA1IFG&UCTXIFG) == 0);
    UCA1TXBUF = c;
}

void bt_str(char *vet){
    unsigned int i = 0;
    while (vet[i] != '\0'){
        bt_char(vet[i++]);
    }
}

/*
void uart_att(char* buffer, int *ct, int mediax, int mediay, char caso){
    if (*ct < 10){
        switch(caso){
        case 1:
            snprintf(buffer,sizeof(buffer),"000%d\n",mediay);
            break;
        case 2:
            snprintf(buffer,sizeof(buffer),"000%d\n",mediay);
            break;
        case 3:
            snprintf(buffer,sizeof(buffer),"000%d\n",mediay);
            break;
        case 4:
            snprintf(buffer,sizeof(buffer),"000%d\n",mediay);
            break;
        }

    }
    else if (*ct < 100){
        switch(caso){
        case 1:
            snprintf(buffer,sizeof(buffer),"00%d\n",mediay);
            break;
        case 2:
            snprintf(buffer,sizeof(buffer),"00%d\n",mediay);
            break;
        case 3:
            snprintf(buffer,sizeof(buffer),"00%d\n",mediay);
            break;
        case 4:
            snprintf(buffer,sizeof(buffer),"00%d\n",mediay);
            break;
        }
    }
    else if (*ct < 1000){
        switch(caso){
        case 1:
            snprintf(buffer,sizeof(buffer),"0%d\n",mediay);
            break;
        case 2:
            snprintf(buffer,sizeof(buffer),"0%d\n",mediay);
            break;
        case 3:
            snprintf(buffer,sizeof(buffer),"0%d\n",mediay);
            break;
        case 4:
            snprintf(buffer,sizeof(buffer),"0%d\n",mediay);
            break;
        }
    }
    else{
        switch(caso){
        case 1:
            snprintf(buffer,sizeof(buffer),"%d\n",mediay);
            break;
        case 2:
            snprintf(buffer,sizeof(buffer),"%d\n",mediay);
            break;
        case 3:
            snprintf(buffer,sizeof(buffer),"%d\n",mediay);
            break;
        case 4:
            snprintf(buffer,sizeof(buffer),"%d\n",mediay);
            break;
        }
    }

    if(*ct == 9999){
        *ct = 0;
    }
    else{
        *ct = *ct + 1;
    }
}*/

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
    minxI = maxxI;
    minyI = (int)(0.0008*miny);
    maxxD = ((int)(100*((0.0008*maxx) - maxxI)));
    maxyD = ((int)(100*((0.0008*maxy) - maxyI)));
    /*if(maxxD - 20 < 0){
        minxI = (maxxI - 1 >= 0) ? maxxI - 1 : 0;
        minxD = (maxxI > 0) ?  100 - maxxD : 0;
        if(minxD > 10){
            snprintf(minxDstr,sizeof(minxDstr),"%d",minxD);
        }
        else{
            snprintf(minxDstr,sizeof(minxDstr),"0%d",minxD);
        }
    }
    else if (maxxD - 20 < 10){
        minxI = maxxI;
        int jaj = maxxD - 20;
        snprintf(minxDstr,sizeof(minxDstr),"0%d",jaj);
    }
    else{
        minxD = maxxD - 20;
        snprintf(minxDstr,sizeof(minxDstr),"%d",minxD);
    }*/
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
    char stringlcd[60];
    char uartstring[60];
    int contador = 0;
    char stringcontador[60];
    char stringvrx[60];
    char stringvry[60];
    char stringdecx [60];
    char stringdecy [60];
    GPIOconfig();
    USCI_A1_config();
    setUSCIB0master();
    UCB0I2CSA = 0x3F;
    i2c_write(0);
    lcd_inic();
    TA0config();
    servoconfig();
	configadc();
	bt_str("\rCont: ---Canal A1----   ---Canal A2----\r\n");
	__enable_interrupt();
	while(TRUE){
	    if(flag == 1){
	        flag = 0;
            lcdbyte(1,0);
            debounce(1200);
            lcdbyte(3,0);
            debounce(1200);
            if(maxxD - 20 < 0){
                minxI = (maxxI - 1 >= 0) ? maxxI - 1 : 0;
                minxD = (maxxI > 0) ?  100 - maxxD : 0;
                if(minxD > 10){
                    snprintf(minxDstr,sizeof(minxDstr),"%d",minxD);
                }
                else{
                    snprintf(minxDstr,sizeof(minxDstr),"0%d",minxD);
                }
            }
            else if (maxxD - 20 < 10){
                minxI = maxxI;
                int jaj = maxxD - 20;
                snprintf(minxDstr,sizeof(minxDstr),"0%d",jaj);
            }
            else{
                minxD = maxxD - 20;
                snprintf(minxDstr,sizeof(minxDstr),"%d",minxD);
            }
            if(contador < 10){
                snprintf(stringcontador,sizeof(stringcontador),"000%d",contador);
            }
            else if(contador < 100){
                snprintf(stringcontador,sizeof(stringcontador),"00%d",contador);
            }
            else if(contador < 1000){
                snprintf(stringcontador,sizeof(stringcontador),"0%d",contador);
            }
            else{
                snprintf(stringcontador,sizeof(stringcontador),"%d",contador);
            }
            if(vrx < 10){
                snprintf(stringvrx,sizeof(stringvrx),"000%d",vrx);
            }
            else if (vrx < 100){
                snprintf(stringvrx,sizeof(stringvrx),"00%d",vrx);
            }
            else if (vrx < 1000){
                snprintf(stringvrx,sizeof(stringvrx),"0%d",vrx);
            }
            else{
                snprintf(stringvrx,sizeof(stringvrx),"%d",vrx);
            }

            if(vry < 10){
                snprintf(stringvry,sizeof(stringvry),"000%d",vrx);
            }
            else if (vry < 100){
                snprintf(stringvry,sizeof(stringvry),"00%d",vry);
            }
            else if (vry < 1000){
                snprintf(stringvry,sizeof(stringvry),"0%d",vry);
            }
            else{
                snprintf(stringvry,sizeof(stringvry),"%d",vry);
            }

	        if(canal == VRX){
                if(vrx < 10){
                    snprintf(stringdecx,sizeof(stringdecx),"0%d",decx);
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%d0V   000%d\nMn=%d,%s  Mx=%d,%d0",tensx,decx,vrx,minxI,minxDstr,maxxI,maxxD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);

                }
                else if(vrx < 100){
                    snprintf(stringdecx,sizeof(stringdecx),"0%d",decx);
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%d0V   00%d\nMn=%d,%s  Mx=%d,%d0",tensx,decx,vrx,minxI,minxDstr,maxxI,maxxD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);


                }
                else if(vrx < 1000){
                    snprintf(stringdecx,sizeof(stringdecx),"%d",decx);
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%dV   0%d\nMn=%d,%s  Mx=%d,%d",tensx,decx,vrx,minxI,minxDstr,maxxI,maxxD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);

                }
                else {
                    snprintf(stringdecx,sizeof(stringdecx),"%d",decx);
                    snprintf(stringlcd,sizeof(stringlcd),"A1=%d,%dV   %d\nMn=%d,%s  Mx=%d,%d",tensx,decx,vrx,minxI,minxDstr,maxxI,maxxD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);

                }
                lcdWrite(stringlcd);
                bt_str(uartstring);
	            TA2CCR2 = 524 + round((0.33*vrx));
	        }
	        else{
                if(vry < 10){
                    snprintf(stringdecy,sizeof(stringdecy),"0%d",decy);
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%d0V   000%d\nMn=%d,%d0  Mx=%d,%d0",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);
                }
                else if(vry < 100){
                    snprintf(stringvry,sizeof(stringvry),"00%d",vry);
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%d0V   00%d\nMn=%d,%d0  Mx=%d,%d0",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);

                }
                else if(vry < 1000){
                    snprintf(stringdecy,sizeof(stringdecy),"%d",decy);
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%dV   0%d\nMn=%d,%d  Mx=%d,%d",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);
                }
                else {
                    snprintf(stringdecy,sizeof(stringdecy),"%d",decy);
                    snprintf(stringlcd,sizeof(stringlcd),"A2=%d,%dV   %d\nMn=%d,%d  Mx=%d,%d",tensy,decy,vry,minyI,minyD,maxyI,maxyD);
                    snprintf(uartstring,sizeof(uartstring),"\r%s: %s --> %d,%sV     %s --> %d,%sV\r\n",stringcontador,stringvrx,tensx,stringdecx,stringvry,tensy,stringdecy);
                }
                lcdWrite(stringlcd);
                bt_str(uartstring);
	            TA2CCR2 =  524 + round((4095 - vry)*0.33);
	        }
	        debounce(1000);
	        if(contador == 9999){
	            contador = 0;
	        }
	        else{
	            contador++;
	        }
	        if(sw_mon() == TRUE) canal ^= 1;
	    }
	}
	return 0;
}
