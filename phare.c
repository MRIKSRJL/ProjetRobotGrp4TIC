#include "phare.h"
#define M_A_DIR BIT1
#define M_B_DIR BIT5

extern void set_drive(int v_l, int v_r);
void init_phare(void) {

    ADC10AE0 |= ADC_AE_BIT;
}


int lire_phare(void) {

        ADC10CTL0 &= ~ENC;
        while(ADC10CTL1 & ADC10BUSY);

        ADC10CTL1 = CANAL_PHARE | ADC10DIV_3;


        ADC10CTL0 |= ENC | ADC10SC;
        while(ADC10CTL1 & ADC10BUSY);

        return ADC10MEM;
}
void scan_maximum_phare(void) {
    int current_val = 0;
    int max_val = 0;
    int best_step = 0;
    int i = 0;
    // --- 强制停止并重置方向 ---
    set_drive(0, 0);
    __delay_cycles(100000);

    set_drive(SCAN_SPEED, SCAN_SPEED); // 启动
    P2OUT |= M_A_DIR;   // A轮置一 (后退)
    P2OUT |= M_B_DIR;   // B轮置一 (尝试让B轮反转)

    for (i = 0; i < FULL_TURN_STEPS; i++) {
        current_val = lire_phare();

        if (current_val > max_val) {
            max_val = current_val;
            best_step = i;
        }
        __delay_cycles(50000);
    }

    set_drive(0, 0);
    __delay_cycles(1000000);

    set_drive(SCAN_SPEED, SCAN_SPEED);
    P2OUT |= M_A_DIR;   // A轮置一 (后退)
    P2OUT |= M_B_DIR;   // B轮置一 (尝试让B轮反转)

    for (i = 0; i < best_step; i++) {
        __delay_cycles(50000);
    }

    set_drive(0, 0);
}





