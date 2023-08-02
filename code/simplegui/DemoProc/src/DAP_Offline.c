/*************************************************************************/
/** Copyright.                                                          **/
/** FileName: RTCNotice.c                                               **/
/** Author: Polarix                                                     **/
/** Version: 1.0.0.0                                                    **/
/** Description: HMI demo for notice box interface and refresh screen.  **/
/*************************************************************************/
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DAP.h"
#include "DemoProc.h"
#include "HMI_Engine.h"
#include "SGUI_Basic.h"
#include "SGUI_FontResource.h"
#include "SGUI_Text.h"
#include "eeprom.h"
#include "ff.h"
#include "flm_parse.h"
#include "hc32_ddl.h"
#include "hex2bin.h"
#include "sd_card.h"
#include "sgui_processbar.h"
#include "stdarg.h"
#include "swd_flash.h"
#include "swd_host.h"
//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
#define PRINTF_THIS printf
#ifndef PRINTF_THIS
#    define PRINTF_THIS(...)
#endif
typedef enum {
  DAP_OFFLINE_MENU_START    = 0,
  DAP_OFFLINE_MENU_OPENFILE = 0,
  DAP_OFFLINE_MENU_AUTO,
  DAP_OFFLINE_MENU_PROG,
  DAP_OFFLINE_MENU_SPEED,
  DAP_OFFLINE_MENU_END,
} DAP_OFFLINE_MENU_Type;
int8_t DAP_OFFLINE_MENU_Index = DAP_OFFLINE_MENU_START;

typedef enum {
  HEX_RECORD_DATA                    = 0,
  HEX_RECORD_END_FILE                = 1,
  HEX_RECORD_EXTENED_SEGMENT_ADDRESS = 2,
  HEX_RECORD_START_SEGMENT_ADDRESS   = 3,
  HEX_RECORD_EXTEND_LINEAR_ADDRESS   = 4,
  HEX_RECORD_START_LINEAR_ADDRESS    = 5,
} HEX_Type;
//: 10 0000 00 F88AFF1F9902000071300000791B0000 80
typedef struct {
  uint8_t char1_maoHao;
  uint8_t char2_length[2];
  uint8_t char4_offset[4];
  uint8_t char2_type[2];
  uint8_t char32_dat[32];
  uint8_t char2_verify[2];
} HEX_Format_Type;
typedef enum { SWD_NO_Err, SWD_FILE_Err, SWD_RESET_Err, SWD_ERASE_Err, SWD_READ_Err, SWD_WRITE_Err } SWD_Error_Type;
//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT HMI_DapOffline_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT HMI_DapOffline_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT HMI_DapOffline_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT HMI_DapOffline_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);
extern char              selectPath[256];
extern char              algorithmPath[256];
extern char              hexPath[256];
extern char              addrPath[256];
extern bool              algorithmFileIsExist;
program_target_t         flash_algo;
extern FLM_PARSE_T       g_tFLM;
uint8_t*                 g_pAlgoBuff           = NULL;
en_result_t              algorithmParseState   = ErrorNotReady;
bool                     getHexFileBinLength   = false;
uint32_t                 progStartAddressFirst = 0;
uint32_t                 progStartAddressSub   = 0;
uint32_t                 progLength            = 0;
bool                     autoProg              = 0;
bool                     progNeedRun           = 0;
#define BIN_TEMP_PATH "0:/temp.bin"
bool             progSuccess           = false;
bool             DapOfflineLastStatus  = true;
uint32_t         DAP_DEFAULT_SWJ_CLOCK = 6000000;
extern DAP_STATE dap_state;
//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_ACTION s_stDapOfflinePaintActions = { NULL, HMI_DapOffline_Prepare, HMI_DapOffline_RefreshScreen, HMI_DapOffline_ProcessEvent,
                                                 HMI_DapOffline_PostProcess
                                               };
//=======================================================================//
//= Global variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_OBJECT g_stHMIDemo_DapOffline = { HMI_SCREEN_ID_DEMO_DAP_OFFLINE, &s_stDapOfflinePaintActions };
// SGUI_PROCBAR_STRUCT stcProcessBar;
//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//
#define MSG_HOLD_TIME_MS 100
void HMI_Dap_Offline_ShortShow(SGUI_SCR_DEV* pstDeviceIF, uint32_t holdTime, char* fmt, ...) {
#ifdef ENABLE_WDT_SUN
  WDT_RefreshCounter();
#endif
  if(fmt != NULL) {
    SGUI_RECT stDisplayArea;
    stDisplayArea.iX      = 2;
    stDisplayArea.iY      = 15;
    stDisplayArea.iWidth  = 128;
    stDisplayArea.iHeight = 36;
    char    str[256]      = "";
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, 64u, fmt, args);
    va_end(args);
    SGUI_Text_DrawMultipleLinesText(pstDeviceIF, str, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  }
  pstDeviceIF->fnSyncBuffer();
  uint32_t timeOut = getTick() + holdTime;
  while(getTick() < timeOut) {
    __NOP();
  };
}
HMI_ENGINE_RESULT HMI_DapOffline_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  DAP_DEFAULT_SWJ_CLOCK = getFlashDat();
  if(DAP_DEFAULT_SWJ_CLOCK > 20000000 || DAP_DEFAULT_SWJ_CLOCK < 2000000) {
    DAP_DEFAULT_SWJ_CLOCK = 6000000;
  }
  if(NULL != pstDeviceIF->fnClear) {
    pstDeviceIF->fnClear();
  } else {
    SGUI_Basic_DrawRectangle(pstDeviceIF, 0, 0, pstDeviceIF->stSize.iWidth, pstDeviceIF->stSize.iHeight, SGUI_COLOR_BKGCLR, SGUI_COLOR_BKGCLR);
  }
  if(pstParameters != NULL) {
    SGUI_RECT stDisplayArea;
    stDisplayArea.iX      = 2;
    stDisplayArea.iY      = 15;
    stDisplayArea.iWidth  = 128;
    stDisplayArea.iHeight = 24;
    char str[256]         = "INFO:";
    strcat(str, pstParameters);
    SGUI_Text_DrawMultipleLinesText(pstDeviceIF, str, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  }
  return HMI_RET_NORMAL;
}
en_result_t       parseFLM(const char* path) {}
uint32_t          RAM_BASE_ADDR = 0x1FFF8000;
HMI_ENGINE_RESULT HMI_DapOffline_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  SGUI_INT iPaintBkgIdx;
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  SGUI_RECT  stDisplayArea = { 2, 1, 128, 12 };
  SGUI_POINT stInnerPos    = { 0, 0 };
  stDisplayArea.iX         = 2;
  stDisplayArea.iY         = 2;
  stDisplayArea.iWidth     = 48;
  stDisplayArea.iHeight    = 12;
  SGUI_Text_DrawText(pstDeviceIF, "离线烧写", &GB2312_FZXS12, &stDisplayArea, &stInnerPos, SGUI_DRAW_NORMAL);
  stDisplayArea.iX     = 50;
  stDisplayArea.iWidth = 60;
  SGUI_Text_DrawText(pstDeviceIF, "[OpenFile]", &GB2312_FZXS12, &stDisplayArea, &stInnerPos,
                     DAP_OFFLINE_MENU_OPENFILE == DAP_OFFLINE_MENU_Index ? SGUI_DRAW_REVERSE : SGUI_DRAW_NORMAL);
  stDisplayArea.iX     = 110;
  stDisplayArea.iWidth = 18;
  SGUI_Text_DrawText(pstDeviceIF, autoProg ? "[A]" : "[M]", &GB2312_FZXS12, &stDisplayArea, &stInnerPos,
                     DAP_OFFLINE_MENU_AUTO == DAP_OFFLINE_MENU_Index ? SGUI_DRAW_REVERSE : SGUI_DRAW_NORMAL);
  stDisplayArea.iX      = 2;
  stDisplayArea.iY      = 15;
  stDisplayArea.iWidth  = 128;
  stDisplayArea.iHeight = 36;
  char str[256]         = "文件:";
  if(strchr(hexPath, '/')) {
    strcat(str, strchr(hexPath, '/'));
    if(algorithmFileIsExist && algorithmParseState == ErrorNotReady) {
      PRINTF_THIS("start parse algorithm Start\n");
      HMI_Dap_Offline_ShortShow(pstDeviceIF, MSG_HOLD_TIME_MS, "开始解析FLM算法");
      uint8_t ramBaseReadSuccess = false;
      FIL     addrFile;
      if(f_open(&addrFile, addrPath, FA_READ) == FR_OK) {
        uint8_t  addrDat[64];
        uint32_t addrReadBytes = 0;
        if(f_read(&addrFile, addrDat, 64, &addrReadBytes) == FR_OK) {
          if(addrReadBytes >= 10 && addrDat[0] == '0' && (addrDat[1] == 'x' || addrDat[1] == 'X')) {
            RAM_BASE_ADDR      = Char8toUint32(&addrDat[2]);
            ramBaseReadSuccess = true;
          }
        }
        f_close(&addrFile);
      }
      en_result_t res = ELF_ParseFile(algorithmPath);
      if(res == Ok && ramBaseReadSuccess) {
        memset(&flash_algo, 0, sizeof(flash_algo));
        algorithmParseState = Ok;
        if(g_tFLM.Func[FuncNameIndex_Init].Valid)
          flash_algo.init = g_tFLM.Func[FuncNameIndex_Init].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_UnInit].Valid)
          flash_algo.uninit = g_tFLM.Func[FuncNameIndex_UnInit].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_EraseChip].Valid)
          flash_algo.erase_chip = g_tFLM.Func[FuncNameIndex_EraseChip].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_EraseSector].Valid)
          flash_algo.erase_sector = g_tFLM.Func[FuncNameIndex_EraseSector].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_ProgramPage].Valid)
          flash_algo.program_page = g_tFLM.Func[FuncNameIndex_ProgramPage].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_Verify].Valid)
          flash_algo.verify = g_tFLM.Func[FuncNameIndex_Verify].Offset + RAM_BASE_ADDR + 0x20;
        if(g_tFLM.Func[FuncNameIndex_BlankCheck].Valid)
          flash_algo.blank_check = g_tFLM.Func[FuncNameIndex_BlankCheck].Offset + RAM_BASE_ADDR + 0x20;
        flash_algo.sys_call_s.breakpoint  = RAM_BASE_ADDR + 1;
        flash_algo.sys_call_s.static_base = RAM_BASE_ADDR + FlashDeviceInfo.szPage +g_tFLM.Load[0].Size ;
        flash_algo.sys_call_s.stack_pointer =
          RAM_BASE_ADDR + FlashDeviceInfo.szPage + g_tFLM.Load[0].Size * 2 ;
        flash_algo.program_buffer      = RAM_BASE_ADDR + g_tFLM.Load[0].Size;
        flash_algo.algo_start          = RAM_BASE_ADDR;
        flash_algo.algo_size           = g_tFLM.Load[0].Size;
        flash_algo.algo_blob           = (uint32_t*)g_pAlgoBuff;
        flash_algo.program_buffer_size = FlashDeviceInfo.szPage;
        PRINTF_THIS("start parse algorithm Success\n");
        HMI_Dap_Offline_ShortShow(pstDeviceIF, MSG_HOLD_TIME_MS, "算法加载成功");
      } else {
        algorithmParseState = Error;
        PRINTF_THIS("start parse algorithm Error\n");
        HMI_Dap_Offline_ShortShow(pstDeviceIF, MSG_HOLD_TIME_MS * 2, "算法加载失败!!!\n%s",
                                  ramBaseReadSuccess ? "地址解析失败" : "算法解析失败");
      }
      if(getHexFileBinLength == false) {
        FIL      hexFile;
        uint32_t readBytesAll = 0;
        uint32_t readBytes    = 0;
        uint32_t writeBytes   = 0;
        uint8_t  progBuff[64];  //分配512K
        uint8_t  readBuff[64];  // mem read buffer
        uint8_t  writeBuf[64];  // write to hex2bin.bin buffer
        FRESULT  fileRet;
        progSuccess = false;
        //获取烧写长度，用于擦除flash
        HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "开始解析HEX并保存一个BIN,需要点时间");
        if(f_open(&hexFile, hexPath, FA_READ) == FR_OK) {
          FIL binFile;
          fileRet = f_open(&binFile, BIN_TEMP_PATH,
                           FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
          if(fileRet == FR_OK) {
            f_chmod(BIN_TEMP_PATH, AM_HID, AM_HID);
            progStartAddressFirst = 0;
            progStartAddressSub   = 0;
            bool isAddrStart = false;
            do {
              memset(progBuff, 0, 64);
              fileRet = f_readLine(&hexFile, progBuff, 64, &readBytes);
              readBytesAll += readBytes;
              HEX_Format_Type* hexFormat  = (HEX_Format_Type*)progBuff;
              uint8_t          recordType = Char2toByte(hexFormat->char2_type);
              switch(recordType) {
              case HEX_RECORD_EXTEND_LINEAR_ADDRESS:
                if(readBytesAll < 64) {
                  progStartAddressFirst = Char4toUint16(hexFormat->char32_dat);
                  progStartAddressFirst <<= 16;
                } else {
                  progStartAddressSub = Char4toUint16(hexFormat->char32_dat);
                  progStartAddressSub <<= 16;
                }
                isAddrStart = true;
                break;
              case HEX_RECORD_DATA:
                progLength         = Char4toUint16(hexFormat->char4_offset);
                uint8_t lineLength = Char2toByte(hexFormat->char2_length);
                for(uint8_t L = 0; L < lineLength; L++) {
                  writeBuf[L] = Char2toByte(&hexFormat->char32_dat[L * 2]);
                }
                if(isAddrStart) {
                  if(progStartAddressSub == 0) {
                    progStartAddressFirst += progLength;
                  } else {
                    progStartAddressSub += progLength;
                  }
                  isAddrStart = false;
                }
                fileRet = f_write(&binFile, writeBuf, lineLength, &writeBytes);
                break;
              case HEX_RECORD_END_FILE:
                progLength += 16;
                if(progStartAddressSub) {
                  progLength += (progStartAddressSub - progStartAddressFirst);
                }
                readBytes = 0;
                f_close(&binFile);
                getHexFileBinLength = true;
                HMI_Dap_Offline_ShortShow(pstDeviceIF, MSG_HOLD_TIME_MS, "解析成功");
                break;
              }
            } while(fileRet == FR_OK && readBytes > 0);
          }
          f_close(&hexFile);
          PRINTF_THIS("FA=%08x SA=%08x len=%x", progStartAddressFirst, progStartAddressSub, progLength);
          PRINTF_THIS("\r\n");
        }
      }
    }
  } else {
    strcpy(str, "离线烧写要独占SD,不可挂载USB使用");
  }
  SGUI_Text_DrawMultipleLinesText(pstDeviceIF, str, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  stDisplayArea.iX      = 5;
  stDisplayArea.iY      = 52;
  stDisplayArea.iWidth  = 42;
  stDisplayArea.iHeight = 12;
  SGUI_Text_DrawText(pstDeviceIF, "[Start]", &GB2312_FZXS12, &stDisplayArea, &stInnerPos,
                     DAP_OFFLINE_MENU_PROG == DAP_OFFLINE_MENU_Index ? SGUI_DRAW_REVERSE : SGUI_DRAW_NORMAL);
  stDisplayArea.iX      = 55;
  stDisplayArea.iWidth  = 42;
  stDisplayArea.iHeight = 12;
  char speedStr[16];
  sprintf(speedStr, "[%dMHz]", DAP_DEFAULT_SWJ_CLOCK / 1000000u);
  SGUI_Text_DrawText(pstDeviceIF, speedStr, &GB2312_FZXS12, &stDisplayArea, &stInnerPos,
                     DAP_OFFLINE_MENU_SPEED == DAP_OFFLINE_MENU_Index ? SGUI_DRAW_REVERSE : SGUI_DRAW_NORMAL);
  stDisplayArea.iX     = 110;
  stDisplayArea.iWidth = 18;
  SGUI_Text_DrawText(pstDeviceIF, DapOfflineLastStatus ? "^_^" : "#_#", &GB2312_FZXS12, &stDisplayArea, &stInnerPos,
                     SGUI_DRAW_NORMAL);
  return HMI_RET_NORMAL;
}
void HMI_DapOffline_DownLoad(SGUI_SCR_DEV* pstDeviceIF) {
  if(algorithmParseState == Ok) {
    progNeedRun = true;
    /***************************************************************************/
    if(getHexFileBinLength == false) {
      FIL      hexFile;
      uint32_t readBytesAll = 0;
      uint32_t readBytes    = 0;
      uint32_t writeBytes   = 0;
      uint8_t  progBuff[64];
      uint8_t writeBuf[64];
      FRESULT fileRet;
      progSuccess = false;
      HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "开始解析HEX并保存一个BIN,需要点时间");
      if(f_open(&hexFile, hexPath, FA_READ) == FR_OK) {
        FIL binFile;
        fileRet = f_open(&binFile, BIN_TEMP_PATH,
                         FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        if(fileRet == FR_OK) {
          f_chmod(BIN_TEMP_PATH, AM_HID, AM_HID);
          progStartAddressFirst = 0;
          progStartAddressSub   = 0;
          bool isAddrStart = false;
          do {
            memset(progBuff, 0, 64);
            fileRet = f_readLine(&hexFile, progBuff, 64, &readBytes);
            readBytesAll += readBytes;
            HEX_Format_Type* hexFormat  = (HEX_Format_Type*)progBuff;
            uint8_t          recordType = Char2toByte(hexFormat->char2_type);
            switch(recordType) {
            case HEX_RECORD_EXTEND_LINEAR_ADDRESS:
              if(readBytesAll < 64) {
                progStartAddressFirst = Char4toUint16(hexFormat->char32_dat);
                progStartAddressFirst <<= 16;
              } else {
                progStartAddressSub = Char4toUint16(hexFormat->char32_dat);
                progStartAddressSub <<= 16;
              }
              isAddrStart = true;
              break;
            case HEX_RECORD_DATA:
              progLength         = Char4toUint16(hexFormat->char4_offset);
              uint8_t lineLength = Char2toByte(hexFormat->char2_length);
              for(uint8_t L = 0; L < lineLength; L++) {
                writeBuf[L] = Char2toByte(&hexFormat->char32_dat[L * 2]);
              }
              if(isAddrStart) {
                if(progStartAddressSub == 0) {
                  progStartAddressFirst += progLength;
                } else {
                  progStartAddressSub += progLength;
                }
                isAddrStart = false;
              }
              fileRet = f_write(&binFile, writeBuf, lineLength, &writeBytes);
              break;
            case HEX_RECORD_END_FILE:
              progLength += 16;
              if(progStartAddressSub) {
                progLength += (progStartAddressSub - progStartAddressFirst);
              }
              readBytes = 0;
              f_close(&binFile);
              getHexFileBinLength = true;
              HMI_Dap_Offline_ShortShow(pstDeviceIF, MSG_HOLD_TIME_MS, "解析成功");
              break;
            }
          } while(fileRet == FR_OK && readBytes > 0);
        }
        f_close(&hexFile);
        PRINTF_THIS("FA=%08x SA=%08x len=%x", progStartAddressFirst, progStartAddressSub, progLength);
        PRINTF_THIS("\r\n");
      }
    }
    if(getHexFileBinLength == true) {
      FIL binFile;
      uint32_t sdReadBytes;
      uint8_t  sdReadBuff[FlashDeviceInfo.szPage];
      FRESULT fileRet = FR_OK;
      error_t swdRet  = ERROR_SUCCESS;
      if(f_open(&binFile, BIN_TEMP_PATH, FA_READ) == FR_OK) {
        PRINTF_THIS("bin file open success\n");
        dap_state.IDCODE       = 0;
        swdRet                 = target_flash_init(FlashDeviceInfo.DevAdr);
        uint8_t readMemMatched = true;
        uint8_t blankCheck     = false;
        uint16_t oledTipsModValue = progLength / FlashDeviceInfo.szPage / 10 * FlashDeviceInfo.szPage;
        if(swdRet == ERROR_SUCCESS && fileRet == FR_OK) {
          fileRet = f_lseek(&binFile, 0);
          if(fileRet == FR_OK) {
            uint8_t flashReadBuf[FlashDeviceInfo.szPage];
            uint8_t breakThis = false;
            for(uint32_t addr = 0; ((addr < progLength) && (swdRet == ERROR_SUCCESS) && (fileRet == FR_OK));
                addr += FlashDeviceInfo.szPage) {
              fileRet = f_read(&binFile, sdReadBuff, FlashDeviceInfo.szPage, &sdReadBytes);
              if(fileRet == FR_OK && sdReadBytes > 0) {
                PRINTF_THIS("read addr=%x\n", addr);
                if(addr % oledTipsModValue == 0) {
                  uint16_t processPercent = addr * 1000 / progLength;
                  HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "预检查:0x%08x\n进度:%d%%", progStartAddressFirst + addr,
                                            processPercent / 10);
                }
                if(g_tFLM.Func[FuncNameIndex_Verify].Valid) {
                  if(target_flash_verify(progStartAddressFirst + addr, sdReadBuff, sdReadBytes) != ERROR_SUCCESS) {
                    readMemMatched = false;
                    break;
                  }
                } else {
                  if(swd_read_memory(progStartAddressFirst + addr, flashReadBuf, sdReadBytes)) {
                    for(uint32_t i = 0; i < sdReadBytes; i++) {
                      if(sdReadBuff[i] != flashReadBuf[i]) {
                        readMemMatched = false;
                        blankCheck     = false;
                        breakThis      = true;
                        break;
                      }
                    }
                    if(breakThis)
                      break;
                  }
                }
              }
            }
          }
        }
        if(swdRet == ERROR_SUCCESS && fileRet == FR_OK && readMemMatched == false && blankCheck == false) {
          for(uint32_t addr = 0; ((addr < progLength) && (swdRet == ERROR_SUCCESS)); addr += FlashDeviceInfo.szPage) {
            PRINTF_THIS("erase addr=%x\n", addr);
            if(addr % oledTipsModValue == 0) {
              uint16_t processPercent = addr * 1000 / progLength;
              HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "擦除地址:0x%08x\n进度:%d%%", progStartAddressFirst + addr,
                                        processPercent / 10);
            }
            swdRet = target_flash_erase_sector(progStartAddressFirst + addr);
            if(swdRet == ERROR_SUCCESS) {}
          }
        }
        if(swdRet == ERROR_SUCCESS && fileRet == FR_OK && readMemMatched == false) {
          fileRet = f_lseek(&binFile, 0);
          for(uint32_t addr = 0; ((addr < progLength) && (swdRet == ERROR_SUCCESS) && (fileRet == FR_OK)); addr += FlashDeviceInfo.szPage) {
            fileRet = f_read(&binFile, sdReadBuff, FlashDeviceInfo.szPage, &sdReadBytes);
            PRINTF_THIS("write addr=%x,read = %x\n", addr, sdReadBytes);
            if(addr % oledTipsModValue == 0) {
              uint16_t processPercent = addr * 1000 / progLength;
              HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "写入:0x%08x\n进度:%d%%", progStartAddressFirst + addr, processPercent / 10);
            }
            if(sdReadBytes) {
              swdRet = target_flash_program_page(progStartAddressFirst + addr, sdReadBuff, sdReadBytes);
            }
          }
        }
        if(swdRet == ERROR_SUCCESS && fileRet == FR_OK && readMemMatched == false) {
          fileRet = f_lseek(&binFile, 0);
          if(fileRet == FR_OK) {
            uint8_t flashReadBuf[FlashDeviceInfo.szPage];
            for(uint32_t addr = 0; ((addr < progLength) && (swdRet == ERROR_SUCCESS) && (fileRet == FR_OK));
                addr += FlashDeviceInfo.szPage) {
              fileRet = f_read(&binFile, sdReadBuff, FlashDeviceInfo.szPage, &sdReadBytes);
              if(fileRet == FR_OK && sdReadBytes > 0) {
                PRINTF_THIS("\n\nread addr=%x\n", addr);
                if(addr % oledTipsModValue == 0) {
                  uint16_t processPercent = addr * 1000 / progLength;
                  if(g_tFLM.Func[FuncNameIndex_Verify].Valid) {
                    HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "函数校验:0x%08x\n进度:%d%%", progStartAddressFirst + addr,
                                              processPercent / 10);
                  } else {
                    HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "回读校验:0x%08x\n进度:%d%%", progStartAddressFirst + addr,
                                              processPercent / 10);
                  }
                }
                if(!g_tFLM.Func[FuncNameIndex_Verify].Valid) {
                  swd_read_memory(progStartAddressFirst + addr, flashReadBuf, sdReadBytes);
                  for(uint32_t i = 0; i < sdReadBytes; i++) {
                    if(sdReadBuff[i] != flashReadBuf[i]) {
                      swdRet = ERROR_FAILURE;
                      break;
                    }
                  }
                } else {
                  swdRet = target_flash_verify(progStartAddressFirst + addr, sdReadBuff, sdReadBytes);
                }
              }
            }
          }
        }
        target_flash_uninit();
        if(swdRet == ERROR_SUCCESS && fileRet == FR_OK) {
          progSuccess = true;
          buzzerCtrl(true);
          if(readMemMatched == false) {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "提示:^_^\n烧写成功^_^");
          } else {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "提示:^_^\n程序匹配^_^\n无需烧写^_^");
          }
          DapOfflineLastStatus = true;
        } else {
          if(dap_state.IDCODE == 0 && autoProg == true) {
            HMI_Dap_Offline_ShortShow(pstDeviceIF, 0, "Wait MCU Connected\n%x", getTick());
          } else if(swdRet != ERROR_SUCCESS) {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "提示:！！！!!\n烧写失败！！！SWD错误");
          } else if(fileRet != FR_OK) {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "提示:！！！!!\n烧写失败！！！READ错误");
          } else {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "提示:！！！!!\n烧写失败！！！ALL错误");
          }
          DapOfflineLastStatus = false;
        }
      } else {
        PRINTF_THIS("bin file open failed\n");
      }
      f_close(&binFile);
    }
    if(autoProg == false) {
      progNeedRun = false;
    }
    /*******************************************************************************************/
  }
}
HMI_ENGINE_RESULT
HMI_DapOffline_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  HMI_ENGINE_RESULT eProcessResult;
  SGUI_INT          iProcessAction;
  KEY_PRESS_EVENT*  pstKeyEvent;
  SGUI_UINT16       uiKeyCode;
  SGUI_UINT16       uiKeyValue;
  eProcessResult = HMI_RET_NORMAL;
  iProcessAction = HMI_DEMO_PROC_NO_ACT;
  pstKeyEvent    = (KEY_PRESS_EVENT*)pstEvent;
  if(SGUI_FALSE == HMI_EVENT_SIZE_CHK(*pstKeyEvent, KEY_PRESS_EVENT)) {
    eProcessResult = HMI_RET_INVALID_DATA;
  } else if(EVENT_ID_KEY_PRESS == pstEvent->iID) {
    uiKeyCode  = pstKeyEvent->Data.uiKeyValue;
    uiKeyValue = KEY_CODE_VALUE(uiKeyCode);
    switch(uiKeyValue) {
    case KEY_VALUE_ESC: {
      iProcessAction = HMI_DEMO_PROC_CANCEL;
    }
    break;
    case KEY_VALUE_ENTER:
      switch(DAP_OFFLINE_MENU_Index) {
      case DAP_OFFLINE_MENU_SPEED:
        if(DAP_DEFAULT_SWJ_CLOCK < 20000000) {
          DAP_DEFAULT_SWJ_CLOCK += 4000000;
          if(DAP_DEFAULT_SWJ_CLOCK > 20000000) {
            DAP_DEFAULT_SWJ_CLOCK = 20000000;
          }
        } else {
          DAP_DEFAULT_SWJ_CLOCK = 2000000;
        }
        if(DAP_DEFAULT_SWJ_CLOCK != getFlashDat()) {
          saveFlashData(DAP_DEFAULT_SWJ_CLOCK);
        }
        break;
      case DAP_OFFLINE_MENU_AUTO: {
        autoProg = !autoProg;
      }
      break;
      case DAP_OFFLINE_MENU_OPENFILE:
        iProcessAction      = HMI_DEMO_PROC_CONFIRM;
        algorithmParseState = ErrorNotReady;
        break;
      case DAP_OFFLINE_MENU_PROG: {
        if(autoProg && progNeedRun) {
          progNeedRun = false;
        } else {
          if(getHexFileBinLength == false) {
            HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "没选择文件，或文件配置错误");
          } else {
            HMI_DapOffline_DownLoad(pstDeviceIF);
          }
        }
      }
      break;
      }
      break;
    case KEY_VALUE_UP:
      if(++DAP_OFFLINE_MENU_Index >= DAP_OFFLINE_MENU_END) {
        DAP_OFFLINE_MENU_Index = DAP_OFFLINE_MENU_START;
      }
      break;
    case KEY_VALUE_DOWN:
      if(--DAP_OFFLINE_MENU_Index < 0) {
        DAP_OFFLINE_MENU_Index = DAP_OFFLINE_MENU_END - 1;
      }
      break;
    }
  } else if(EVENT_ID_TIMER == pstEvent->iID) {
    HMI_DapOffline_RefreshScreen(pstDeviceIF, NULL);
    if(autoProg && progNeedRun) {
      HMI_DapOffline_DownLoad(pstDeviceIF);
    }
  }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }
  return eProcessResult;
}
HMI_ENGINE_RESULT
HMI_DapOffline_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
  if(HMI_PROCESS_SUCCESSFUL(eProcResult)) {
    if(HMI_DEMO_PROC_CONFIRM == iActionID) {
      HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_OPENFILE, NULL);
    } else if(HMI_DEMO_PROC_CANCEL == iActionID) {
      HMI_GoBack(NULL);
    }
  }
  return HMI_RET_NORMAL;
}
