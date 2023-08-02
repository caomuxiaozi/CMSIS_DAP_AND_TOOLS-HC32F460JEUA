#include "hc32_ddl.h"
#include "ff.h"
extern FATFS FatFs;
extern FIL font16File;
extern FIL font12File;
/*
点阵编码范围列表:

编码范围					字符序号范围			每字节编码含字符数(高字节->次低字节)
  0xA1A1 - 0xA9FE			0 - 845				*94, *1.
  0xB0A1 - 0xF7FE			846 - 7613				*94, *1.

文件共有字符数: 7614 个.

点阵宽高: 16 * 16.
点阵存储格式: 横向扫描, 字节高位在前.

点阵宽高: 16 * 16.
点阵存储格式: 横向扫描, 字节高位在前.

点阵宽高: 16 * 16.
点阵存储格式: 竖向扫描, 字节低位在前.

点阵宽高: 16 * 16.
点阵存储格式: 竖向扫描, 字节低位在前.

*/
uint16_t getZhFont(uint8_t *pZh,uint8_t *pZk){
  uint8_t parseZhLen = 0;
//  uint16_t pZkIndex  = 0;
  uint16_t ZH_H,ZH_L;
  uint32_t addressStart;
  uint32_t fReadlen;
  FRESULT ret;
  while(*pZh != 0){
    ZH_H = *pZh;
    ZH_L = *(pZh+1);
    if(ZH_H <= 0xA9){
      addressStart = ((ZH_H - 0xA1)*94 + ZH_L - 0xA1)*32;
    }else{
      addressStart = ((ZH_H - 0xB0)*94 + ZH_L - 0xA1)*32 + 846*32;
    }
    f_lseek(&font16File,addressStart);
    ret = f_read(&font16File,pZk+parseZhLen*32,32,&fReadlen);
    if(ret != FR_OK){
      break;
    }
    pZh += 2;
    parseZhLen++;
  }
  return parseZhLen;
}
uint16_t getZh12Font(uint32_t code,uint8_t *pZk){
  uint16_t ZH_H,ZH_L;
  uint32_t addressStart;
  uint32_t fReadlen;
  FRESULT ret;
    ZH_H = code>>8;
    ZH_L = code & 0xff;
    if(ZH_H <= 0xA9){
      addressStart = ((ZH_H - 0xA1)*94 + ZH_L - 0xA1)*24;
    }else{
      addressStart = ((ZH_H - 0xB0)*94 + ZH_L - 0xA1)*24 + 846*24;
    }
     ret = f_lseek(&font12File,addressStart);
    ret = f_read(&font12File,pZk,24,&fReadlen);
    if(ret != FR_OK){
      return 0;
    }
  
  return fReadlen;
}
