#include <msp430.h> 

#define BL BIT3
#define RS BIT0
#define EN BIT2
#define BR10K 105
#define BR100K 11
#define TRUE 1
#define FALSE 0

typedef unsigned char uint8;


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	ledsconfig();
	setUSCIB0master();
	UCB0I2CSA = 0x3F;
	i2c_write(0);
	lcd_inic();
	char b[] = "Vini\nBolinhas";
	lcdWrite(b);
	while(1);


	return 0;
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

void ledsconfig(){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT |= ~BIT7;
}

void delay(long x){
    volatile long i;
    for(i = 0; i < x; i++);
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
    P4OUT |= BIT7;
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
    delay(50);
    while((UCB0IFG & UCTXIFG) == 0);
    UCB0TXBUF = ((dado<<4)&0XF0) | BL | EN;
    delay(50);
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
    delay(20000);
    lcd_aux(3);
    delay(10000);
    lcd_aux(3);
    delay(10000);
    lcd_aux(3);
    delay(10000);
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
    delay(50);


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
