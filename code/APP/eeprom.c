#include "eeprom.h"
#include "hc32f460_efm.h"
/* Exported constants --------------------------------------------------------*/
/*
容量高达 256 KBytes(其中有 32bytes 为功能保留位)
分为 32 个扇区，每个扇区为 8KBytes。
编程单位为 4bytes，擦除单位为 8Kbytes
分为sector0-sector31
sector31 最高32个byte为功能保留地址，数据无法修改所以不使用此区域
存储参数-8k-SECTOR30
0x0003C000 – 0x0003DFFF Sector30
存储运行时间 8K - SECTOR29
0x0003A000 – 0x0003BFFF Sector29
存储上电时间 8K -SECTOR28
0x00038000 – 0x00039FFF Sector28

//OTP区域 1024个Byte [960个数据区，60个锁存区]
0x03000C00 - 0x03000FFB
*/
/**
 *****************************************************************************
 ** \brief FLASH 字写
 **
 ** 用于向FLASH写入1个字的数据.
 **
 ** \param [in]  u32Addr         Flash地址
 ** \param [in]  u32Data         1个字数据
 **
 ** \retval Ok                    写入成功.
 ** \retval ErrorInvalidParameter FLASH地址无效
 ** \retval ErrorTimeout          操作超时
 *****************************************************************************/
#define FLASH_END_ADDR              (0x0003DFFFu)
#define FLASH_TIMEOUT_PGM           (0xFFFu)
en_result_t Flash_WriteWordMulti(uint32_t u32Addr, uint32_t pBuf[], uint32_t u32Len) {
  EFM_Unlock();
  EFM_FlashCmd(Enable);
  for(uint8_t i = 0; i < u32Len; i++) {
    EFM_SingleProgramRB(u32Addr, pBuf[i]);
    u32Addr += 4;
  }
  EFM_Lock();
  return 0;
}


//extern SYSDatDef sysDat;
#define PARAM_START_ADDRESS 0x0003C000
#define PARAM_END_ADDRESS 0x0003DFFF
#define PARAM_SPACE 0x2000

#define PARAM_MAX_ADDRESS ((uint32_t)PARAM_START_ADDRESS + ((uint32_t)PARAM_SPACE / (uint32_t)(4))*(uint32_t)(4))

uint32_t paramIndexAddr = PARAM_START_ADDRESS;//指向内存空的地址
uint32_t getFlashDat() {
  uint32_t dat;
  if(paramIndexAddr == PARAM_START_ADDRESS) {
    while(*((uint32_t*)(paramIndexAddr)) != 0xffffffff) {
      paramIndexAddr += (4);//看看下一段数据是否为空
      if(paramIndexAddr >= PARAM_MAX_ADDRESS) {
        paramIndexAddr = PARAM_MAX_ADDRESS;
        break;
      }
    }
    if(paramIndexAddr > PARAM_START_ADDRESS) {
      paramIndexAddr -= 4;
    }
  }
  dat = *((uint32_t*)((paramIndexAddr)));
  return dat;
}
/*
 *保存 累积的 运行时间
*/
void saveFlashData(uint32_t dat) {
  while(true) {
    if(paramIndexAddr >= PARAM_MAX_ADDRESS) { //到最大地址就不能写了
      paramIndexAddr = PARAM_START_ADDRESS;
      EFM_Unlock();
      EFM_FlashCmd(Enable);
      EFM_SectorErase(PARAM_START_ADDRESS);//擦除两页
      EFM_Lock();
    }

    if(*((uint32_t*)(paramIndexAddr)) == dat) {
      return;
    } else {
      if(0xffffffff == (*((uint32_t*)paramIndexAddr))) {
        Flash_WriteWordMulti(paramIndexAddr, &dat, 1);
      } else {
        paramIndexAddr += 4;
      }
    }
  }
}
#define RUN_START_ADDRESS 0x0003A000
#define RUN_END_ADDRESS 0x0003BFFF
#define RUN_SPACE 0x2000
#define RUN_MAX_ADDRESS ((uint32_t)RUN_START_ADDRESS + ((uint32_t)RUN_SPACE / (uint32_t)4)*(uint32_t)4)
uint32_t runIndexAddr = RUN_START_ADDRESS;
uint32_t motorRunMinute = 0;
uint32_t getKeyFromFlash() {
  //读取这一页所有直到未存储区域
  uint32_t dat;
  if(runIndexAddr == RUN_START_ADDRESS) {
    while(0xffffffff != (*((uint32_t*)runIndexAddr)) && runIndexAddr < RUN_MAX_ADDRESS) {
      runIndexAddr += 4;
      if(runIndexAddr >= RUN_MAX_ADDRESS) {
        runIndexAddr = RUN_MAX_ADDRESS;
        break;
      }
    }
    if(runIndexAddr > RUN_START_ADDRESS) {
      runIndexAddr -= 4;
    }
  }
  dat = *((uint32_t*)(runIndexAddr));
  if(dat == 0xffffffff)dat = 0;
  return dat;
}
/*
 * 保存重启的次数和原因
*/
void saveKeyToFlash(uint32_t dat) {
  while(true) {
    if(runIndexAddr >= RUN_END_ADDRESS) {
      EFM_Unlock();
      EFM_FlashCmd(Enable);
      EFM_SectorErase(RUN_START_ADDRESS);
      EFM_Lock();
      runIndexAddr = RUN_START_ADDRESS;
    }
    if(0xffffffff == (*((uint32_t*)runIndexAddr))) {
      Flash_WriteWordMulti(runIndexAddr, &dat, 1);
    } else {
      runIndexAddr += 4;
    }
    if(runIndexAddr < RUN_END_ADDRESS) {
      if((*((uint32_t*)runIndexAddr)) == dat)
        break;
    }
  }
}
#define SYS_START_ADDRESS 0x00038000
#define SYS_END_ADDRESS 0x00039FFF
#define SYS_SPACE 0x2000
#define SYS_MAX_ADDRESS ((uint32_t)SYS_START_ADDRESS + ((uint32_t)SYS_SPACE / (uint32_t)4)*(uint32_t)4)
uint32_t sysIndexAddr = SYS_START_ADDRESS;
uint32_t sysRunMinute = 0;
uint32_t getSysMinuteFromFlash() {
  //读取这一页所有直到未存储区域
  uint32_t dat;
  if(sysIndexAddr == SYS_START_ADDRESS) {
    while(0xffffffff != (*((uint32_t*)sysIndexAddr)) && sysIndexAddr < SYS_MAX_ADDRESS) {
      sysIndexAddr += 4;
      if(sysIndexAddr >= SYS_MAX_ADDRESS) {
        sysIndexAddr = SYS_MAX_ADDRESS;
        break;
      }
    }
    if(sysIndexAddr > SYS_START_ADDRESS) {
      sysIndexAddr -= 4;
    }
  }
  dat = *((uint32_t*)(sysIndexAddr));
  if(dat == 0xffffffff)dat = 0;
  return dat;
}
/*
 *保存 累计的 上电时间
*/
void saveSysMinuteToFlash(uint32_t dat) {
  while(true) {
    if(sysIndexAddr >= SYS_MAX_ADDRESS) {
      EFM_Unlock();
      EFM_FlashCmd(Enable);
      EFM_SectorErase(SYS_START_ADDRESS);
      EFM_Lock();
      sysIndexAddr = SYS_START_ADDRESS;
    }
    if(0xffffffff == (*((uint32_t*)sysIndexAddr))) {
      Flash_WriteWordMulti(sysIndexAddr, &sysRunMinute, 1);
    } else {
      sysIndexAddr += 4;
    }
    if(sysIndexAddr < SYS_MAX_ADDRESS) {
      if((*((uint32_t*)sysIndexAddr)) == dat)
        break;
    }
  }

}
