#include <msp430.h>
#include "phare.h"


extern void init_all_hw(void);
extern void set_drive(int v_l, int v_r);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;


    init_all_hw();
    init_phare();


    set_drive(0, 0);
    __delay_cycles(2000000);

    scan_maximum_phare();


    while(1) {

        P1OUT |= BIT6;
    }
}
