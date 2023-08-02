/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file usb_dev_custom_hid_class.c
 **
 ** A detailed description is available at
 ** @link
        This file provides the CUSTOM HID core functions.
    @endlink
 **
 **   - 2021-04-14  CDT   First version for USB demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "usb_dev_custom_hid_class.h"
#include "usb_dev_desc.h"
#include "usb_dev_stdreq.h"
#include "usb_bsp.h"
#include "hc32_ddl.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
extern uint8_t PrevXferDone;

uint8_t Report_buf[64u];
uint8_t Send_Buf[64u];
__IO uint32_t IsReportAvailable = 0ul;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
void usb_dev_hid_init(void *pdev);
void usb_dev_hid_deinit(void *pdev);
uint8_t usb_dev_hid_setup(void *pdev, USB_SETUP_REQ *req);
uint8_t *usb_dev_hid_getcfgdesc(uint16_t *length);
void usb_dev_hid_datain(void *pdev, uint8_t epnum);
void usb_dev_hid_dataout(void *pdev, uint8_t epnum);
void usb_dev_hid_EP0_RxReady(void *pdev);
uint8_t *usb_dev_hid_GetDeviceQualifierDesc(uint16_t *length);

void USBD_HID0_Initialize (void);
void USBD_HID0_Uninitialize (void);
extern int32_t USBD_HID0_GetReport (uint8_t rtype, uint8_t req, uint8_t rid, uint8_t *buf);
extern bool USBD_HID0_SetReport (uint8_t rtype, uint8_t req, uint8_t rid, const uint8_t *buf, int32_t len);
extern usb_core_instance  usb_dev;
#define HID_REPORT_INPUT            0x81
#define HID_REPORT_OUTPUT           0x91
#define HID_REPORT_FEATURE          0xB1

#define USBD_HID_REQ_EP_CTRL        0x01
#define USBD_HID_REQ_PERIOD_UPDATE  0x02
#define USBD_HID_REQ_EP_INT         0x03

//#define PRINTF_THIS printf
#ifndef PRINTF_THIS
#define PRINTF_THIS(...)
#endif
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
usb_dev_class_func  class_customhid_cbk =
{
    &usb_dev_hid_init,
    &usb_dev_hid_deinit,
    &usb_dev_hid_setup,
    NULL,//EP0 ONLY SENT 
    &usb_dev_hid_EP0_RxReady, 
    &usb_dev_hid_getcfgdesc,
    NULL,
    &usb_dev_hid_datain, 
    &usb_dev_hid_dataout, 
    NULL,
    NULL,
    &usb_dev_hid_GetDeviceQualifierDesc,
};

__USB_ALIGN_BEGIN static uint32_t  USB_DEV_HID_AltSet  __USB_ALIGN_END = 0u;

__USB_ALIGN_BEGIN static uint32_t  USB_DEV_HID_Protocol  __USB_ALIGN_END = 0u;

__USB_ALIGN_BEGIN static uint32_t  USB_DEV_HID_IdleState __USB_ALIGN_END = 0u;

__USB_ALIGN_BEGIN static uint8_t usb_dev_hid_cfgdesc[USB_CUSTOM_HID_CONFIG_DESC_SIZ] __USB_ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_CUSTOM_HID_CONFIG_DESC_SIZ,/* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
/*9*/
  /************** Descriptor of CUSTOM HID interface ****************/
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0x03,         /*bInterfaceClass: CUSTOM_HID*/
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0x00,            /*iInterface: Index of string descriptor*/
/*18*/
  /******************** Descriptor of CUSTOM_HID *************************/
  0x09,         /*bLength: CUSTOM_HID Descriptor size*/
  CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
  0x11,         /*bCUSTOM_HIDUSTOM_HID: CUSTOM_HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
/* 27 */
  /******************** Descriptor of Custom HID endpoints ********************/
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
  HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  HID_IN_PACKET, /*wMaxPacketSize: 2 Byte max */
  0x00,
  0x01,          /*bInterval: Polling Interval */
/* 34 */
  0x07,          /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */
  HID_OUT_EP,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  HID_OUT_PACKET,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  0x01,  /* bInterval: Polling Interval */
/* 41 */
} ;


__USB_ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc[CUSTOM_HID_REPORT_DESC_SIZE] __USB_ALIGN_END =
{
      /* USER CODE BEGIN 0 */ /* A minimal Report Desc with INPUT/OUTPUT/FEATURE report. Zach Lee */
  0x06,0x00,0xFF,         /*  Usage Page (vendor defined) ($FF00) global */
  0x09,0x01,              /*  Usage (vendor defined) ($01) local */
  0xA1,0x01,              /*  Collection (Application) */
  0x15,0x81,//0x00,              /*   LOGICAL_MINIMUM (0) */
  0x25,0x7f,//0xFF,              /*   LOGICAL_MAXIMUM (255) */
  0x75,0x08,              /*   REPORT_SIZE (8bit) */

  // Input Report
  0x95,0x40,                /*   Report Length (64 REPORT_SIZE) */
  0x09,0x01,              /*   USAGE (Vendor Usage 1) */
  0x81,0x02,              /*   Input(data,var,absolute) */

  // Output Report
  0x95,0x40,                /*   Report Length (64 REPORT_SIZE) */
  0x09,0x01,              /*   USAGE (Vendor Usage 1) */
  0x91,0x02,              /*   Output(data,var,absolute) */

  // Feature Report
  0x95,0x40,                /*   Report Length (64 REPORT_SIZE) */
  0x09,0x01,              /*   USAGE (Vendor Usage 1) */
  0xB1,0x02,              /*   Feature(data,var,absolute) */
  /* USER CODE END 0 */
  0xC0                    /*  END_COLLECTION	             */

};
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CUSTOM_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __USB_ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
uint8_t  *usb_dev_hid_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_CUSTOM_HID_DeviceQualifierDesc);
  return USBD_CUSTOM_HID_DeviceQualifierDesc;
}
/**
 *******************************************************************************
 ** \brief  Initialize the HID application
 ** \param  pdev: Device instance
 ** \retval none
 ******************************************************************************/
void usb_dev_hid_init(void *pdev)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    hd_usb_opendevep(pdev, HID_IN_EP, HID_IN_PACKET, USB_EP_INT);
    hd_usb_opendevep(pdev, HID_OUT_EP, HID_OUT_PACKET, USB_EP_INT);
    hd_usb_readytorx(pdev, HID_OUT_EP, Report_buf, HID_IN_PACKET);
    hd_usb_devepstatusset(pdev , HID_OUT_EP , USB_EP_RX_VALID);
  USBD_HID0_Initialize();
}

/**
 *******************************************************************************
 ** \brief  Deinitialize  the HID application
 ** \param  pdev: Device instance
 ** \retval none
 ******************************************************************************/
void usb_dev_hid_deinit(void *pdev)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    hd_usb_shutdevep(pdev, HID_IN_EP);
    hd_usb_shutdevep(pdev, HID_OUT_EP);
  USBD_HID0_Uninitialize();
}

/**
 *******************************************************************************
 ** \brief  Handle the setup requests
 ** \param  pdev: Device instance
 ** \param  req: usb requests
 ** \retval status
 ******************************************************************************/
uint8_t usb_dev_hid_setup(void *pdev, USB_SETUP_REQ *req)
{
  PRINTF_THIS("%s->%s REQ_TYPE=%d R=%d\r\n",__filename(__FILE__),__func__,req->bmRequest & USB_REQ_TYPE_MASK,req->bRequest);
    uint8_t hid_report_length = 0u;
    uint16_t len              = 0u;
    uint8_t  *pbuf            = NULL;
    uint8_t u8Ret             = USBD_OK;
uint16_t status_info = 0U;
    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS :
            switch (req->bRequest)
            {
                case CUSTOM_HID_REQ_SET_PROTOCOL:
                    USB_DEV_HID_Protocol = (uint8_t)(req->wValue);
                    break;
                case CUSTOM_HID_REQ_GET_PROTOCOL:
                    hd_usb_ctrldatatx(pdev, (uint8_t *)&USB_DEV_HID_Protocol, 1u);
                    break;
                case CUSTOM_HID_REQ_SET_IDLE:
                    USB_DEV_HID_IdleState = (uint8_t)(req->wValue >> 8);
                    break;
                case CUSTOM_HID_REQ_GET_IDLE:
                    hd_usb_ctrldatatx(pdev, (uint8_t *)&USB_DEV_HID_IdleState, 1u);
                    break;
                case CUSTOM_HID_REQ_SET_REPORT:
                    IsReportAvailable = 1u;
                    hid_report_length = (uint8_t)(req->wLength);
                    hd_usb_ctrldatarx(pdev, Report_buf, (uint16_t)hid_report_length);
                    break;
                default:
                    hd_usb_ctrlerr(pdev);
                    u8Ret = USBD_FAIL;
                    break;
            }
            break;
        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
              case USB_REQ_GET_STATUS:
                if(((usb_core_instance*)pdev)->dev.device_cur_status == USB_DEV_CONFIGURED){
                  hd_usb_ctrldatatx(pdev, (uint8_t *)&status_info, 2u);
                }else{
                  hd_usb_ctrlerr(pdev);
                  u8Ret = USBD_FAIL;
                }
                break;
                case USB_REQ_GET_DESCRIPTOR:
                    if( req->wValue >> 8u == CUSTOM_HID_REPORT_DESC)
                    {
                        len = __MIN(CUSTOM_HID_REPORT_DESC_SIZE, req->wLength);
                        pbuf = (uint8_t*)CUSTOM_HID_ReportDesc;
                    }
                    else if( req->wValue >> 8u == CUSTOM_HID_DESCRIPTOR_TYPE)
                    {
                        pbuf = (uint8_t*)usb_dev_hid_cfgdesc + 0x12u;
                        len = __MIN(USB_CUSTOM_HID_DESC_SIZ, req->wLength);
                    }
                    else
                    {
                        //
                    }

                    hd_usb_ctrldatatx(pdev, pbuf, len);
                    break;
                case USB_REQ_GET_INTERFACE :
                  
                    hd_usb_ctrldatatx(pdev, (uint8_t *)&USB_DEV_HID_AltSet, 1u);
                    break;
                case USB_REQ_SET_INTERFACE :
                    USB_DEV_HID_AltSet = (uint8_t)(req->wValue);
                    break;
            }
            break;
    }
    return u8Ret;
}

/**
 *******************************************************************************
 ** \brief  Send HID Report
 ** \param  pdev: device instance
 ** \param  buff: pointer to report
 ** \param  len: the length in bytes would be sent
 ** \retval status
 ******************************************************************************/
uint8_t usb_dev_hid_txreport(usb_core_instance *pdev, uint8_t *report, uint16_t len)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    if (pdev->dev.device_cur_status == USB_DEV_CONFIGURED )
    {
        hd_usb_deveptx(pdev, HID_IN_EP, report, (uint32_t)len);
    }
    return USBD_OK;
}

/**
 *******************************************************************************
 ** \brief  return configuration descriptor
 ** \param  length : length of configuration descriptor in bytes
 ** \retval pointer to configuration descriptor buffer
 ******************************************************************************/
uint8_t *usb_dev_hid_getcfgdesc(uint16_t *length)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    *length = (uint8_t)sizeof(usb_dev_hid_cfgdesc);
    return usb_dev_hid_cfgdesc;
}
uint32_t usbGetReportLast = 0;
////////////////////////////////////////////////////////////////////
void USBD_HID_GetReportTrigger(int a, int b, void * report, int len)
{
  (void)a;
  (void)b;
  PRINTF_THIS("%s->%s:",__filename(__FILE__),__func__);
//  for(uint8_t i = 0;i<len;i++){
//    PRINTF_THIS("0x%02x ",((uint8_t*)report)[i]);
//  }
//  PRINTF_THIS("\r\n");
    usbGetReportLast = SysTick_GetTick();
  usb_dev_hid_txreport(&usb_dev, report, len);
    
  return;
}
/////////////////////////////////////////////////////////////////////
void USBD_InEvent(void){
  int32_t len;
    PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);

  len = USBD_HID0_GetReport(HID_REPORT_INPUT,USBD_HID_REQ_EP_INT,0,Report_buf);
  if(len >0){
    USBD_HID_GetReportTrigger(0,0,Report_buf,len);
  }
}
////////////////////////////////////////////////////////////////////
void USBD_OutEvent(void){
    PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);

  USBD_HID0_SetReport(HID_REPORT_OUTPUT,0,0,Report_buf,64u);
}
/**
 *******************************************************************************
  ** \brief  processing for data in
  ** \param  pdev: device instance
  ** \param  epnum: endpoint index
  ** \retval none
 ******************************************************************************/
void usb_dev_hid_datain(void *pdev, uint8_t epnum)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    hd_usb_flsdevep(pdev, HID_IN_EP);

    if (epnum == (HID_IN_EP & (uint8_t)~0x80u))
    {
      USBD_InEvent();
        PrevXferDone = 1u;
    }
}

/**
 *******************************************************************************
  ** \brief  processing for data out
  ** \param  pdev: device instance
  ** \param  epnum: endpoint index
  ** \retval none
 ******************************************************************************/
void usb_dev_hid_dataout(void *pdev, uint8_t epnum)
{
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    en_flag_status_t Led_State;
    if (epnum == HID_OUT_EP)
    {
      USBD_OutEvent();
//        if (Report_buf[1] == 0u)
//        {
//            Led_State = Reset;
//        }
//        else
//        {
//            Led_State = Set;
//        }

//        switch (Report_buf[0])
//        {
//            case 1: /* Led 1 */
//                LED0_CTL(Led_State);
//                break;

//            case 2: /* Led 2 */
//                LED1_CTL(Led_State);
//                break;
//            case 3: /* Led 3 */
//                LED2_CTL(Led_State);
//                break;
//            case 4: /* Led 4 */
//                LED3_CTL(Led_State);
//                break;
//            default:
//                LED0_CTL(Reset);
//                LED1_CTL(Reset);
//                LED2_CTL(Reset);
//                LED3_CTL(Reset);
//                break;
//        }
    }

    hd_usb_readytorx(pdev,HID_OUT_EP,Report_buf,HID_OUT_PACKET);
    hd_usb_devepstatusset(pdev , HID_OUT_EP , USB_EP_RX_VALID);
}
extern void USBD_OutEvent(void);

/**
 *******************************************************************************
 ** \brief  processing for request data in control endpoint
 ** \param  pdev: device instance
 ** \retval none
 ******************************************************************************/
void usb_dev_hid_EP0_RxReady(void *pdev)
{//OUTÊÂÎñ
  PRINTF_THIS("%s->%s\r\n",__filename(__FILE__),__func__);
    en_flag_status_t Led_State;
    
    if (IsReportAvailable == 1u)
    {
//      USBD_OutEvent();
        IsReportAvailable = 0u;
//        if (Report_buf[1] == 0u)
//        {
//            Led_State = Reset;
//        }
//        else
//        {
//            Led_State = Set;
//        }

//        switch (Report_buf[0])
//        {
//            case 1: /* Led 1 */
//                LED0_CTL(Led_State);
//                break;

//            case 2: /* Led 2 */
//                LED1_CTL(Led_State);
//                break;
//            case 3: /* Led 3 */
//                LED2_CTL(Led_State);
//                break;
//            case 4: /* Led 4 */
//                LED3_CTL(Led_State);
//                break;
//            default:
//                LED0_CTL(Reset);
//                LED1_CTL(Reset);
//                LED2_CTL(Reset);
//                LED3_CTL(Reset);
//                break;
//        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
