#include "quantum.h"


void ws2812_setleds(rgb_led_t *ledarray, uint16_t leds)
{
  uint8_t r, g, b;


  for (int i=0; i<leds; i++)
  {
    r = ledarray[i].r;
    g = ledarray[i].g;
    b = ledarray[i].b;
    ws2812SetColor(HW_WS2812_RGB + i, WS2812_COLOR(g, r, b));
  }
  ws2812Refresh();
}


const rgblight_driver_t rgblight_driver = {
  .setleds = ws2812_setleds,
};

