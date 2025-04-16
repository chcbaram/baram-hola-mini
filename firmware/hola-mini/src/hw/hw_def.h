#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V250416R1"
#define _DEF_BOARD_NAME           "HOLA-MINI"




#define _USE_HW_EEPROM
#define _USE_HW_USB_CDC

#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         2
#define      HW_UART_CH_USB         _DEF_UART1
#define      HW_UART_CH_DEBUG       _DEF_UART2
#define      HW_UART_CH_CLI         HW_UART_CH_USB

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_DEBUG
#define      HW_LOG_BOOT_BUF_MAX    1024
#define      HW_LOG_LIST_BUF_MAX    1024

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1

#define _USE_HW_WS2812
#define      HW_WS2812_MAX_CH       3


//-- CLI
//
#define _USE_CLI_HW_EEPROM          1
#define _USE_CLI_HW_I2C             1
#define _USE_CLI_HW_KEYS            1
#define _USE_CLI_HW_RESET           1


#endif