#ifndef __MENU_H
#define __MENU_H

#include "stm32f10x.h"

extern uint8_t main_menu_last_selection;

int menu(void);
int menu_ADC(void);
int menu_Store(void);
int menu_IMU(void);


#endif
