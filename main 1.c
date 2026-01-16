#include <msp430.h>
#include "phare.h"


extern void init_all_hw(void);
extern void set_drive(int v_l, int v_r);

void main(void) {
     WDTCTL = WDTPW | WDTHOLD;


    init_all_hw();
    init_phare();


    set_drive(0, 0);
    P1OUT |= BIT0;//等待阶段：红灯闪烁 2 秒
    __delay_cycles(2000000);
    P1OUT &= ~BIT0;

    scan_maximum_phare();//原地自转找灯塔

    P1OUT |= BIT6;// 绿灯亮

    __delay_cycles(1500000);

    set_drive(0, 0);

    while(1) {

        P1OUT |= BIT6;
    }
}
