#include <msp430g2553.h>


#define REF_VAL         512     
#define SPD_HIGH        650     
#define SPD_MID         350     
#define SPD_LOW         250     
#define LOOP_DLY        10000

/* --- ç¡¬ä»¶å¼•è„šæ˜ å°„ --- */
#define ADC_CH_L        INCH_4
#define ADC_CH_R        INCH_5

#define M_A_DIR         BIT1
#define M_A_PWM         BIT2
#define M_B_PWM         BIT4
#define M_B_DIR         BIT5

#define LAMP_ERR        BIT0    
#define LAMP_OK         BIT6    

/* --- å…¨å±€å�˜é‡� --- */
volatile unsigned int g_adc_l = 0;
volatile unsigned int g_adc_r = 0;

/* --- è¾…åŠ©åŠŸèƒ½å‡½æ•° --- */

// å»¶æ—¶é—ªçƒ�é€šç”¨å‡½æ•°
void sig_flash(unsigned char mask, unsigned char loop_cnt) {
    unsigned char k;
    for(k = 0; k < loop_cnt; k++) {
        P1OUT |= mask;
        __delay_cycles(200000);
        P1OUT &= ~mask;
        __delay_cycles(200000);
    }
}

// ç¡¬ä»¶åˆ�å§‹åŒ–ç»Ÿä¸€å…¥å�£
void init_all_hw(void) {
    // æ—¶é’Ÿé…�ç½®
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // GPIO & LED
    P1SEL |= (BIT4 | BIT5);
    P1SEL2 |= (BIT4 | BIT5);
    P1DIR |= (LAMP_ERR | LAMP_OK);
    P1OUT &= ~(LAMP_ERR | LAMP_OK);

    // ç”µæœºç«¯å�£
    P2DIR |= (M_A_DIR | M_B_DIR | M_A_PWM | M_B_PWM);
    P2OUT &= ~(M_A_DIR | M_B_DIR);
    P2SEL |= (M_A_PWM | M_B_PWM);

    // å®šæ—¶å™¨ PWM
    TA1CTL = TASSEL_2 | MC_1 | TACLR;
    TA1CCR0 = 1000;
    TA1CCTL1 = OUTMOD_7; 
    TA1CCR1 = 0;
    TA1CCTL2 = OUTMOD_7; 
    TA1CCR2 = 0;

    // ADC é…�ç½®
    ADC10CTL0 = ADC10SHT_2 | ADC10ON;
    ADC10CTL1 = ADC_CH_L | ADC10DIV_3;
    ADC10AE0 = BIT4 | BIT5;
}

// é‡‡é›†ä¼ æ„Ÿå™¨æ•°æ�®
void update_adc(void) {
    // é‡‡æ ·å·¦é€šé�“
    ADC10CTL0 &= ~ENC;
    while(ADC10CTL1 & ADC10BUSY);
    ADC10CTL1 = ADC_CH_L | ADC10DIV_3;
    ADC10CTL0 |= (ENC | ADC10SC);
    while(ADC10CTL1 & ADC10BUSY);
    g_adc_l = ADC10MEM;

    __delay_cycles(100);

    // é‡‡æ ·å�³é€šé�“
    ADC10CTL0 &= ~ENC;
    while(ADC10CTL1 & ADC10BUSY);
    ADC10CTL1 = ADC_CH_R | ADC10DIV_3;
    ADC10CTL0 |= (ENC | ADC10SC);
    while(ADC10CTL1 & ADC10BUSY);
    g_adc_r = ADC10MEM;
}

// ç”µæœºé©±åŠ¨æŽ¥å�£
void set_drive(int v_l, int v_r) {
    // é™�å¹…ä¿�æŠ¤
    if(v_l > 1000) v_l = 1000; else if(v_l < 0) v_l = 0;
    if(v_r > 1000) v_r = 1000; else if(v_r < 0) v_r = 0;

    // è®¾ç½®æ–¹å�‘ä¸ŽPWM
    P2OUT |= ~M_A_DIR;  // ç»´æŒ�åŽŸé€»è¾‘
    TA1CCR1 = v_l;
    P2OUT &= M_B_DIR;   // ç»´æŒ�åŽŸé€»è¾‘
    TA1CCR2 = v_r;
}

// è·¯å¾„è¿½è¸ªé€»è¾‘
void task_track(void) {
    unsigned char is_white_l = (g_adc_l > REF_VAL);
    unsigned char is_white_r = (g_adc_r > REF_VAL);

    // çŠ¶æ€�æœºå¤„ç�†
    if(!is_white_l && !is_white_r) {
        // å�Œé»‘ -> ç›´è¡Œ
        set_drive(SPD_HIGH, SPD_HIGH);
        P1OUT &= ~(LAMP_ERR | LAMP_OK);
    } 
    else if(is_white_l && !is_white_r) {
        // å·¦ç™½ -> å�‘å�³ä¿®æ­£
        set_drive(SPD_HIGH, SPD_MID);
        P1OUT = (P1OUT & ~LAMP_OK) | LAMP_ERR;
    } 
    else if(!is_white_l && is_white_r) {
        // å�³ç™½ -> å�‘å·¦ä¿®æ­£
        set_drive(SPD_MID, SPD_HIGH);
        P1OUT = (P1OUT & ~LAMP_ERR) | LAMP_OK;
    } 
    else {
        // å�Œç™½ -> æ�œç´¢æ¨¡å¼�
        set_drive(SPD_LOW, SPD_LOW);
        P1OUT ^= (LAMP_ERR | LAMP_OK);
    }
}

// å�¯åŠ¨è‡ªæ£€åº�åˆ—
void check_boot(void) {
    sig_flash(LAMP_ERR, 2);
    sig_flash(LAMP_OK, 2);
    __delay_cycles(500000);

    update_adc();

    // ä¼ æ„Ÿå™¨æ•°å€¼æ£€æŸ¥
    if(g_adc_l > 50 && g_adc_r > 50) {
        sig_flash(LAMP_OK, 3);
    } else {
        unsigned char i;
        for(i=0; i<5; i++) {
            P1OUT |= LAMP_ERR;
            __delay_cycles(500000);
            P1OUT &= ~LAMP_ERR;
            __delay_cycles(500000);
        }
        // æ•…éšœæ­»å¾ªçŽ¯
        while(1) set_drive(0, 0);
    }
    __delay_cycles(1000000);
}

/* --- ä¸»ç¨‹åº�å…¥å�£ --- */
int main_test(void) {
    WDTCTL = WDTPW | WDTHOLD;

    init_all_hw();
    __enable_interrupt();
    
    check_boot();

    while(1) {
        update_adc();
        task_track();
        __delay_cycles(LOOP_DLY);
    }
}
