/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file usb_dev_msc_msd.c
 **
 ** A detailed description is available at
 ** @link
        This file includes the user MSC application layer.
    @endlink
 **
 **   - 2019-05-15  1.0  CDT First version for USB MSC device demo.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "usb_dev_msc_fops.h"
#include "usb_dev_msc_mem.h"
#include "usb_app_conf.h"
#include "sd_Card.h"
//#include "w25qxx.h"
//#define PRINTF_THIS printf
#ifndef PRINTF_THIS
#define PRINTF_THIS(...)
#endif
int8_t msc_init(uint8_t lun);
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size);
int8_t msc_ifready(uint8_t lun);
int8_t msc_ifwrprotected(uint8_t lun);
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t msc_getmaxlun(void);

USB_DEV_MSC_cbk_TypeDef flash_fops =
{
    &msc_init,
    &msc_getcapacity,
    &msc_getmaxlun,
    &msc_ifready,
    &msc_read,
    &msc_write,    
    &msc_ifwrprotected,    
    (int8_t *)msc_inquirydata
};

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/* Variable for Storage operation status */
volatile uint8_t USB_STATUS_REG = 0u;

/* USB Mass storage querty data (36 bytes for each lun) */
const int8_t msc_inquirydata[] =
{
    /* LUN 0 */
    0x00,//磁盘设备
    0x80,//可移除设备
    0x02,//ANSC指令集版本号
    0x02,//对应的数据响应格式
    (USB_DEV_INQUIRY_LENGTH - 4u),
    0x00,//保留
    0x00,//保留
    0x00,//保留
    /* Vendor Identification *///8 bytes manufacturer
    'Z', 'o', 'o', 'n', 'l', 'i', 'n', ' ',
    /* Product Identification *///16 bytes product
    'D','i','s','k',' ','R','A','M',' ',' ',' ',' ',' ',' ',' ',' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             //4 bytes version
    /* LUN 1 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USB_DEV_INQUIRY_LENGTH - 4u),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification *///8 bytes manufacturer
    'Z', 'o', 'o', 'n', 'l', 'i', 'n', ' ',
    /* Product Identification *///16 bytes product
    'D', 'i', 's', 'k', ' ', 'S', 'D', ' ', 'N','A','N','D',' ',' ',' ',' ',
    /* Product Revision Level */
    '1', '.', '0', ' ',                             //4 bytes version
};
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/* Pointer to flash_fops */
USB_DEV_MSC_cbk_TypeDef *msc_fops = &flash_fops;
//#define RAMDISK_BLOCK_SIZE 512
//#define RAMDISK_BLOCK_NUM  64
//uint8_t ramDisk[RAMDISK_BLOCK_SIZE*RAMDISK_BLOCK_NUM]={//第一扇区填充
//0xeb,0x3c,0x90,0x4d,0x53,0x44,0x4f,0x53,0x35,0x2e,0x30,0x00,0x02,0x01,0x06,0x00,
//0x02,0x00,0x02,0x40,0x00,0xf8,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x80,0x00,0x29,0xb9,0xe9,0xae,0x38,0x4e,0x4f,0x20,0x4e,0x41,
//0x4d,0x45,0x20,0x20,0x20,0x20,0x46,0x41,0x54,0x31,0x32,0x20,0x20,0x20,0x33,0xc9,
//0x8e,0xd1,0xbc,0xf0,0x7b,0x8e,0xd9,0xb8,0x00,0x20,0x8e,0xc0,0xfc,0xbd,0x00,0x7c,
//0x38,0x4e,0x24,0x7d,0x24,0x8b,0xc1,0x99,0xe8,0x3c,0x01,0x72,0x1c,0x83,0xeb,0x3a,
//0x66,0xa1,0x1c,0x7c,0x26,0x66,0x3b,0x07,0x26,0x8a,0x57,0xfc,0x75,0x06,0x80,0xca,
//0x02,0x88,0x56,0x02,0x80,0xc3,0x10,0x73,0xeb,0x33,0xc9,0x8a,0x46,0x10,0x98,0xf7,
//0x66,0x16,0x03,0x46,0x1c,0x13,0x56,0x1e,0x03,0x46,0x0e,0x13,0xd1,0x8b,0x76,0x11,
//0x60,0x89,0x46,0xfc,0x89,0x56,0xfe,0xb8,0x20,0x00,0xf7,0xe6,0x8b,0x5e,0x0b,0x03,
//0xc3,0x48,0xf7,0xf3,0x01,0x46,0xfc,0x11,0x4e,0xfe,0x61,0xbf,0x00,0x00,0xe8,0xe6,
//0x00,0x72,0x39,0x26,0x38,0x2d,0x74,0x17,0x60,0xb1,0x0b,0xbe,0xa1,0x7d,0xf3,0xa6,
//0x61,0x74,0x32,0x4e,0x74,0x09,0x83,0xc7,0x20,0x3b,0xfb,0x72,0xe6,0xeb,0xdc,0xa0,
//0xfb,0x7d,0xb4,0x7d,0x8b,0xf0,0xac,0x98,0x40,0x74,0x0c,0x48,0x74,0x13,0xb4,0x0e,
//0xbb,0x07,0x00,0xcd,0x10,0xeb,0xef,0xa0,0xfd,0x7d,0xeb,0xe6,0xa0,0xfc,0x7d,0xeb,
//0xe1,0xcd,0x16,0xcd,0x19,0x26,0x8b,0x55,0x1a,0x52,0xb0,0x01,0xbb,0x00,0x00,0xe8,
//0x3b,0x00,0x72,0xe8,0x5b,0x8a,0x56,0x24,0xbe,0x0b,0x7c,0x8b,0xfc,0xc7,0x46,0xf0,
//0x3d,0x7d,0xc7,0x46,0xf4,0x29,0x7d,0x8c,0xd9,0x89,0x4e,0xf2,0x89,0x4e,0xf6,0xc6,
//0x06,0x96,0x7d,0xcb,0xea,0x03,0x00,0x00,0x20,0x0f,0xb6,0xc8,0x66,0x8b,0x46,0xf8,
//0x66,0x03,0x46,0x1c,0x66,0x8b,0xd0,0x66,0xc1,0xea,0x10,0xeb,0x5e,0x0f,0xb6,0xc8,
//0x4a,0x4a,0x8a,0x46,0x0d,0x32,0xe4,0xf7,0xe2,0x03,0x46,0xfc,0x13,0x56,0xfe,0xeb,
//0x4a,0x52,0x50,0x06,0x53,0x6a,0x01,0x6a,0x10,0x91,0x8b,0x46,0x18,0x96,0x92,0x33,
//0xd2,0xf7,0xf6,0x91,0xf7,0xf6,0x42,0x87,0xca,0xf7,0x76,0x1a,0x8a,0xf2,0x8a,0xe8,
//0xc0,0xcc,0x02,0x0a,0xcc,0xb8,0x01,0x02,0x80,0x7e,0x02,0x0e,0x75,0x04,0xb4,0x42,
//0x8b,0xf4,0x8a,0x56,0x24,0xcd,0x13,0x61,0x61,0x72,0x0b,0x40,0x75,0x01,0x42,0x03,
//0x5e,0x0b,0x49,0x75,0x06,0xf8,0xc3,0x41,0xbb,0x00,0x00,0x60,0x66,0x6a,0x00,0xeb,
//0xb0,0x42,0x4f,0x4f,0x54,0x4d,0x47,0x52,0x20,0x20,0x20,0x20,0x0d,0x0a,0x52,0x65,
//0x6d,0x6f,0x76,0x65,0x20,0x64,0x69,0x73,0x6b,0x73,0x20,0x6f,0x72,0x20,0x6f,0x74,
//0x68,0x65,0x72,0x20,0x6d,0x65,0x64,0x69,0x61,0x2e,0xff,0x0d,0x0a,0x44,0x69,0x73,
//0x6b,0x20,0x65,0x72,0x72,0x6f,0x72,0xff,0x0d,0x0a,0x50,0x72,0x65,0x73,0x73,0x20,
//0x61,0x6e,0x79,0x20,0x6b,0x65,0x79,0x20,0x74,0x6f,0x20,0x72,0x65,0x73,0x74,0x61,
//0x72,0x74,0x0d,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xac,0xcb,0xd8,0x55,0xaa};
enum{
  MSC_RAM = 1,
  MSC_SD_NAND=0,
}MSC_INDEX;
/**
 *******************************************************************************
 ** \brief  initialize storage
 ** \param  lun: logic number
 ** \retval status
 ******************************************************************************/
int8_t msc_init(uint8_t lun)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
//    W25QXX_Init();
    return Ok;
}

/**
 *******************************************************************************
 ** \brief  Get Storage capacity
 ** \param  lun: logic number
 ** \param  block_num: sector number
 ** \param  block_size: sector size
 ** \retval status
 ******************************************************************************/
int8_t msc_getcapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
  switch(lun){
    case MSC_RAM:
    *block_size = 0;//RAMDISK_BLOCK_SIZE;
    *block_num  = 0;//RAMDISK_BLOCK_NUM;//1024u * 1024u * 8u / 512u;
    break;
    case MSC_SD_NAND:
      *block_size = stcSdhandle.stcSdCardInfo.u32LogBlockSize;
      *block_num = stcSdhandle.stcSdCardInfo.u32LogBlockNbr;
      break;
  }
    return Ok;
}

/**
 *******************************************************************************
 ** \brief  Check if storage is ready
 ** \param  lun: logic number
 ** \retval status
 ******************************************************************************/
int8_t  msc_ifready(uint8_t lun)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
  switch(lun){
    case MSC_RAM:
    USB_STATUS_REG |= (uint8_t)0X10;
    break;
    case MSC_SD_NAND:
//      if(sdCardLocked){
//        return OperationInProgress;
//      }else{
        return Ok;
//      }
    break;
  }
    return Ok;
}

/**
 *******************************************************************************
 ** \brief  Check if storage is write protected
 ** \param  lun: logic number
 ** \retval status
 ******************************************************************************/
int8_t msc_ifwrprotected(uint8_t lun)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
    return Ok;
}

/**
 *******************************************************************************
 ** \brief  read data from storage devices
 ** \param  lun: logic number
 ** \param  buf: data buffer be read
 ** \param  blk_addr: sector address
 ** \param  blk_len: sector count
 ** \retval status
 ******************************************************************************/
int8_t msc_read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
    int8_t res = (int8_t)0;
    USB_STATUS_REG |= (uint8_t)0X02;
  switch(lun){
    case MSC_RAM:
//    memcpy(buf,ramDisk + blk_addr * RAMDISK_BLOCK_SIZE,blk_len*RAMDISK_BLOCK_SIZE);
    break;
    case MSC_SD_NAND:
       res = sdCard_Read_Lock(blk_addr,blk_len,buf);
    break;
  }
//    W25QXX_Read(buf, blk_addr * 512u, blk_len * 512u);
    return res;
}

/**
 *******************************************************************************
 ** \brief  Write data to storage devices
 ** \param  lun: logic number
 ** \param  buf: data buffer be written
 ** \param  blk_addr: sector address
 ** \param  blk_len: sector count
 ** \retval status
 ******************************************************************************/

int8_t msc_write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  PRINTF_THIS("%s->%s lun=%d\r\n",__filename(__FILE__),__func__,lun);
    int8_t res = (int8_t)0;
    USB_STATUS_REG |= (uint8_t)0X01;
  switch(lun){
    case MSC_RAM:
//    memcpy(ramDisk+blk_addr*RAMDISK_BLOCK_SIZE,buf,blk_len*RAMDISK_BLOCK_SIZE);
    break;
    case MSC_SD_NAND:
      res =  sdCard_Write_Lock(blk_addr,blk_len,buf);
      break;
  }
//    W25QXX_Write(buf, blk_addr * 512u, blk_len * 512u);
    return res;
}

/**
 *******************************************************************************
 ** \brief  Get supported logic number
 ** \param  none
 ** \retval 1
 ******************************************************************************/
int8_t msc_getmaxlun(void)
{
  PRINTF_THIS("%s->%s \r\n",__filename(__FILE__),__func__);
    return 2;
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
