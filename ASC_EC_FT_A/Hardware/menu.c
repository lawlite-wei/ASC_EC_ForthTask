#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "OLED.h"
#include "Delay.h"

static uint8_t menu_flag = 1;
static uint8_t menu_last_flag = 0;
static uint8_t menu_initialized = 0;
uint8_t main_menu_last_selection = 1;

int menu(void)
{
	if(!menu_initialized) {
        // 初始显示菜单
        OLED_Clear();
        OLED_ShowString(8,0,"ADC",OLED_8X16);
		OLED_ShowString(8,16,"Store",OLED_8X16);
		OLED_ShowString(8,32,"IMU",OLED_8X16);
		OLED_Update();
        
        // 使用上次保存的选择位置
        menu_flag = main_menu_last_selection;
        OLED_ShowString(0,0 + (menu_flag - 1) * 16,">",OLED_8X16);
		OLED_Update();
        menu_initialized = 1;
        menu_last_flag = menu_flag;
    }
	
	if(Key_Check(KEY_1, KEY_DOWN)) {
        menu_flag--;
        if(menu_flag == 0) menu_flag = 3;
    }
    if(Key_Check(KEY_2, KEY_DOWN)) {
        menu_flag++;
        if(menu_flag == 4) menu_flag = 1;
    }
    if(Key_Check(KEY_3, KEY_DOWN)) {
        // 保存当前选择并进入子菜单
        main_menu_last_selection = menu_flag;
        menu_initialized = 0;
        return menu_flag;
    }
}
