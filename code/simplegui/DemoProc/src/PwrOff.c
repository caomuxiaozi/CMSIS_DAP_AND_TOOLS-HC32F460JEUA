/*************************************************************************/
/** Copyright.                                                          **/
/** FileName: Notice.c                                                  **/
/** Author: Polarix                                                     **/
/** Description: HMI demo for notice box interface.                     **/
/*************************************************************************/
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DemoProc.h"
#include "SGUI_Notice.h"
#include "SGUI_FontResource.h"

//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
#define                 NOTICE_TEXT_BUFFER_SIZE             (64)
extern bool             progNeedRun;
//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT    HMI_PwrOff_Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT    HMI_PwrOff_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_PwrOff_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_PwrOff_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT    HMI_PwrOff_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
static SGUI_NOTICT_BOX  s_stPwrOffBox =             {0x00};
static SGUI_CHAR        s_szPwrOffText[NOTICE_TEXT_BUFFER_SIZE + 1] = {0x00};

HMI_SCREEN_ACTION       s_stPwrOffActions =         {   HMI_PwrOff_Initialize,
                                                        HMI_PwrOff_Prepare,
                                                        HMI_PwrOff_RefreshScreen,
                                                        HMI_PwrOff_ProcessEvent,
                                                        HMI_PwrOff_PostProcess,
                                                    };

//=======================================================================//
//= Global variable declaration.                                        =//
//=======================================================================//
HMI_SCREEN_OBJECT       g_stHMIDemo_PwrOff =            {   HMI_SCREEN_ID_DEMO_PWR_OFF,
                                                            &s_stPwrOffActions
                                                        };
uint32_t pwrOffTickStart = 0;
//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//
HMI_ENGINE_RESULT HMI_PwrOff_Initialize(SGUI_SCR_DEV* pstDeviceIF) {
  SGUI_SystemIF_MemorySet(s_szPwrOffText, 0x00, sizeof(SGUI_CHAR) * (NOTICE_TEXT_BUFFER_SIZE + 1));
  s_stPwrOffBox.cszNoticeText = s_szPwrOffText;
  s_stPwrOffBox.pstIcon = NULL;
  s_stPwrOffBox.stLayout.iX = 8;
  s_stPwrOffBox.stLayout.iY = 8;
  s_stPwrOffBox.stLayout.iWidth  = pstDeviceIF->stSize.iWidth - 16;
  s_stPwrOffBox.stLayout.iHeight = pstDeviceIF->stSize.iHeight - 16;
//    SGUI_Notice_FitArea(pstDeviceIF, &(s_stPwrOffBox.stLayout));
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_PwrOff_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
//      pstDeviceIF->fnClear();
//    s_stPwrOffBox.pstIcon = NULL;
//    s_stPwrOffBox.stLayout.iX = 8+2;
//    s_stPwrOffBox.stLayout.iY = 8+2;
//    s_stPwrOffBox.stLayout.iWidth = pstDeviceIF->stSize.iWidth-16;
//    s_stPwrOffBox.stLayout.iHeight = pstDeviceIF->stSize.iHeight-16;
////    SGUI_Notice_FitArea(pstDeviceIF, &(s_stPwrOffBox.stLayout));

  if(NULL == pstParameters) {
    SGUI_SystemIF_StringLengthCopy(s_szPwrOffText, "NULL", NOTICE_TEXT_BUFFER_SIZE);

  } else {
//      SGUI_SystemIF_StringLengthCopy(s_szPwrOffText, "是否关机？向左←拨动关机，向右→拨动返回。", NOTICE_TEXT_BUFFER_SIZE);
    SGUI_SystemIF_StringLengthCopy(s_szPwrOffText, (SGUI_SZSTR)pstParameters, NOTICE_TEXT_BUFFER_SIZE);
    s_szPwrOffText[NOTICE_TEXT_BUFFER_SIZE] = '\0';
  }
  pwrOffTickStart = getTick();


  SGUI_Notice_Repaint(pstDeviceIF, &s_stPwrOffBox, &GB2312_FZXS12, 2);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_PwrOff_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  SGUI_Notice_Repaint(pstDeviceIF, &s_stPwrOffBox, &GB2312_FZXS12, 2);
  return HMI_RET_NORMAL;
}
extern uint8_t powerOff;
HMI_ENGINE_RESULT HMI_PwrOff_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_ENGINE_RESULT           eProcessResult;
  SGUI_UINT16                 uiKeyValue;
  KEY_PRESS_EVENT*            pstKeyEvent;
  SGUI_INT                    iProcessAction;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  eProcessResult =            HMI_RET_NORMAL;
  iProcessAction =            HMI_DEMO_PROC_NO_ACT;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  if(EVENT_ID_KEY_PRESS == pstEvent->iID) {
    pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;
    uiKeyValue = KEY_CODE_VALUE(pstKeyEvent->Data.uiKeyValue);

    switch(uiKeyValue) {
    case KEY_VALUE_ENTER:
      iProcessAction = HMI_DEMO_PROC_CANCEL;
      if(g_stDemoEngine.CurrentScreenObject->pstPrevious->iScreenID == HMI_SCREEN_ID_DEMO_DAP_OFFLINE) {
        progNeedRun = false;
      }
      break;
    case KEY_VALUE_ESC: {
      iProcessAction = HMI_DEMO_PROC_CANCEL;
      if(g_stDemoEngine.CurrentScreenObject->pstPrevious->iScreenID == HMI_SCREEN_ID_DEMO_DAP_OFFLINE) {
        progNeedRun = false;
      }
    }
    break;
    case KEY_VALUE_DOWN:
      if(strstr(s_szPwrOffText, "关机")) {
        powerOff = 3;
      }
      break;
    case KEY_VALUE_UP:
      if(strstr(s_szPwrOffText, "关机")) {
        iProcessAction = HMI_DEMO_PROC_CANCEL;
      }
      break;
    default:
      break;
    }
  } else if(EVENT_ID_TIMER == pstEvent->iID) {
    if(strstr(s_szPwrOffText, "关机") == NULL && strstr(s_szPwrOffText, "电量低") == NULL) {
      buzzerCtrl(false);
      if(getTick() - pwrOffTickStart > 3000) {
        iProcessAction = HMI_DEMO_PROC_CANCEL;
      }
    }
    HMI_PwrOff_RefreshScreen(pstDeviceIF, NULL);
  }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }

  return eProcessResult;
}

HMI_ENGINE_RESULT HMI_PwrOff_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
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

