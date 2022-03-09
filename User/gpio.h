#ifndef __GPIO_H_
#define __GPIO_H_
#include "stm32f10x.h"

#define blue 1
#define green 2
#define red 3
/*1：左负右正 2：左正右负 3;停*/
#define zhengz 1
#define fanz 2
#define stop 3
#define LED_ve PEout(6)
#define BEE PDout(7)
void GpioInit(void);
void light(int num);
void windows(int nums);
void fan_control(int nums);
void blind_control(int nums);
void light_control(int nums);
#endif
