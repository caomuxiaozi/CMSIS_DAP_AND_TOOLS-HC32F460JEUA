#include "driver.h"
#include "math.h"
#include "softiic.h"
////////////////////////////////

void IIC_SDA_I() {
}
void IIC_SDA_O() {
}
void IIC_SCL(uint8_t x) {
  if(x)
    M4_PORT->POSRB |= (1u << 12u); //HAL_GPIO_WritePin(PORT_I2C3_SCL, PIN__I2C3_SCL, GPIO_PIN_SET);
  else
    M4_PORT->PORRB |= (1u << 12u); //HAL_GPIO_WritePin(PORT_I2C3_SCL, PIN__I2C3_SCL, GPIO_PIN_RESET);
}
void IIC_SDA(uint8_t x) {
  if(x)
    M4_PORT->POSRB |= (1u << 10u); //HAL_GPIO_WritePin(PORT_I2C3_SDA, PIN__I2C3_SDA, GPIO_PIN_SET);
  else
    M4_PORT->PORRB |= (1u << 10u); //HAL_GPIO_WritePin(PORT_I2C3_SDA, PIN__I2C3_SDA, GPIO_PIN_RESET);
}
uint8_t READ_SDA() {
  //   PBin(7)  //ÊäÈëSDA
  return (M4_PORT->PIDRB & (1 << 10)); //HAL_GPIO_ReadPin(PORT_I2C3_SDA, PIN__I2C3_SDA);
}

void Delay_10us(void) {
  unsigned char n;
  for(n = 0; n < 5; n++) {
    __NOP();
  }
}
//================================================
void IIC_Start(void) {
  IIC_SDA(1);
  Delay_10us();
  IIC_SCL(1);
  Delay_10us();
  IIC_SDA(0);
  Delay_10us();
  IIC_SCL(0);
  Delay_10us();
}
//================================================
void IIC_Stop(void) {
  IIC_SCL(0);
  Delay_10us();
  IIC_SDA(0);
  Delay_10us();
  IIC_SCL(1);
  Delay_10us();
  IIC_SDA(1);
  Delay_10us();
}
//================================================
void IIC_ACK(void) {
  IIC_SDA(0);
  Delay_10us();
  IIC_SCL(1);
  Delay_10us();
  IIC_SCL(0);
  Delay_10us();
}
//================================================
void IIC_NoAck(void) {
  IIC_SDA(1);
  Delay_10us();
  IIC_SCL(1);
  Delay_10us();
  IIC_SCL(0);
  Delay_10us();
}
//================================================
uint8_t IIC_ReadByte(void) {
  uint8_t ucValue;
  uint8_t ucIndex;
  // IIC_SDA(1);
  // Delay_10us();
  IIC_SDA_I();
  // Delay_10us();
  ucValue = 0;
  for(ucIndex = 0; ucIndex < 8; ucIndex++) {
    ucValue <<= 1;
    IIC_SCL(0);
    Delay_10us();
    IIC_SCL(1);
    Delay_10us();
    if(READ_SDA()) {
      ucValue |= 1;
    }
    Delay_10us();
    IIC_SCL(0);
    Delay_10us();
  }
  IIC_SDA_O();
  return ucValue;
}
//================================================
void IIC_WriteByte(uint8_t ucData) {
  uint8_t i;
  for(i = 0; i < 8; i++) {
    IIC_SCL(0);
    Delay_10us();
    if((ucData & 0x80) == 0x80) {
      IIC_SDA(1);
    } else {
      IIC_SDA(0);
      Delay_10us();
    }
    Delay_10us();
    IIC_SCL(1);
    Delay_10us();
    ucData <<= 1;
    IIC_SCL(0);
    Delay_10us();
  }
  IIC_SDA_I();
  Delay_10us();
  IIC_SCL(0);
  Delay_10us();
  IIC_SCL(1);
  Delay_10us();
  IIC_SCL(0);
  Delay_10us();
  IIC_SDA_O();
}
void     SOFT_IIC_WriteCmd(uint8_t DEV_ID, uint8_t uCmd) {
  IIC_Start();
  IIC_WriteByte(DEV_ID);
  IIC_WriteByte(uCmd);
  IIC_Stop();
}

