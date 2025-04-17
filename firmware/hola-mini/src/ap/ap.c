#include "ap.h"


static void apMain2(void);




void apInit(void)
{
  cliOpen(HW_UART_CH_CLI, 115200);

  multicore_launch_core1(apMain2);
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);      
    } 
    cliMain();     
  }
}

void apMain2(void)
{
  while(1)
  {
    keysUpdate();
    usbHidUpdate();
    delay(1);
  }
}

