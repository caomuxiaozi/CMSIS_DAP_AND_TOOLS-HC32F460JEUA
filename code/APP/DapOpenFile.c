

//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DemoProc.h"
#include "HMI_Engine.h"
#include "SGUI_Basic.h"
#include "SGUI_Text.h"
#include "SGUI_FontResource.h"
#include "sgui_processbar.h"
#include "sgui_itemsBase.h"
#include "sgui_list.h"
#include "ff.h"
#include "hc32_ddl.h"
//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
//#define PRINTF_THIS printf
#ifndef PRINTF_THIS
#define PRINTF_THIS(...)
#endif
//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT    HMI_DapOpenFile_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_DapOpenFile_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_DapOpenFile_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT    HMI_DapOpenFile_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_ACTION       s_stDapOpenFilePaintActions =     {   NULL,
                                                              HMI_DapOpenFile_Prepare,
                                                              HMI_DapOpenFile_RefreshScreen,
                                                              HMI_DapOpenFile_ProcessEvent,
                                                              HMI_DapOpenFile_PostProcess
                                                          };
//=======================================================================//
//= Global variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_OBJECT       g_stHMIDemo_DapOpenFile =        {   HMI_SCREEN_ID_DEMO_OPENFILE,
                                                             &s_stDapOpenFilePaintActions
                                                         };
SGUI_LIST stcFileList;
FILINFO  fileInfo;
FIL selectFile;
DIR openDir;
char dapOpenFile_AltName[256][256];
uint8_t dapOpenFile_FileLength = 0;
SGUI_ITEMS_ITEM dapOpenFile_FileItem[256];
char selectPath[256] = "0:/";//目录路径
char algorithmPath[256] = ""; //算法路径
char hexPath[256] = "";//文件路径
char addrPath[256] = "";//内存地址保存文件
bool algorithmFileIsExist = false;
extern en_result_t algorithmParseState;
extern bool getHexFileBinLength;
//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//
HMI_ENGINE_RESULT HMI_DapOpenFile_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  algorithmParseState = ErrorNotReady;
  getHexFileBinLength = false;
  SGUI_RECT                   stListLayout;
  if(NULL != pstDeviceIF->fnClear) {
    pstDeviceIF->fnClear();
  } else {
    SGUI_Basic_DrawRectangle(pstDeviceIF, 0, 0, pstDeviceIF->stSize.iWidth - 1, pstDeviceIF->stSize.iHeight - 1, SGUI_COLOR_BKGCLR, SGUI_COLOR_BKGCLR);
  }
  stListLayout.iX = 2;
  stListLayout.iY = 2;
  stListLayout.iWidth = pstDeviceIF->stSize.iWidth - 4;
  stListLayout.iHeight = pstDeviceIF->stSize.iHeight - 4;
  dapOpenFile_FileLength = 0;
  algorithmFileIsExist = false;
  memset(algorithmPath, 0, sizeof(algorithmPath));
  memset(hexPath, 0, sizeof(hexPath));
  memset(addrPath, 0, sizeof(addrPath));
  FRESULT res = FR_OK;
  if(pstParameters == NULL) {
    strcpy(selectPath, "0:/");
  }
  res = f_opendir(&openDir, selectPath);

  if(res == FR_OK) {

    while(true) {
      res = f_readdir(&openDir, &fileInfo);
      if(res == FR_OK && fileInfo.fname[0] != 0) {
        memcpy(dapOpenFile_AltName[dapOpenFile_FileLength], fileInfo.fname, FF_LFN_BUF);
        if((fileInfo.fattrib & AM_DIR)) {
          strcat(dapOpenFile_AltName[dapOpenFile_FileLength], "/");
        }
        if(strstr(dapOpenFile_AltName[dapOpenFile_FileLength], ".FLM")) {
          algorithmFileIsExist = true;
          strcat(algorithmPath, selectPath);
          strcat(algorithmPath, dapOpenFile_AltName[dapOpenFile_FileLength]);
        }
        if(strstr(dapOpenFile_AltName[dapOpenFile_FileLength], ".ADDR")) {
          algorithmFileIsExist = true;
          strcat(addrPath, selectPath);
          strcat(addrPath, dapOpenFile_AltName[dapOpenFile_FileLength]);
        }
        if((fileInfo.fattrib & (AM_HID | AM_SYS)) == 0) {
          dapOpenFile_FileItem[dapOpenFile_FileLength].szVariableText = dapOpenFile_AltName[dapOpenFile_FileLength];
          dapOpenFile_FileLength++;
          SGUI_List_InsertItem(&stcFileList, &dapOpenFile_FileItem[dapOpenFile_FileLength], SGUI_List_Count(&stcFileList));
        }
        if(dapOpenFile_FileLength >= 255) {
          break;
        }

      } else {
        break;
      }
    }

  }
  f_closedir(&openDir);
  SGUI_List_Initialize(&stcFileList, &stListLayout, &GB2312_FZXS12, "选择文件", dapOpenFile_FileItem, dapOpenFile_FileLength);

  HMI_DapOpenFile_RefreshScreen(pstDeviceIF, pstParameters);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_DapOpenFile_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  SGUI_INT                iPaintBkgIdx;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  SGUI_List_Repaint(pstDeviceIF, &stcFileList);
  // Paint background.
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_DapOpenFile_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_ENGINE_RESULT           eProcessResult;
  SGUI_INT                    iProcessAction;
  KEY_PRESS_EVENT*            pstKeyEvent;
  SGUI_UINT16             uiKeyCode;
  SGUI_UINT16             uiKeyValue;
  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  eProcessResult =            HMI_RET_NORMAL;
  iProcessAction =            HMI_DEMO_PROC_NO_ACT;
  pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  // Check event is valid.
  if(SGUI_FALSE == HMI_EVENT_SIZE_CHK(*pstKeyEvent, KEY_PRESS_EVENT)) {
    // Event data is invalid.
    eProcessResult = HMI_RET_INVALID_DATA;
  } else if(EVENT_ID_KEY_PRESS == pstEvent->iID) {
    uiKeyCode = pstKeyEvent->Data.uiKeyValue;
    uiKeyValue = KEY_CODE_VALUE(uiKeyCode);
    switch(uiKeyValue) {
    case KEY_VALUE_ESC:
      iProcessAction = HMI_DEMO_PROC_CANCEL;
      break;
    case KEY_VALUE_ENTER:
      if(strstr(SGUI_List_GetSelection(&stcFileList)->pstItem->szVariableText, "/")) {
        PRINTF_THIS("select is dir\n");
        strcat(selectPath, SGUI_List_GetSelection(&stcFileList)->pstItem->szVariableText);
        HMI_DapOpenFile_Prepare(pstDeviceIF, pstDeviceIF);
      } else {
        PRINTF_THIS("select is file\n");
        if(strstr(SGUI_List_GetSelection(&stcFileList)->pstItem->szVariableText, ".HEX") ||
            strstr(SGUI_List_GetSelection(&stcFileList)->pstItem->szVariableText, ".hex")) {
          strcat(hexPath, selectPath);
          strcat(hexPath, SGUI_List_GetSelection(&stcFileList)->pstItem->szVariableText);
          iProcessAction = HMI_DEMO_PROC_CANCEL;
        }
      }
      break;
    case KEY_VALUE_DOWN:
      if(SGUI_List_GetSelection(&stcFileList)->iIndex > 0) {
        SGUI_List_Selecte(&stcFileList, SGUI_List_GetSelection(&stcFileList)->iIndex - 1);
        stcFileList.stItems.stSelection.pstItem->xOffset = 20;
      }
      SGUI_List_Repaint(pstDeviceIF, &stcFileList);
      break;
    case KEY_VALUE_UP:
      if(SGUI_List_GetSelection(&stcFileList)->iIndex < (stcFileList.stItems.iCount - 1)) {
        SGUI_List_Selecte(&stcFileList, SGUI_List_GetSelection(&stcFileList)->iIndex + 1);
        stcFileList.stItems.stSelection.pstItem->xOffset = 20;
      }
      SGUI_List_Repaint(pstDeviceIF, &stcFileList);
      break;

    }
  } else if(EVENT_ID_TIMER == pstEvent->iID) {
    HMI_DapOpenFile_RefreshScreen(pstDeviceIF, NULL);
  }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }

  return eProcessResult;
}

HMI_ENGINE_RESULT HMI_DapOpenFile_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  if(HMI_PROCESS_SUCCESSFUL(eProcResult)) {
    if(HMI_DEMO_PROC_CANCEL == iActionID) {
      HMI_GoBack(NULL);
    }
  }

  return HMI_RET_NORMAL;
}

