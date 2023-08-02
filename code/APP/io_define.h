#ifndef __IO_DEFINE_H_
#define __IO_DEFINE_H_
#include "hc32_ddl.h"

#define PORT_KEY_OK PortH
#define PIN__KEY_OK Pin02

#define PORT_BAT_CHG PortC
#define PIN__BAT_CHG Pin13

#define PORT_PWR_CTRL PortC
#define PIN__PWR_CTRL Pin14

#define PORT_BUZZER PortC
#define PIN__BUZZER Pin15


#define PORT_BAT_AIN0 PortA
#define PIN__BAT_AIN0 Pin00

#define PORT_KEY_INC PortA
#define PIN__KEY_INC Pin01

#define PORT_AIN2_IV PortA
#define PIN__AIN2_IV Pin02

#define PORT_AIN4_VV PortA
#define PIN__AIN4_VV Pin03

#define PORT_AIN5_NTC PortA
#define PIN__AIN5_NTC Pin03

#define CH422G_IIC M4_I2C1

#define PORT_I2C1_SCL_CH422G PortA
#define PIN__I2C1_SCL_CH422G Pin06

#define PORT_I2C1_SDA_CH422G PortA
#define PIN__I2C1_SDA_CH422G Pin07

#define PORT_SDIO2_CMD PortB
#define PIN__SDIO2_CMD Pin00

#define PORT_SDIO2_D3 PortB
#define PIN__SDIO2_D3 Pin01

#define PORT_SDIO2_D2 PortB
#define PIN__SDIO2_D2 Pin02

#define PORT_KEY_DEC PortB
#define PIN__KEY_DEC Pin10

#define PORT_SINE_GENER PortB
#define PIN__SINE_GENER Pin12

#define PORT_CAN_RX PortB
#define PIN__CAN_RX Pin13

#define PORT_CAN_TX PortB
#define PIN__CAN_TX Pin14

#define PORT_JTAG_TCK_SWCLK PortB
#define PIN__JTAG_TCK_SWCLK Pin15

#define PORT_JTAG_TMS_SWDIO PortA
#define PIN__JTAG_TMS_SWDIO Pin08

#define PORT_USB_VBUS PortA
#define PIN__USB_VBUS Pin09

#define PORT_PWM_GENER PortA
#define PIN__PWM_GENER Pin10

#define PORT_USB_DM PortA
#define PIN__USB_DM Pin11

#define PORT_USB_DP PortA
#define PIN__USB_DP Pin12

#define PORT_SDIO2_D1 PortA
#define PIN__SDIO2_D1 Pin15

#define PORT_SDIO2_D0 PortB
#define PIN__SDIO2_D0 Pin03

#define PORT_UART3_DE PortB
#define PIN__UART3_DE Pin04

#define PORT_UART3_TX PortB
#define PIN__UART3_TX Pin05

#define PORT_SDIO2_CK PortB
#define PIN__SDIO2_CK Pin06

#define PORT_UART3_RX PortB
#define PIN__UART3_RX Pin07


#define OLED_IIC M4_I2C3

#define PORT_OLED_I2C3_SDA PortB
#define PIN__OLED_I2C3_SDA Pin08

#define PORT_OLED_I2C3_SCL PortB
#define PIN__OLED_I2C3_SCL Pin09

#endif
