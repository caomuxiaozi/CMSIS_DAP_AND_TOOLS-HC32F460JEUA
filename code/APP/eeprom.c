#include "eeprom.h"
#include "hc32f460_efm.h"
/* Exported constants --------------------------------------------------------*/
/*
�����ߴ� 256 KBytes(������ 32bytes Ϊ���ܱ���λ)
��Ϊ 32 ��������ÿ������Ϊ 8KBytes��
��̵�λΪ 4bytes��������λΪ 8Kbytes
��Ϊsector0-sector31
sector31 ���32��byteΪ���ܱ�����ַ�������޷��޸����Բ�ʹ�ô�����
�洢����-8k-SECTOR30
0x0003C000 �C 0x0003DFFF Sector30
�洢����ʱ�� 8K - SECTOR29
0x0003A000 �C 0x0003BFFF Sector29
�洢�ϵ�ʱ�� 8K -SECTOR28
0x00038000 �C 0x00039FFF Sector28

//OTP���� 1024��Byte [960����������60��������]
0x03000C00 - 0x03000FFB
*/
/**
 *****************************************************************************
 ** \brief FLASH ��д
 **
 ** ������FLASHд��1���ֵ�����.
 **
 ** \param [in]  u32Addr         Flash��ַ
 ** \param [in]  u32Data         1��������
 **
 ** \retval Ok                    д��ɹ�.
 ** \retval ErrorInvalidParameter FLASH��ַ��Ч
 ** \retval ErrorTimeout          ������ʱ
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

uint32_t paramIndexAddr = PARAM_START_ADDRESS;//ָ���ڴ�յĵ�ַ
uint32_t getFlashDat() {
  uint32_t dat;
  if(paramIndexAddr == PARAM_START_ADDRESS) {
    while(*((uint32_t*)(paramIndexAddr)) != 0xffffffff) {
      paramIndexAddr += (4);//������һ�������Ƿ�Ϊ��
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
 *���� �ۻ��� ����ʱ��
*/
void saveFlashData(uint32_t dat) {
  while(true) {
    if(paramIndexAddr >= PARAM_MAX_ADDRESS) { //������ַ�Ͳ���д��
      paramIndexAddr = PARAM_START_ADDRESS;
      EFM_Unlock();
      EFM_FlashCmd(Enable);
      EFM_SectorErase(PARAM_START_ADDRESS);//������ҳ
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
  //��ȡ��һҳ����ֱ��δ�洢����
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
 * ���������Ĵ�����ԭ��
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
  //��ȡ��һҳ����ֱ��δ�洢����
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
 *���� �ۼƵ� �ϵ�ʱ��
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
