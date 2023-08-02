#ifndef __softiic_H_
#define __softiic_H_
#include "stdint.h"

/* Ä£ÄâI2CÐÅºÅ */
void    IIC_Start(void);
void    IIC_Stop(void);
uint8_t IIC_Wait_Ack(void);
void    IIC_Ack(void);
void    IIC_NAck(void);
void    IIC_Send_Byte(uint8_t txd);
uint8_t IIC_Read_Byte(void);
void IIC_WriteByte(uint8_t ucData);
void     SOFT_IIC_WriteCmd(uint8_t DEV_ID, uint8_t uCmd);
#endif
