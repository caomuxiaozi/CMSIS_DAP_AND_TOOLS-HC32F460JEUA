/*************************************************************************/
/** Copyright.                                                          **/
/** FileName: DemoProc.c                                                **/
/** Author: Polarix                                                     **/
/** Description: User operation interface.                              **/
/*************************************************************************/
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DemoProc.h"
#include "SGUI_Basic.h"

#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
#include "SDKInterface.h"
#include "SGUI_FontResource.h"
#else
#include "screen.h"
//#include "usart.h"
//#include "rtc.h"
//#include "base_timer.h"
#include "DemoActions.h"
#include "driver.h"
//#include "trigger_flags.h"
//#include "keyboard.h"
#endif

//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
#define     BMP_DATA_BUFFER_SIZE    (512)

//=======================================================================//
//= Static variable declaration.                                        =//
//=======================================================================//
SGUI_SCR_DEV                g_stDeviceInterface;
HMI_SCREEN_OBJECT*          g_arrpstScreenObjs[] = {
  &g_stHMIDemo_MainMenu,
  &g_stHMIDemo_DapOnline,
  &g_stHMIDemo_DapOffline,
  &g_stHMIDemo_About,
  &g_stHMIDemo_PwrOff,
  &g_stHMIDemo_DapOpenFile,
};
HMI_ENGINE_OBJECT           g_stDemoEngine;

SGUI_BYTE                   s_pBmpDataBuffer[BMP_DATA_BUFFER_SIZE];
//=======================================================================//
//= Static function declare.                                            =//
//=======================================================================//
static void                 KeyPressEventProc(void);
static void                 RTCEventProc(void);
static void                 SysTickTimerEventProc(void);
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
static bool                 CheckEventFlag(ENV_FLAG_INDEX eIndex);
#endif //_SIMPLE_GUI_IN_VIRTUAL_SDK_

//=======================================================================//
//= Function define.                                                    =//
//=======================================================================//

/*****************************************************************************/
/** Function Name:  InitializeHMIEngineObj                                  **/
/** Purpose:        Simple GUI HMI engine and interface demo process.       **/
/** Parameters:     None.                                                   **/
/** Return:         HMI_ENGINE_RESULT.                                      **/
/** Notice:         This function demonstrates how to use the interface and **/
/**                 HMI engine of Simple GUI.                               **/
/*****************************************************************************/
HMI_ENGINE_RESULT InitializeHMIEngineObj(void) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_ENGINE_RESULT           eProcessResult;
  int                         iIndex;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  eProcessResult =            HMI_RET_NORMAL;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  /* Clear structure. */
  SGUI_SystemIF_MemorySet(&g_stDeviceInterface, 0x00, sizeof(SGUI_SCR_DEV));
  SGUI_SystemIF_MemorySet(&g_stDemoEngine, 0x00, sizeof(HMI_ENGINE_OBJECT));
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  /* Initialize display size. */
  g_stDeviceInterface.stSize.iWidth = 128;
  g_stDeviceInterface.stSize.iHeight = 64;
  /* Initialize interface object. */
  g_stDeviceInterface.fnSetPixel = SGUI_SDK_SetPixel;
#ifdef SGUI_GET_POINT_FUNC_EN
  g_stDeviceInterface.fnGetPixel = SGUI_SDK_GetPixel;
#endif // SGUI_GET_POINT_FUNC_EN
  g_stDeviceInterface.fnFillRect = SGUI_SDK_FillRectangle;
  g_stDeviceInterface.fnClear = SGUI_SDK_ClearDisplay;
  g_stDeviceInterface.fnSyncBuffer = SGUI_SDK_RefreshDisplay;
#else
//    #error Add screen device object initialize process here.
  g_stDeviceInterface.stSize.iWidth = 128;
  g_stDeviceInterface.stSize.iHeight = 64;
  g_stDeviceInterface.fnSetPixel = SCREEN_SetPixel;
  g_stDeviceInterface.fnClear = SCREEN_ClearDisplay;
  g_stDeviceInterface.fnSyncBuffer = SCREEN_RefreshScreen;
#endif
  g_stDeviceInterface.stBuffer.pBuffer = s_pBmpDataBuffer;
  g_stDeviceInterface.stBuffer.sSize = BMP_DATA_BUFFER_SIZE;
  do {
    /* Prepare HMI engine object. */
    g_stDemoEngine.ScreenCount = sizeof(g_arrpstScreenObjs) / sizeof(*g_arrpstScreenObjs);
    g_stDemoEngine.ScreenObjPtr = g_arrpstScreenObjs;
    g_stDemoEngine.Interface = &g_stDeviceInterface;

    /* Initialize all screen object. */
    if(NULL != g_stDemoEngine.ScreenObjPtr) {
      for(iIndex = 0; iIndex < g_stDemoEngine.ScreenCount; iIndex++) {
        if((NULL != g_stDemoEngine.ScreenObjPtr[iIndex])
            && (NULL != g_stDemoEngine.ScreenObjPtr[iIndex]->pstActions)
            && (NULL != g_stDemoEngine.ScreenObjPtr[iIndex]->pstActions->Initialize)
          ) {
          g_stDemoEngine.ScreenObjPtr[iIndex]->pstActions->Initialize(&g_stDeviceInterface);
          g_stDemoEngine.ScreenObjPtr[iIndex]->pstPrevious = NULL;
        }
      }
    } else {

    }
    /* Active engine object. */
    eProcessResult = HMI_ActiveEngine(&g_stDemoEngine, /*HMI_SCREEN_ID_DEMO_MULTIMETER*/HMI_SCREEN_ID_DEMO_MAIN_MENU);
    if(HMI_PROCESS_FAILED(eProcessResult)) {
      /* Active engine failed. */
      break;
    }
    /* Start engine process. */
    eProcessResult = HMI_StartEngine(NULL);
    if(HMI_PROCESS_FAILED(eProcessResult)) {
      /* Start engine failed. */
      break;
    }
  } while(0);

  return eProcessResult;
}

#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
/*****************************************************************************/
/** Function Name:  CheckEventFlag                                          **/
/** Purpose:        Check SimpleGUI virtual SDK event trigger flag and      **/
/**                 reset for next trigger and check.                       **/
/** Parameters:                                                             **/
/** @ eIndex[in]:   Checked flag index.                                     **/
/** Return:         true for event is trigger, false for not.               **/
/** Notice:         This function only used in SimpleGUI virtual SDK        **/
/*****************************************************************************/
bool CheckEventFlag(ENV_FLAG_INDEX eIndex) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  bool                    bReturn;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  bReturn = SGUI_SDK_GetEventSyncFlag(eIndex);

  if(true == bReturn) {
    SGUI_SDK_SetEvnetSyncFlag(eIndex, false);
  }

  return bReturn;
}
#endif // _SIMPLE_GUI_VIRTUAL_ENVIRONMENT_SIMULATOR_

/*****************************************************************************/
/** Function Name:  DemoMainProcess                                         **/
/** Purpose:        It is a dummy main function for SimpleGUI Virtual SDK,  **/
/**                 or run demo process in demo process.                    **/
/** Parameters:     None.                                                   **/
/** Return:         Terminal code, seam as main function return code.       **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
void DemoMainProcess(void) {
  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  // Initialize HMI Engine.


  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
//    while(1)
//    {
  // Check and process heart-beat timer event.
//        if(true == SysTickTimerTriggered())
//        {

//        }
  // Check and process key press event.
//        if(true == UserEventTriggered())
  if(Btn_Ok.shortPressed + Btn_Ok.longPressd + Btn_Dec.shortPressed + Btn_Inc.shortPressed) {
    KeyPressEventProc();
  } else {
    SysTickTimerEventProc();
  }
  // Check and process RTC event.
//        if(true == RTCTimerTriggered())
//        {
//            RTCEventProc();
//        }
//    }
}

/*****************************************************************************/
/** Function Name:  KeyPressEventProc                                       **/
/** Purpose:        Deal with user key process in demo process.             **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         The button is just abstract, and the message may come   **/
/**                 from a touch screen, a serial port, and so on.          **/
/*****************************************************************************/
void KeyPressEventProc(void) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  KEY_PRESS_EVENT         stEvent;
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  const SDK_KB_EVENT*     pstSDKEvent;
#endif // _SIMPLE_GUI_IN_VIRTUAL_SDK_

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  HMI_EVENT_INIT(stEvent);

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  stEvent.Head.iID = EVENT_ID_KEY_PRESS;
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  pstSDKEvent = SGUI_SDK_GetKeyEventData();
  stEvent.Data.uiKeyValue = pstSDKEvent->iKeyCode;
  if(pstSDKEvent->bShift) {
    stEvent.Data.uiKeyValue |= KEY_OPTION_SHIFT;
  }
  if(pstSDKEvent->bCtrl) {
    stEvent.Data.uiKeyValue |= KEY_OPTION_CTRL;
  }
  if(pstSDKEvent->bAlt) {
    stEvent.Data.uiKeyValue |= KEY_OPTION_ALT;
  }
#else
//    #error Add key event data prepare process here.
  if(Btn_Dec.shortPressed) {
    Btn_Dec.shortPressed = 0;
    stEvent.Data.uiKeyValue = KEY_VALUE_UP;
  }
  if(Btn_Inc.shortPressed) {
    Btn_Inc.shortPressed = 0;
    stEvent.Data.uiKeyValue = KEY_VALUE_DOWN;
  }
  if(Btn_Ok.shortPressed) {
    Btn_Ok.shortPressed = 0;
    stEvent.Data.uiKeyValue = KEY_VALUE_ENTER;
  }
  if(Btn_Ok.longPressd) {
    Btn_Ok.longPressd = 0;
    stEvent.Data.uiKeyValue = KEY_VALUE_ESC;
  }

#endif
  // Post key press event.
  if(stEvent.Data.uiKeyValue != KEY_VALUE_NONE)
    HMI_ProcessEvent((HMI_EVENT_BASE*)(&stEvent));
  stEvent.Data.uiKeyValue = KEY_VALUE_NONE;
}

/*****************************************************************************/
/** Function Name:  SysTickTimerEventProc                                   **/
/** Purpose:        Deal with heartbeat timer event in demo process.        **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         It is usually used to simulate ADC or other sensors.    **/
/*****************************************************************************/
void SysTickTimerEventProc(void) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  DATA_EVENT              stEvent;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  HMI_EVENT_INIT(stEvent);

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  stEvent.Head.iID =      EVENT_ID_TIMER;
  stEvent.Data.iValue = (rand() % 256);
  // Post timer event.
  HMI_ProcessEvent((HMI_EVENT_BASE*)&stEvent);
}

/*****************************************************************************/
/** Function Name:  RTCEventProc                                            **/
/** Purpose:        Deal with RTC timer event in demo process.              **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         It is usually used to simulate ADC or other sensors.    **/
/*****************************************************************************/
void RTCEventProc(void) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_GENERAL_EVENT       stEvent;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  HMI_EVENT_INIT(stEvent);

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  stEvent.Head.iID =      EVENT_ID_RTC;
  // Post RTC update message to a screen.
  HMI_ProcessEvent((HMI_EVENT_BASE*)&stEvent);
}

/*****************************************************************************/
/** Function Name:  SysTickTimerTriggered                                   **/
/** Purpose:        Check sys-tick timer interrupt is triggered.            **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
bool SysTickTimerTriggered(void) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  return CheckEventFlag(ENV_FLAG_IDX_SDK_TIM_EVENT);
#else
//    #error Add sys-tick timer trigger process here.
  return false;
#endif
}

/*****************************************************************************/
/** Function Name:  RTCTimerTriggered                                       **/
/** Purpose:        Check RTC timer interrupt is triggered.                 **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
bool RTCTimerTriggered(void) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  return CheckEventFlag(ENV_FLAG_IDX_SDK_SEC_EVENT);
#else
  // // Add Dummy RTC trigger process here.
  return false;
#endif
}

/*****************************************************************************/
/** Function Name:  UserEventTriggered                                      **/
/** Purpose:        Check user event is triggered.                          **/
/** Parameters:     None.                                                   **/
/** Return:         None.                                                   **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
bool UserEventTriggered(void) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  return CheckEventFlag(ENV_FLAG_IDX_SDK_KEY_EVENT);
#else
//    #error Add user event trigger process here.

  return false;
#endif
}

/*****************************************************************************/
/** Function Name:  SysTickTimerEnable                                      **/
/** Purpose:        Enable or disable sys-tick timer.                       **/
/** Parameters:                                                             **/
/** @ bEnable[in]:  True for enable sys-tick timer, false for disable.      **/
/** Return:         None.                                                   **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
void SysTickTimerEnable(bool bEnable) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  (void)SGUI_SDK_ConfigGeneralTimer(bEnable ? DEMO_HEART_BEAT_INTERVAL_MS : 0);
#else
//    #error Add sys-tick timer enable/disable process here.

#endif
}

/*****************************************************************************/
/** Function Name:  RTCTimerEnable                                          **/
/** Purpose:        Enable or disable RTC timer.                            **/
/** Parameters:                                                             **/
/** @ bEnable[in]:  True for enable RTC timer, false for disable.           **/
/** Return:         None.                                                   **/
/** Notice:         None.                                                   **/
/*****************************************************************************/
void RTCTimerEnable(bool bEnable) {

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
  (void)SGUI_SDK_EnableSecondInterrupt(bEnable);
#endif
}

