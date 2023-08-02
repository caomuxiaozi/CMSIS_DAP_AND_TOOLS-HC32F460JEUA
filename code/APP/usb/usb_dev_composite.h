#ifndef __USB_DEV_COMPOSITE_
#define __USB_DEV_COMPOSITE_
#include "hc32f460.h"
#include "usb_dev_ctrleptrans.h"
typedef enum{
  HID_INTERFACE       =((uint8_t)(0x0u)),
  CDC_COM_CMD_INTERFACE,
  CDC_COM_DAT_INTERFACE,
  MSC_INTERFACE
}COMPOSITE_INTERFACE_INDEX_Type;
extern usb_dev_class_func class_composite_cbk;

#endif
