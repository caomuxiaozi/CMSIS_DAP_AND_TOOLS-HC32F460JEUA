#include "driver.h"
#include "io_define.h"
#define CompileTime __TIME__
#define CompileYear                                                                                \
  (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 +            \
  (__DATE__[10] - '0')
#define CompileMonth                                                                               \
  (__DATE__[2] == 'n' ? (__DATE__[1] == 'a' ? 1 : 6) :                                           \
   __DATE__[2] == 'b' ? 2 :                                                                      \
   __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 3 : 4) :                                           \
   __DATE__[2] == 'y' ? 5 :                                                                      \
   __DATE__[2] == 'n' ? 6 :                                                                      \
   __DATE__[2] == 'l' ? 7 :                                                                      \
   __DATE__[2] == 'g' ? 8 :                                                                      \
   __DATE__[2] == 'p' ? 9 :                                                                      \
   __DATE__[2] == 't' ? 10 :                                                                     \
   __DATE__[2] == 'v' ? 11 :                                                                     \
   12)
#define CompileDay ((__DATE__[4] == ' ' ? 0 : ((__DATE__[4] - '0') * 10)) + (__DATE__[5] - '0'))

#define CompileDate (CompileYear*10000 + CompileMonth * 100 + CompileDay)

uint32_t RSTF0;
void     SYSClk_Config(void) {
#define CLK_HP_FREQ (168 * 1000 * 1000u)
#define CLK_HS_FREQ (8 * 1000 * 1000u)
  CLK_HrcCmd(Enable);
  RSTF0                       = M4_SYSREG->RMU_RSTF0;
  M4_SYSREG->RMU_RSTF0_f.CLRF = 1;
  PORT_Unlock();
  M4_PORT->PSPCR       = 3;  // enable swd  disable jtag
  M4_PORT->PCCR_f.RDWT = 3;
  PORT_Lock();
  PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_SRAMH, Enable);
  stc_clk_output_cfg_t stcOutputClkCfg;
  MEM_ZERO_STRUCT(stcOutputClkCfg);
  {
    //晶振驱动配置
    stc_clk_xtal_cfg_t stcXtalCfg;
    MEM_ZERO_STRUCT(stcXtalCfg);
    /* Config Xtal and Enable Xtal */
    stcXtalCfg.enMode        = ClkXtalModeOsc;
    stcXtalCfg.enDrv         = ClkXtalMidDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);
  }
  {
    //主时钟和ram时钟配置
    stc_sram_config_t stcSramConfig;
    MEM_ZERO_STRUCT(stcSramConfig);
    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx     = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC      = SramCycle4;
    stcSramConfig.enSramWC      = SramCycle4;
    stcSramConfig.enSramEccMode = EccMode3;
    stcSramConfig.enSramEccOp   = SramNmi;
    stcSramConfig.enSramPyOp    = SramNmi;
    SRAM_Init(&stcSramConfig);
    /* flash 读等待周期*/
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_8);
    EFM_Lock();
    stc_clk_mpll_cfg_t stcMpllCfg;
    MEM_ZERO_STRUCT(stcMpllCfg);
    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllCfg.pllmDiv = 2ul;  // 8M MPLL的VCO输入频率介于1MHz到12MHz之间。
    stcMpllCfg.plln    = 48ul;  // 384M 确保MPLL的VCO频率介于240MHz到480MHz之间
    stcMpllCfg.PllpDiv = 2ul;  // 192M
    stcMpllCfg.PllqDiv = 2ul;  // 192M
    stcMpllCfg.PllrDiv = 2ul;  // 192M
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);
    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
  }
  {
    //外设时钟分频
    stc_clk_sysclk_cfg_t stcSysClkCfg;
    MEM_ZERO_STRUCT(stcSysClkCfg);
    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;  // 192M CPU
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv8;  // 24M SDIO CAN
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;  // 192M TIM6计数
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;  // 96M UART SPI USB逻辑 TIM0 TIMA TIM4 TIM6逻辑
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;  // 48M ADC变换
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;  // 48M RTC I2C CMP WDT SWDT
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;  // 96M ADC逻辑控制，TRNG
    CLK_SysClkConfig(&stcSysClkCfg);
  }
  PWC_HS2HP();  // high speed -> high performance
  CLK_SetSysClkSource(CLKSysSrcMPLL);
  CLK_SetUsbClkSource(ClkUsbSrcSysDiv4);
  CLK_SetPeriClkSource(ClkPeriSrcPclk);  // adc trng 逻辑96M 变换48M
  NVIC_SetPriorityGrouping(DDL_IRQ_PRIORITY_08);
  SysTick_Config(SystemCoreClock / 1000u);
}
void I2C3_OLED_Config(uint32_t baudrate) {
  // PCLK3 48M
  PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_I2C3, Enable);
  PORT_SetFunc(PORT_OLED_I2C3_SDA, PIN__OLED_I2C3_SDA, Func_I2c3_Sda, Disable);
  PORT_SetFunc(PORT_OLED_I2C3_SCL, PIN__OLED_I2C3_SCL, Func_I2c3_Scl, Disable);
  stc_i2c_init_t stcI2cInit;
  //    stc_clk_freq_t stcClkFreq;
  I2C_Cmd(OLED_IIC, Disable);
  I2C_DeInit(OLED_IIC);
  MEM_ZERO_STRUCT(stcI2cInit);
  stcI2cInit.u32ClockDiv = I2C_CLK_DIV1;  // 48M / 16 = 3M
  stcI2cInit.u32Baudrate = baudrate;
  stcI2cInit.u32SclTime  = 0ul;
  float err;
  //    OLED_IIC->CR1_f.PE    = 0;
  //    OLED_IIC->CR1_f.SWRST = 1;
  //    OLED_IIC->CR1_f.PE    = 1;
  en_result_t enRes = Ok;
  do {
    enRes = I2C_Init(OLED_IIC, &stcI2cInit, &err);
    //        if(enRes == ErrorInvalidParameterSmall) {
    //            stcI2cInit.u32ClockDiv--;
    //        }
    //        else
    if(enRes == ErrorInvalidParameter) {
      stcI2cInit.u32ClockDiv++;
    }
  } while(enRes != Ok && stcI2cInit.u32ClockDiv <= I2C_CLK_DIV128);
//    I2C_DigitalFilterConfig(OLED_IIC,Filter1BaseCycle);
//    I2C_DigitalFilterCmd(OLED_IIC,Enable);
//    I2C_AnalogFilterCmd(OLED_IIC,Enable);
  //    OLED_IIC->CR1_f.SWRST = 0;
  stc_clock_timeout_init_t stcI2cTimeout;
  stcI2cTimeout.enClkTimeOutSwitch = TimeoutFunOff;
  stcI2cTimeout.u16TimeOutHigh = 0xffff;
  stcI2cTimeout.u16TimeOutLow = 0xffff;
  I2C_ClkTimeOutConfig(OLED_IIC, &stcI2cTimeout);
  I2C_BusWaitCmd(OLED_IIC, Enable);
  I2C_Cmd(OLED_IIC, Enable);
}
__IO uint16_t ADC_VREF = 3300;  // mv
static void   AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode) {
  en_port_t       enPort = PortA;
  en_pin_t        enPin  = Pin00;
  bool            bFlag  = true;
  stc_port_init_t stcPortInit;
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = enMode;
  stcPortInit.enPullUp  = Disable;
  switch(u8AdcPin) {
  case ADC1_IN0:
    enPort = PortA;
    enPin  = Pin00;
    break;
  case ADC1_IN1:
    enPort = PortA;
    enPin  = Pin01;
    break;
  case ADC1_IN2:
    enPort = PortA;
    enPin  = Pin02;
    break;
  case ADC1_IN3:
    enPort = PortA;
    enPin  = Pin03;
    break;
  case ADC12_IN4:
    enPort = PortA;
    enPin  = Pin04;
    break;
  case ADC12_IN5:
    enPort = PortA;
    enPin  = Pin05;
    break;
  case ADC12_IN6:
    enPort = PortA;
    enPin  = Pin06;
    break;
  case ADC12_IN7:
    enPort = PortA;
    enPin  = Pin07;
    break;
  case ADC12_IN8:
    enPort = PortB;
    enPin  = Pin00;
    break;
  case ADC12_IN9:
    enPort = PortB;
    enPin  = Pin01;
    break;
  case ADC12_IN10:
    enPort = PortC;
    enPin  = Pin00;
    break;
  case ADC12_IN11:
    enPort = PortC;
    enPin  = Pin01;
    break;
  case ADC1_IN12:
    enPort = PortC;
    enPin  = Pin02;
    break;
  case ADC1_IN13:
    enPort = PortC;
    enPin  = Pin03;
    break;
  case ADC1_IN14:
    enPort = PortC;
    enPin  = Pin04;
    break;
  case ADC1_IN15:
    enPort = PortC;
    enPin  = Pin05;
    break;
  default:
    bFlag = false;
    break;
  }
  if(true == bFlag) {
    PORT_Init(enPort, enPin, &stcPortInit);
  }
}

ButtonTypeDef Btn_Ok, Btn_Inc, Btn_Dec;
void          KeyPwr_IRQHandler(void) {
  if(EXINT_IrqFlgGet(ExtiCh02)) {
    if(PORT_GetBit(PORT_KEY_OK, PIN__KEY_OK)) {
      Btn_Ok.releaseTime = getTick();
      if(Btn_Ok.isPressing) {
        uint32_t holdTime = Btn_Ok.releaseTime - Btn_Ok.pressedTime;
        if(holdTime >= KEY_LONG_VALID_TIME) {
          Btn_Ok.longPressd = 1;
        } else if(holdTime >= KEY_SHORT_VALID_TIME) {
          Btn_Ok.shortPressed = 1;
        }
        Btn_Ok.isPressing = 0;
      }
    } else {
      //            if(getTick() - sysDat.Btn_Ok.releaseTime < KEY_INTERVAL_TIME) {
      //                sysDat.Btn_Ok.isPressing = 0;
      //            }else
      if(Btn_Ok.isPressing == 0 /*getTick() - Btn_Ok.pressedTime > KEY_INTERVAL_TIME*/) {
        Btn_Ok.pressedTime = getTick();
        Btn_Ok.isPressing  = 1;
      }
    }
    EXINT_IrqFlgClr(ExtiCh02);
  }
}
void KeyInc_IRQHandler(void) {
  if(EXINT_IrqFlgGet(ExtiCh01)) {
    if(PORT_GetBit(PORT_KEY_INC, PIN__KEY_INC)) {
      Btn_Inc.releaseTime = getTick();
      if(Btn_Inc.isPressing) {
        uint32_t holdTime = Btn_Inc.releaseTime - Btn_Inc.pressedTime;
        //                if(holdTime > KEY_LONG_VALID_TIME) {
        //                    sysDat.Btn_Inc.longPressd = 1;
        //                }else
        if(holdTime >= KEY_SHORT_VALID_TIME) {
          Btn_Inc.shortPressed = 1;
        }
        Btn_Inc.isPressing = 0;
      }
    } else {
      //            if(getTick() - sysDat.Btn_Inc.releaseTime < KEY_INTERVAL_TIME) {
      //                sysDat.Btn_Inc.isPressing = 0;
      //            }
      if(Btn_Inc.isPressing == 0 /*getTick() - Btn_Inc.releaseTime > KEY_INTERVAL_TIME*/) {
        Btn_Inc.pressedTime = getTick();
        Btn_Inc.isPressing  = 1;
      }
    }
    EXINT_IrqFlgClr(ExtiCh01);
  }
}
void KeyDec_IRQHandler(void) {
  if(EXINT_IrqFlgGet(ExtiCh10)) {
    if(PORT_GetBit(PORT_KEY_DEC, PIN__KEY_DEC)) {
      Btn_Dec.releaseTime = getTick();
      if(Btn_Dec.isPressing) {
        uint32_t holdTime = Btn_Dec.releaseTime - Btn_Dec.pressedTime;
        //                if(holdTime > KEY_LONG_VALID_TIME) {
        //                    Btn_Dec.longPressd = 1;
        //                }
        //                else
        if(holdTime >= KEY_SHORT_VALID_TIME) {
          Btn_Dec.shortPressed = 1;
        }
        Btn_Dec.isPressing = 0;
      }
    } else {
      //            if(getTick() - Btn_Dec.releaseTime < KEY_INTERVAL_TIME) {
      //                Btn_Dec.isPressing = 0;
      //            }
      if(Btn_Dec.isPressing == 0 /*getTick() - Btn_Dec.releaseTime > KEY_INTERVAL_TIME*/) {
        Btn_Dec.pressedTime = getTick();
        Btn_Dec.isPressing  = 1;
      }
    }
    EXINT_IrqFlgClr(ExtiCh10);
  }
}
void EXTI_Config(void) {
  stc_exint_config_t  stcExtiConfig;
  stc_irq_regi_conf_t stcIrqRegiConf;
  stc_port_init_t     stcPortInit;
  MEM_ZERO_STRUCT(stcExtiConfig);
  MEM_ZERO_STRUCT(stcIrqRegiConf);
  MEM_ZERO_STRUCT(stcPortInit);
  // Ph2 ok
  stcExtiConfig.enExitCh   = ExtiCh02;
  stcExtiConfig.enFilterEn = Enable;
  stcExtiConfig.enFltClk   = Pclk3Div64;
  stcExtiConfig.enExtiLvl  = ExIntBothEdge;
  EXINT_Init(&stcExtiConfig);
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = Pin_Mode_In;
  stcPortInit.enExInt   = Enable;
  PORT_Init(PORT_KEY_OK, PIN__KEY_OK, &stcPortInit);
  stcIrqRegiConf.enIntSrc    = INT_PORT_EIRQ2;
  stcIrqRegiConf.enIRQn      = Int006_KeyOk;
  stcIrqRegiConf.pfnCallback = &KeyPwr_IRQHandler;
  enIrqRegistration(&stcIrqRegiConf);
  NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
  NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
  NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
  // Pa1 inc
  stcExtiConfig.enExitCh   = ExtiCh01;
  stcExtiConfig.enFilterEn = Enable;
  stcExtiConfig.enFltClk   = Pclk3Div64;
  stcExtiConfig.enExtiLvl  = ExIntBothEdge;
  EXINT_Init(&stcExtiConfig);
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = Pin_Mode_In;
  stcPortInit.enExInt   = Enable;
  PORT_Init(PORT_KEY_INC, PIN__KEY_INC, &stcPortInit);
  stcIrqRegiConf.enIntSrc    = INT_PORT_EIRQ1;
  stcIrqRegiConf.enIRQn      = Int007_KeyInc;
  stcIrqRegiConf.pfnCallback = &KeyInc_IRQHandler;
  enIrqRegistration(&stcIrqRegiConf);
  NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
  NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
  NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
  // PB10
  stcExtiConfig.enExitCh   = ExtiCh10;
  stcExtiConfig.enFilterEn = Enable;
  stcExtiConfig.enFltClk   = Pclk3Div64;
  stcExtiConfig.enExtiLvl  = ExIntBothEdge;
  EXINT_Init(&stcExtiConfig);
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = Pin_Mode_In;
  stcPortInit.enExInt   = Enable;
  PORT_Init(PORT_KEY_DEC, PIN__KEY_DEC, &stcPortInit);
  stcIrqRegiConf.enIntSrc    = INT_PORT_EIRQ10;
  stcIrqRegiConf.enIRQn      = Int008_KeyDec;
  stcIrqRegiConf.pfnCallback = &KeyDec_IRQHandler;
  enIrqRegistration(&stcIrqRegiConf);
  NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
  NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
  NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}
void WDT_Config(void) {
  // PCLK3
  stc_wdt_init_t stcWdtInit;
  /* configure structure initialization */
  MEM_ZERO_STRUCT(stcWdtInit);
  stcWdtInit.enClkDiv           = WdtPclk3Div8192;  // 48M/8192 = 5859
  stcWdtInit.enCountCycle       = WdtCountCycle65536;  // 65536/5859=11s
  stcWdtInit.enRefreshRange     = WdtRefresh0To100Pct;
  stcWdtInit.enSleepModeCountEn = Disable;
  stcWdtInit.enRequestType      = WdtTriggerResetRequest;
  WDT_Init(&stcWdtInit);
}
float map(float in, float in_max, float in_min, float out_max, float out_min) {
  float out = (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if(out_max >= out_min) {
    if(out > out_max) {
      out = out_max;
    } else if(out < out_min) {
      out = out_min;
    }
  } else {
    if(out > out_min) {
      out = out_min;
    } else if(out < out_max) {
      out = out_max;
    }
  }
  return out;
}

stc_sd_handle_t stcSdhandle;
#define SDIOC_USE (M4_SDIOC2)
en_result_t SDIO2_Config(void) {
  PORT_SetFunc(PORT_SDIO2_CK, PIN__SDIO2_CK, Func_Sdio, Disable);
  PORT_SetFunc(PORT_SDIO2_CMD, PIN__SDIO2_CMD, Func_Sdio, Disable);
  PORT_SetFunc(PORT_SDIO2_D0, PIN__SDIO2_D0, Func_Sdio, Disable);
  PORT_SetFunc(PORT_SDIO2_D1, PIN__SDIO2_D1, Func_Sdio, Disable);
  PORT_SetFunc(PORT_SDIO2_D2, PIN__SDIO2_D2, Func_Sdio, Disable);
  PORT_SetFunc(PORT_SDIO2_D3, PIN__SDIO2_D3, Func_Sdio, Disable);
  stc_sdcard_init_t stcCardInitCfg;
  stcCardInitCfg.enBusWidth  = SdiocBusWidth4Bit;
  stcCardInitCfg.enClkFreq   = SdiocClk25M;
  stcCardInitCfg.enSpeedMode = SdiocNormalSpeedMode;
  stcCardInitCfg.pstcInitCfg = NULL;
  stcSdhandle.SDIOCx         = SDIOC_USE;
  en_result_t res;
  //  do{
  res = SDCARD_Init(&stcSdhandle, &stcCardInitCfg);
  if(Ok != res) {
    DDL_Printf("SD init Error %d\r\n", res);
    return Error;
  } else {
    DDL_Printf("SD init Done\r\n");
    return Ok;
  }
  //  }
  //  while(res != Ok);
}

void buzzerCtrl(bool b) {
  if(b) {
    PORT_SetBits(PORT_BUZZER, PIN__BUZZER);
  } else {
    PORT_ResetBits(PORT_BUZZER, PIN__BUZZER);
  }
}
void buzzerConfig(void) {
  stc_port_init_t stcPortInit;
  MEM_ZERO_STRUCT(stcPortInit);
  stcPortInit.enPinMode = Pin_Mode_Out;
  PORT_Init(PORT_BUZZER, PIN__BUZZER, &stcPortInit);
  PORT_SetFunc(PORT_BUZZER, PIN__BUZZER, Func_Gpio, Disable);

}
