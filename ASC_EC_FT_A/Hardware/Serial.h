#ifndef __SERIAL_H 
#define __SERIAL_H

#include <stdio.h>


extern uint8_t Serial_RxPacket[];
extern uint8_t Serial_RxFlag;
extern int16_t Serial_bright;  

extern uint8_t Serial_RxPacket2[];
extern uint8_t Serial_RxFlag2;
extern int16_t Serial_time;  

extern char Serial_RxPacket3[];
extern uint8_t Serial_RxFlag3;

#define SERIAL_RX_PACKET_SIZE 64   // 定义缓冲区大小


void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number,uint8_t Length);
void Serial_Printf(char *format,  ...);

uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxFlag2(void);
uint8_t Serial_GetRxFlag3(void);

#endif
