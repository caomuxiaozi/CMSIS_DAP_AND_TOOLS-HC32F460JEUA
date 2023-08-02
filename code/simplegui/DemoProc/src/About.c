/*************************************************************************/
/** Copyright.                                                          **/
/** FileName: Text.c                                                    **/
/** Author: Polarix                                                     **/
/** Description: HMI demo for text paint interface.                     **/
/*************************************************************************/

//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DemoProc.h"
#include "SGUI_Text.h"
#include "SGUI_FontResource.h"
#include "Resource.h"
#include "ff.h"
//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT    HMI_About_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_About_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_About_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT    HMI_About_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_ACTION           s_stAboutActions =   {   NULL,
                                                     HMI_About_Prepare,
                                                     HMI_About_RefreshScreen,
                                                     HMI_About_ProcessEvent,
                                                     HMI_About_PostProcess
                                                 };

//=======================================================================//
//= Global variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_OBJECT       g_stHMIDemo_About =         {   HMI_SCREEN_ID_DEMO_ABOUT,
                                                        &s_stAboutActions
                                                    };

//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//
char* AboutStr = "open Version\r\nBÕ¾UP¸üÄ¾½£°ËÇ§Á÷\r\nweChat:15877977992\r\nEmail:296938087@qq.com";

int16_t i8AboutTextLines = 0;
int16_t i8AboutTextLinesIndex = 0;
HMI_ENGINE_RESULT HMI_About_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  SGUI_RECT                   stDisplayArea;
  SGUI_POINT                  stInnerPos = {0, 0};

  i8AboutTextLines = 0;
  i8AboutTextLinesIndex = 0;
  if(NULL != pstDeviceIF->fnClear) {
    pstDeviceIF->fnClear();
    stDisplayArea.iX = 2;
    stDisplayArea.iY = 2;
    stDisplayArea.iWidth = 128;
    stDisplayArea.iHeight = 64;
    uint16_t index = 0;
    uint8_t column = 2;
    while(AboutStr[index] != '\0') {
      if((uint8_t)AboutStr[index] > 0xA0) {
        column += 12;
        if(column > 128) {
          column = 14;
          i8AboutTextLines++;
        }
        index += 2;
      } else {
        if(AboutStr[index] == '\n') {
          column = 129;
        } else {
          column += 6;
        }
        if(column > 128) {
          column = 8;
          i8AboutTextLines++;
        }
        index += 1;
      }
    }
    SGUI_Text_DrawMultipleLinesText(pstDeviceIF, AboutStr + i8AboutTextLinesIndex * 20, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);

  }
//    else
//    {
//
////        SGUI_Basic_DrawRectangle(pstDeviceIF, 0, 0, pstDeviceIF->stSize.iWidth, pstDeviceIF->stSize.iHeight, SGUI_COLOR_BKGCLR, SGUI_COLOR_BKGCLR);
//    }

  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_About_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  SGUI_RECT                   stDisplayArea;
  SGUI_POINT                  stInnerPos;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  stInnerPos.iX =         0;
  stInnerPos.iY =         0;
  stDisplayArea.iX = 2;
  stDisplayArea.iY = 2;
  stDisplayArea.iWidth = 128;
  stDisplayArea.iHeight = 64;
  uint16_t index = 0;
  for(uint8_t page = 0; page < i8AboutTextLinesIndex; page++) {
    uint8_t column = stDisplayArea.iX;
    while(column < 128) {
      if((uint8_t)AboutStr[index] > 0xA0) {
        column += 12;
        if(column <= 128)
          index += 2;
      } else {
        if(AboutStr[index] == '\n') {
          column = 128;
        } else {
          column += 6;
        }
        if(column <= 128)
          index += 1;
      }
    }
//      page++;
  }
//    pstDeviceIF->fnClear();
  SGUI_Text_DrawMultipleLinesText(pstDeviceIF, AboutStr + index, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
//    SGUI_Text_DrawText(pstDeviceIF, SCR3_TEXT_MIN_NUM, &SGUI_DEFAULT_FONT_MiniNum, &stDisplayArea, &stInnerPos, SGUI_DRAW_NORMAL);

//    stDisplayArea.iY += stDisplayArea.iHeight;
//    stDisplayArea.iWidth = 62;
//    stDisplayArea.iHeight = SGUI_DEFAULT_FONT_8.iHeight+1;
//    SGUI_Text_DrawText(pstDeviceIF, SCR3_TEXT_INNER_8, &SGUI_DEFAULT_FONT_8, &stDisplayArea, &stInnerPos, SGUI_DRAW_NORMAL);

//    stDisplayArea.iY += stDisplayArea.iHeight;
//    stDisplayArea.iWidth = 62;
//    stDisplayArea.iHeight = SGUI_DEFAULT_FONT_12.iHeight+1;
//    SGUI_Text_DrawText(pstDeviceIF, SCR3_TEXT_INNER_12, &SGUI_DEFAULT_FONT_12, &stDisplayArea, &stInnerPos, SGUI_DRAW_REVERSE);

//    stDisplayArea.iY += stDisplayArea.iHeight;
//    stDisplayArea.iWidth = 62;
//    stDisplayArea.iHeight = SGUI_DEFAULT_FONT_12.iHeight+1;
//    stDisplayArea.iX = 64;
//    stDisplayArea.iY = 1;
//    stDisplayArea.iWidth = 62;
//    stDisplayArea.iHeight = 62;
//    SGUI_Text_DrawMultipleLinesText(pstDeviceIF, SCR3_TEXT_MULTILINE_12, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_About_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_ENGINE_RESULT           eProcessResult;
  SGUI_INT                    iProcessAction;
  KEY_PRESS_EVENT*            pstKeyEvent;
  DATA_EVENT*                 pstDataEvent;
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
  switch(pstEvent->iID) {
  case EVENT_ID_TIMER:
    pstDataEvent = (DATA_EVENT*)pstEvent;
//        HMI_About_RefreshScreen(pstDeviceIF,NULL);
    break;
  case EVENT_ID_KEY_PRESS:
    pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;
    SGUI_UINT16 uiKeyValue = KEY_CODE_VALUE(pstKeyEvent->Data.uiKeyValue);
    if(KEY_VALUE_ESC == uiKeyValue) {
      iProcessAction = HMI_DEMO_PROC_CANCEL;
    } else if(KEY_VALUE_UP == uiKeyValue) {
      if(++i8AboutTextLinesIndex > i8AboutTextLines) {
        i8AboutTextLinesIndex = i8AboutTextLines;
      }
//          pstDeviceIF->fnClear();
      HMI_About_RefreshScreen(pstDeviceIF, NULL);
    } else if(KEY_VALUE_DOWN == uiKeyValue) {
      if(--i8AboutTextLinesIndex < 0) {
        i8AboutTextLinesIndex = 0;
      }
//          pstDeviceIF->fnClear();
      HMI_About_RefreshScreen(pstDeviceIF, NULL);
    }
    break;
  default:
    break;
  }
//    if(SGUI_FALSE == HMI_EVENT_SIZE_CHK(*pstKeyEvent, KEY_PRESS_EVENT))
//    {
//        // Event data is invalid.
//        eProcessResult = HMI_RET_INVALID_DATA;
//    }
//    else if(EVENT_ID_KEY_PRESS == pstEvent->iID)
//    {
//        iProcessAction = HMI_DEMO_PROC_CANCEL;
//    }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }

  return eProcessResult;
}

HMI_ENGINE_RESULT HMI_About_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  if(HMI_PROCESS_SUCCESSFUL(eProcResult)) {
    if(HMI_DEMO_PROC_CANCEL == iActionID) {
      if(AboutStr) {
        mFREE(AboutStr);
      }
      HMI_GoBack(NULL);
    }
  }

  return HMI_RET_NORMAL;
}

