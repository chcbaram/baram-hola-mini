#pragma once


#define KBD_NAME                    "BARAM-HOLA-MINI"

#define USB_VID                     0x0483
#define USB_PID                     0x5230

#define KBD_NAME                    "BARAM-HOLA-MINI"


// hw_def.h
//
#define _USE_HW_WS2812
#define     HW_WS2812_MAX_CH        3
#define     HW_WS2812_RGB           0
#define     HW_WS2812_RGB_CNT       3


// eeprom
//
#define EEPROM_CHIP_ZD24C128 
#define EECONFIG_USER_DATA_SIZE     512               
#define TOTAL_EEPROM_BYTE_COUNT     4096

#define DYNAMIC_KEYMAP_LAYER_COUNT  8

#define MATRIX_ROWS                 4
#define MATRIX_COLS                 12

#define DEBOUNCE                    20


// #define DEBUG_KEY_SEND
#define DEBUG_MATRIX_SCAN_RATE
#define GRAVE_ESC_ENABLE
#define KILL_SWITCH_ENABLE
#define KKUK_ENABLE


// RGB LIGHT : set(RGBLIGHT_ENABLE true)
// 
#define EEPROM_ENABLE
#define RGBLIGHT_SLEEP
#define RGBLIGHT_DEFAULT_ON         false
#define RGBLIGHT_DEFAULT_HUE        0
#define RGBLIGHT_DEFAULT_SAT        0
#define RGBLIGHT_DEFAULT_VAL        128
#define RGBLIGHT_LED_COUNT          HW_WS2812_RGB_CNT
#define RGBLIGHT_LIMIT_VAL          255
#define RGBLIGHT_SAT_STEP           8
#define RGBLIGHT_VAL_STEP           8
#define RGBLIGHT_EFFECT_RGB_TEST
#define RGBLIGHT_EFFECT_BREATHING
#define RGBLIGHT_EFFECT_SNAKE
#define RGBLIGHT_EFFECT_STATIC_GRADIENT