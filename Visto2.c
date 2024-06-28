#include <msp430.h>
#include <stdio.h>

#define PARTIDA 14051   //PULSO
#define TOLMAX 14600     //13,9 ms
#define TOLMIN 13950     //13,3 ms
#define PERIODO 10485    //Período, 10 ms
#define PASSO 1047      // 1 ms

void config_leds(void){
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}

void tb0_config(void){
    TB0CTL = TBSSEL_2 | MC__UP; //TB0 com SMCLK e Modo Up
    TB0CCR0 = PERIODO; //Per�odo PWM
    TB0CCTL1 = OUTMOD_6; //Sa�da Modo 6
    //   TB0CCR1 = POT100/2; //Iniciar com 50%

    P4DIR |= BIT7;
    P4SEL |= BIT7;
    PMAPKEYID = 0X02D52; //Escrever chave
    P4MAP7 = PM_TB0CCR1A; //TB0.1 mapeado para P4.7
}
void ta1_config(void){
    TA1CTL= TASSEL_2 | MC_2;
    TA1CCTL1 = CM_2 | SCS | CAP; //Habilitar Modo Captura
    TA1CCR0 = PARTIDA;

    P2DIR &= ~BIT0; //P2.0 = entrada
    P2SEL |= BIT0; //dedicada � captura

}

void ta2_config(void){
    TA2CTL = TASSEL_1 | //Selecionar SMCLK (1.048.576)
            MC__UP; //Modo 1 = ascendente
            //ID_3; //SMCLK / 8 = 131.072
    TA2CCR0 = 32767; //(131.075 x 0,25) - 1
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    config_leds();
    ta1_config();
    tb0_config();
    ta2_config();
    volatile unsigned long cod = 0;
    int vet[32];
    int i=0;
    int cap= 0;


    while(1){
        while((TA1CCTL1 & CCIFG)==0);
        TA1CTL |= TACLR;
        TA1CCTL1 &= ~CCIFG; //apagou no primeiro flanco de descida

        while((TA1CCTL1 & CCIFG)==0);
        cap = TA1CCR1;

        if((cap > TOLMIN) && (cap < TOLMAX)){
            TA1CCTL1 = CM_1 | SCS | CAP;

            while((TA1CCTL1 & CCIFG)==0);
            TA1CTL |= TACLR;
            TA1CCTL1 &= ~CCIFG; //apagou no primeiro flanco de subida

            for(i=0;i<32;i++){
                while((TA1CCTL1 & CCIFG)==0);
                vet[i] = TA1CCR1;
                TA1CTL |= TACLR;
                TA1CCTL1 &= ~CCIFG;
            }
            for(i=0;i<32;i++){
                if(vet[i]>1120 && vet[i]<1320){
                    vet[i] = 0;

                }
                else{
                    if(vet[i]>2230 && vet[i]<2470){

                        vet[i] = 1;

                    }

                }


            }

            for(i=0;i<32;i++){

                cod= cod>>1;
                if(vet[i]==1){
                    cod|=0x80000000L;
                }


            }
            volatile unsigned long x=0;
            x=cod>>24;

            __delay_cycles(1000);
            switch(x){

            /*  LED VERMELHO  */
            case 0xBA: P1OUT |= BIT0; break; //ACENDE
            case 0xB9: P1OUT &= ~BIT0; break;//APAGA
            case 0xB8: P1OUT ^= BIT0; break;//INVERTE


                /*  LED VERDE  */
            case 0xBB: TB0CCR1= PASSO*10; break;//ACENDE
            case 0xBF: TB0CCR1= PASSO*0; break;//APAGA
            case 0xBC: TB0CCR1 = (PASSO*10) - TB0CCR1; break;//INVERTE

            /*  0 a 90%  */
            case 0xE9: TB0CCR1= PASSO*0; break;//0
            case 0xF2: TB0CCR1= PASSO*1; break;//10%

            case 0xE7: TB0CCR1= PASSO*2; break;//20%
            case 0xE6: TB0CCR1= PASSO*3; break;//30%

            case 0xF7: TB0CCR1= PASSO*4; break;//40%
            case 0xE3: TB0CCR1= PASSO*5; break;//50%

            case 0xA5: TB0CCR1= PASSO*6; break;//60%
            case 0xF6: TB0CCR1= PASSO*7; break;//70%

            case 0xAD: TB0CCR1= PASSO*8; break;//80%
            case 0xEA: TB0CCR1= PASSO*9; break;//90%
            }

        }

        TA1CCTL1 = CM_2 | SCS | CAP;
    }




    return 0;

}
