/*************************************************************************/
/** Copyright.                                                          **/
/** FileName: List.c                                                    **/
/** Author: Polarix                                                     **/
/** Description: HMI demo for list control interface.                   **/
/*************************************************************************/
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "DemoProc.h"
#include "SGUI_List.h"
#include "SGUI_FontResource.h"
#include "Resource.h"
#include "hc32_ddl.h"
uint16_t batVoltagemV = 3700;
uint8_t batIsCharging = false;
//=======================================================================//
//= Macro definition.                                                   =//
//=======================================================================//
#define                 NOTICE_TEXT_BUFFER_SIZE             (64)

//=======================================================================//
//= Static function declaration.                                        =//
//=======================================================================//
static HMI_ENGINE_RESULT    HMI_MainMenu_Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT    HMI_MainMenu_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_MainMenu_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT    HMI_MainMenu_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT    HMI_MainMenu_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);


static SGUI_ITEMS_ITEM      s_arrstListItems[] = {
  {SCR1_LIST_ITEM1, NULL},
  {SCR1_LIST_ITEM2, NULL},
  {SCR1_LIST_ITEM8, NULL},
  {SCR1_LIST_ITEM9, NULL}
};

typedef struct {
  SGUI_ITEMS_ITEM stItem;
  SGUI_BOOL       bUsed;
} SGUI_TEST_ITEM;


static SGUI_LIST        s_stMainMenuObject =        {0x00};

HMI_SCREEN_ACTION       s_stMainMenuActions =           {   HMI_MainMenu_Initialize,
                                                            HMI_MainMenu_Prepare,
                                                            HMI_MainMenu_RefreshScreen,
                                                            HMI_MainMenu_ProcessEvent,
                                                            HMI_MainMenu_PostProcess
                                                        };
HMI_SCREEN_OBJECT       g_stHMIDemo_MainMenu =              {   HMI_SCREEN_ID_DEMO_MAIN_MENU,
                                                                &s_stMainMenuActions
                                                            };

HMI_ENGINE_RESULT HMI_MainMenu_Initialize(SGUI_SCR_DEV* pstDeviceIF) {
  SGUI_RECT                   stListLayout;
  SGUI_SystemIF_MemorySet(&s_stMainMenuObject, 0x00, sizeof(SGUI_LIST));
  stListLayout.iX = 0;
  stListLayout.iY = 0;
  stListLayout.iWidth = pstDeviceIF->stSize.iWidth;
  stListLayout.iHeight = pstDeviceIF->stSize.iHeight;
  SGUI_List_Initialize(&s_stMainMenuObject, &stListLayout, &GB2312_FZXS12, SCR1_TITLE, s_arrstListItems, sizeof(s_arrstListItems) / sizeof(SGUI_ITEMS_ITEM));
  return HMI_RET_NORMAL;
}
extern uint8_t oled_Brightness;
stc_clk_mpll_cfg_t stcMpllCfg;
void HMI_MainMenu_SetClkHigh(bool en) {
  if(en) {
    stcMpllCfg.pllmDiv = 2ul;  // 8M MPLL的VCO输入频率介于1MHz到12MHz之间。
    stcMpllCfg.plln    = 48ul;  // 384M 确保MPLL的VCO频率介于240MHz到480MHz之间
    stcMpllCfg.PllpDiv = 2ul;  // 192M
    stcMpllCfg.PllqDiv = 2ul;  // 192M
    stcMpllCfg.PllrDiv = 2ul;  // 192M
    CLK_MpllConfig(&stcMpllCfg);
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);
  } else {
    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllCfg.pllmDiv = 2ul;  // 8M MPLL的VCO输入频率介于1MHz到12MHz之间。
    stcMpllCfg.plln    = 6ul;  // 384M 确保MPLL的VCO频率介于240MHz到480MHz之间
    stcMpllCfg.PllpDiv = 2ul;  // 192M
    stcMpllCfg.PllqDiv = 2ul;  // 192M
    stcMpllCfg.PllrDiv = 2ul;  // 192M
    CLK_MpllConfig(&stcMpllCfg);
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000u);
  }
}
HMI_ENGINE_RESULT HMI_MainMenu_Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  MEM_ZERO_STRUCT(stcMpllCfg);
  HMI_MainMenu_SetClkHigh(true);
  SGUI_List_Repaint(pstDeviceIF, &s_stMainMenuObject);
  return HMI_RET_NORMAL;
}

HMI_ENGINE_RESULT HMI_MainMenu_RefreshScreen(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  SGUI_List_Repaint(pstDeviceIF, &s_stMainMenuObject);
  char batVtgChar[5];
  if(batIsCharging) {
    sprintf(batVtgChar, "C%.2f", batVoltagemV / 1000.0f);
  } else {
    sprintf(batVtgChar, "%.2fV", batVoltagemV / 1000.0f);
  }
  SGUI_RECT           stTextDisplayArea;
  SGUI_POINT          stInnerPos;

  /*----------------------------------*/
  /* Process                          */
  /*----------------------------------*/
  if(oled_Brightness == OLED_BRIGHTNESS_MIN) {
    HMI_MainMenu_SetClkHigh(false);
  } else {
    HMI_MainMenu_SetClkHigh(true);
  }


  // Paint title
  stTextDisplayArea.iX = 95;
  stTextDisplayArea.iY = 2;
  stTextDisplayArea.iWidth = 30;
  stTextDisplayArea.iHeight = 8;
  stInnerPos.iX = 0;
  stInnerPos.iY = 0;
  SGUI_Text_DrawText(pstDeviceIF, batVtgChar, &SGUI_DEFAULT_FONT_8, &stTextDisplayArea, &stInnerPos, batIsCharging ? SGUI_DRAW_REVERSE : SGUI_DRAW_NORMAL);
  return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT HMI_MainMenu_ProcessEvent(SGUI_SCR_DEV* pstDeviceIF, const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
  /*----------------------------------*/
  /* Variable Declaration             */
  /*----------------------------------*/
  HMI_ENGINE_RESULT           eProcessResult;
  SGUI_UINT16                 uiKeyCode;
  SGUI_UINT16                 uiKeyValue;
  KEY_PRESS_EVENT*            pstKeyEvent;
  SGUI_INT                    iProcessAction;

  /*----------------------------------*/
  /* Initialize                       */
  /*----------------------------------*/
  eProcessResult =            HMI_RET_NORMAL;
  pstKeyEvent = (KEY_PRESS_EVENT*)pstEvent;
  iProcessAction =            HMI_DEMO_PROC_NO_ACT;

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
    case KEY_VALUE_ENTER: {
      iProcessAction = HMI_DEMO_PROC_CONFIRM;
      break;
    }
    case KEY_VALUE_ESC: {
      iProcessAction = HMI_DEMO_PROC_CANCEL;
      break;
    }
    case KEY_VALUE_DOWN: {
      if(SGUI_List_GetSelection(&s_stMainMenuObject)->iIndex > 0) {
        SGUI_List_Selecte(&s_stMainMenuObject, SGUI_List_GetSelection(&s_stMainMenuObject)->iIndex - 1);
      }
      SGUI_List_Repaint(pstDeviceIF, &s_stMainMenuObject);
      break;
    }
    case KEY_VALUE_UP: {
      if(SGUI_List_GetSelection(&s_stMainMenuObject)->iIndex < (s_stMainMenuObject.stItems.iCount - 1)) {
        SGUI_List_Selecte(&s_stMainMenuObject, SGUI_List_GetSelection(&s_stMainMenuObject)->iIndex + 1);
      }
      SGUI_List_Repaint(pstDeviceIF, &s_stMainMenuObject);
      break;
    }
    default: {
      break;
    }
    }
  } else if(EVENT_ID_TIMER == pstEvent->iID) {
    HMI_MainMenu_RefreshScreen(pstDeviceIF, NULL);
  }
  if(NULL != piActionID) {
    *piActionID = iProcessAction;
  }

  return eProcessResult;
}

HMI_ENGINE_RESULT HMI_MainMenu_PostProcess(SGUI_SCR_DEV* pstDeviceIF, HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
  if(HMI_DEMO_PROC_CONFIRM == iActionID) {
    HMI_MainMenu_SetClkHigh(true);
    switch(SGUI_List_GetSelection(&s_stMainMenuObject)->iIndex) {
    case 0: {
      HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_DAP_ONLINE, NULL);
      break;
    }
    case 1: {
      HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_DAP_OFFLINE, NULL);
      break;
    }
    case 2: {
      HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_ABOUT, NULL);
      break;
    }
    case 3: {
      HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "是否关机？\n向左←拨动关机，向右→拨动返回。");
      break;
    }
    default: {
      /* do nothing. */
    }
    }
  } else if(HMI_DEMO_PROC_CANCEL == iActionID) {
    HMI_GoBack(NULL);
  }

  return HMI_RET_NORMAL;
}



