#include "usbd_hid.h"



#define USB_HID_LOG                 0


#if USB_HID_LOG == 1
#define logDebug(...)                              \
  {                                                \
    logPrintf(__VA_ARGS__);                        \
  }
#else
#define logDebug(...) 
#endif

#include "cli.h"
#include "log.h"
#include "keys.h"
#include "qbuffer.h"



#include "tusb.h"
#include "class/hid/hid_device.h"
#include "class/cdc/cdc_device.h"
#include "bsp/board_api.h"

enum
{
  ITF_ID_KEYBOARD = 0,
  ITF_ID_VIA,
  ITF_ID_COUNT
};

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_COUNT
};

enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#define HID_KEYBOARD_REPORT_SIZE (HW_KEYS_PRESS_MAX + 2U)

typedef struct
{
  uint8_t  buf[HID_KEYBOARD_REPORT_SIZE];
} report_info_t;


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,

  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0100,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,

  .bNumConfigurations = 0x01
};

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
static const uint8_t hid_keyboard_descriptor[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
};

static const uint8_t hid_via_descriptor[HID_KEYBOARD_VIA_REPORT_DESC_SIZE] = 
{
  //
  0x06, 0x60, 0xFF, // Usage Page (Vendor Defined)
  0x09, 0x61,       // Usage (Vendor Defined)
  0xA1, 0x01,       // Collection (Application)
  // Data to host
  0x09, 0x62,       //   Usage (Vendor Defined)
  0x15, 0x00,       //   Logical Minimum (0)
  0x26, 0xFF, 0x00, //   Logical Maximum (255)
  0x95, 32,         //   Report Count
  0x75, 0x08,       //   Report Size (8)
  0x81, 0x02,       //   Input (Data, Variable, Absolute)
  // Data from host
  0x09, 0x63,       //   Usage (Vendor Defined)
  0x15, 0x00,       //   Logical Minimum (0)
  0x26, 0xFF, 0x00, //   Logical Maximum (255)
  0x95, 32,         //   Report Count
  0x75, 0x08,       //   Report Size (8)
  0x91, 0x02,       //   Output (Data, Variable, Absolute)
  0xC0              // End Collection
};


const char *hid_string_descriptor[] =
{
  (char[]){0x09, 0x04},     // 0: is supported language is English (0x0409)
  "BARAM",                  // 1: Manufacturer
  KBD_NAME,                 // 2: Product
  "123456",                 // 3: Serials, should use chip ID
  "HID Keyboard",           // 4: HID
};

static const uint8_t hid_configuration_descriptor[] = {

  TUD_CONFIG_DESCRIPTOR(1,  // Configuration number,
                        ITF_ID_COUNT,                       // interface count
                        0,                                  // string index
                        TUSB_DESC_TOTAL_LEN,                // total length
                        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, // attribute
                        500                                 // power in mA
                        ),

  TUD_HID_DESCRIPTOR(ITF_ID_KEYBOARD,                       // Interface number
                     0,                                     // string index
                     HID_ITF_PROTOCOL_KEYBOARD,             // boot protocol
                     sizeof(hid_keyboard_descriptor),       // report descriptor len
                     HID_EPIN_ADDR,                         // EP In address
                     HID_EPIN_SIZE,                         // size
                     1                                      // polling inerval
                     ),

  TUD_HID_INOUT_DESCRIPTOR(ITF_ID_VIA,                      // Interface number
                           0,                               // string index
                           HID_ITF_PROTOCOL_NONE,           // protocol
                           sizeof(hid_via_descriptor),      // report descriptor len
                           HID_VIA_EP_OUT,                  // EP Out Address
                           HID_VIA_EP_IN,                   // EP In Address
                           64,                              // size
                           10                               // polling interval
                           )
};

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
{
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = 0x0200,

  .bDeviceClass       = TUSB_CLASS_MISC,
  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

static void (*via_hid_receive_func)(uint8_t *data, uint8_t length) = NULL;
static uint8_t via_hid_usb_report[32];

static qbuffer_t     report_q;
static report_info_t report_buf[128];


#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif





bool usbHidInit(void)
{
  bool ret = true;

  qbufferCreateBySize(&report_q, (uint8_t *)report_buf, sizeof(report_info_t), 128); 


  tud_init(BOARD_TUD_RHPORT);

#ifdef _USE_HW_CLI
  cliAdd("usbhid", cliCmd);
#endif  
  return ret;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
  uint8_t const *p_ret;

  logPrintf("tud_hid_descriptor_report_cb(%d)\n", itf);

  switch(itf)
  {
    case ITF_ID_VIA:
      p_ret = hid_via_descriptor;
      break;

    default:
      p_ret = hid_keyboard_descriptor;
      break;
  }
  
  return p_ret;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  logDebug("tud_hid_get_report_cb()\n");
  logDebug("  itf         : %d\n", itf);
  logDebug("  report_id   : %d\n", report_id);
  logDebug("  report_type : %d\n", (int)report_type);
  logDebug("  reqlen      : %d\n", (int)reqlen);

  return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void) itf;
  (void) report_id;
  (void) report_type;

  logDebug("tud_hid_set_report_cb()\n");
  logDebug("  itf         : %d\n", itf);
  logDebug("  report_id   : %d\n", report_id);
  logDebug("  report_type : %d\n", (int)report_type);
  logDebug("  reqlen      : %d\n", (int)bufsize);

  switch(itf)
  {
    case ITF_ID_KEYBOARD:
      break;

    case ITF_ID_VIA:
      memcpy(via_hid_usb_report, buffer, HID_VIA_EP_SIZE);
      if (via_hid_receive_func != NULL)
      {
        via_hid_receive_func(via_hid_usb_report, HID_VIA_EP_SIZE);
      }
      tud_hid_n_report(itf, report_id, via_hid_usb_report, HID_VIA_EP_SIZE);
      break;
  }
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const* report, uint16_t len)
{
  (void) itf;
  (void) report;
  (void) len;

  // logPrintf("tud_hid_report_complete_cb()\n");
  // logPrintf("  itf         : %d\n", itf);
  // logPrintf("  len         : %d\n", (int)len);                              
}                              

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
  return (uint8_t const*) &desc_device_qualifier;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  return hid_configuration_descriptor;
}

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;
  size_t chr_count;

  switch (index)
  {
  case STRID_LANGID:
    memcpy(&_desc_str[1], hid_string_descriptor[0], 2);
    chr_count = 1;
    break;

  case STRID_SERIAL:
    chr_count = board_usb_get_serial(_desc_str + 1, 32);
    break;

  default:
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0])))
      return NULL;

    const char *str = hid_string_descriptor[index];

    // Cap at max char
    chr_count = strlen(str);
    size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
    if (chr_count > max_count)
      chr_count = max_count;

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < chr_count; i++)
    {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}

bool usbHidSetViaReceiveFunc(void (*func)(uint8_t *, uint8_t))
{
  via_hid_receive_func = func;
  return true;
}

bool usbHidSendReport(uint8_t *p_data, uint16_t length)
{
  bool ret = true;

  if (!tud_suspended())
  {
    report_info_t report_info;

    memcpy(report_info.buf, p_data, HID_KEYBOARD_REPORT_SIZE);
    qbufferWrite(&report_q, (uint8_t *)&report_info, 1);   
  }
  else
  {
    tud_remote_wakeup();
    ret = false;
  }

  return ret;
}

bool usbHidSendReportEXK(uint8_t *p_data, uint16_t length)
{
  // exk_report_info_t report_info;

  // if (length > HID_EXK_EP_SIZE)
  //   return false;

  // if (!USBD_is_suspended())
  // {
  //   memcpy(hid_buf_exk, p_data, length);
  //   if (!USBD_HID_SendReportEXK((uint8_t *)hid_buf_exk, length))
  //   {
  //     report_info.len = length;
  //     memcpy(report_info.buf, p_data, length);
  //     qbufferWrite(&report_exk_q, (uint8_t *)&report_info, 1);        
  //   }    
  // }
  // else
  // {
  //   usbHidUpdateWakeUp(&USBD_Device);
  // }
  
  return true;
}

bool usbHidUpdate(void)
{
  bool ret;
  report_info_t report_info;

  if (tud_hid_n_ready(ITF_ID_KEYBOARD))
  {
    if (qbufferAvailable(&report_q) > 0)
    {
      qbufferRead(&report_q, (uint8_t *)&report_info, 1);
      ret = tud_hid_n_report(ITF_ID_KEYBOARD, REPORT_ID_KEYBOARD, report_info.buf, sizeof(report_info.buf));
      if (!ret)
      {
        logPrintf("usbHidSendReport() Fail\n");
      }
    }
  }

  tud_task(); 

  return true;
}

#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("usbhid info\n");
  }
}
#endif
