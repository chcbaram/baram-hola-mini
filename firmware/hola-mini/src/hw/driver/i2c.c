#include "i2c.h"





#ifdef _USE_HW_I2C
#include "cli.h"
#include "hardware/i2c.h"


#ifdef _USE_HW_CLI
static void cliI2C(cli_args_t *args);
#endif



static uint32_t i2c_timeout[I2C_MAX_CH];
static uint32_t i2c_errcount[I2C_MAX_CH];
static uint32_t i2c_freq[I2C_MAX_CH];

static bool is_init = false;
static bool is_begin[I2C_MAX_CH];




typedef struct
{
  i2c_inst_t    *h_i2c;

  int           scl_pin;
  int           sda_pin;
} i2c_tbl_t;

static i2c_tbl_t i2c_tbl[I2C_MAX_CH] =
    {
        {i2c0, 5,  4},
    };


static void delayUs(uint32_t us);



bool i2cInit(void)
{
  uint32_t i;


  for (i=0; i<I2C_MAX_CH; i++)
  {
    i2c_timeout[i] = 10;
    i2c_errcount[i] = 0;
    is_begin[i] = false;
  }

#ifdef _USE_HW_CLI
  cliAdd("i2c", cliI2C);
#endif

  is_init = true;
  return true;
}

bool i2cIsInit(void)
{
  return is_init;
}

bool i2cReservedAddr(uint8_t addr) 
{
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

bool i2cBegin(uint8_t ch, uint32_t freq_khz)
{
  bool ret = false;


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }


  switch(ch)
  {
    case _DEF_I2C1:
      i2c_freq[ch] = freq_khz;

      i2cReset(ch);

      i2c_init(i2c_tbl[ch].h_i2c, freq_khz * 1000);
      
      gpio_set_function(i2c_tbl[ch].scl_pin, GPIO_FUNC_I2C);
      gpio_set_function(i2c_tbl[ch].sda_pin, GPIO_FUNC_I2C);
      gpio_pull_up(i2c_tbl[ch].scl_pin);
      gpio_pull_up(i2c_tbl[ch].sda_pin);

      ret = true;
      is_begin[ch] = true;
      break;
  }

  return ret;
}

bool i2cIsBegin(uint8_t ch)
{
  return is_begin[ch];
}

void i2cReset(uint8_t ch)
{
  i2c_tbl_t *p_pin = &i2c_tbl[ch];


  gpio_init(p_pin->scl_pin);
  gpio_set_dir(p_pin->scl_pin, GPIO_OUT);

  gpio_init(p_pin->sda_pin);
  gpio_set_dir(p_pin->sda_pin, GPIO_OUT);


  gpio_put(p_pin->scl_pin, 1);
  gpio_put(p_pin->sda_pin, 1);
  delayUs(5);

  for (int i = 0; i < 9; i++)
  {
    gpio_put(p_pin->scl_pin, 0);
    delayUs(5);
    gpio_put(p_pin->scl_pin, 1);
    delayUs(5);
  }

  gpio_put(p_pin->scl_pin, 0);
  delayUs(5);
  gpio_put(p_pin->sda_pin, 0);
  delayUs(5);

  gpio_put(p_pin->scl_pin, 1);
  delayUs(5);
  gpio_put(p_pin->sda_pin, 1);
}

bool i2cIsDeviceReady(uint8_t ch, uint8_t dev_addr)
{
  uint8_t rx_data;

  if (i2cReservedAddr(dev_addr))
  {
    return false;
  }

  if (i2c_read_blocking(i2c_tbl[ch].h_i2c, dev_addr, &rx_data, 1, false) > 0)
  {
    return true;
  }

  return false;
}

bool i2cRecovery(uint8_t ch)
{
  bool ret;

  i2cReset(ch);

  ret = i2cBegin(ch, i2c_freq[ch]);

  return ret;
}

bool i2cReadByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
  return i2cReadBytes(ch, dev_addr, reg_addr, p_data, 1, timeout);
}

bool i2cReadBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_write_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, (const uint8_t *)&reg_addr, 1, true, timeout*1000);
  i2c_ret = i2c_read_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, p_data, length, false, timeout*1000);
  if (i2c_ret == length)
  {
    ret = true;
  }

  return ret;
}

bool i2cRead16Bytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;
  uint8_t addr_buf[2];


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  addr_buf[0] = (reg_addr >> 8) & 0xFF;
  addr_buf[1] = (reg_addr >> 0) & 0xFF; 
  i2c_write_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, (const uint8_t *)addr_buf, 2, true, timeout*1000);
  i2c_ret = i2c_read_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, p_data, length, false, timeout*1000);
  if (i2c_ret == length)
  {
    ret = true;
  }

  return ret;
}

bool i2cWrite16Bytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;
  uint8_t tx_buf[length+2];


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  tx_buf[0] = (reg_addr >> 8) & 0xFF;
  tx_buf[1] = (reg_addr >> 0) & 0xFF;
  memcpy(&tx_buf[2], p_data, length);

  i2c_ret = i2c_write_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, tx_buf, length+2, false, timeout*1000); 
  if (i2c_ret > 0)
  {
    ret = true;
  }

  return ret;
}


bool i2cReadData(uint8_t ch, uint16_t dev_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = i2c_read_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, p_data, length, false, timeout*1000);
  if (i2c_ret == length)
  {
    ret = true;
  }

  return ret;
}

bool i2cWriteByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
  return i2cWriteBytes(ch, dev_addr, reg_addr, &data, 1, timeout);
}

bool i2cWriteBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;
  uint8_t tx_buf[length+1];


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  tx_buf[0] = reg_addr;
  for (int i=0; i<length; i++)
  {
    tx_buf[1+i] = p_data[i];
  }
  i2c_ret = i2c_write_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, tx_buf, length+1, false, timeout*1000); 
  if (i2c_ret > 0)
  {
    ret = true;
  }

  return ret;
}

bool i2cWriteData(uint8_t ch, uint16_t dev_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
  int i2c_ret;


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }


  i2c_ret = i2c_write_timeout_us(i2c_tbl[ch].h_i2c, dev_addr, p_data, length, false, timeout*1000); 
  if (i2c_ret == length)
  {
    ret = true;
  }

  return ret;
}

void i2cSetTimeout(uint8_t ch, uint32_t timeout)
{
  i2c_timeout[ch] = timeout;
}

uint32_t i2cGetTimeout(uint8_t ch)
{
  return i2c_timeout[ch];
}

void i2cClearErrCount(uint8_t ch)
{
  i2c_errcount[ch] = 0;
}

uint32_t i2cGetErrCount(uint8_t ch)
{
  return i2c_errcount[ch];
}

void delayUs(uint32_t us)
{
  volatile uint32_t i;

  for (i=0; i<us*1000; i++)
  {

  }
}


#ifdef _USE_HW_CLI
void cliI2C(cli_args_t *args)
{
  bool ret = false;
  bool i2c_ret;

  uint8_t print_ch;
  uint8_t ch;
  uint16_t dev_addr;
  uint16_t reg_addr;
  uint16_t length;

  uint32_t i;
  uint8_t i2c_data[128];
  uint32_t pre_time;


  if (args->argc == 2 && args->isStr(0, "scan") == true)
  {
    print_ch = (uint16_t) args->getData(1);

    print_ch = constrain(print_ch, 1, I2C_MAX_CH);
    print_ch -= 1;

    for (i=0x00; i<= 0x7F; i++)
    {
      if (i2cIsDeviceReady(print_ch, i) == true)
      {
        cliPrintf("I2C CH%d Addr 0x%X : OK\n", print_ch+1, i);
      }
    }
    ret = true;  
  }

  if (args->argc == 2 && args->isStr(0, "begin") == true)
  {
    print_ch = (uint16_t) args->getData(1);

    print_ch = constrain(print_ch, 1, I2C_MAX_CH);
    print_ch -= 1;

    i2c_ret = i2cBegin(print_ch, 400);
    if (i2c_ret == true)
    {
      cliPrintf("I2C CH%d Begin OK\n", print_ch + 1);
    }
    else
    {
      cliPrintf("I2C CH%d Begin Fail\n", print_ch + 1);
    }
    ret = true;
  }

  if (args->argc == 5 && args->isStr(0, "read") == true)
  {
    print_ch = (uint16_t) args->getData(1);
    print_ch = constrain(print_ch, 1, I2C_MAX_CH);

    dev_addr = (uint16_t) args->getData(2);
    reg_addr = (uint16_t) args->getData(3);
    length   = (uint16_t) args->getData(4);
    ch       = print_ch - 1;

    for (i=0; i<length; i++)
    {
      i2c_ret = i2cReadByte(ch, dev_addr, reg_addr+i, i2c_data, 100);

      if (i2c_ret == true)
      {
        cliPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, reg_addr+i, i2c_data[0]);
      }
      else
      {
        cliPrintf("%d I2C - Fail \n", print_ch);
        break;
      }
    }
    ret = true;
  }

  if (args->argc == 5 && args->isStr(0, "write") == true)
  {
    print_ch = (uint16_t) args->getData(1);
    print_ch = constrain(print_ch, 1, I2C_MAX_CH);

    dev_addr = (uint16_t) args->getData(2);
    reg_addr = (uint16_t) args->getData(3);
    length   = (uint16_t) args->getData(4);
    ch       = print_ch - 1;

    pre_time = millis();
    i2c_ret = i2cWriteByte(ch, dev_addr, reg_addr, (uint8_t)length, 100);

    if (i2c_ret == true)
    {
      cliPrintf("%d I2C - 0x%02X : 0x%02X, %d ms\n", print_ch, reg_addr, length, millis()-pre_time);
    }
    else
    {
      cliPrintf("%d I2C - Fail \n", print_ch);
    }
    ret = true;
  }

  if (args->argc == 4 && args->isStr(0, "read_d") == true)
  {
    print_ch = (uint16_t) args->getData(1);
    print_ch = constrain(print_ch, 1, I2C_MAX_CH);

    dev_addr = (uint16_t) args->getData(2);
    length   = (uint16_t) args->getData(3);
    ch       = print_ch - 1;

    length   = constrain(length, 0, 256);  
    

    uint8_t buf[length];

    i2c_ret = i2cReadData(ch, dev_addr, buf, length, 100);
    if (i2c_ret == true)
    {
      for (int i=0; i<length; i++)
      {
        cliPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, i, buf[i]);
      }
    }
    else
    {
      cliPrintf("%d I2C - Fail \n", print_ch);
    }
    ret = true;
  }


  if (ret == false)
  {
    cliPrintf( "i2c begin channel[1~%d]\n", I2C_MAX_CH);
    cliPrintf( "i2c scan channel[1~%d]\n", I2C_MAX_CH);
    cliPrintf( "i2c read channel dev_addr reg_addr length\n");
    cliPrintf( "i2c write channel dev_addr reg_addr data\n");
    cliPrintf( "i2c read_d channel dev_addr length\n");
    cliPrintf( "i2c write_d channel dev_addr data\n");
  }
}

#endif

#endif