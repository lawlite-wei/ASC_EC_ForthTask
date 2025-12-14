#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "menu.h"
#include "Key.h"
#include "AD.h"
#include "Timer.h"
#include "Serial.h"
#include "W25Q64.h"

typedef enum {
    MENU,
    MENU_ADC,
    MENU_Store,
    MENU_IMU,
} MenuState;

MenuState current_menu = MENU;

int main(void)
{
	OLED_Init();
	Key_Init();
	Timer_Init();
	AD_Init();
	W25Q64_Init();
	Serial_Init();
	
	while (1)
	{
		switch(current_menu) {
            case MENU: {
                int result = menu();
                if(result > 0) {
                    current_menu = result;  // 切换到子菜单
                }
                break;
            }
            case MENU_ADC: {
                int result = menu_ADC();
                if(result == 0) {
                    current_menu = MENU;  // 返回主菜单
                }
                break;
            }
            case MENU_Store: {
                int result = menu_Store();
                if(result == 0) {
                    current_menu = MENU;
                }
                break;
            }
            case MENU_IMU: {
                int result = menu_IMU();
                if(result == 0) {
                    current_menu = MENU;
                }
                break;
            }
        }
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Tick();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
