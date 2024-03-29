/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/**
 ** A detailed description is available at
 ** @link Specific api's relative to the used hardware platform @endlink
 **
 **   - 2021-04-16  CDT First version for USB demo.
 **
 ******************************************************************************/
#ifndef __USB_BSP__H__
#define __USB_BSP__H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "usb_core_driver.h"
#include "usb_app_conf.h"
#include "hc32_ddl.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* KEY10 Port/Pin definition */
#define KEY10_PORT                      (PortB)
#define KEY10_PIN                       (Pin01)
#define KEY10_EXINT_CH                  (ExtiCh01)
#define KEY10_INT_SRC                   (INT_PORT_EIRQ1)
#define KEY10_IRQn                      (Int007_IRQn)

/* LED0~3 Control definition */
#define  LED0_CTL(x)      //((Reset != (x))?BSP_LED_On(LED_RED):BSP_LED_Off(LED_RED))
#define  LED1_CTL(x)      //((Reset != (x))?BSP_LED_On(LED_GREEN):BSP_LED_Off(LED_GREEN))
#define  LED2_CTL(x)      //((Reset != (x))?BSP_LED_On(LED_YELLOW):BSP_LED_Off(LED_YELLOW))
#define  LED3_CTL(x)      //((Reset != (x))?BSP_LED_On(LED_BLUE):BSP_LED_Off(LED_BLUE))
/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
void BSP_Init(void);
void hd_usb_bsp_init (usb_core_instance *pdev);
void hd_usb_udelay(const uint32_t usec);
void hd_usb_mdelay(const uint32_t msec);
void hd_usb_bsp_nvicconfig (void);

#endif //__USB_BSP__H__

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
