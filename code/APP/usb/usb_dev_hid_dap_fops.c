/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file usb_dev_user.c
 **
 ** A detailed description is available at
 ** @link
        This file includes the user application layer.
    @endlink
 **
 **   - 2021-04-16  CDT   First version for USB demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "usb_dev_hid_dap_fops.h"
#include "usb_dev_ctrleptrans.h"
#include "usb_app_conf.h"
#include "usb_bsp.h"
#include "stdio.h"

//#define PRINTF_THIS printf
#ifndef PRINTF_THIS
#define PRINTF_THIS(...)
#endif
void usb_dev_user_init(void);
void usb_dev_user_rst (void);
void usb_dev_user_devcfg (void);
void usb_dev_user_devsusp(void);
void usb_dev_user_devresume(void);
void usb_dev_user_conn(void);
void usb_dev_user_disconn(void); 
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
usb_dev_user_func user_cb =
{
    &usb_dev_user_init,
    &usb_dev_user_rst,
    &usb_dev_user_devcfg,
    &usb_dev_user_devsusp,
    &usb_dev_user_devresume,
    &usb_dev_user_conn,
    &usb_dev_user_disconn,
};

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  usb_dev_user_init
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_init(void)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
//    if (SysTick_Config(8400000))
//    {
//        while (1)
//        {
//            ;
//        }
//    }
}

/**
 *******************************************************************************
 ** \brief  usb_dev_user_rst
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_rst(void)
{
    PRINTF_THIS("%s>>USB Device has reseted.\n" ,__filename(__FILE__));
//    PRINTF_THIS(">>The interval report is 100ms\n");
}

/**
 *******************************************************************************
 ** \brief  usb_dev_user_devcfg
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_devcfg (void)
{
    PRINTF_THIS("%s>>HID interface starts.\n",__filename(__FILE__));
}

/**
 *******************************************************************************
 ** \brief  usb_dev_user_conn
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_conn (void)
{
    PRINTF_THIS("%s>>USB device connects.\n",__filename(__FILE__));
}

/**
 *******************************************************************************
 ** \brief  USBD_USR_DeviceDisonnected
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_disconn (void)
{
    PRINTF_THIS("%s>>USB device disconnected.\n",__filename(__FILE__));
}

/**
 *******************************************************************************
 ** \brief  usb_dev_user_devsusp
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_devsusp(void)
{
    PRINTF_THIS("%s>>USB device in suspend status.\n",__filename(__FILE__));
}

/**
 *******************************************************************************
 ** \brief  usb_dev_user_devresume
 ** \param  none
 ** \retval none
 ******************************************************************************/
void usb_dev_user_devresume(void)
{
    PRINTF_THIS("%s>>USB device resumes.\n",__filename(__FILE__));
}

/*******************************************************************************
 * EOF 
 ******************************************************************************/
