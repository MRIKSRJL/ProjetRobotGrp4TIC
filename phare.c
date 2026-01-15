#include "phare.h"
#include "ADC.h"

void init_phare(void) {
    // 启用 P1.2 的模拟输入功能
    ADC10AE0 |= ADC_AE_BIT;
}

// 读取实时光强值
int lire_phare(void) {
    // 停止转换以安全更改通道
        ADC10CTL0 &= ~ENC;
        while(ADC10CTL1 & ADC10BUSY);

        // 设置为通道 2 (A2)
        ADC10CTL1 = CANAL_PHARE | ADC10DIV_3;

        // 开启转换并触发
        ADC10CTL0 |= ENC | ADC10SC;
        while(ADC10CTL1 & ADC10BUSY);

        return ADC10MEM; //
}
void scan_maximum_phare(void) {
    int current_val = 0;
    int max_val = 0;
    int best_step = 0;
    int i = 0;

    // --- 第一阶段：开始原地旋转扫描 ---
    // 调用组员的函数：左轮正转，右轮不转（实现原地绕轴转动）[cite: 20, 21]
    motor_control(SCAN_SPEED, 0);

    for (i = 0; i < FULL_TURN_STEPS; i++) {
        current_val = lire_phare();

        if (current_val > max_val) {
            max_val = current_val;
            best_step = i; // 记录最亮的时刻
        }
        // 采样延时，必须保持恒定以便后续回转对齐 [cite: 6]
        __delay_cycles(50000);
    }

    // --- 第二阶段：短暂停顿，消除惯性 ---
    motor_control(0, 0);
    __delay_cycles(1000000); // 停 1 秒

    // --- 第三阶段：精准旋转回 best_step ---
    motor_control(SCAN_SPEED, 0);
    for (i = 0; i < best_step; i++) {
        __delay_cycles(50000); // 使用完全一致的延时周期
    }

    // --- 第四阶段：到达灯塔方向，停车等待指令 ---
    motor_control(0, 0);
}





