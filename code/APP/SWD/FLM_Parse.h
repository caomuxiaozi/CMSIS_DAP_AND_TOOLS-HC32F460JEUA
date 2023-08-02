/*
*********************************************************************************************************
*
*    模块名称 : elf文件解析模块
*    文件名称 : elf_file.h
*    说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __ELF_FILE_H_
#define __ELF_FILE_H_

#include "stdint.h"

#define FUNC_NUM    8       /* 全局符号个数（函数和一个常量数组） */
#define LOAD_NUM    4       /* 需要加载到RAM的段个数 */
typedef enum {
  FuncNameIndex_FlashDevice   = 0,
  FuncNameIndex_Init          = 1,
  FuncNameIndex_UnInit        = 2,
  FuncNameIndex_BlankCheck    = 3,
  FuncNameIndex_EraseChip     = 4,
  FuncNameIndex_EraseSector   = 5,
  FuncNameIndex_ProgramPage   = 6,
  FuncNameIndex_Verify        = 7
} FLASH_FUNC_INDEX_Type;
struct FlashSectors  {
  unsigned long   szSector;    // Sector Size in Bytes
  unsigned long AddrSector;    // Address of Sector
};

typedef struct   {
  unsigned short     Vers;    // Version Number and Architecture
  char       DevName[128];    // Device Name and Description
  unsigned short  DevType;    // Device Type: ONCHIP, EXT8BIT, EXT16BIT, ...
  unsigned long    DevAdr;    // Default Device Start Address
  unsigned long     szDev;    // Total Size of Device
  unsigned long    szPage;    // Programming Page Size
  unsigned long       Res;    // Reserved for future Extension
  unsigned char  valEmpty;    // Content of Erased Memory

  unsigned long    toProg;    // Time Out of Program Page Function
  unsigned long   toErase;    // Time Out of Erase Sector Function

  struct FlashSectors sectors[4];
} FlashDevice_Type;
extern FlashDevice_Type FlashDeviceInfo;
enum {
  IDX_FlashDevice = 0,
  IDX_Init,
  IDX_UnInit,
  IDX_BlankCheck,
  IDX_EraseChip,
  IDX_EraseSector,
  IDX_ProgramPage,
  IDX_Verify,
};

typedef struct {
  uint32_t Valid;
  uint32_t Offset;
  uint32_t Size;
}
ELF_FUNC_T;

typedef struct {
  uint32_t Valid;
  uint32_t Offset;
  uint32_t Addr;
  uint32_t Size;
}
ELF_LOAD_T;

/* FLM文件分析结构 */
typedef struct {
  uint32_t FileOk;

  ELF_LOAD_T Load[LOAD_NUM];

  ELF_FUNC_T Func[FUNC_NUM];
} FLM_PARSE_T;

extern FLM_PARSE_T g_tFLM;

uint8_t ELF_ParseFile(char* _path);

#endif
