#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "OLED.h"
#include "Delay.h"
#include "AD.h"
#include "W25Q64.h"
#include "Serial.h"

static uint8_t menu_flag = 1;
static uint8_t menu_last_flag = 0;
static uint8_t menu_initialized = 0;
uint8_t main_menu_last_selection = 1;

static uint8_t menu_ADC_initialized = 0;

static uint8_t menu_Store_initialized = 0;

static uint8_t menu_IMU_initialized = 0;

uint8_t Byte_Buf_Write[6];
uint8_t Byte_Buf_Read[6];
uint16_t W25Q64_Read[3];

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
	
	if(menu_flag != menu_last_flag) {
        OLED_ShowString(0,0 + (menu_last_flag - 1) * 16," ",OLED_8X16);
        OLED_ShowString(0,0 + (menu_flag - 1) * 16,">",OLED_8X16);
		OLED_Update();
        menu_last_flag = menu_flag;
    }
	
    return 0;
}

int menu_ADC(void)
{
	if(!menu_ADC_initialized)
	{
		OLED_Clear();
		OLED_ShowString(0,16,"POT   val:",OLED_8X16);
		OLED_ShowString(0,32,"NTC   val:",OLED_8X16);
		OLED_ShowString(0,48,"LDR   val:",OLED_8X16);
		OLED_ShowNum(80,16,AD_Value[0],4,OLED_8X16);
		OLED_ShowNum(80,32,AD_Value[1],4,OLED_8X16);
		OLED_ShowNum(80,48,AD_Value[2],4,OLED_8X16);
		OLED_Update();
	}
	
	if(Key_Check(KEY_4, KEY_DOWN)) {
        menu_ADC_initialized = 0;
        return 0;  // 返回主菜单
    }
	
	return 1;
}

int menu_Store(void)
{
	if(!menu_Store_initialized)
	{
		OLED_Clear();
		OLED_ShowString(0,16,"POT   val:",OLED_8X16);
		OLED_ShowString(0,32,"NTC   val:",OLED_8X16);
		OLED_ShowString(0,48,"LDR   val:",OLED_8X16);
		
		
		W25Q64_ReadData(0x000000, Byte_Buf_Read, 6);
		W25Q64_Read[0] = (uint16_t)Byte_Buf_Read[1] << 8 | Byte_Buf_Read[0];
		W25Q64_Read[1] = (uint16_t)Byte_Buf_Read[3] << 8 | Byte_Buf_Read[2];
		W25Q64_Read[2] = (uint16_t)Byte_Buf_Read[5] << 8 | Byte_Buf_Read[4];
		OLED_ShowNum(80,16,W25Q64_Read[0],4,OLED_8X16);
		OLED_ShowNum(80,32,W25Q64_Read[1],4,OLED_8X16);
		OLED_ShowNum(80,48,W25Q64_Read[2],4,OLED_8X16);
		OLED_Update();
	}
	
	if(Key_Check(KEY_3, KEY_REPEAT))
	{
		Byte_Buf_Write[0] = AD_Value[0] & 0xFF;        
		Byte_Buf_Write[1] = (AD_Value[0] >> 8) & 0xFF; 
		Byte_Buf_Write[2] = AD_Value[1] & 0xFF;        
		Byte_Buf_Write[3] = (AD_Value[1] >> 8) & 0xFF; 
		Byte_Buf_Write[4] = AD_Value[2] & 0xFF;        
		Byte_Buf_Write[5] = (AD_Value[2] >> 8) & 0xFF; 
				
		W25Q64_SectorErase(0x000000);					
		W25Q64_PageProgram(0x000000, Byte_Buf_Write, 6);
	}
	
	if(Key_Check(KEY_1, KEY_DOWN))
	{
		OLED_ShowNum(80,16,0,4,OLED_8X16);
        OLED_ShowNum(80,32,0,4,OLED_8X16);
        OLED_ShowNum(80,48,0,4,OLED_8X16);
        OLED_Update();
	}
	
	if(Key_Check(KEY_4, KEY_DOWN)) {
        menu_Store_initialized = 0;
        return 0;  // 返回主菜单
    }
	
	return 1;
}

int menu_IMU(void)
{
	if(!menu_IMU_initialized)
	{
		OLED_Clear();
		OLED_ShowString(0,16,"Pitch:",OLED_8X16);
		OLED_ShowString(0,32,"Roll:",OLED_8X16);
		OLED_ShowString(0,48,"Yaw:",OLED_8X16);
		OLED_Update();
	}
	
	if(Key_Check(KEY_4, KEY_DOWN)) {
        menu_IMU_initialized = 0;
        return 0;  // 返回主菜单
    }
	
	return 1;
}

//int USART_flag(void)
//{
//	OLED_ShowString(100, 0, "-",OLED_8X16);
//	OLED_Update();
//	
//	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
//	{
//		
//		OLED_ShowString(100, 0, "+",OLED_8X16);
//		OLED_Update();
//	}
//}
