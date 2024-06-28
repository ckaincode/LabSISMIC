#include <msp430.h>
#include <stdio.h>

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

void ta2_config(void){
    TA2CTL = TASSEL_1 | MC__UP;
    TA2CCR0 = 32767;
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

            case UM: P1OUT |= BIT0; break;
            case DOIS: P1OUT &= ~BIT0; break;
            case TRES: P1OUT ^= BIT0; break;
            case QUATRO: P4OUT |= BIT7; break;
            case CINCO: P4OUT &= ~BIT7; break;
            case SEIS: P4OUT ^= BIT7; break;
            }

        }

        TA1CCTL1 = CM_2 | SCS | CAP;
    }




    return 0;

}
