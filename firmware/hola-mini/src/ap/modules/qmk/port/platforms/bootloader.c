#include "bootloader.h"
#include "eeprom.h"

void bootloader_jump(void)
{
  reset_usb_boot(0,0);
}

void mcu_reset(void)
{
  for (int i=0; i<32; i++)
  {
    eeprom_update();
  }
  watchdog_reboot(0, 0, 0);
}