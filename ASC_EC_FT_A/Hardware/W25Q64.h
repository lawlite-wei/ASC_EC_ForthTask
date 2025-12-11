#ifndef __W25Q64_H
#define __W25Q64_H

extern uint16_t W25Q64_Read[3];

void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q64_PageProgram(uint32_t Address, uint16_t *DataArray, uint16_t Count);
void W25Q64_SectorErase(uint32_t Address);
void W25Q64_ReadData(uint32_t Address, uint16_t *DataArray, uint32_t Count);

#endif
