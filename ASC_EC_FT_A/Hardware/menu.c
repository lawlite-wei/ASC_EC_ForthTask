#include "stm32f10x.h"
#include "Key.h"
#include "OLED.h"
#include "Delay.h"
#include "AD.h"
#include "W25Q64.h"
#include "menu.h"
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

// 全局连接状态变量
static uint8_t global_connection_lost = 0;
static uint16_t global_no_data_counter = 0;

// 更新全局连接状态的函数
static void UpdateGlobalConnectionStatus(void)
{
    // 检查是否有新数据
    if (Serial_IsIMUDataReady())
    {
        uint16_t yaw, pitch, roll;
        Serial_GetIMUData(&yaw, &pitch, &roll);
        
        // 重置计数器
        global_no_data_counter = 0;
        global_connection_lost = 0;
    }
    else
    {
        // 增加无数据计数器
        global_no_data_counter++;
        
        // 如果连续100次没有收到数据（约2-4秒），判定为断开
        if (global_no_data_counter > 30)
        {
            global_connection_lost = 1;
        }
    }
}

// 在所有菜单中显示连接状态的函数
static void ShowConnectionStatus(void)
{
    if (global_connection_lost)
    {
        OLED_ShowString(112, 0, "-", OLED_8X16);  // 右上角显示"-"
    }
    else
    {
        OLED_ShowString(112, 0, "+", OLED_8X16);  // 右上角显示"+"
    }
}

int menu(void)
{
    if(!menu_initialized)
    {
        OLED_Clear();
        OLED_ShowString(8,0,"ADC",OLED_8X16);
        OLED_ShowString(8,16,"Store",OLED_8X16);
        OLED_ShowString(8,32,"IMU",OLED_8X16);
        OLED_Update();
        
        menu_flag = main_menu_last_selection;
        OLED_ShowString(0,0 + (menu_flag - 1) * 16,">",OLED_8X16);
        
        // 显示连接状态
        ShowConnectionStatus();
        
        OLED_Update();
        
        menu_initialized = 1;
        menu_last_flag = menu_flag;
    }
    
    // 更新全局连接状态
    UpdateGlobalConnectionStatus();
    
    if(Key_Check(KEY_1, KEY_DOWN))
    {
        menu_flag--;
        if(menu_flag == 0) menu_flag = 3;
    }
    
    if(Key_Check(KEY_2, KEY_DOWN))
    {
        menu_flag++;
        if(menu_flag == 4) menu_flag = 1;
    }
    
    if(Key_Check(KEY_3, KEY_DOWN))
    {
        main_menu_last_selection = menu_flag;
        menu_initialized = 0;
        return menu_flag;
    }
    
    if(menu_flag != menu_last_flag)
    {
        OLED_ShowString(0,0 + (menu_last_flag - 1) * 16," ",OLED_8X16);
        OLED_ShowString(0,0 + (menu_flag - 1) * 16,">",OLED_8X16);
        
        // 更新连接状态显示
        ShowConnectionStatus();
        
        OLED_Update();
        menu_last_flag = menu_flag;
    }
    else
    {
        // 定时更新连接状态（每10次调用更新一次）
        static uint8_t update_counter = 0;
        update_counter++;
        if (update_counter >= 10)
        {
            update_counter = 0;
            ShowConnectionStatus();
            OLED_Update();
        }
    }
    
    return 0;
}

int menu_ADC(void)
{
    static uint8_t update_counter = 0;
    
    if(!menu_ADC_initialized)
    {
        OLED_Clear();
        OLED_ShowString(0,16,"POT   val:",OLED_8X16);
        OLED_ShowString(0,32,"NTC   val:",OLED_8X16);
        OLED_ShowString(0,48,"LDR   val:",OLED_8X16);
        OLED_ShowNum(80,16,AD_Value[0],4,OLED_8X16);
        OLED_ShowNum(80,32,AD_Value[1],4,OLED_8X16);
        OLED_ShowNum(80,48,AD_Value[2],4,OLED_8X16);
        
        // 显示连接状态
        ShowConnectionStatus();
        
        OLED_Update();
        
        menu_ADC_initialized = 1;
        update_counter = 0;
    }
    
    // 更新全局连接状态
    UpdateGlobalConnectionStatus();
    
    // 定期更新显示（包括连接状态）
    update_counter++;
    if (update_counter >= 20)  // 每20次调用更新一次
    {
        update_counter = 0;
        
        // 更新ADC值
        OLED_ShowNum(80,16,AD_Value[0],4,OLED_8X16);
        OLED_ShowNum(80,32,AD_Value[1],4,OLED_8X16);
        OLED_ShowNum(80,48,AD_Value[2],4,OLED_8X16);
        
        // 更新连接状态
        ShowConnectionStatus();
        
        OLED_Update();
    }
    
    if(Key_Check(KEY_4, KEY_DOWN))
    {
        menu_ADC_initialized = 0;
        return 0;
    }
    
    return 1;
}

int menu_Store(void)
{
    static uint8_t update_counter = 0;
    
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
        
        // 显示连接状态
        ShowConnectionStatus();
        
        OLED_Update();
        
        menu_Store_initialized = 1;
        update_counter = 0;
    }
    
    // 更新全局连接状态
    UpdateGlobalConnectionStatus();
    
    // 定期更新连接状态
    update_counter++;
    if (update_counter >= 30)  // 每30次调用更新一次
    {
        update_counter = 0;
        ShowConnectionStatus();
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
        
        // 更新存储的值显示
        OLED_ShowNum(80,16,AD_Value[0],4,OLED_8X16);
        OLED_ShowNum(80,32,AD_Value[1],4,OLED_8X16);
        OLED_ShowNum(80,48,AD_Value[2],4,OLED_8X16);
        
        // 更新连接状态
        ShowConnectionStatus();
        
        OLED_Update();
    }
    
    if(Key_Check(KEY_1, KEY_DOWN))
    {
        OLED_ShowNum(80,16,0,4,OLED_8X16);
        OLED_ShowNum(80,32,0,4,OLED_8X16);
        OLED_ShowNum(80,48,0,4,OLED_8X16);
        
        // 更新连接状态
        ShowConnectionStatus();
        
        OLED_Update();
    }
    
    if(Key_Check(KEY_4, KEY_DOWN))
    {
        menu_Store_initialized = 0;
        return 0;
    }
    
    return 1;
}

int menu_IMU(void)
{
    static uint16_t last_yaw = 0, last_pitch = 0, last_roll = 0;
    static uint8_t data_received = 0;
    static uint8_t display_initialized = 0;
    static uint32_t last_success_time = 0;
    static uint8_t connection_lost = 0;  // 连接丢失标志
    static uint8_t delay_counter = 0;    // 延时计数器
    
    if(!menu_IMU_initialized)
    {
        OLED_Clear();
        OLED_ShowString(0,16,"Yaw:",OLED_8X16);
        OLED_ShowString(0,32,"Pitch:",OLED_8X16);
        OLED_ShowString(0,48,"Roll:",OLED_8X16);
        
        menu_IMU_initialized = 1;
        display_initialized = 0;
        data_received = 0;
        last_success_time = 0;
        connection_lost = 0;
        delay_counter = 0;
    }
    
    // 检查IMU数据是否就绪
    if (Serial_IsIMUDataReady())
    {
        uint16_t yaw, pitch, roll;
        Serial_GetIMUData(&yaw, &pitch, &roll);
        
        // 重置连接状态
        connection_lost = 0;
        last_success_time = 0;
        delay_counter = 0;
        
        // 只在数据变化时更新显示
        if (yaw != last_yaw || pitch != last_pitch || roll != last_roll)
        {
            int16_t yaw_value = (int16_t)yaw;
            int16_t pitch_value = (int16_t)pitch;
            int16_t roll_value = (int16_t)roll;
            
            OLED_ShowSignedNum(48, 16, yaw_value / 100, 4, OLED_8X16);
            OLED_ShowSignedNum(48, 32, pitch_value / 100, 4, OLED_8X16);
            OLED_ShowSignedNum(48, 48, roll_value / 100, 4, OLED_8X16);
            
            last_yaw = yaw;
            last_pitch = pitch;
            last_roll = roll;
            data_received = 1;
        }
        
        // 显示接收状态
        OLED_ShowString(100, 0, "+", OLED_8X16);
        OLED_Update();
    }
    else
    {
        // 延时计数器递增（降低检测频率）
        delay_counter++;
        if (delay_counter >= 5)  // 每5次调用检测一次
        {
            delay_counter = 0;
            
            // 如果之前有成功接收过数据
            if (data_received)
            {
                // 开始计时
                if (last_success_time == 0)
                {
                    last_success_time = 1;
                }
                else
                {
                    last_success_time++;
                    
                    // 如果超过50次没有收到数据（约2-3秒）
                    if (last_success_time > 10 && !connection_lost)
                    {
                        connection_lost = 1;
                    }
                }
            }
        }
        
        // 根据连接状态显示
        if (connection_lost)
        {
            OLED_ShowString(100, 0, "-", OLED_8X16);
        }
        else
        {
            OLED_ShowString(100, 0, "+", OLED_8X16);
        }
        
        // 如果没有收到过数据，显示0
        if (!data_received && !display_initialized)
        {
            OLED_ShowSignedNum(48, 16, 0, 4, OLED_8X16);
            OLED_ShowSignedNum(48, 32, 0, 4, OLED_8X16);
            OLED_ShowSignedNum(48, 48, 0, 4, OLED_8X16);
            OLED_Update();
            display_initialized = 1;
        }
        else
        {
            OLED_Update();
        }
    }
    
    if(Key_Check(KEY_4, KEY_DOWN))
    {
        menu_IMU_initialized = 0;
        return 0;
    }
    
    return 1;
}
