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
#error "ûѡ����ɶ��Ļ,��Ҫ�� modeCtrl.h ���涨��1.3��С����#define OLED_1p3INCH������2.4�������#define OLED_2p4INCH��"
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
//����������
#define RIGHT 0x26
#define LEFT 0x27
#define chineseText 1
#define asciiText 0
//-----------------OLED�˿ڶ��� 4��SPI----------------
/* ����IO��ģ��SPIͨѶ��ʽ����oledģ��
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

#define OLED_CMD 0  //д����
#define OLED_DATA 1 //д����

//void delay_ms(uint32_t num);
//void delay_us(uint32_t num);
// OLED��ʼ������ www.lcdsoc.com
void OLED_Init(void);

// OLED�������ú�����
#ifdef  OLED_2p4INCH
void Set_Start_Column(unsigned char d);    //����ҳģʽ��ʼ��
void Set_Addressing_Mode(unsigned char d); //�����Դ�Ѱַģʽ
void Set_Column_Address(unsigned char a,
                        unsigned char b); //�����е�ַ   ˮƽѰַģʽ��ʹ��
void Set_Page_Address(unsigned char a,
                      unsigned char b); //����ҳ��ַ  ��ֱѰַģʽ��ʹ��
void Set_Start_Line(unsigned char d); //��ʾ��ʼ�ж�Ӧ�Դ�ĵ�ַ
void Set_Contrast_Control(unsigned char d); //���öԱȶ� 0x00~0xff��������
void Set_Segment_Remap(unsigned char d);    //�е�ַ��ӳ��
void Set_Entire_Display(unsigned char d); //�����Դ���ʾ������ʾǿ��ȫ��
void Set_Inverse_Display(unsigned char d); //���÷�����ʾ������RAM 1��ʾ������
void Set_Multiplex_Ratio(unsigned char d); //����ռ�ձ�
void Set_Display_On_Off(unsigned char d); //���ÿ�����ʾ���غ�OLED����˯��ģʽ
void Set_Start_Page(unsigned char d);     //����ҳģʽ�µ�ҳ��ʼ��ַ
void Set_Common_Remap(unsigned char d);   //��������ӳ��
void Set_Display_Offset(unsigned char d); //������ƫ���� Ĭ��0
void Set_Display_Clock(unsigned char d);  //������ʾʱ��
void Set_Precharge_Period(unsigned char d); //����Ԥ���ʱ��
void Set_Common_Config(unsigned char d);    //����com pin��Ӳ������
void Set_VCOMH(unsigned char d);            //����VCOMH��ѹ����ѹ
void Set_Command_Lock(unsigned char d); //���������� 0x16����Ӧָ�� ����Ϊ 0x12

/*IO��ģ��SPI�ӿ�ͨѶ���� www.lcdsoc.com 2��*/
void Write_Command(unsigned char Data); //ģ��SPI�ӿ���OLEDд����
void Write_Data(unsigned char Data);    //ģ��SPI�ӿ���OLEDд����

/******************OLED��ʾӦ�ú�����*www.lcdsoc.com***********************/
void Set_Pos(unsigned char x, unsigned char y);   //������ʾλ��
void Set_Pixel(unsigned char x, unsigned char y); //��һ����

#endif

void Asc6_8(unsigned char x, unsigned char y,
            char ch[]); //д��һ���׼ASCII�ַ��� 6*8
//void Asc8_16(unsigned char x, unsigned char y,
//             char ch[]); //д��һ���׼ASCII�ַ��� 8*16
//void Show_Line(unsigned char a, unsigned char b, unsigned char c,
//               unsigned char Data); // ��one page ��Χ�ڻ�����
//void Draw_Rectangle(unsigned char p1, unsigned char p2, unsigned char x1,
//                    unsigned char x2); //  �����ο�
//void HZ16_16(unsigned char x, unsigned char y,
//             unsigned char num); //��ʾ16*16������
//void Show_HZ16_16(unsigned char x, unsigned char y, unsigned char num1,
//                  unsigned char num2); //д��һ��16*16����
//void HZ12_16(unsigned char x, unsigned char y,
//             unsigned char num); // ��ʾ12*16������
//void Show_HZ12_16(unsigned char x, unsigned char y, unsigned char num1,
//                  unsigned char num2); // д��һ��12*16����
void Fill_RAM(unsigned char Data);     //ȫ�����
//void Fill_Block(unsigned char Data, unsigned char a, unsigned char b,
//                unsigned char c, unsigned char d); //���ֻ���ȫ�����
//void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b,
//                  unsigned char c, unsigned char d); // ���ֻ���ȫ����ʾͼƬ
//void Vertical_Scroll(unsigned char a, unsigned char b,
//                     unsigned char c); //��ֱ����
//void Horizontal_Scroll(unsigned char DIRECTION, unsigned char p1,
//                       unsigned char p2, unsigned char t, unsigned char c1,
//                       unsigned char c2); //ˮƽ����
//void Deactivate_Scroll(void); //  ֹͣˮƽ���ߴ�ֱ������ֹͣRAM���ݱ�����д
//void Fade_In(void);          //����
//void Fade_Out(void);         //����
//void Sleep(unsigned char a); //˯��ģʽ
//void ShowSymbol(const unsigned char *DP, uint8_t page, uint8_t column,
//                int kind); // 16���ַ�

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
