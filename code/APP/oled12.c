// 20150417升级 www.lcdsoc.com
/* 采用IO口模拟SPI通讯方式驱动oled模块
CS   ~ PE6
RES ~ PE5
DC   ~ PE4
CLK  ~ PE3
DIN ~ PE2
*/
#include "oled.h"
#ifdef HARD_IIC
#include "hc32f460_i2c.h"
#elif defined SOFT_IIC
#include "softiic.h"
#endif
#include "stdarg.h"
#include "stdio.h"
#include "driver.h"
extern const uint8_t SGUI_FONT_H8[];
extern const uint8_t SGUI_FONT_H16[];
#define TIMEOUT ((uint32_t)0x1000)
#define OLED_ADDR (0x78>>1)

#define  ASCII6x8  SGUI_FONT_H8
#define  ASCII8x16 SGUI_FONT_H16


void OLED_IIC_Write(uint8_t DEV_ID, uint8_t* buf, uint8_t len) {
#ifdef HARD_IIC
  en_result_t ret;
  ret = I2C_Start(OLED_IIC, TIMEOUT);
//    if(ret == Ok){
  ret = I2C_TransAddr(OLED_IIC, DEV_ID, I2CDirTrans, TIMEOUT);
//      if(ret == Ok){
  I2C_TransData(OLED_IIC, buf, len, TIMEOUT);
//      }
//    }
  I2C_Stop(OLED_IIC, TIMEOUT);
//    if(ret != Ok){
//    I2C_SoftwareResetCmd(OLED_IIC, Enable);
//    I2C_SoftwareResetCmd(OLED_IIC, Disable);
//    }

#else
  IIC_Start();
  IIC_WriteByte(DEV_ID << 1);
  while(len--) {
    IIC_WriteByte(*buf);
    buf++;
  }
  IIC_Stop();
#endif
}
extern void delay1ms(uint16_t);
#define delayms(x) delay1ms(x);
void        delay_us(uint32_t num) {
  uint32_t t = num * 2;
  while(t--)
    ;
}
/*************初始化SSD1309与 stm32 io口
 * www.lcdsoc.com*************************/
void OLED_Write_Cmd(uint8_t cmd) {
  uint8_t buf[2];
  buf[0] = 0x00;  // control byte
  buf[1] = cmd;
  //使用HAL库的API实现
  OLED_IIC_Write(OLED_ADDR, buf, 2);
//  delayms(1);
}
void OLED_Write_Dat(uint8_t dat) {
  uint8_t buf[2];
  buf[0] = 0x40;  // control byte
  buf[1] = dat;
  //使用HAL库的API实现
  OLED_IIC_Write(OLED_ADDR, buf, 2);
//  delayms(1);
}
/**
 * @brief OLED设置显示位置
 * @param x - X方向位置Column
 * @param y - Y方向位置Page
 */
void OLED_Set_Pos(uint8_t x, uint8_t y) {

  OLED_Write_Cmd(0xb0 + y);
  OLED_Write_Cmd((x & 0x0f) /*| 0x01*/);
  OLED_Write_Cmd(((x & 0xf0) >> 4) | 0x10);

}
/**
 * @brief OLED开启显示
 */
void OLED_Display_On(void) {
  OLED_Write_Cmd(0X8D);  // SET DCDC命令
  OLED_Write_Cmd(0X14);  // DCDC ON
  OLED_Write_Cmd(0XAF);  // DISPLAY ON
}
/**
 * @brief OLED关闭显示
 */
void OLED_Display_Off(void) {
//    OLED_Write_Cmd(0X8D);  // SET DCDC命令
//    OLED_Write_Cmd(0X10);  // DCDC OFF
  OLED_Write_Cmd(0XAE);  // DISPLAY OFF
}
void Fill_RAM(uint8_t x) {
  uint8_t i, n;
  for(i = 0; i < 8; i++) {
    OLED_Write_Cmd(0xb0 + i);  //设置页地址（0~7）
    OLED_Write_Cmd(0x02);  //设置显示位置—列低地址
    OLED_Write_Cmd(0x10);  //设置显示位置—列高地址
    for(n = 0; n < 128; n++) {
      OLED_Write_Dat(x);
    }
  }
}
/**
 * @brief OLED清屏函数（清屏之后屏幕全为黑色）
 */
void OLED_Clear(uint8_t x) {
  Fill_RAM(0);
}
/**
 * @brief OLED显示全开（所有像素点全亮）
 */
void OLED_On(void) {
  Fill_RAM(0xff);
}
/**
 * @brief 在指定位置显示一个ASCII字符
 * @param x - 0 - 127
 * @param y - 0 - 7
 * @param chr  - 待显示的ASCII字符
 * @param size - ASCII字符大小
 *        字符大小有12(6*8)/16(8*16)两种大小
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t isNeg) {
  uint8_t c = 0, i = 0;
  c = chr - ' ';
  if(x > 128 - 1) {
    x = 0;
    y++;
  }
//    uint8_t  j = 0;
//    if(size == 64){//32*64
//      if(chr == '-'){
//        c = 10;
//      }else if(chr >= '0' && chr <= '9'){
//        c = chr - '0';
//      }else{
//        return;
//      }
//      for (j = 0;j < 8;j++){
//        OLED_Set_Pos(x,y+j);
//        for(i = 0;i<32;i++){
//          if(isNeg){
//            OLED_Write_Dat(~ASC32x64[c][32*j+i]);
//          }else{
//            OLED_Write_Dat(ASC32x64[c][32*j+i]);
//          }
//        }
//      }
//    }
  /*else if(size == 32) {  // 16*32
      for(j = 0; j < 4; j++) {
  //            if(y + j > 7)
  //                return;
          OLED_Set_Pos(x, y + j);
          for(i = 0; i < 16; i++) {
              OLED_Write_Dat(ASC16x32[c][j * 16 + i]);
          }
      }
  }*/
//    else
  if(size == 16) {  // 8*16
    OLED_Set_Pos(x, y);
    for(i = 0; i < 8; i++) {
      if(isNeg)
        OLED_Write_Dat(~ASCII8x16[c * 16 +/*][*/i]);
      else
        OLED_Write_Dat(ASCII8x16[c * 16 +/*][*/i]);
    }
    OLED_Set_Pos(x, y + 1);
    for(i = 0; i < 8; i++) {
      if(isNeg)
        OLED_Write_Dat(~ASCII8x16[c * 6 +/*][*/i + 8]);
      else
        OLED_Write_Dat(ASCII8x16[c * 6 +/*][*/i + 8]);
    }
  } else { // default 6*8
    OLED_Set_Pos(x, y);
    for(i = 0; i < 6; i++) {
      OLED_Write_Dat(ASCII6x8[c * 6 +/*][*/i]);
    }
  }
}
void Asc6_8(uint8_t column, uint8_t page, char* c) {
  while(*c != 0) {
    OLED_ShowChar(column, page, *c, 8, 0);
    c++;
    column += 6;
  }
}
void Asc8_16(uint8_t column, uint8_t page, char* c) {
  while(*c != 0) {
    OLED_ShowChar(column, page, *c, 16, 0);
    c++;
    column += 8;
  }
}
/**
 * @brief OLED 专用pow函数
 * @param m - 底数
 * @param n - 指数
 */
static uint32_t oled_pow(uint8_t m, uint8_t n) {
  uint32_t result = 1;
  while(n--)
    result *= m;
  return result;
}
/**
 * @brief 在指定位置显示一个整数
 * @param x - 0 - 127
 * @param y - 0 - 7
 * @param num - 待显示的整数(0-4294967295)
 * @param len - 数字的位数
 * @param size - ASCII字符大小
 *        字符大小有12(6*8)/16(8*16)两种大小
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
  uint8_t t, temp;
  uint8_t enshow = 0;
  for(t = 0; t < len; t++) {
    temp = (num / oled_pow(10, len - t - 1)) % 10;
    if(enshow == 0 && t < (len - 1)) {
      if(temp == 0) {
        OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
        continue;
      } else
        enshow = 1;
    }
    OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
  }
}
/**
 * @brief 在指定位置显示一个字符串
 * @param x - 0 - 127
 * @param y - 0 - 7
 * @param chr - 待显示的字符串指针
 * @param size - ASCII字符大小
 *        字符大小有12(6*8)/16(8*16)两种大小
 */
void OLED_ShowString(uint8_t x, uint8_t y, char* chr, uint8_t size, uint8_t isNeg) {
  uint8_t j = 0;
  while(chr[j] != '\0') {
    OLED_ShowChar(x, y, chr[j], size, isNeg);
    x += (size >> 1);
    if(x > 128 - (size >> 1)) {
      //      x = 0;
      //      y += 2;
      return;
    }
    j++;
  }
}
/**
 * @brief 在指定位置显示一个汉字
 * @param x  - 0 - 127
 * @param y  - 0 - 7
 * @param no - 汉字在中文字库数组中的索引（下标）
 * @note
 * 中文字库在oledfont.h文件中的Hzk数组中，需要提前使用软件对汉字取模
 */
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t* no) {
  uint8_t t, adder = 0;
  OLED_Set_Pos(x, y);
  for(t = 0; t < 16; t++) {
    OLED_Write_Dat(no[t]);
    adder += 1;
  }
  OLED_Set_Pos(x, y + 1);
  for(t = 0; t < 16; t++) {
    OLED_Write_Dat(no[16 + t]);
    adder += 1;
  }
}
/**
 * @brief 在指定位置显示一幅图片
 * @param x1,x2  - 0 - 127
 * @param y1,y2  - 0 - 7(8表示全屏显示)
 * @param BMP - 图片数组地址
 * @note  图像数组BMP存放在bmp.h文件中
 */
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]) {
  uint16_t j = 0;
  uint8_t  x, y;
  if(y1 % 8 == 0) {
    y = y1 / 8;
  } else {
    y = y1 / 8 + 1;
  }
  for(y = y0; y < y1; y++) {
    OLED_Set_Pos(x0, y);
    for(x = x0; x < x1; x++) {
      OLED_Write_Dat(BMP[j++]);
    }
  }
}
/**
 * @brief OLED初始化
 */
void OLED_SetBrightness(uint8_t brightness) {
  OLED_Write_Cmd(0x81);  // 设置对比度
  OLED_Write_Cmd(brightness);  //最高对比度
}
extern void oledDmaTransComplete(void);
void OLED_Init(void) {
#ifdef HARD_IIC
  I2C3_OLED_Config(1500000);
#elif defined SOFT_IIC
  stc_port_init_t gpioInitStruct;
  MEM_ZERO_STRUCT(gpioInitStruct);
  gpioInitStruct.enPinMode = Pin_Mode_Out;
  gpioInitStruct.enPinDrv = Pin_Drv_H;
  gpioInitStruct.enPinOType = Pin_OType_Od;
  gpioInitStruct.enPullUp = Enable;
  PORT_Init(PORT_OLED_I2C3_SCL, PIN__OLED_I2C3_SCL, &gpioInitStruct);
  PORT_Init(PORT_OLED_I2C3_SDA, PIN__OLED_I2C3_SDA, &gpioInitStruct);

#endif
  delayms(1);
#if 0
  OLED_Write_Cmd(0xAE);  //--display off

  OLED_Write_Cmd(0x00);  //---set low column address
  OLED_Write_Cmd(0x10);  //---set high column address

  OLED_Write_Cmd(0xA8);  //--set multiplex ratio(1 to 64)
  OLED_Write_Cmd(0x3F);  //--设置为64行,扫描起始行

  OLED_Write_Cmd(0xD3);  //-set display offset
  OLED_Write_Cmd(0x00);  //滚动效果

  OLED_Write_Cmd(0x40);  //--set start line address
  {
    OLED_Write_Cmd(0xAD);
    OLED_Write_Cmd(0x8B);
  }
  OLED_Write_Cmd(0xA1);  // set segment remap,左右反向

  OLED_Write_Cmd(0xC8);  // Com scan direction

  OLED_Write_Cmd(0xDA);  // set com pin configuartion
  OLED_Write_Cmd(0x12);  //

  OLED_Write_Cmd(0x81);  // 设置对比度
  OLED_Write_Cmd(0xaF);  //最高对比度

//    OLED_Write_Cmd(0xD8);
//    OLED_Write_Cmd(0x05);

  OLED_Write_Cmd(0xD9);  // Set Pre-Charge Period
  OLED_Write_Cmd(0xF1);  //

  OLED_Write_Cmd(0xDB);  // set Vcomh
  OLED_Write_Cmd(0x40);  //0.43*verf
  {
    OLED_Write_Cmd(0x30);
  }
  OLED_Write_Cmd(0xA6);  //--normal / reverse

  OLED_Write_Cmd(0xD5);  // set osc division
  OLED_Write_Cmd(0x50);

  OLED_Write_Cmd(0x20);
  OLED_Write_Cmd(0x00);
//    OLED_Write_Cmd(0x8D);  // set charge pump enable
//    OLED_Write_Cmd(0x14);  //
//    OLED_Write_Cmd(0xA4);
//    OLED_Write_Cmd(0xA6);
  OLED_Write_Cmd(0xAF);  //--turn on oled panel
//    OLED_Write_Cmd(0xA5); //全亮
  OLED_Clear(0);
  OLED_Set_Pos(0, 0);
  OLED_Write_Cmd(0xA4);
#endif
  /* 模块厂家提供初始化代码 */
  OLED_Write_Cmd(0xAE);        /* 关闭OLED面板显示(休眠) */
  OLED_Write_Cmd(0x00);        /* 设置列地址低4bit */
  OLED_Write_Cmd(0x10);        /* 设置列地址高4bit */
  OLED_Write_Cmd(0x40);        /* 设置起始行地址（低5bit 0-63）， 硬件相关*/

  OLED_Write_Cmd(0x81);        /* 设置对比度命令(双字节命令），第1个字节是命令，第2个字节是对比度参数0-255 */
  OLED_Write_Cmd(0xCF);        /* 设置对比度参数,缺省CF */

#ifdef DIR_NORMAL
  OLED_Write_Cmd(0xA0);        /* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
  OLED_Write_Cmd(0xC0);        /* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
#endif

//#ifdef DIR_180
  OLED_Write_Cmd(0xA1);        /* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
  OLED_Write_Cmd(0xC8);        /* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
//#endif

  OLED_Write_Cmd(0xA6);        /* A6 : 设置正常显示模式; A7 : 设置为反显模式 */

  OLED_Write_Cmd(0xA8);        /* 设置COM路数 */
  OLED_Write_Cmd(0x3F);        /* 1 ->（63+1）路 */

  OLED_Write_Cmd(0xD3);        /* 设置显示偏移（双字节命令）*/
  OLED_Write_Cmd(0x02);        /* 无偏移 */

  OLED_Write_Cmd(0xD5);        /* 设置显示时钟分频系数/振荡频率 */
  OLED_Write_Cmd(0x80);        /* 设置分频系数,高4bit是分频系数，低4bit是振荡频率 */

  OLED_Write_Cmd(0xD9);        /* 设置预充电周期 */
  OLED_Write_Cmd(0xF1);        /* [3:0],PHASE 1; [7:4],PHASE 2; */

  OLED_Write_Cmd(0xDA);        /* 设置COM脚硬件接线方式 */
  OLED_Write_Cmd(0x12);

  OLED_Write_Cmd(0xDB);        /* 设置 vcomh 电压倍率 */
  OLED_Write_Cmd(0x40);        /* [6:4] 000 = 0.65 x VCC; 0.77 x VCC (RESET); 0.83 x VCC  */

//        OLED_Write_Cmd(0x20);
//        OLED_Write_Cmd(0x00);

  OLED_Write_Cmd(0x8D);        /* 设置充电泵（和下个命令结合使用） */
  OLED_Write_Cmd(0x14);        /* 0x14 使能充电泵， 0x10 是关闭 */
  OLED_Write_Cmd(0xAF);        /* 打开OLED面板 */

  {
    stc_dma_config_t stcDmaCfg;
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA2, Enable);
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);
    MEM_ZERO_STRUCT(stcDmaCfg);
    stcDmaCfg.u16BlockSize           = 1;
    stcDmaCfg.u16TransferCnt         = 0;
    stcDmaCfg.u32SrcAddr             = (uint32_t)(0);
    stcDmaCfg.u32DesAddr             = (uint32_t)(&M4_I2C3->DTR);
    stcDmaCfg.u16SrcRptSize          = 0;
    stcDmaCfg.stcDmaChCfg.enSrcRptEn = Disable;
    stcDmaCfg.stcDmaChCfg.enDesRptEn = Disable;
    stcDmaCfg.stcDmaChCfg.enSrcInc   = AddressIncrease;
    stcDmaCfg.stcDmaChCfg.enDesInc   = AddressFix;
    stcDmaCfg.stcDmaChCfg.enTrnWidth = Dma8Bit;
    stcDmaCfg.stcDmaChCfg.enIntEn    = Enable;
    DMA_InitChannel(M4_DMA2, DmaCh1, &stcDmaCfg);
    DMA_SetTriggerSrc(M4_DMA2, DmaCh1, EVT_I2C3_TEI);//IIC传输完成触发DMA更新传输


    stc_irq_regi_conf_t stcIrqRegiCfg;
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stcIrqRegiCfg.enIRQn      = Int000_DMA2_CH1_TXC_IRQn;
    stcIrqRegiCfg.pfnCallback = &oledDmaTransComplete;
    stcIrqRegiCfg.enIntSrc    = INT_DMA2_TC1;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_00);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
  }
}
//const char errFont[] = {0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,
//0xC0,0x80,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0x3F,0x1F,0x0F,0x07,0x03,0x01,
//0x00,0x00};
extern uint16_t getZh16Font(uint8_t* pZh, uint8_t* pZk);
void ShowStr(uint8_t page, uint8_t column, uint8_t size, void* strs) {

  uint8_t* str       = strs;
  uint8_t  isChinese = 0;
  while(*str != 0) {
    if(!isChinese) {
      if(*str > 0x80) { // chinese
        isChinese = 1;
      } else {
        if(*str == 0x0d || *str == 0x0a) {
          column += size / 2;
        } else {
          if(size == 64) {
            OLED_ShowChar(column, page, *str, 64, 0);
            column += 32;
          } else if(size == 16) {
            OLED_ShowChar(column, page, *str, 16, 0);
            column += 8;
          } else if(size == 8) {
            OLED_ShowChar(column, page, *str, 8, 0);
            column += 6;
          }
        }
        str++;
      }
    } else {
      isChinese = 0;
//            uint8_t charHaveCode = 0;
      /*if(size == 16)*/ {
//第一个中文 啊 的编码是0XB0A1 INDEX=((GBH-0xA1)×94+GBL-0XA1)×(size)；
        unsigned char GB2312_16x16[32];
        uint16_t GB2312Code = (*str)  | (*(str + 1)) << 8;
        if(getZh16Font((uint8_t*)&GB2312Code, GB2312_16x16)) {
          OLED_ShowCHinese(column, page, GB2312_16x16);
        }
        (str) += 2;
        column += size;
      }
    }
  }
}
extern bool OLED_DMA_RUNING;
void ShowStrAndParamBase(uint8_t page, uint8_t column, uint8_t size, const char* fmt, ...) {
  if(OLED_DMA_RUNING == true) {
    return;
  }
  char strs[64];
  MEM_ZERO_STRUCT(strs);
  va_list args;
  va_start(args, fmt);
  vsnprintf(strs, 64u, fmt, args);
  va_end(args);
  ShowStr(page, column, size, strs);
}
uint8_t oledLog[8][21];
void OLED_LOG(const char* fmt, ...) {
  if(OLED_DMA_RUNING == true) {
    return;
  }
  char strs[64];
  MEM_ZERO_STRUCT(strs);
  va_list args;
  va_start(args, fmt);
  uint8_t len = vsnprintf(strs, 64u, fmt, args);
  va_end(args);
  static uint8_t logIndex = 0;
  memcpy((char*)&oledLog[logIndex][0], strs, 20);
  uint8_t startIndex = logIndex + 1;
  if(startIndex > 7)
    startIndex = 0;
  Fill_RAM(0);
  for(uint8_t i = 0; i < 8; i++) {
    ShowStr(i, 2, 8, oledLog[startIndex]);
    if(++startIndex > 7) {
      startIndex = 0;
    }
  }
  if(++logIndex > 7) {
    logIndex = 0;
  }

}

extern uint32_t sysRunMinute;
extern uint32_t fireRunMinute;
void ShowBoardTimeInfo(uint16_t year, uint8_t month, uint8_t day, char* time) {
  ShowStrAndParamBase(4, 0, 8, "Code Compile Data:");
  ShowStrAndParamBase(5, 0, 8, "%04d-%02d-%02d %s", year, month, day, time);
//    ShowStrAndParamBase(6,0,8,"pwrnRun:%ldmin",sysRunMinute * SAVE_TIME_INTERVAL);
//    ShowStrAndParamBase(7,0,8,"fireRun:%ldmin",fireRunMinute * SAVE_TIME_INTERVAL);
}
