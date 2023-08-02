#ifndef _EEPROM_H_
#define _EEPROM_H_
#include "hc32f460.h"

uint32_t        getFlashDat(void);
void            saveFlashData(uint32_t);
uint32_t        getKeyFromFlash(void);
void            saveKeyToFlash(uint32_t dat);
uint32_t        getSysMinuteFromFlash(void);
void            saveSysMinuteToFlash(uint32_t dat);
extern uint32_t motorRunMinute;
extern uint32_t sysRunMinute;
#endif
