#ifndef __DRIVER_H__
#define __DRIVER_H__
#include "hc32_ddl.h"
#include "io_define.h"
#include "sd_card.h"
#include "sd_card.h"
#define KEY_SHORT_VALID_TIME 80  // ms
#define KEY_LONG_VALID_TIME 500  // ms
#define KEY_INTERVAL_TIME 50  // ms
typedef struct {
  __IO uint32_t pressedTime;
  __IO uint32_t releaseTime;
  __IO uint32_t holdTime;
  __IO uint8_t  isPressing : 1;
  __IO uint8_t  shortPressed : 1;
  __IO uint8_t  longPressd : 1;
} ButtonTypeDef;
extern ButtonTypeDef Btn_Ok, Btn_Inc, Btn_Dec;
void EXTI_Config(void);
uint32_t getTick(void);
void I2C3_OLED_Config(uint32_t);
void WDT_Config(void);
en_result_t SDIO2_Config(void);
extern uint8_t batIsCharging;
extern uint16_t batVoltagemV;
extern __IO uint8_t u8Usb_is_connected;

void buzzerCtrl(bool b);
void buzzerConfig(void);

extern bool flashKeyVerifyState;
#endif

