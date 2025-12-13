#include "stm32f10x.h"                  // Device header
#include "MySPI.h"
#include "W25Q64_Ins.h"

uint16_t W25Q64_Read[3];

void W25Q64_Init(void)
{
	MySPI_Init();
}

void W25Q64_ReadID(uint8_t *MID, uint16_t *DID)
{
	MySPI_Start();
	MySPI_SwapByte(W25Q64_JEDEC_ID);
	*MID = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	*DID = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	*DID <<= 8;
	*DID |= MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	MySPI_Stop();
}

void W25Q64_WriteEnable(void)
{
	MySPI_Start();
	MySPI_SwapByte(W25Q64_WRITE_ENABLE);
	MySPI_Stop();
}

void W25Q64_WaitBusy(void)
{
	uint32_t Timeout;
	MySPI_Start();
	MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	Timeout = 100000;
	while ((MySPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		Timeout --;
		if (Timeout == 0)
		{
			break;
		}
	}
	MySPI_Stop();
}

void W25Q64_PageProgram(uint32_t Address, uint16_t *DataArray, uint16_t Count)
{
    uint16_t i;
    
    W25Q64_WriteEnable();
    
    MySPI_Start();
    MySPI_SwapByte(W25Q64_PAGE_PROGRAM);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);
    for (i = 0; i < Count; i++)
    {
        // 发送16位数据，先高字节后低字节
        MySPI_SwapByte((DataArray[i] >> 8) & 0xFF);  // 高字节
        MySPI_SwapByte(DataArray[i] & 0xFF);         // 低字节
    }
    MySPI_Stop();
    
    W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t Address)
{
	W25Q64_WriteEnable();
	
	MySPI_Start();
	MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	MySPI_SwapByte(Address >> 16);
	MySPI_SwapByte(Address >> 8);
	MySPI_SwapByte(Address);
	MySPI_Stop();
	
	W25Q64_WaitBusy();
}

void W25Q64_ReadData(uint32_t Address, uint16_t *DataArray, uint32_t Count)
{
    uint32_t i;
    MySPI_Start();
    MySPI_SwapByte(W25Q64_READ_DATA);
    MySPI_SwapByte(Address >> 16);
    MySPI_SwapByte(Address >> 8);
    MySPI_SwapByte(Address);
    for (i = 0; i < Count; i++)
    {
        // 读取16位数据，先高字节后低字节
        DataArray[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE) << 8;  // 高字节
        DataArray[i] |= MySPI_SwapByte(W25Q64_DUMMY_BYTE);      // 低字节
    }
    MySPI_Stop();
}
