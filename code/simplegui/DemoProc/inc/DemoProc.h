#ifndef __INCLUDE_DEMO_PROC_H__
#define __INCLUDE_DEMO_PROC_H__
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include <stdlib.h>
#include <stdio.h>
#include "SGUI_Typedef.h"
#include "SGUI_Common.h"
#include "HMI_Engine.h"
#include "DemoActions.h"
#include "Resource.h"
#include "oled.h"
#ifdef _SIMPLE_GUI_IN_VIRTUAL_SDK_
#include "SDKInterface.h"
#endif

//=======================================================================//
//= Data type definition.                                               =//
//=======================================================================//
// HMI action type
typedef enum {
  EVENT_ID_UNKNOW =               0,
  // User defined action type.
  EVENT_ID_KEY_PRESS,
  EVENT_ID_TIMER,
  EVENT_ID_RTC,
  // User defined action above.
  EVENT_ID_MAX
} EVENT_ID;

// HMI event type.
typedef enum {
  EVENT_TYPE_ANY =                0,
  EVENT_TYPE_ACTION,
  EVENT_TYPE_DATA,
} EVENT_TYPE;

enum DEMO_PROC_ACTION_ID {
  HMI_DEMO_PROC_NO_ACT =          0,
  HMI_DEMO_PROC_CONFIRM =         1,
  HMI_DEMO_PROC_CANCEL =          2,
};

typedef struct {
  SGUI_UINT16     uiKeyValue;
} KEY_EVENT_DATA;

typedef struct {
  SGUI_INT        iValue;
} DUMMY_DATA_EVENT_DATA;

HMI_EVENT_TYPE_DECLARE(KEY_PRESS_EVENT, KEY_EVENT_DATA)
HMI_EVENT_TYPE_DECLARE(DATA_EVENT, DUMMY_DATA_EVENT_DATA);

//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
#define     HMI_SCREEN_ID_DEMO_MAIN_MENU                (1001)
#define     HMI_SCREEN_ID_DEMO_DAP_ONLINE               (1002)
#define     HMI_SCREEN_ID_DEMO_DAP_OFFLINE              (1003)
#define     HMI_SCREEN_ID_DEMO_ABOUT                    (1009)
#define     HMI_SCREEN_ID_DEMO_PWR_OFF                  (1010)
#define     HMI_SCREEN_ID_DEMO_OPENFILE                 (1100)

#define     DEMO_HEART_BEAT_INTERVAL_MS                 (20)

//=======================================================================//
//= Public variable declaration.                                        =//
//=======================================================================//
/* HMI engine object. */
extern HMI_ENGINE_OBJECT        g_stDemoEngine;
/* Screen display objects. */
extern HMI_SCREEN_OBJECT        g_stHMIDemo_MainMenu;
extern HMI_SCREEN_OBJECT        g_stHMIDemo_DapOnline;
extern HMI_SCREEN_OBJECT        g_stHMIDemo_DapOffline;
extern HMI_SCREEN_OBJECT        g_stHMIDemo_PwrOff;
extern HMI_SCREEN_OBJECT        g_stHMIDemo_About;
extern HMI_SCREEN_OBJECT        g_stHMIDemo_DapOpenFile;

//=======================================================================//
//= Function declare.                                                   =//
//=======================================================================//
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
HMI_ENGINE_RESULT   InitializeHMIEngineObj(void);
void                DemoMainProcess(void);

bool                SysTickTimerTriggered(void);
bool                RTCTimerTriggered(void);
bool                UserEventTriggered(void);

void                SysTickTimerEnable(bool bEnable);
void                RTCTimerEnable(bool bEnable);
extern uint32_t getTick(void);
extern uint32_t getTick_Sec(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // __INCLUDE_DEMO_PROC_H__
