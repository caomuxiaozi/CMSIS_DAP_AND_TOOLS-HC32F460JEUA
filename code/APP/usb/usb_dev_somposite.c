#include "usb_dev_composite.h"
#include "usb_dev_custom_hid_class.h"
#include "usb_dev_cdc_class.h"
#include "usb_dev_msc_class.h"
#include "usb_dev_desc.h"
#include "usb_dev_stdreq.h"

#define USB_COMPOSITE_CFGDESC_SIZE  (USB_CUSTOM_HID_CONFIG_DESC_SIZ  + USB_CDC_DESC_SIZ  + 8u/*IAP*/ +USB_MSC_DESC_SIZ)


/*USB回调函数*/
void usb_dev_composite_init(void *pdev);
void usb_dev_composite_deinit(void *pdev);
uint8_t usb_dev_composite_setup(void *pdev, USB_SETUP_REQ *req);
void usb_dev_composite_rxready(void *pdev);
void usb_dev_composite_datain(void *pdev, uint8_t epnum);
void usb_dev_composite_dataout(void *pdev, uint8_t epnum);
uint8_t usb_dev_composite_sof(void *pdev);
uint8_t *usb_dev_composite_getcfgdesc(uint16_t *length);
uint8_t *usb_dev_composite_GetDeviceQualifierDesc(uint16_t *length);
usb_dev_class_func  class_composite_cbk =
{
    &usb_dev_composite_init,
    &usb_dev_composite_deinit,
    &usb_dev_composite_setup,
    NULL,
    &usb_dev_composite_rxready,
    &usb_dev_composite_getcfgdesc,
    &usb_dev_composite_sof,
    &usb_dev_composite_datain,
    &usb_dev_composite_dataout,
    NULL,
    NULL,
    &usb_dev_composite_GetDeviceQualifierDesc,
};
__USB_ALIGN_BEGIN static uint8_t usb_dev_composite_cfgdesc[USB_COMPOSITE_CFGDESC_SIZE] __USB_ALIGN_END =
{
    0x09,                              /* bLength: Configuration Descriptor size */
    USB_CFG_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    USB_COMPOSITE_CFGDESC_SIZE,       /* wTotalLength: Bytes returned */
    0x00,
    0x04,         /*bNumInterfaces: 4 interfaces (2 for CDC, 1 for HID，1 for MSC)*/
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing the configuration*/
    0xE0,         /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
/*9*/
    /************** Descriptor of HID interface ****************/
    /* 09 */
    0x09,                          /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    HID_INTERFACE,                /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x02,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0X00,//0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0x00,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of HID ********************/
/* 18 */
    0x09,         /*bLength: HID Descriptor size*/
    CUSTOM_HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11,         /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,         /*bDescriptorType*/
    CUSTOM_HID_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of HID endpoint ********************/
/* 27 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
    HID_IN_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    HID_IN_PACKET, /*wMaxPacketSize: 4 Byte max */
    0x00,
    0x01,          /*bInterval: Polling Interval (10 ms)*/
/* 34 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
    HID_OUT_EP,     /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    HID_OUT_PACKET, /*wMaxPacketSize: 4 Byte max */
    0x00,
    0x01,          /*bInterval: Polling Interval (10 ms)*/
/* 41 */
    /******** /IAD should be positioned just before the CDC interfaces ******
                IAD to associate the two CDC interfaces */
    0x08, /* bLength */
    0x0B, /* bDescriptorType */
    0x01, /* bFirstInterface IAD从第一个接口开始*/
    0x02, /* bInterfaceCount */
    0x02, /* bFunctionClass */
    0x02, /* bFunctionSubClass */
    0x01, /* bFunctionProtocol */
    0x00, /* iFunction (Index of string descriptor describing this function) */
/*49*/
    /*Interface Descriptor */
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface descriptor type */
    CDC_COM_CMD_INTERFACE,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x01,   /* iInterface: */
/*58*/
    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
/*63*/
    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x02,   /* bDataInterface: 2 */
/*68*/
    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */
/*72*/
    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x01,   /* bMasterInterface: Communication class interface */
    0x02,   /* bSlaveInterface0: Data Class Interface */
/*77*/
    /*Endpoint 2 Descriptor*/
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
    0xFF,                           /* bInterval: */
/*84*/
    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    CDC_COM_DAT_INTERFACE,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */
/*93*/
    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(MAX_CDC_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(MAX_CDC_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */
/*100*/
    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType: Endpoint */
    CDC_IN_EP,                        /* bEndpointAddress */
    0x02,                             /* bmAttributes: Bulk */
    LOBYTE(MAX_CDC_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(MAX_CDC_PACKET_SIZE),
    0x00,                              /* bInterval */
/*107*/
    //Mass Storage interface ********************
    0x09,                           /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,                           /* bDescriptorType: */
    MSC_INTERFACE,                           /* bInterfaceNumber: Number of Interface */
    0x00,                           /* bAlternateSetting: Alternate setting */
    0x02,                           /* bNumEndpoints*/
    0x08,                           /* bInterfaceClass: MSC Class */
    0x06,                           /* bInterfaceSubClass : SCSI transparent*/
    0x50,                           /* nInterfaceProtocol */
    0x05,                           /* iInterface: */
/*116*/
    //Mass Storage Endpoints ********************
    0x07,                           /*Endpoint descriptor length = 7*/
    USB_ENDPOINT_DESCRIPTOR_TYPE,                           /*Endpoint descriptor type */
    MSC_IN_EP,                      /*Endpoint address (IN, address 1) */
    0x02,                           /*Bulk endpoint type */
    LOBYTE(MSC_MAX_PACKET),
    HIBYTE(MSC_MAX_PACKET),
    0x00,                           /*Polling interval in milliseconds */
/*123*/
    0x07,                           /*Endpoint descriptor length = 7 */
    0x05,                           /*Endpoint descriptor type */
    MSC_OUT_EP,                     /*Endpoint address (OUT, address 1) */
    0x02,                           /*Bulk endpoint type */
    LOBYTE(MSC_MAX_PACKET),
    HIBYTE(MSC_MAX_PACKET),
    0x00                            /*Polling interval in milliseconds*/
/*130*/
} ;

void usb_dev_composite_init(void *pdev)
{
    usb_dev_hid_init(pdev);
    usb_dev_cdc_init(pdev);
    usb_dev_msc_init(pdev);
}
void usb_dev_composite_deinit(void *pdev)
{
    usb_dev_hid_deinit(pdev);
    usb_dev_cdc_deinit(pdev);
    usb_dev_msc_deinit(pdev);
}

uint8_t usb_dev_composite_setup(void *pdev, USB_SETUP_REQ *req)
{
    uint8_t u8Res = USBD_OK;
    switch (req->bmRequest & USB_REQ_RECIPIENT_MASK)
    {
        case USB_REQ_RECIPIENT_INTERFACE:
            switch (req->wIndex){
              case HID_INTERFACE:{
                  u8Res = usb_dev_hid_setup(pdev, req);
              }break;
              case CDC_COM_CMD_INTERFACE:
              case CDC_COM_DAT_INTERFACE:{
                  u8Res = usb_dev_cdc_setup(pdev, req);
              }break;
              case MSC_INTERFACE:{
                u8Res = usb_dev_msc_setup(pdev,req);
              }break;
              default:
                break;
            }
            break;

        case USB_REQ_RECIPIENT_ENDPOINT:
            switch (req->wIndex){
              case HID_IN_EP:{
                  u8Res = usb_dev_hid_setup (pdev, req);
              }break;
              case CDC_IN_EP:{
                  u8Res = usb_dev_cdc_setup(pdev, req);
              }break;
              case MSC_IN_EP:{
                u8Res = usb_dev_msc_setup(pdev,req);
              }
              default:
                break;
            }
            break;
        default:
            break;
    }
    return u8Res;
}
uint8_t *usb_dev_composite_getcfgdesc(uint16_t *length)
{
    *length = (uint16_t)sizeof (usb_dev_composite_cfgdesc);
    return usb_dev_composite_cfgdesc;
}
void usb_dev_composite_datain(void *pdev, uint8_t epnum)
{
    switch (epnum){
      case ((uint8_t)CDC_IN_EP&((uint8_t)~0x80u)):{
          usb_dev_cdc_datain(pdev, epnum);
      }break;
      case ((uint8_t)HID_IN_EP&((uint8_t)~0x80u)):{
          usb_dev_hid_datain(pdev, epnum);
      }break;
      case ((uint8_t)MSC_IN_EP&((uint8_t)~0x80u)):{
          usb_dev_msc_datain(pdev,epnum);
      }break;
      default:
        break;
    }
}
void usb_dev_composite_dataout(void *pdev, uint8_t epnum)
{
    switch(epnum){
      case ((uint8_t)CDC_OUT_EP&(uint8_t)~0x80u):{
        usb_dev_cdc_dataout(pdev, epnum);
      }break;
      case ((uint8_t)HID_OUT_EP&(uint8_t)~0x80u):{
        usb_dev_hid_dataout(pdev, epnum);
      }break;
      case ((uint8_t)MSC_OUT_EP&(uint8_t)~0x80u):{
        usb_dev_msc_dataout(pdev,epnum);
      }break;
      default:
        break;
    }
    
}
uint8_t usb_dev_composite_sof(void *pdev)
{
    return (usb_dev_cdc_sof(pdev));
}
void usb_dev_composite_rxready(void *pdev)
{
    usb_dev_cdc_ctrlep_rxready(pdev);
    usb_dev_hid_EP0_RxReady(pdev);
}

uint8_t *usb_dev_composite_GetDeviceQualifierDesc(uint16_t *length){
  return usb_dev_hid_GetDeviceQualifierDesc(length);
}
