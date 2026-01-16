#ifndef PHARE_H_
#define PHARE_H_

#include <msp430.h>

#define CANAL_PHARE INCH_2      // 光敏电阻连接在 P1.2 (ADC通道2)
#define ADC_AE_BIT  BIT2       // 对应的模拟启用位 (P1.2)
#define M_A_DIR     BIT1        // 左电机方向
#define M_B_DIR     BIT5        // 右电机方向

#define SCAN_SPEED 450     // 旋转扫描时的速度 (0-1000)
#define FULL_TURN_STEPS 55     // 转满一圈所需的采样次数 (实测后在此修改)

extern void set_drive(int v_l, int v_r);

void init_phare(void);
int lire_phare(void);
void scan_maximum_phare(void);

#endif
