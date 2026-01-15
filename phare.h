#ifndef PHARE_H_
#define PHARE_H_

#include <msp430.h>

#define CANAL_PHARE INCH_2      // 光敏电阻连接在 P1.2 (ADC通道2)
#define ADC_AE_BIT  BIT2        // 对应的模拟启用位 [cite: 14]

#define SCAN_SPEED 450     // 旋转扫描时的速度 (0-1000)
#define FULL_TURN_STEPS 80     // 旋转一圈大约需要的采样步数 (需实测)

extern void motor_control(int left_speed, int right_speed);

void init_phare(void);         // 初始化引脚
int lire_phare(void);          // 读取实时光强 (0-1023)
void scan_maximum_phare(void); // 核心算法：转一圈找最亮的方向

#endif
