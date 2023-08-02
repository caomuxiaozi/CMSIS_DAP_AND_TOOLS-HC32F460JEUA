#include "DAP_Config.h"
#include "dap.h"
#include "demoProc.h"
#include "driver.h"
#include "eeprom.h"
#include "ff.h"
#include "hc32_ddl.h"
#include "hc32f460.h"
#include "screen.h"
#include "stdio.h"
#include "usb_dev_composite.h"
#include "usb_dev_core.h"
#include "usb_dev_custom_hid_class.h"
#include "usb_dev_desc.h"
#include "usb_dev_hid_dap_fops.h"

#ifdef EVR_DEBUG
#    include "EventRecorder.h"
#endif
#include "ff.h"
#include "stdlib.h"
FATFS                   FatSd;
FIL                     font16File;
FIL                     font12File;
uint8_t                 powerOff = 0;
volatile uint32_t       msTick;
volatile uint8_t oledFrame;//统计刷新率的
volatile uint8_t oledFrameLast;
bool initializeDone = false;
void SysTick_IrqHandler(void) {
  msTick++;
  if(msTick % 1000 == 0) {
    oledFrameLast = oledFrame;
    oledFrame     = 0;
  }
  if(Btn_Ok.isPressing) {//长按5秒也是关机
    if(msTick - Btn_Ok.pressedTime > 5000) {
      Btn_Ok.isPressing = 0;
      powerOff          = 3;
    }
  }
}
uint32_t getTick(void) {
  return msTick;
}
uint32_t getTick_Sec(void) {
  return msTick / 1000;
}
void delay1ms(uint16_t ms) {
  uint32_t target = getTick() + ms;
  while(getTick() < target) {
  }
}
void                   SYSClk_Config(void);
usb_core_instance      usb_dev;
extern void            DAP_Thread(void* argument);
extern uint8_t         ramDisk[];
uint8_t                printRAMDisk = 0;
extern stc_sd_handle_t stcSdhandle;
uint8_t                readSdCard = 1;
uint8_t                au32WriteBlocks[512];
uint32_t               au32ReadBlocks[512];
extern void            OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t* no);
extern uint16_t        getZh16Font(uint8_t* pZh, uint8_t* pZk);
uint32_t               sysRunMinuteOld;
uint32_t               frameTick = 0;
extern uint8_t         dapIsConnected;
uint8_t                oled_Brightness = 0xCF;

#define KEY_VALUE_GET *((uint32_t*)(4 * 7))
// software version save address
#define VERSION_VALUE_GET *((uint32_t*)(4 * 8))
uint32_t lowPowerTips = 0;
#define BAT_ADC_CONV ADC_VREF
uint32_t SN[3];
uint64_t SNPCB;
extern bool canRcvDone;
extern bool canUsbSend;
int      main() {
  SYSClk_Config();
  PORT_SetFunc(PORT_KEY_OK, PIN__KEY_OK, Func_Gpio, Disable);
  PORT_SetFunc(PORT_PWR_CTRL, PIN__PWR_CTRL, Func_Gpio, Disable);
  PORT_SetFunc(PORT_BAT_CHG, PIN__BAT_CHG, Func_Gpio, Disable);
  PORT_SetFunc(PORT_USB_DP, PIN__USB_DP, Func_Gpio, Disable);
  stc_port_init_t stcPortInit;
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = Pin_Mode_In;
  PORT_Init(PORT_KEY_OK, PIN__KEY_OK, &stcPortInit);
  PORT_Init(PORT_BAT_CHG, PIN__BAT_CHG, &stcPortInit);
  stcPortInit.enPinMode  = Pin_Mode_Out;
  stcPortInit.enPinDrv   = Pin_Drv_L;
  stcPortInit.enPinOType = Pin_OType_Cmos;
  PORT_Init(PORT_PWR_CTRL, PIN__PWR_CTRL, &stcPortInit);
  PORT_ResetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);
  sysRunMinuteOld = getSysMinuteFromFlash();
  sysRunMinute    = sysRunMinuteOld;
#ifdef EVR_DEBUG
  EventRecorderStop();
  EventRecorderInitialize(EventRecordAll, 0u);
  EventRecorderClockUpdate();
  EventRecorderStart();
#endif
  en_result_t ret = Ok;
  OLED_Init();
  OLED_LOG("oled log start");
  OLED_LOG(" ");
  OLED_LOG("KunMing BoYuan design");
  OLED_LOG(" ");
  OLED_LOG("pwron time:%dm", sysRunMinute * 2);
  OLED_LOG(" "); 
//    delay1ms(500);
  uint8_t pwrCtrlOn = 0;
  MEM_ZERO_STRUCT(Btn_Ok);
  if(PORT_GetBit(PORT_KEY_OK, PIN__KEY_OK) == Reset) {
    OLED_LOG("power on start");
    pwrCtrlOn = 1;
  } else {
    OLED_LOG("power on NULL");
  }
  OLED_LOG("start mount sd");
  if(FR_OK != f_mount(&FatSd, "0:", 0)) {
    OLED_LOG("mount failed\r\n");
    ret = Error;
  } else {
    OLED_LOG("mount Successful\r\n");
  }

  if(Ok == f_open(&font16File, FontLibZh16Path, FA_READ)) {
    OLED_LOG("hz16Lib open success\r\n");
    font16File.isOpen = true;
  } else {
    ret = Error;
    OLED_LOG("hz16Lib open Failed\r\n");
  }
  if(Ok == f_open(&font12File, FontLibZh12Path, FA_READ)) {
    OLED_LOG("hz12Lib open success\r\n");
    font12File.isOpen = true;
  } else {
    OLED_LOG("hz12Lib open Failed\r\n");
    ret = Error;
  }
  char sn_char[64];
  sprintf(sn_char, "Version:%10d", VERSION_VALUE_GET);
  OLED_LOG(sn_char);
  SN[1] = (uint32_t)(M4_EFM->UQID1 & stcSdhandle.CID[0]);
  SN[1] = (uint32_t)(M4_EFM->UQID2 & stcSdhandle.CID[1]);
  SN[2] = (uint32_t)(M4_EFM->UQID3 & stcSdhandle.CID[2]);
  SNPCB = (uint64_t)SN[0] + SN[1] + SN[2];
  sprintf(sn_char, "sn:%llx", SNPCB);
  OLED_LOG(sn_char);
  initializeDone = true;
  OLED_LOG("usb init start");
  hd_usb_dev_init(&usb_dev, &USR_desc, &class_composite_cbk, &user_cb);
  hd_usb_dev_deinit(&usb_dev);
  OLED_LOG("usb init End");

  OLED_LOG("cmsid-dap init");
  DAP_Setup();
  OLED_LOG("Button init");
  EXTI_Config();
  if(PORT_GetBit(PORT_KEY_OK, PIN__KEY_OK) == Reset && pwrCtrlOn) {
    OLED_LOG("power on Success");
    PORT_SetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);
    OLED_LOG("pls release Button");
    Btn_Ok.pressedTime = 0;
    Btn_Ok.isPressing  = 0;
    Btn_Ok.pressedTime = 0;
    Btn_Ok.isPressing  = 0;
    //    }
  } else {
    PORT_ResetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);
  }
  if(ret == Error) {
    OLED_LOG("system init failed");
  }
  buzzerConfig();
  buzzerCtrl(true);
  InitializeHMIEngineObj();
  buzzerCtrl(false);
  ADC_StartConvert(M4_ADC1);  //第一次采样电池电压

#ifdef ENABLE_WDT_SUN
  WDT_Config();  //刷新在screen RefreshScreen里面
#endif
  PORT_SetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);

  while(1) {
#ifdef ENABLE_WDT_SUN
    WDT_RefreshCounter();  //刷新看门狗
#endif
    DAP_Thread(NULL);  // cmsis dap online烧写
    if(PORT_GetBit(PORT_BAT_CHG, PIN__BAT_CHG) == Reset) {  //是否在充电
      batIsCharging = true;
    } else {
      batIsCharging = false;
    }
    {
      //无操作3分钟暗屏10分钟自动关机
      if(getTick() - MAX(MAX(Btn_Ok.releaseTime, Btn_Inc.releaseTime), MAX(Btn_Inc.releaseTime, Btn_Dec.releaseTime)) > 3 * 60000) {
        if(oled_Brightness != OLED_BRIGHTNESS_MIN) {
          oled_Brightness = OLED_BRIGHTNESS_MIN;
        }
        if(getTick() - MAX(MAX(Btn_Ok.releaseTime, Btn_Inc.releaseTime), MAX(Btn_Inc.releaseTime, Btn_Dec.releaseTime)) > 10 * 60000) {
          PORT_ResetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);
        }
      } else {
        if(oled_Brightness != OLED_BRIGHTNESS_MAX) {
          oled_Brightness = OLED_BRIGHTNESS_MAX;
        }
      }
    }
    if(getTick() % 100 < 10) {   //屏幕更新
      if(!dapIsConnected) {  // DAP未使用才更新屏幕。尽量保证烧写速度吧。
        DemoMainProcess();  //约50ms完成一次
        oledFrame++;
      }
    }
    {
      //低电量提醒
      if(ADC_GetEocFlag(M4_ADC1, ADC_SEQ_A)) {  //获取电池电压
        batVoltagemV = M4_ADC1->DR0 * 5300 / (M4_ADC1->CR0_f.ACCSEL == AdcResolution_8Bit ? 256 : 4096);
        ADC_ClrEocFlag(M4_ADC1, ADC_SEQ_A);
        ADC_StartConvert(M4_ADC1);
      }
      if(batVoltagemV < 3100 && getTick() - lowPowerTips > 10000) {
        lowPowerTips = getTick();
        HMI_SwitchScreen(HMI_SCREEN_ID_DEMO_PWR_OFF, "电池电量低!!!!!\n\n#_#  #_#  #_#");
        if(batVoltagemV < 2900) {
          powerOff = 3;
        }
      }
    }
    {
      //保存运行时间
      if(getTick() / (2 * 60000) + sysRunMinuteOld > sysRunMinute) {
        sysRunMinute = getTick() / (2 * 60000) + sysRunMinuteOld;
        saveSysMinuteToFlash(sysRunMinute);
      }
    }
    if(powerOff) {  //关机操作
      powerOff--;
      if(powerOff == 0) {
        PORT_ResetBits(PORT_PWR_CTRL, PIN__PWR_CTRL);
        while(PORT_GetBit(PORT_KEY_OK, PIN__KEY_OK) == Reset)
          ;
      } else {
      }
    }
  }
}
