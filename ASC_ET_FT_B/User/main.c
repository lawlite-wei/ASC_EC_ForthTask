#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "PWM.h"
#include "Servo.h"
#include "Timer.h"
#include "MPU6050.h"
#include <math.h>

int16_t AX, AY, AZ, GX, GY, GZ;

float RollAcc;    		// 加速度计计算的横滚角
float RollGyro;   		// 陀螺仪积分的横滚角
float Roll;       		// 融合后的横滚角

float Yaw = 0;			//偏航角
float Angle;

float PitchAcc;			//加速度计算的俯仰角
float PitchGyro;		//陀螺仪积分的俯仰角
float Pitch;			//融合后的俯仰角

uint8_t TimeErrorFlag;

int main(void)
{ 
	LED_Init();
    OLED_Init();     
    Timer_Init();
    Servo_Init();
    MPU6050_Init();
	
    
    OLED_Clear();
	
    while (1)
    {
        OLED_Clear();
		OLED_ShowString(0,0,"Yaw",OLED_8X16);
		OLED_ShowString(0,16,"Pitch",OLED_8X16);
		OLED_ShowString(0,32,"Roll",OLED_8X16);
		OLED_ShowFloatNum(64,0,Yaw,3,3,OLED_8X16);
		OLED_ShowFloatNum(64,16,Roll,3,3,OLED_8X16);
		OLED_ShowFloatNum(64,32,Pitch,3,3,OLED_8X16);
		OLED_Update();
		
		Angle = Yaw;
		Servo_SetAngle(Angle);
		
		if(Angle >= 150 && Angle <= 180)
		{
			LED1_ON();
			Delay_ms(20);
			LED1_OFF();
			Delay_ms(20);
		}
    }	
}


void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
		
		//校准零飘
		GX += 55;
		GY += 18;
		GZ += 10;
	
		 // 横滚角计算
        RollAcc = atan2(AY, AZ) / 3.14159 * 180;  				// 横滚角（绕X轴）
        RollGyro = Roll + GX / 32768.0 * 2000 * 0.001;  		
        Roll = 0.001 * RollAcc + (1 - 0.001) * RollGyro;  		
		
		 // 偏航角：仅陀螺仪积分（无加速度计校准，会漂移）
        Yaw += GZ / 32768.0 * 2000 * 0.001;  
		
		// 俯仰角计算
		PitchAcc = -atan2(AX, AZ) / 3.14159 * 180;  			// 俯仰角（绕Y轴）
        PitchGyro = Pitch + GY / 32768.0 * 2000 * 0.001;  		
        Pitch = 0.001 * PitchAcc + (1 - 0.001) * PitchGyro;  	
		
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
		{
			TimeErrorFlag = 1;
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		}
	}
}
