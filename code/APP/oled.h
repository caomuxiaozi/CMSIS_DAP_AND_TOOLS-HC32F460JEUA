#ifndef __OLED_H_
#define __OLED_H_

#include "driver.h"

#define OLED_1p3INCH
//#define OLED_2p4INCH
#define HARD_IIC
//#define SOFT_IIC

#define OLED_BRIGHTNESS_MAX 0xcf
#define OLED_BRIGHTNESS_MIN 0x0f

#if (!defined OLED_1p3INCH) && (!defined OLED_2p4INCH)
//#define OLED_1p3INCH
#error "没选择用啥屏幕,需要在 modeCtrl.h 里面定义1.3寸小屏【#define OLED_1p3INCH】或者2.4寸大屏【#define OLED_2p4INCH】"
#define OLED_2p4INCH
#endif


// extern const unsigned char ASC8X16[][16];
// extern const unsigned char ASC6X8[][6];
// extern const typFNT_GB16 HZ16x16[];
#define OLED_MODE 0
#define SIZE 16
#define XLevelL 0x02
#define XLevelH 0x10
#define XLevel ((XLevelH & 0x0F) * 16 + XLevelL)
#define Max_Column 128
#define Max_Row 64
#define Brightness 0x8f
#define X_WIDTH 128
#define Y_WIDTH 64
//滚动方向定义
#define RIGHT 0x26
#define LEFT 0x27
#define chineseText 1
#define asciiText 0
//-----------------OLED端口定义 4线SPI----------------
/* 采用IO口模拟SPI通讯方式驱动oled模块
CS   ~ PE6
RES ~ PE5
DC   ~ PE4
CLK  ~ PE3
DIN ~ PE2
*/

#define OLED_DC_Clr()                                                          \
  PORT_ResetBits(PORT_OLEDDC, PIN__OLEDDC) // CS
#define OLED_DC_Set()                                                          \
  PORT_SetBits(PORT_OLEDDC, PIN__OLEDDC)

#define OLED_RST_Clr()                                                         \
  PORT_ResetBits(PORT_OLEDRES, PIN__OLEDRES) // RES
#define OLED_RST_Set()                                                         \
  PORT_SetBits(PORT_OLEDRES, PIN__OLEDRES)

#define OLED_CS_Clr()                                                          \
  PORT_ResetBits(PORT_OLEDDC, PIN__OLEDDC) // DC
#define OLED_CS_Set()                                                          \
  PORT_SetBits(PORT_OLEDDC, PIN__OLEDDC)

#define OLED_SDIN_Clr()                                                        \
  PORT_ResetBits(PORT_OLEDDIN, PIN__OLEDDIN) // DIN
#define OLED_SDIN_Set()                                                        \
  PORT_SetBits(PORT_OLEDDIN, PIN__OLEDDIN)
#define OLED_SCLK_Clr() \
  PORT_ResetBits(PORT_OLEDCLK, PIN__OLEDCLK)
#define OLED_SCLK_Set() \
  PORT_SetBits(PORT_OLEDCLK, PIN__OLEDCLK)

#define OLED_CMD 0  //写命令
#define OLED_DATA 1 //写数据

//void delay_ms(uint32_t num);
//void delay_us(uint32_t num);
// OLED初始化函数 www.lcdsoc.com
void OLED_Init(void);

// OLED命令设置函数集
#ifdef  OLED_2p4INCH
void Set_Start_Column(unsigned char d);    //设置页模式开始列
void Set_Addressing_Mode(unsigned char d); //设置显存寻址模式
void Set_Column_Address(unsigned char a,
                        unsigned char b); //设置列地址   水平寻址模式中使用
void Set_Page_Address(unsigned char a,
                      unsigned char b); //设置页地址  垂直寻址模式中使用
void Set_Start_Line(unsigned char d); //显示起始行对应显存的地址
void Set_Contrast_Control(unsigned char d); //设置对比度 0x00~0xff（最亮）
void Set_Segment_Remap(unsigned char d);    //列地址重映射
void Set_Entire_Display(unsigned char d); //设置显存显示或者显示强制全亮
void Set_Inverse_Display(unsigned char d); //设置反向显示，正常RAM 1表示像素亮
void Set_Multiplex_Ratio(unsigned char d); //设置占空比
void Set_Display_On_Off(unsigned char d); //设置开关显示，关后OLED进入睡眠模式
void Set_Start_Page(unsigned char d);     //设置页模式下的页开始地址
void Set_Common_Remap(unsigned char d);   //设置行重映射
void Set_Display_Offset(unsigned char d); //设置行偏移量 默认0
void Set_Display_Clock(unsigned char d);  //设置显示时钟
void Set_Precharge_Period(unsigned char d); //设置预充电时间
void Set_Common_Config(unsigned char d);    //行与com pin的硬件连接
void Set_VCOMH(unsigned char d);            //设置VCOMH稳压器电压
void Set_Command_Lock(unsigned char d); //设置命令锁 0x16不响应指令 解锁为 0x12

/*IO口模拟SPI接口通讯函数 www.lcdsoc.com 2个*/
void Write_Command(unsigned char Data); //模拟SPI接口向OLED写命令
void Write_Data(unsigned char Data);    //模拟SPI接口向OLED写数据

/******************OLED显示应用函数集*www.lcdsoc.com***********************/
void Set_Pos(unsigned char x, unsigned char y);   //设置显示位置
void Set_Pixel(unsigned char x, unsigned char y); //置一个点

#endif

void Asc6_8(unsigned char x, unsigned char y,
            char ch[]); //写入一组标准ASCII字符串 6*8
//void Asc8_16(unsigned char x, unsigned char y,
//             char ch[]); //写入一组标准ASCII字符串 8*16
//void Show_Line(unsigned char a, unsigned char b, unsigned char c,
//               unsigned char Data); // 在one page 范围内画横线
//void Draw_Rectangle(unsigned char p1, unsigned char p2, unsigned char x1,
//                    unsigned char x2); //  画矩形框
//void HZ16_16(unsigned char x, unsigned char y,
//             unsigned char num); //显示16*16点阵汉字
//void Show_HZ16_16(unsigned char x, unsigned char y, unsigned char num1,
//                  unsigned char num2); //写入一串16*16汉字
//void HZ12_16(unsigned char x, unsigned char y,
//             unsigned char num); // 显示12*16点阵汉字
//void Show_HZ12_16(unsigned char x, unsigned char y, unsigned char num1,
//                  unsigned char num2); // 写入一串12*16汉字
void Fill_RAM(unsigned char Data);     //全屏填充
//void Fill_Block(unsigned char Data, unsigned char a, unsigned char b,
//                unsigned char c, unsigned char d); //部分或者全屏填充
//void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b,
//                  unsigned char c, unsigned char d); // 部分或者全屏显示图片
//void Vertical_Scroll(unsigned char a, unsigned char b,
//                     unsigned char c); //垂直滚动
//void Horizontal_Scroll(unsigned char DIRECTION, unsigned char p1,
//                       unsigned char p2, unsigned char t, unsigned char c1,
//                       unsigned char c2); //水平滚动
//void Deactivate_Scroll(void); //  停止水平或者垂直滚动，停止RAM数据必须重写
//void Fade_In(void);          //淡入
//void Fade_Out(void);         //淡出
//void Sleep(unsigned char a); //睡眠模式
//void ShowSymbol(const unsigned char *DP, uint8_t page, uint8_t column,
//                int kind); // 16高字符

void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Write_Dat(uint8_t dat);

void ShowStr(uint8_t page, uint8_t column, uint8_t size, void* str);
void ShowStrAndParamBase(uint8_t page, uint8_t column, uint8_t size, const char* strs, ...);
#define ShowStrAndParam(page, column, size,  str, value) \
  ShowStrAndParamBase(page, column, size,  str, (int16_t)(value))
#define ShowStrAnd2Param(page, column, size,  str, value, value2) \
  ShowStrAndParamBase(page, column, size,  str, (int16_t)(value), (int16_t)(value2))
#define ShowStrAnd3Param(page, column, size,  str, value, value2,value3) \
  ShowStrAndParamBase(page, column, size,  str, (int16_t)(value), (int16_t)(value2),(int16_t)(value3))
#define ShowStrAnd4Param(page, column, size,  str, value, value2,value3,value4) \
  ShowStrAndParamBase(page, column, size,  str, (int16_t)(value), (int16_t)(value2),(int16_t)(value3),(int16_t)(value4))
#define ShowStrAndFloat(page,column,size,str,value) \
  ShowStrAndParamBase(page,column,size,str,(float)(value))
void ShowBoardTimeInfo(uint16_t year, uint8_t month, uint8_t day, char* time);
void ShowStrAndParamBaseReverse(uint8_t page, uint8_t column, uint8_t size, const char* fmt, ...);
void OLED_LOG(const char* fmt, ...);
void OLED_SetBrightness(uint8_t brightness);
//void OLED_Set_Pos(uint8_t x, uint8_t y);
//void OLED_Display_On(void);
//void OLED_Display_Off(void);
//void OLED_Clear(uint8_t x);
//void OLED_On(void);

#endif
