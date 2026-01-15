#include <msp430g2553.h>


#define REF_VAL         512     
#define SPD_HIGH        650     
#define SPD_MID         350     
#define SPD_LOW         250     
#define LOOP_DLY        10000

/* --- 硬件引脚映射 --- */
#define ADC_CH_L        INCH_4
#define ADC_CH_R        INCH_5

#define M_A_DIR         BIT1
#define M_A_PWM         BIT2
#define M_B_PWM         BIT4
#define M_B_DIR         BIT5

#define LAMP_ERR        BIT0    
#define LAMP_OK         BIT6    

/* --- 全局变量 --- */
volatile unsigned int g_adc_l = 0;
volatile unsigned int g_adc_r = 0;

/* --- 辅助功能函数 --- */

// 延时闪烁通用函数
void sig_flash(unsigned char mask, unsigned char loop_cnt) {
    unsigned char k;
    for(k = 0; k < loop_cnt; k++) {
        P1OUT |= mask;
        __delay_cycles(200000);
        P1OUT &= ~mask;
        __delay_cycles(200000);
    }
}

// 硬件初始化统一入口
void init_all_hw(void) {
    // 时钟配置
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // GPIO & LED
    P1SEL |= (BIT4 | BIT5);
    P1SEL2 |= (BIT4 | BIT5);
    P1DIR |= (LAMP_ERR | LAMP_OK);
    P1OUT &= ~(LAMP_ERR | LAMP_OK);

    // 电机端口
    P2DIR |= (M_A_DIR | M_B_DIR | M_A_PWM | M_B_PWM);
    P2OUT &= ~(M_A_DIR | M_B_DIR);
    P2SEL |= (M_A_PWM | M_B_PWM);

    // 定时器 PWM
    TA1CTL = TASSEL_2 | MC_1 | TACLR;
    TA1CCR0 = 1000;
    TA1CCTL1 = OUTMOD_7; 
    TA1CCR1 = 0;
    TA1CCTL2 = OUTMOD_7; 
    TA1CCR2 = 0;

    // ADC 配置
    ADC10CTL0 = ADC10SHT_2 | ADC10ON;
    ADC10CTL1 = ADC_CH_L | ADC10DIV_3;
    ADC10AE0 = BIT4 | BIT5;
}

// 采集传感器数据
void update_adc(void) {
    // 采样左通道
    ADC10CTL0 &= ~ENC;
    while(ADC10CTL1 & ADC10BUSY);
    ADC10CTL1 = ADC_CH_L | ADC10DIV_3;
    ADC10CTL0 |= (ENC | ADC10SC);
    while(ADC10CTL1 & ADC10BUSY);
    g_adc_l = ADC10MEM;

    __delay_cycles(100);

    // 采样右通道
    ADC10CTL0 &= ~ENC;
    while(ADC10CTL1 & ADC10BUSY);
    ADC10CTL1 = ADC_CH_R | ADC10DIV_3;
    ADC10CTL0 |= (ENC | ADC10SC);
    while(ADC10CTL1 & ADC10BUSY);
    g_adc_r = ADC10MEM;
}

// 电机驱动接口
void set_drive(int v_l, int v_r) {
    // 限幅保护
    if(v_l > 1000) v_l = 1000; else if(v_l < 0) v_l = 0;
    if(v_r > 1000) v_r = 1000; else if(v_r < 0) v_r = 0;

    // 设置方向与PWM
    P2OUT |= ~M_A_DIR;  // 维持原逻辑
    TA1CCR1 = v_l;
    P2OUT &= M_B_DIR;   // 维持原逻辑
    TA1CCR2 = v_r;
}

// 路径追踪逻辑
void task_track(void) {
    unsigned char is_white_l = (g_adc_l > REF_VAL);
    unsigned char is_white_r = (g_adc_r > REF_VAL);

    // 状态机处理
    if(!is_white_l && !is_white_r) {
        // 双黑 -> 直行
        set_drive(SPD_HIGH, SPD_HIGH);
        P1OUT &= ~(LAMP_ERR | LAMP_OK);
    } 
    else if(is_white_l && !is_white_r) {
        // 左白 -> 向右修正
        set_drive(SPD_HIGH, SPD_MID);
        P1OUT = (P1OUT & ~LAMP_OK) | LAMP_ERR;
    } 
    else if(!is_white_l && is_white_r) {
        // 右白 -> 向左修正
        set_drive(SPD_MID, SPD_HIGH);
        P1OUT = (P1OUT & ~LAMP_ERR) | LAMP_OK;
    } 
    else {
        // 双白 -> 搜索模式
        set_drive(SPD_LOW, SPD_LOW);
        P1OUT ^= (LAMP_ERR | LAMP_OK);
    }
}

// 启动自检序列
void check_boot(void) {
    sig_flash(LAMP_ERR, 2);
    sig_flash(LAMP_OK, 2);
    __delay_cycles(500000);

    update_adc();

    // 传感器数值检查
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
        // 故障死循环
        while(1) set_drive(0, 0);
    }
    __delay_cycles(1000000);
}

/* --- 主程序入口 --- */
int main(void) {
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