/*
*********************************************************************************************************
*
*    ģ������ : elf�ļ�����ģ��
*    �ļ����� : elf_file.c
*    ��    �� : V1.0
*    ˵    �� : ���ڽ���KEIL��FLM�㷨�ļ�����ȡ���ص�CPU RAM�Ĵ��롣
*               FLM�ļ��е��ַ�����ͷ��ű��Ȳ��ܳ���4096��
*
*    �޸ļ�¼ :
*        �汾��  ����        ����     ˵��
*        V1.0    2019-12-29  armfly   ��ʽ����
*
*    Copyright (C), 2019-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "flm_parse.h"
#include "elf.h"
#include "ff.h"
#include "string.h"
#include "hc32_ddl.h"
#include "stdlib.h"
extern void delay1ms(uint16_t ms);

//#define PRINTF_THIS printf(...);delay1ms(10);
#ifndef PRINTF_THIS
#define PRINTF_THIS(...)
#endif
uint32_t haltCode[] = {0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2};
const char* strFuncName[FUNC_NUM] = {"FlashDevice", "Init", "UnInit", "BlankCheck",
                                     "EraseChip", "EraseSector", "ProgramPage", "Verify"
                                    };
FlashDevice_Type FlashDeviceInfo;
FLM_PARSE_T g_tFLM;
extern uint8_t* g_pAlgoBuff;
/* ���Դ���
    ELF_ParseFile(ELF_TEST_FILE_1);
    ELF_ParseFile(ELF_TEST_FILE_2);
    ELF_ParseFile(ELF_TEST_FILE_3);
    while(1);
*/

/*
*********************************************************************************************************
*    �� �� ��: ELF_ParseFile
*    ����˵��: ����elf�ļ�.  ʹ���� FsReadBuf[4096]ȫ������������������������ŵ�ȫ�ֱ���g_tFLM
*    ��    ��: _path : �ļ�·��
*    �� �� ֵ: 0 = ok�� ����ֵ��ʾ����
*********************************************************************************************************
*/
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define Elf_Shdr Elf32_Shdr
#define Elf_Sym  Elf32_Sym
uint8_t* FsReadBuf = NULL;

uint8_t ELF_ParseFile(char* _path) {
  FIL algoFile;
  FRESULT res;
  uint32_t bytes = 0;
  uint32_t i, j;
  char* p;
  uint32_t StrFoud[FUNC_NUM] = {0};
  uint32_t StrIdx[FUNC_NUM] = {0};
  Elf_Ehdr Ehdr;
  Elf_Phdr* pPhdr;
  Elf_Shdr* pShdr;
  Elf_Sym* pSym;

  Elf_Shdr ShdrSym;   /* ���ű�ͷ */
  Elf_Shdr ShdrStr;   /* �ַ�����ͷ */

  /* ���������ȫ������ */
  for(i = 0; i < LOAD_NUM; i++) {
    g_tFLM.Load[i].Valid = 0;
    g_tFLM.Load[i].Offset = 0;
    g_tFLM.Load[i].Addr = 0;
    g_tFLM.Load[i].Size = 0;
  }

  for(i = 0; i < FUNC_NUM; i++) {
    g_tFLM.Func[i].Valid = 0;
    g_tFLM.Func[i].Offset = 0;
    g_tFLM.Func[i].Size = 0;
  }

  /* �����ļ�ͷ�� ELF Header */
  res = f_open(&algoFile, _path, FA_READ);
  if(res != FR_OK) {
    goto ELF_ParseFile_Error;
  }
  if(res == FR_OK) {
    if(FsReadBuf != NULL) {
      mFREE(FsReadBuf);
    }
    FsReadBuf = calloc(52, 1);
    if(FsReadBuf) {
      res = f_read(&algoFile, FsReadBuf, 52, &bytes);
    }
  }
  if(res != FR_OK && bytes != 52) {
    goto ELF_ParseFile_Error;   /* ���ļ����� */
  }
  memcpy((char*)&Ehdr, FsReadBuf, sizeof(Elf_Ehdr));
  if(strstr((const char*)Ehdr.e_ident, "ELF") == NULL) {
    goto ELF_ParseFile_Error;   /* ����ELF�ļ� */
  }

  if(Ehdr.e_type != ET_EXEC) {
    goto ELF_ParseFile_Error;   /* ���ǿ�ִ�еľ����ļ� */
  }

  /* ��������ͷ����Program Header�� - 2�� */
  if(Ehdr.e_phnum > LOAD_NUM) {
    goto ELF_ParseFile_Error;   /* Program Header �������� */
  }
  if(f_lseek(&algoFile, Ehdr.e_phoff) != FR_OK) {
    goto ELF_ParseFile_Error;
  }
  if(FsReadBuf != NULL) {
    mFREE(FsReadBuf);
  }
  FsReadBuf = calloc(Ehdr.e_phnum * 32, 1);
  if(f_read(&algoFile, FsReadBuf, Ehdr.e_phnum * 32, &bytes) != FR_OK) {
    goto ELF_ParseFile_Error;
  }
  if(bytes != Ehdr.e_phnum * 32) {
    return Error;   /* ���ļ����� */
  }

  for(i = 0; i < Ehdr.e_phnum; i++) {
    pPhdr = (Elf_Phdr*)(FsReadBuf + i * 32);
    if(pPhdr->p_type == PT_LOAD && (pPhdr->p_flags & (PF_X | PF_W | PF_R)) == (PF_X | PF_W | PF_R)) {
      PRINTF_THIS("find PT_LOAD\n");
      g_tFLM.Load[0].Valid = 1;
      g_tFLM.Load[0].Offset = pPhdr->p_offset;
      g_tFLM.Load[0].Addr = pPhdr->p_vaddr;
      g_tFLM.Load[0].Size = pPhdr->p_filesz + 32;
      char* ramcode = calloc(pPhdr->p_filesz, 1);
      if(f_lseek(&algoFile, pPhdr->p_offset) != FR_OK) {
        goto ELF_ParseFile_Error;
      }
      if(f_read(&algoFile, ramcode, pPhdr->p_filesz, &bytes) != FR_OK) {
        goto ELF_ParseFile_Error;
      }
      if(bytes) {
        if(g_pAlgoBuff != NULL) {
          mFREE(g_pAlgoBuff);
        }
        g_pAlgoBuff = calloc(bytes + 32, 1);
        memcpy(g_pAlgoBuff, haltCode, 32);
        memcpy(g_pAlgoBuff + 32, ramcode, bytes);
      }
      for(uint16_t k = 0; k < (pPhdr->p_filesz + 32) / 4; k++) {
        PRINTF_THIS("%d:0x%08x\n", k, *((uint32_t*)(g_pAlgoBuff + k * 4)));
      }
      mFREE(ramcode);
      break;
    }
  }

  /* ��������ͷ�� ��Sections Header�� */
  if(Ehdr.e_shnum < 25) {
    uint8_t found = 0;

    if(f_lseek(&algoFile, Ehdr.e_shoff) != FR_OK) {
      goto ELF_ParseFile_Error;
    }
    if(FsReadBuf != NULL) {
      mFREE(FsReadBuf);
    }
    FsReadBuf = calloc(40 * Ehdr.e_shnum, 1);
    if(FsReadBuf == NULL) { //�ڴ����ʧ��
      goto ELF_ParseFile_Error;
    }
    if(f_read(&algoFile, FsReadBuf, 40 * Ehdr.e_shnum, &bytes) != FR_OK) {
      goto ELF_ParseFile_Error;
    }
    /* �Ȳ��ҷ��ű� */
    for(i = 0; i < Ehdr.e_shnum; i++) {
      pShdr = (Elf_Shdr*)(FsReadBuf + 40 * i);
      if(pShdr->sh_type == SHT_SYMTAB) {
        memcpy((char*)&ShdrSym, (char*)pShdr, sizeof(Elf_Shdr));
        found++;
      }
    }
    if(found == 0) {
      goto ELF_ParseFile_Error;   /* δ�ҵ����ű� */
    }

    if(ShdrSym.sh_link >= Ehdr.e_shnum) {
      goto ELF_ParseFile_Error;   /* δ�ҵ��ַ����� */
    }

    pShdr = (Elf_Shdr*)(FsReadBuf + 40 * ShdrSym.sh_link);
    if(pShdr->sh_type == SHT_STRTAB) {
      memcpy((char*)&ShdrStr, (char*)pShdr, sizeof(Elf_Shdr));
    } else {
      goto ELF_ParseFile_Error;   /* δ�ҵ��ַ����� */
    }
  } else {
    goto ELF_ParseFile_Error;   /* Sections�������� */;
  }

  /* �ַ����� */
  if(FsReadBuf != NULL) {
    mFREE(FsReadBuf);
  }
  FsReadBuf = calloc(ShdrStr.sh_size, 1);
  if(FsReadBuf == NULL) {
    goto ELF_ParseFile_Error;
  }
  if(FsReadBuf) {
    if(f_lseek(&algoFile, ShdrStr.sh_offset) != FR_OK) {
      goto ELF_ParseFile_Error;
    }

    if(f_read(&algoFile, FsReadBuf, ShdrStr.sh_size, &bytes) != FR_OK) {
      goto ELF_ParseFile_Error;
    }
    if(bytes == ShdrStr.sh_size) {
      p = (char*)FsReadBuf;
      for(i = 0; i < bytes - 1; i++) {
        if(*p++ == 0) {
          for(j = 0; j < 8; j++) {
            if(strcmp(p, strFuncName[j]) == 0) {
              PRINTF_THIS("find Shdrstr:%d->%s\n", j, p);
              StrFoud[j] = 1;
              StrIdx[j] = i + 1;
            }
          }
        }
      }
    } else {
      goto ELF_ParseFile_Error;   /* ���ļ�ʧ�� */
    }
  }

  /* �������ű� */
  if(FsReadBuf != NULL) {
    mFREE(FsReadBuf);
  }
  FsReadBuf = calloc(ShdrSym.sh_size, 1);
  if(FsReadBuf/*ShdrSym.sh_size < sizeof(FsReadBuf)*/) {
    if(f_lseek(&algoFile, ShdrSym.sh_offset) != FR_OK) {
      goto ELF_ParseFile_Error;
    }
    mFREE(FsReadBuf);
    FsReadBuf = calloc(ShdrSym.sh_size, 1);
    if(FsReadBuf == NULL) {
      goto ELF_ParseFile_Error;
    }
    if(f_read(&algoFile, FsReadBuf, ShdrSym.sh_size, &bytes) != FR_OK) {
      goto ELF_ParseFile_Error;
    }
    for(i = 0; i < bytes / sizeof(Elf_Sym); i++) {
      for(j = 0; j < FUNC_NUM; j++) {
        pSym = (Elf_Sym*)(FsReadBuf + 16 * i);
        if(pSym->st_name == StrIdx[j] && StrFoud[j] == 1) {
          PRINTF_THIS("find ShdrSym:%d->%s,offset=%d,length = %d\n", j, strFuncName[j], pSym->st_value, pSym->st_size);
          g_tFLM.Func[j].Valid = 1;
          g_tFLM.Func[j].Offset = pSym->st_value;
          g_tFLM.Func[j].Size = pSym->st_size;
          if(j == FuncNameIndex_FlashDevice) {
            if(f_lseek(&algoFile, g_tFLM.Load[0].Offset + pSym->st_value) != FR_OK) {
              goto ELF_ParseFile_Error;
            }
            if(f_read(&algoFile, &FlashDeviceInfo, sizeof(FlashDeviceInfo), &bytes) != FR_OK) {
              goto ELF_ParseFile_Error;
            }

          }
        }
      }
    }
  }
  if(FsReadBuf != NULL) {
    mFREE(FsReadBuf);
  }
  f_close(&algoFile);


  return Ok;   /* �����ɹ� */

ELF_ParseFile_Error:
  f_close(&algoFile);
  return Error;   /* ����ʧ�� */
}
