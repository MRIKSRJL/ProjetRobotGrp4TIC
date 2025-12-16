#include <msp430.h>


#define bouton_start BIT3  
#define cap_SLG BIT5      
#define cap_SLD BIT6       


#define vmax_g 900
#define vmax_d 950

int start = 0; 


void init_moteur(void)
{
    P2SEL &= ~(BIT1 | BIT4);
    P2SEL2 &= ~(BIT1 | BIT4);
    P2SEL |= BIT2 | BIT5;
    P2SEL2 &= ~(BIT2 | BIT5);
    P2DIR |= BIT1 | BIT2 | BIT4 | BIT5;

    P2OUT &= ~BIT1;            
    P2OUT |= BIT4;             

    TA1CTL = 0x0210;            
    TA1CCR0 = 1000;             

    TA1CCTL1 = 0x00E0;
    TA1CCTL2 = 0x00E0;


    TA1CCR1 = 0;
    TA1CCR2 = 500;              
}


void init_suiveurLigne(void)
{
    P1SEL &= ~(cap_SLG | cap_SLD);
    P1SEL2 &= ~(cap_SLG | cap_SLD);
    P1DIR &= ~(cap_SLG | cap_SLD); 
    P1IE |= cap_SLG | cap_SLD;     
    P1IES |= cap_SLG | cap_SLD;    
    P1IFG &= ~(cap_SLG | cap_SLD); 
}


void init_bouton_start(void)
{
    P1SEL &= ~(bouton_start);
    P1SEL2 &= ~(bouton_start);
    P1DIR &= ~(bouton_start);
    P1REN |= bouton_start;       
    P1OUT |= bouton_start;       
    P1IE |= bouton_start;        
    P1IES |= bouton_start;       
    P1IFG &= ~(bouton_start);    
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   


    init_moteur();
    init_suiveurLigne();
    init_bouton_start();

    __delay_cycles(100000);     
    __enable_interrupt();       

    while (1);                   
}


#pragma vector = PORT1_VECTOR
__interrupt void tourner(void) {


    if (((P1IFG & bouton_start) == bouton_start) && (start == 0))
    {
        TA1CCR1 = 1000;            
        TA1CCR2 = 1000;             

        while (((P1IN & cap_SLG) == 0) | ((P1IN & cap_SLD) == 0));
        start = 1;                 
        P1IFG &= ~(bouton_start);  
    }

   
    else if (((P1IFG & cap_SLG) == cap_SLG) && (start == 1))
    {
        TA1CCR1 = 0;   
        TA1CCR2 = 750; 


        while ((P1IN & cap_SLG) == 0)
        {
            if (TA1CCR2 < vmax_d)
                TA1CCR2 = TA1CCR2 + 5; 
            __delay_cycles(10000);

  
            if ((TA1CCR2 >= vmax_d - 100) && ((P1IN & cap_SLD) == 0))
            {
                start = 0; 
            }
        }

 
        if (start == 0)
        {
            TA1CCR2 = 250;
            while ((P1IN & cap_SLD) == 0);
            TA1CCR1 = vmax_g - 100;
            TA1CCR2 = vmax_d;
            __delay_cycles(100000);
            while (((P1IN & cap_SLG) == 0) && ((P1IN & cap_SLD) == 0));
            TA1CCR1 = 0;
            TA1CCR2 = 500;
            P1IFG &= ~(cap_SLG | cap_SLD);
        }
        else
        {
 
            TA1CCR1 = vmax_g;
            TA1CCR2 = vmax_d;
            P1IFG &= ~(cap_SLG | cap_SLD);
        }

    }

    else if (((P1IFG & cap_SLD) == cap_SLD) && (start == 1))
    {
        TA1CCR1 = 500;
        TA1CCR2 = 500;

        while ((P1IN & cap_SLD) == 0)
        {
            if (TA1CCR1 < vmax_g)
                TA1CCR1 = TA1CCR1 + 10; 
            __delay_cycles(10000);

            if ((TA1CCR1 >= vmax_g - 200) && ((P1IN & cap_SLG) == 0))
            {
                start = 0;
            }
        }

        if (start == 0)
        {
            P2OUT |= BIT1;
            TA1CCR1 = 500;
            while ((P1IN & cap_SLG) == 0);
            P2OUT &= ~BIT1;
            TA1CCR1 = vmax_g;
            TA1CCR2 = vmax_d - 50;
            __delay_cycles(100000);
            while (((P1IN & cap_SLG) == 0) && ((P1IN & cap_SLD) == 0));
            TA1CCR1 = 0;
            TA1CCR2 = 500;
            P1IFG &= ~(cap_SLG | cap_SLD);
        }
        else
        {
            TA1CCR1 = vmax_g;
            TA1CCR2 = vmax_d;
            P1IFG &= ~(cap_SLG | cap_SLD);
        }

    }
}