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
#include "eeprom.h"
//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT    HMI_DapOnline_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_DapOnline_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_DapOnline_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT    HMI_DapOnline_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_ACTION           s_stDapOnlineActions =   {   NULL,
                                                         HMI_DapOnline_Prepare,
                                                         HMI_DapOnline_RefreshScreen,
                                                         HMI_DapOnline_ProcessEvent,
                                                         HMI_DapOnline_PostProcess
                                                     };

//=======================================================================//
//= Global variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_OBJECT       g_stHMIDemo_DapOnline =         {   HMI_SCREEN_ID_DEMO_DAP_ONLINE,
                                                            &s_stDapOnlineActions
                                                        };

//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//

HMI_ENGINE_RESULT HMI_DapOnline_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  PORT_SetFunc(PortA, Pin09, Func_UsbF, Disable);
  f_close(&font12File);
  f_close(&font16File);
  f_mount(NULL, "0:", 0);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_DapOnline_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  SGUI_RECT                   stDisplayArea;
  SGUI_POINT                  stInnerPos;
  stInnerPos.iX =         0;
  stInnerPos.iY =         0;
  stDisplayArea.iX = 2;
  stDisplayArea.iY = 2;
  stDisplayArea.iWidth = 128;
  stDisplayArea.iHeight = 64;
  char DapOnlineTips[256];
  snprintf(DapOnlineTips, 256, "DAP-Link Online Mode\nPWR Time:%dm\nyou can use:\nKeil or IAR or BYDown", sysRunMinute * 2);
  SGUI_Text_DrawMultipleLinesText(pstDeviceIF, DapOnlineTips, &GB2312_FZXS12, &stDisplayArea, 0, SGUI_DRAW_NORMAL);
  char strs[21];
  uint32_t tick = getTick();
  uint32_t second = tick / 1000;
  uint32_t minutes = (second / 60) % 60;
  uint16_t hour = second / (60 * 60);
  sprintf(strs, "SysTick:%03d:%02d:%02d", hour, minutes, second % 60);
  stDisplayArea.iY = 49;
  stDisplayArea.iWidth = 128;
  stDisplayArea.iHeight = 12;
  SGUI_Text_DrawText(pstDeviceIF, strs, &GB2312_FZXS12, &stDisplayArea, &stInnerPos, SGUI_DRAW_NORMAL);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_DapOnline_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  HMI_ENGINE_RESULT           eProcessResult;
  SGUI_INT                    iProcessAction;
  KEY_PRESS_EVENT*            pstKeyEvent;
  DATA_EVENT*                 pstDataEvent;
  eProcessResult =            HMI_RET_NORMAL;
  iProcessAction =            HMI_DEMO_PROC_NO_ACT;
  pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;
  switch(pstEvent->iID) {
  case EVENT_ID_TIMER:
    pstDataEvent = (DATA_EVENT*)pstEvent;
    HMI_DapOnline_RefreshScreen(pstDeviceIF, NULL);
    break;
  case EVENT_ID_KEY_PRESS:
    pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;
    SGUI_UINT16 uiKeyValue = KEY_CODE_VALUE(pstKeyEvent->Data.uiKeyValue);
    if(KEY_VALUE_ESC == uiKeyValue) {
      iProcessAction = HMI_DEMO_PROC_CANCEL;
    }
    break;
  default:
    break;
  }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }

  return eProcessResult;
}

HMI_ENGINE_RESULT HMI_DapOnline_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
  if(HMI_PROCESS_SUCCESSFUL(eProcResult)) {
    if(HMI_DEMO_PROC_CANCEL == iActionID) {
      PORT_SetFunc(PortA, Pin09, Func_Gpio, Disable);
      f_mount(&FatSd, "0:", 0);
      f_open(&font12File, FontLibZh12Path, FA_READ);
      f_open(&font16File, FontLibZh16Path, FA_READ);
      HMI_GoBack(NULL);
    }
  }

  return HMI_RET_NORMAL;
}

