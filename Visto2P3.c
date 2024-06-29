#include <msp430.h>
#include <stdio.h>

#define GRAU10 116
#define GRAU0 524
#define GRAU90 1572
#define GRAU180 2621
#define PARTIDA 14051   //PULSO
#define TOLMAX 14607     //13,9 ms
#define TOLMIN 13951     //13,3 ms
#define PERIODO 10485    //Período, 10 ms
#define PASSO 1047      // 1 ms
#define UM 0xBA
#define DOIS 0xB9
#define TRES 0xB8
#define QUATRO 0xBB
#define CINCO 0xBF
#define SEIS 0xBC
#define SETE 0xF8
#define OITO 0xEA
#define NOVE 0xF6
#define ZERO 0xE6
#define ASTRSK 0xE9
#define HSHT 0xF2
#define ARRUP 0xE7
#define ARRR 0xA5
#define ARRL 0xF7
#define ARRDOWN 0xAD
#define OK 0xE3

void debounce(long x){
    volatile long i;
    for(i = 0; i < x;i++);
}

void led(void){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}

void tb0_config(void){
    TB0CTL = TBSSEL_2 | MC__UP;
    TB0CCR0 = PERIODO;
    TB0CCTL1 = OUTMOD_6;


    P4DIR |= BIT7;
    P4SEL |= BIT7;
    PMAPKEYID = 0X02D52;
    P4MAP7 = PM_TB0CCR1A;
}

void ta2_config(void){
    TA2CTL = TASSEL__SMCLK | MC__UP;
    TA2CCTL2 = OUTMOD_7;
    TA2CCR0 = 20971;
    TA2CCR2 = GRAU0;
    P2SEL |= BIT5;
    P2DIR |= BIT5;
}

void ta1_config(void){
    TA1CTL= TASSEL_2 | MC_2;
    TA1CCTL1 = CM_2 | SCS | CAP; //Habilitar Modo Captura
    TA1CCR0 = PARTIDA;

    P2DIR &= ~BIT0; //P2.0 = entrada
    P2SEL |= BIT0; //dedicada � captura

}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    led();
    ta1_config();
    tb0_config();
    ta2_config();
    unsigned char BOTAO = 0;
    int word[32];
    unsigned int curr=0;
    int edge= 0;


    while(1){
        while((TA1CCTL1 & CCIFG)==0);
        TA1CTL |= TACLR;
        TA1CCTL1 &= ~CCIFG;

        while((TA1CCTL1 & CCIFG)==0);
        edge = TA1CCR1;

        if((edge > TOLMIN) && (edge < TOLMAX)){
            TA1CCTL1 = CM_1 | SCS | CAP;

            while((TA1CCTL1 & CCIFG)==0);
            TA1CTL |= TACLR;
            TA1CCTL1 &= ~CCIFG;

            for(curr=0;curr<=31;curr++){
                while((TA1CCTL1 & CCIFG)==0);
                if(curr < 24){
                    TA1CTL |= TACLR;
                    TA1CCTL1 &= ~CCIFG;
                    continue;
                }
                word[curr] = TA1CCR1;
                TA1CTL |= TACLR;
                TA1CCTL1 &= ~CCIFG;
            }
            for(curr=24;curr<=31;curr++){
                if(word[curr]>1120 && word[curr]<1320){
                    word[curr] = 0;

                }
                else{
                    if(word[curr]>2230 && word[curr]<2470){

                        word[curr] = 1;

                    }

                }


            }

            for(curr=24;curr<=31;curr++){
                BOTAO = BOTAO >> 1;
                if(word[curr]==1){
                    BOTAO |=0x80L;
                }


            }

            debounce(1000);
            switch(BOTAO){
            /*  LED VERMELHO  */
            case 0xBA: P1OUT |= BIT0; break;
            case 0xB9: P1OUT &= ~BIT0; break;
            case 0xB8: P1OUT ^= BIT0; break;

                /*  LED VERDE  */
            case 0xBB: TB0CCR1= PASSO*10; break;
            case 0xBF: TB0CCR1= PASSO*0; break;
            case 0xBC: TB0CCR1 = (PASSO*10) - TB0CCR1; break;

            case 0xF8: TA2CCR2 = GRAU0; break;
            case 0xEA: TA2CCR2 = GRAU90; break;

            case 0xF6: TA2CCR2 = GRAU180; break;
            case 0xE7: if(TA2CCR2 != GRAU180){TA2CCR2 += GRAU10;}
                       break;

            case 0xAD: if(TA2CCR2 != GRAU0){TA2CCR2 -= GRAU10;}
                       break;

        }

        TA1CCTL1 = CM_2 | SCS | CAP;
        }

    }
    return 0;
}
