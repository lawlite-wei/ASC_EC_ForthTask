#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define SERIAL_RX_PACKET_SIZE 64

// 数据包1：亮度控制，格式：@数据%
char Serial_RxPacket[100];
uint8_t Serial_RxFlag = 0;
int16_t Serial_bright = 0;

// 数据包2：时间控制，格式：#数据%
char Serial_RxPacket2[100];
uint8_t Serial_RxFlag2 = 0;
int16_t Serial_time = 0;

//数据包3：led控制，格式：&led%
char Serial_RxPacket3[100];
uint8_t Serial_RxFlag3 = 0;

void Serial_Init(void)
{
	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	
	//初始化GPIO引脚(Tx)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;       //初始化Tx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       //初始化Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//初始化USART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;                                        //波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //流控
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                   //模式
	USART_InitStructure.USART_Parity = USART_Parity_No;                               //校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                            //停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                       //字长
	USART_Init(USART1,&USART_InitStructure);
	
	//开启中断
	USART_ITConfig(USART1, USART_IT_RXNE,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);    //不需要手动清零
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for(i = 0; i < Length ; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for(i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while(Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number,uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10,Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format,  ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

// 统一的串口中断处理函数
void USART1_IRQHandler(void)
{
    static uint8_t RxState = 0;
    static uint8_t pRxPacket = 0;
    static uint8_t pRxPacket2 = 0;
    static uint8_t pRxPacket3 = 0;
    static uint8_t CurrentPacketType = 0;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);
        
        switch(RxState)
        {
            case 0: // 等待包头
                if (RxData == '@' && Serial_RxFlag == 0)
                {
                    RxState = 1;
                    pRxPacket = 0;
                    CurrentPacketType = 1;
                }
                else if (RxData == '#' && Serial_RxFlag2 == 0)
                {
                    RxState = 1;
                    pRxPacket2 = 0;
                    CurrentPacketType = 2;
                }
                else if (RxData == '&' && Serial_RxFlag3 == 0)
                {
                    RxState = 1;
                    pRxPacket3 = 0;
                    CurrentPacketType = 3;
                    // 清空缓冲区
                    memset(Serial_RxPacket3, 0, sizeof(Serial_RxPacket3));
                }
                break;
                
            case 1: // 接收数据内容
                if (RxData == '%')
                {
                    RxState = 2; // 转到等待包尾状态
                }
                else
                {
                    if (CurrentPacketType == 1)
                    {
                        if (pRxPacket < sizeof(Serial_RxPacket) - 1)
                            Serial_RxPacket[pRxPacket++] = RxData;
                    }
                    else if (CurrentPacketType == 2)
                    {
                        if (pRxPacket2 < sizeof(Serial_RxPacket2) - 1)
                            Serial_RxPacket2[pRxPacket2++] = RxData;
                    }
                    else if (CurrentPacketType == 3)
                    {
                        if (pRxPacket3 < sizeof(Serial_RxPacket3) - 1)
                            Serial_RxPacket3[pRxPacket3++] = RxData;
                    }
                }
                break;
                
            case 2: // 等待包尾
                if(RxData == '\n')
                {
                    RxState = 0;
                    
                    if (CurrentPacketType == 1)
                    {
                        Serial_RxPacket[pRxPacket] = '\0';
                        Serial_bright = atoi((char*)Serial_RxPacket);
                        Serial_RxFlag = 1;
                    }
                    else if (CurrentPacketType == 2)
                    {
                        Serial_RxPacket2[pRxPacket2] = '\0';
                        Serial_time = atoi((char*)Serial_RxPacket2);
                        Serial_RxFlag2 = 1;
                    }
                    else if (CurrentPacketType == 3)
                    {
                        Serial_RxPacket3[pRxPacket3] = '\0';
                        Serial_RxFlag3 = 1;
                        // 调试：可以通过串口输出接收到的内容
                        // Serial_SendString("Received: ");
                        // Serial_SendString(Serial_RxPacket3);
                        // Serial_SendString("\r\n");
                    }
                }
                break;
        }
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// 获取数据包1标志
uint8_t Serial_GetRxFlag(void)
{
	if(Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

// 获取数据包2标志
uint8_t Serial_GetRxFlag2(void)
{
	if(Serial_RxFlag2 == 1)
	{
		Serial_RxFlag2 = 0;
		return 1;
	}
	return 0;
}

uint8_t Serial_GetRxFlag3(void)
{
	if(Serial_RxFlag3 == 1)
	{
		Serial_RxFlag3 = 0;
		return 1;
	}
	return 0;
}
