#include "keys.h"


#ifdef _USE_HW_KEYS
#include "cli.h"



#define KEYS_ROWS   MATRIX_ROWS
#define KEYS_COLS   MATRIX_COLS



#if CLI_USE(HW_KEYS)
static void cliCmd(cli_args_t *args);
#endif
static bool keysInitGpio(void);
static void keysScan(void);


static uint16_t cols_buf[KEYS_ROWS];

static uint32_t rows_gpio_tbl[KEYS_ROWS] = {
    6,
    7,
    8,
    9,
};
static uint32_t cols_gpio_tbl[KEYS_COLS] = {
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
};

static bool is_ready = false;




bool keysInit(void)
{

  keysInitGpio();
  keysScan();

#if CLI_USE(HW_KEYS)
  cliAdd("keys", cliCmd);
#endif

  return true;
}

bool keysInitGpio(void)
{
  for (int i=0; i<KEYS_ROWS; i++)
  {
    gpio_init(rows_gpio_tbl[i]);
    gpio_set_dir(rows_gpio_tbl[i], GPIO_OUT);    
    gpio_put(rows_gpio_tbl[i], _DEF_LOW);
    gpio_disable_pulls(rows_gpio_tbl[i]);
  }

  for (int i=0; i<KEYS_COLS; i++)
  {    
    gpio_init(cols_gpio_tbl[i]);
    gpio_set_dir(cols_gpio_tbl[i], GPIO_IN);    
    gpio_pull_down(cols_gpio_tbl[i]);    
    gpio_set_input_enabled(cols_gpio_tbl[i], false);
  }

  return true;
}

bool keysIsBusy(void)
{
  return false;
}

bool keysIsReady(void)
{
  bool ret = is_ready;

  is_ready = false;

  return ret;
}

bool keysUpdate(void)
{
  keysScan();
  return true;
}

bool keysReadBuf(uint8_t *p_data, uint32_t length)
{
  return true;
}

bool keysReadColsBuf(uint16_t *p_data, uint32_t rows_cnt)
{
  memcpy(p_data, cols_buf, rows_cnt * sizeof(uint16_t));
  return true;
}

bool keysGetPressed(uint16_t row, uint16_t col)
{
  bool     ret = false;
  uint16_t col_bit;

  col_bit = cols_buf[row];

  if (col_bit & (1<<col))
  {
    ret = true;
  }

  return ret;
}

void keysScan(void)
{
  uint16_t scan_buf[KEYS_ROWS];


  memset(scan_buf, 0, sizeof(scan_buf));

  for (int rows_i = 0; rows_i < KEYS_ROWS; rows_i++)
  {
    gpio_put(rows_gpio_tbl[rows_i], _DEF_HIGH);
    make_timeout_time_us(10);        
    for (int cols_i = 0; cols_i < KEYS_COLS; cols_i++)
    {      
      gpio_set_input_enabled(cols_gpio_tbl[cols_i], true);      
      make_timeout_time_us(10);        
      if (gpio_get(cols_gpio_tbl[cols_i]) == _DEF_HIGH)
      {
        scan_buf[rows_i] |= (1<<cols_i);
      }      
      gpio_set_input_enabled(cols_gpio_tbl[cols_i], false);      
    }   
    gpio_put(rows_gpio_tbl[rows_i], _DEF_LOW);
    make_timeout_time_us(100);
  }

  memcpy(cols_buf, scan_buf, sizeof(scan_buf));

  is_ready = true;
}

#if CLI_USE(HW_KEYS)
void cliCmd(cli_args_t *args)
{
  bool ret = false;



  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliShowCursor(false);


    while(cliKeepLoop())
    {
      delay(10);

      cliPrintf("     ");
      for (int cols=0; cols<MATRIX_COLS; cols++)
      {
        cliPrintf("%02d ", cols);
      }
      cliPrintf("\n");

      for (int rows=0; rows<MATRIX_ROWS; rows++)
      {
        cliPrintf("%02d : ", rows);

        for (int cols=0; cols<MATRIX_COLS; cols++)
        {
          if (keysGetPressed(rows, cols))
            cliPrintf("O  ");
          else
            cliPrintf("_  ");
        }
        cliPrintf("\n");
      }
      cliMoveUp(MATRIX_ROWS+1);
    }
    cliMoveDown(MATRIX_ROWS+1);

    cliShowCursor(true);
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("keys info\n");
  }
}
#endif

#endif