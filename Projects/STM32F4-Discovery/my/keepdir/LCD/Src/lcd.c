/**
  ******************************************************************************
  * @file    lcd.c
  * @brief   SPI LCD driver for 240x320 modules (ILI9341 / ST7789)
  *          Target board: STM32F4-Discovery (STM32F407VGT6)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lcd.h"
#include "fonts.h"
#include "spi_bus.h"

/* Private defines -----------------------------------------------------------*/
#define LCD_CMD_SWRESET             0x01U
#define LCD_CMD_SLPOUT              0x11U
#define LCD_CMD_DISPON              0x29U
#define LCD_CMD_CASET               0x2AU
#define LCD_CMD_PASET               0x2BU
#define LCD_CMD_RAMWR               0x2CU
#define LCD_CMD_MADCTL              0x36U
#define LCD_CMD_COLMOD              0x3AU
#define LCD_CMD_RDID4               0xD3U
#define LCD_CMD_RDID_ST7789         0x04U

#define LCD_MADCTL_MY               0x80U
#define LCD_MADCTL_MX               0x40U
#define LCD_MADCTL_MV               0x20U

#if (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341)
  #define LCD_MADCTL_BGR            0x08U
  #define LCD_CONTROLLER_ID         0x9341U
#else
  #define LCD_MADCTL_BGR            0x08U
  #define LCD_CONTROLLER_ID         0x7789U
#endif

#if (LCD_PANEL_MIRROR_X != 0)
  #define LCD_MADCTL_XOR            LCD_MADCTL_MX
#else
  #define LCD_MADCTL_XOR            0x00U
#endif

/* Private variables ---------------------------------------------------------*/
LCD_InfoTypeDef LCD_Info;
volatile LCD_DebugTypeDef LCD_Debug;

static uint8_t s_lineBuffer[LCD_MAX_WIDTH * 2U];

/* Private function prototypes -----------------------------------------------*/
static void LCD_Cmd(uint8_t Command);
static void LCD_Data(const uint8_t *Data, uint32_t Length);
static void LCD_CmdParam(uint8_t Command, const uint8_t *Param, uint16_t Length);
static void LCD_SetWindowInternal(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1);
static void LCD_ControllerInit(void);
static void LCD_ErrorHandler(uint32_t Error);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Fatal LCD/SPI error: record the reason, blink the backlight and stop.
  *         Inspect LCD_Debug with J-Link to see ErrorCode/Stage.
  */
static void LCD_ErrorHandler(uint32_t Error)
{
  LCD_Debug.ErrorCode = Error;
  LCD_Debug.Stage     = 0xEEU;

  while (1)
  {
    LCD_BL_HIGH();
    HAL_Delay(100U);
    LCD_BL_LOW();
    HAL_Delay(100U);
  }
}

/**
  * @brief  Send a command byte. CS must already be low.
  */
static void LCD_Cmd(uint8_t Command)
{
  HAL_StatusTypeDef status;

  LCD_DC_LOW();
  status = SPI_Bus_Transmit(&Command, 1U, 1000U);
  LCD_Debug.LastHalStatus = (int32_t)status;
  LCD_Debug.SpiTxCount++;
  if (status != HAL_OK)
  {
    LCD_Debug.SpiErrorCount++;
    LCD_ErrorHandler(LCD_ERROR_SPI_TX);
  }
}

/**
  * @brief  Send data bytes. CS must already be low.
  */
static void LCD_Data(const uint8_t *Data, uint32_t Length)
{
  HAL_StatusTypeDef status;
  uint16_t chunk;

  LCD_DC_HIGH();
  while (Length > 0U)
  {
    chunk = (Length > 0xFFFFU) ? 0xFFFFU : (uint16_t)Length;
    status = SPI_Bus_Transmit(Data, chunk, 1000U);
    LCD_Debug.LastHalStatus = (int32_t)status;
    LCD_Debug.SpiTxCount++;
    if (status != HAL_OK)
    {
      LCD_Debug.SpiErrorCount++;
      LCD_ErrorHandler(LCD_ERROR_SPI_TX);
    }
    Data   += chunk;
    Length -= chunk;
  }
}

/**
  * @brief  Send a command followed by its parameters.
  */
static void LCD_CmdParam(uint8_t Command, const uint8_t *Param, uint16_t Length)
{
  LCD_CS_LOW();
  LCD_Cmd(Command);
  if (Length > 0U)
  {
    LCD_Data(Param, Length);
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Set the drawing window and prepare for RAM write. CS must be low.
  */
static void LCD_SetWindowInternal(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1)
{
  uint8_t param[4];

  param[0] = (uint8_t)(X0 >> 8);
  param[1] = (uint8_t)(X0 & 0xFFU);
  param[2] = (uint8_t)(X1 >> 8);
  param[3] = (uint8_t)(X1 & 0xFFU);
  LCD_Cmd(LCD_CMD_CASET);
  LCD_Data(param, 4U);

  param[0] = (uint8_t)(Y0 >> 8);
  param[1] = (uint8_t)(Y0 & 0xFFU);
  param[2] = (uint8_t)(Y1 >> 8);
  param[3] = (uint8_t)(Y1 & 0xFFU);
  LCD_Cmd(LCD_CMD_PASET);
  LCD_Data(param, 4U);

  LCD_Cmd(LCD_CMD_RAMWR);
}

/**
  * @brief  Controller specific initialization sequence.
  */
static void LCD_ControllerInit(void)
{
#if (LCD_USE_MINIMAL_INIT != 0)
  static const uint8_t pixfmt[]     = {0x55U};
  static const uint8_t madctlIli[]  = {0x48U};
  static const uint8_t madctlSt[]   = {0xC8U ^ LCD_MADCTL_XOR};
  const uint8_t *madctl = (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341) ? madctlIli : madctlSt;

  LCD_CmdParam(LCD_CMD_SWRESET, 0, 0U);
  HAL_Delay(120U);
  LCD_CmdParam(LCD_CMD_SLPOUT, 0, 0U);
  HAL_Delay(120U);
  LCD_CmdParam(LCD_CMD_COLMOD, pixfmt, 1U);
  LCD_CmdParam(LCD_CMD_MADCTL, madctl, 1U);
  LCD_CmdParam(LCD_CMD_DISPON, 0, 0U);
  HAL_Delay(20U);
#else
#if (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341)
  static const uint8_t pwrB[]   = {0x00U, 0xC1U, 0x30U};
  static const uint8_t pwrSeq[] = {0x64U, 0x03U, 0x12U, 0x81U};
  static const uint8_t drvA[]   = {0x85U, 0x00U, 0x78U};
  static const uint8_t pwrA[]   = {0x39U, 0x2CU, 0x00U, 0x34U, 0x02U};
  static const uint8_t prc[]    = {0x20U};
  static const uint8_t drvB[]   = {0x00U, 0x00U};
  static const uint8_t pwr1[]   = {0x23U};
  static const uint8_t pwr2[]   = {0x10U};
  static const uint8_t vcom1[]  = {0x3EU, 0x28U};
  static const uint8_t vcom2[]  = {0x86U};
  static const uint8_t madctl[] = {0x48U};
  static const uint8_t pixfmt[] = {0x55U};
  static const uint8_t frmctr[] = {0x00U, 0x18U};
  static const uint8_t dfunc[]  = {0x08U, 0x82U, 0x27U};
  static const uint8_t gammaDis[] = {0x00U};
  static const uint8_t gammaCurve[] = {0x01U};
  static const uint8_t gammaPos[] =
  {
    0x0FU, 0x31U, 0x2BU, 0x0CU, 0x0EU, 0x08U, 0x4EU, 0xF1U,
    0x37U, 0x07U, 0x10U, 0x03U, 0x0EU, 0x09U, 0x00U
  };
  static const uint8_t gammaNeg[] =
  {
    0x00U, 0x0EU, 0x14U, 0x03U, 0x11U, 0x07U, 0x31U, 0xC1U,
    0x48U, 0x08U, 0x0FU, 0x0CU, 0x31U, 0x36U, 0x0FU
  };

  LCD_CmdParam(LCD_CMD_SWRESET, 0, 0U);
  HAL_Delay(120U);

  LCD_CmdParam(0xEFU, pwrB, 3U);
  LCD_CmdParam(0xCFU, pwrSeq, 4U);
  LCD_CmdParam(0xEDU, drvA, 3U);
  LCD_CmdParam(0xE8U, pwrA, 5U);
  LCD_CmdParam(0xCBU, prc, 1U);
  LCD_CmdParam(0xF7U, drvB, 1U);
  LCD_CmdParam(0xEAU, drvB, 2U);
  LCD_CmdParam(0xC0U, pwr1, 1U);
  LCD_CmdParam(0xC1U, pwr2, 1U);
  LCD_CmdParam(0xC5U, vcom1, 2U);
  LCD_CmdParam(0xC7U, vcom2, 1U);
  LCD_CmdParam(LCD_CMD_MADCTL, madctl, 1U);
  LCD_CmdParam(LCD_CMD_COLMOD, pixfmt, 1U);
  LCD_CmdParam(0xB1U, frmctr, 2U);
  LCD_CmdParam(0xB6U, dfunc, 3U);
  LCD_CmdParam(0xF2U, gammaDis, 1U);
  LCD_CmdParam(0x26U, gammaCurve, 1U);
  LCD_CmdParam(0xE0U, gammaPos, 15U);
  LCD_CmdParam(0xE1U, gammaNeg, 15U);

  LCD_CmdParam(LCD_CMD_SLPOUT, 0, 0U);
  HAL_Delay(120U);
  LCD_CmdParam(LCD_CMD_DISPON, 0, 0U);
  HAL_Delay(20U);
#else
  static const uint8_t madctl[] = {0x00U};
  static const uint8_t pixfmt[] = {0x55U};
  static const uint8_t porch[]  = {0x0CU, 0x0CU, 0x00U, 0x33U, 0x33U};
  static const uint8_t gate[]   = {0x35U};
  static const uint8_t vcom[]   = {0x19U};
  static const uint8_t lcmctl[] = {0x2CU};
  static const uint8_t vdvvrh[] = {0x01U};
  static const uint8_t vdvrvh[] = {0x12U};
  static const uint8_t vdvrvl[] = {0x20U};
  static const uint8_t frctrl[] = {0x0FU};
  static const uint8_t pwrctrl[] = {0xA4U, 0xA1U};
  static const uint8_t gammaPos[] =
  {
    0xD0U, 0x04U, 0x11U, 0x10U, 0x11U, 0x16U, 0x18U, 0x10U,
    0x0DU, 0x0AU, 0x08U, 0x23U, 0x20U, 0x1AU
  };
  static const uint8_t gammaNeg[] =
  {
    0x0FU, 0x0CU, 0x0BU, 0x0AU, 0x07U, 0x0DU, 0x0BU, 0x08U,
    0x07U, 0x04U, 0x05U, 0x09U, 0x07U, 0x18U
  };

  LCD_CmdParam(LCD_CMD_SWRESET, 0, 0U);
  HAL_Delay(150U);

  LCD_CmdParam(LCD_CMD_SLPOUT, 0, 0U);
  HAL_Delay(120U);
  LCD_CmdParam(LCD_CMD_MADCTL, madctl, 1U);
  LCD_CmdParam(LCD_CMD_COLMOD, pixfmt, 1U);
  LCD_CmdParam(0xB2U, porch, 5U);
  LCD_CmdParam(0xB7U, gate, 1U);
  LCD_CmdParam(0xBBU, vcom, 1U);
  LCD_CmdParam(0xC0U, lcmctl, 1U);
  LCD_CmdParam(0xC2U, vdvvrh, 1U);
  LCD_CmdParam(0xC3U, vdvrvh, 1U);
  LCD_CmdParam(0xC4U, vdvrvl, 1U);
  LCD_CmdParam(0xC6U, frctrl, 1U);
  LCD_CmdParam(0xD0U, pwrctrl, 2U);
  LCD_CmdParam(0xE0U, gammaPos, 14U);
  LCD_CmdParam(0xE1U, gammaNeg, 14U);
  LCD_CmdParam(0x21U, 0, 0U);
  LCD_CmdParam(0x13U, 0, 0U);
  LCD_CmdParam(LCD_CMD_DISPON, 0, 0U);
  HAL_Delay(20U);
#endif
#endif
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the LCD (GPIO, SPI, reset and controller).
  */
void LCD_Init(void)
{
  GPIO_InitTypeDef gpio;

  LCD_Debug.Stage     = 1U;
  LCD_Debug.ErrorCode = LCD_ERROR_NONE;

  LCD_Info.Width      = LCD_NATIVE_WIDTH;
  LCD_Info.Height     = LCD_NATIVE_HEIGHT;
  LCD_Info.Controller = LCD_CONTROLLER_ID;
  LCD_Info.Rotation   = 2U;

  /* Control pins first: keep LCD_CS high before the SPI clock is enabled. */
  LCD_CTRL_GPIO_CLK_ENABLE();
  LCD_Debug.Stage = 2U;

  gpio.Pin   = LCD_CS_PIN | LCD_DC_PIN | LCD_RST_PIN | LCD_BL_PIN;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CTRL_GPIO_PORT, &gpio);
  LCD_Debug.Stage = 3U;

  LCD_CS_HIGH();
  LCD_DC_HIGH();
  LCD_RST_HIGH();
  LCD_BL_LOW();

  /* Shared SPI2 bus: also initializes SD_CS high. */
  if (SPI_Bus_Init() != HAL_OK)
  {
    LCD_ErrorHandler(LCD_ERROR_SPI_INIT);
  }
  LCD_Debug.Stage = 4U;

  LCD_Debug.Stage = 5U;
  LCD_Reset();
  LCD_Debug.Stage = 6U;

  LCD_ControllerInit();
  LCD_Debug.Stage = 7U;

  /* Best effort; requires MISO/SDO connected. Does not halt on failure. */
  LCD_Debug.ReadID = LCD_ReadID();

  LCD_SetRotation(2U);
  LCD_Backlight(1U);
  LCD_FillScreen(LCD_COLOR_BLACK);
  LCD_Debug.Stage = 8U;

  LCD_Debug.SpiCR1     = SPI_Bus_GetHandle()->Instance->CR1;
  LCD_Debug.SpiSR      = SPI_Bus_GetHandle()->Instance->SR;
  LCD_Debug.GpioBMode  = GPIOB->MODER;
  LCD_Debug.GpioBAfr1  = GPIOB->AFR[1];
  LCD_Debug.GpioEMode  = GPIOE->MODER;
}

/**
  * @brief  Hardware reset the panel.
  */
void LCD_Reset(void)
{
  LCD_CS_HIGH();
  LCD_RST_HIGH();
  HAL_Delay(10U);
  LCD_RST_LOW();
  HAL_Delay(20U);
  LCD_RST_HIGH();
  HAL_Delay(120U);
}

/**
  * @brief  Turn the backlight on or off.
  * @param  On: 0 = off, non-zero = on
  */
void LCD_Backlight(uint8_t On)
{
  if (On != 0U)
  {
    LCD_BL_HIGH();
  }
  else
  {
    LCD_BL_LOW();
  }
}

/**
  * @brief  Set display rotation (0..3). Updates LCD_Info.Width/Height.
  */
void LCD_SetRotation(uint8_t Rotation)
{
  uint8_t madctl;

  Rotation &= 3U;

#if (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341)
  switch (Rotation)
  {
    case 0U:
      madctl = LCD_MADCTL_MX | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_WIDTH;
      LCD_Info.Height = LCD_NATIVE_HEIGHT;
      break;
    case 1U:
      madctl = LCD_MADCTL_MV | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_HEIGHT;
      LCD_Info.Height = LCD_NATIVE_WIDTH;
      break;
    case 2U:
      madctl = LCD_MADCTL_MY | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_WIDTH;
      LCD_Info.Height = LCD_NATIVE_HEIGHT;
      break;
    default:
      madctl = LCD_MADCTL_MX | LCD_MADCTL_MY | LCD_MADCTL_MV | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_HEIGHT;
      LCD_Info.Height = LCD_NATIVE_WIDTH;
      break;
  }
#else
  switch (Rotation)
  {
    case 0U:
      madctl = 0x00U | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_WIDTH;
      LCD_Info.Height = LCD_NATIVE_HEIGHT;
      break;
    case 1U:
      madctl = 0x60U | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_HEIGHT;
      LCD_Info.Height = LCD_NATIVE_WIDTH;
      break;
    case 2U:
      madctl = 0xC0U | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_WIDTH;
      LCD_Info.Height = LCD_NATIVE_HEIGHT;
      break;
    default:
      madctl = 0xA0U | LCD_MADCTL_BGR;
      LCD_Info.Width  = LCD_NATIVE_HEIGHT;
      LCD_Info.Height = LCD_NATIVE_WIDTH;
      break;
  }
#endif

  madctl ^= LCD_MADCTL_XOR;
  LCD_CmdParam(LCD_CMD_MADCTL, &madctl, 1U);
  LCD_Info.Rotation = Rotation;
}

/**
  * @brief  Write one command byte (CS managed by this function).
  */
void LCD_WriteCommand(uint8_t Command)
{
  LCD_CS_LOW();
  LCD_Cmd(Command);
  LCD_CS_HIGH();
}

/**
  * @brief  Write one data byte (CS managed by this function).
  */
void LCD_WriteData(uint8_t Data)
{
  LCD_CS_LOW();
  LCD_DC_HIGH();
  (void)SPI_Bus_Transmit(&Data, 1U, HAL_MAX_DELAY);
  LCD_CS_HIGH();
}

/**
  * @brief  Set drawing window (CS managed by this function).
  */
void LCD_SetWindow(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1)
{
  LCD_CS_LOW();
  LCD_SetWindowInternal(X0, Y0, X1, Y1);
  LCD_CS_HIGH();
}

/**
  * @brief  Draw one pixel.
  */
void LCD_DrawPixel(uint16_t X, uint16_t Y, uint16_t Color)
{
  uint8_t data[2];

  if ((X >= LCD_Info.Width) || (Y >= LCD_Info.Height))
  {
    return;
  }

  data[0] = (uint8_t)(Color >> 8);
  data[1] = (uint8_t)(Color & 0xFFU);

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, X, Y);
  LCD_Data(data, 2U);
  LCD_CS_HIGH();
}

/**
  * @brief  Fill a rectangle.
  */
void LCD_FillRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color)
{
  uint32_t pixels;
  uint16_t i;

  if ((X >= LCD_Info.Width) || (Y >= LCD_Info.Height) ||
      (Width == 0U) || (Height == 0U))
  {
    return;
  }

  if ((uint32_t)X + Width > LCD_Info.Width)
  {
    Width = (uint16_t)(LCD_Info.Width - X);
  }
  if ((uint32_t)Y + Height > LCD_Info.Height)
  {
    Height = (uint16_t)(LCD_Info.Height - Y);
  }

  for (i = 0U; i < Width; i++)
  {
    s_lineBuffer[2U * i]      = (uint8_t)(Color >> 8);
    s_lineBuffer[2U * i + 1U] = (uint8_t)(Color & 0xFFU);
  }

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, (uint16_t)(X + Width - 1U), (uint16_t)(Y + Height - 1U));
  pixels = (uint32_t)Width * Height;
  while (pixels >= Width)
  {
    LCD_Data(s_lineBuffer, (uint32_t)Width * 2U);
    pixels -= Width;
  }
  if (pixels > 0U)
  {
    LCD_Data(s_lineBuffer, pixels * 2U);
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Fill the whole screen.
  */
void LCD_FillScreen(uint16_t Color)
{
  LCD_FillRect(0U, 0U, LCD_Info.Width, LCD_Info.Height, Color);
}

/**
  * @brief  Draw a horizontal line.
  */
void LCD_DrawHLine(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color)
{
  LCD_FillRect(X, Y, Width, 1U, Color);
}

/**
  * @brief  Draw a vertical line.
  */
void LCD_DrawVLine(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color)
{
  LCD_FillRect(X, Y, 1U, Height, Color);
}

/**
  * @brief  Draw a rectangle outline.
  */
void LCD_DrawRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color)
{
  if ((Width == 0U) || (Height == 0U))
  {
    return;
  }
  LCD_DrawHLine(X, Y, Width, Color);
  LCD_DrawHLine(X, (uint16_t)(Y + Height - 1U), Width, Color);
  LCD_DrawVLine(X, Y, Height, Color);
  LCD_DrawVLine((uint16_t)(X + Width - 1U), Y, Height, Color);
}

/**
  * @brief  Draw a line using Bresenham's algorithm.
  */
void LCD_DrawLine(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t Color)
{
  int32_t x0 = (int32_t)X0;
  int32_t y0 = (int32_t)Y0;
  int32_t x1 = (int32_t)X1;
  int32_t y1 = (int32_t)Y1;
  int32_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
  int32_t sx = (x0 < x1) ? 1 : -1;
  int32_t dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
  int32_t sy = (y0 < y1) ? 1 : -1;
  int32_t err = dx + dy;
  int32_t e2;

  while (1)
  {
    LCD_DrawPixel((uint16_t)x0, (uint16_t)y0, Color);
    if ((x0 == x1) && (y0 == y1))
    {
      break;
    }
    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x0  += sx;
    }
    if (e2 <= dx)
    {
      err += dx;
      y0  += sy;
    }
  }
}

/**
  * @brief  Draw an RGB565 image whose bytes are stored high byte first.
  */
void LCD_DrawImage(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image)
{
  uint32_t stride;
  uint16_t drawWidth;
  uint16_t drawHeight;
  uint16_t row;
  const uint8_t *rowData;

  if ((Image == 0) || (Width == 0U) || (Height == 0U) ||
      (X >= LCD_Info.Width) || (Y >= LCD_Info.Height))
  {
    return;
  }

  stride = (uint32_t)Width * 2U;
  drawWidth = Width;
  drawHeight = Height;

  if ((uint32_t)X + drawWidth > LCD_Info.Width)
  {
    drawWidth = (uint16_t)(LCD_Info.Width - X);
  }
  if ((uint32_t)Y + drawHeight > LCD_Info.Height)
  {
    drawHeight = (uint16_t)(LCD_Info.Height - Y);
  }

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, (uint16_t)(X + drawWidth - 1U), (uint16_t)(Y + drawHeight - 1U));

  rowData = Image;
  for (row = 0U; row < drawHeight; row++)
  {
    LCD_Data(rowData, (uint32_t)drawWidth * 2U);
    rowData += stride;
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Draw an RGB565 image scaled 2x. Bytes are stored high byte first.
  */
void LCD_DrawImageScaled2x(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image)
{
  uint32_t stride;
  uint16_t drawWidth;
  uint16_t drawHeight;
  uint16_t row;
  uint16_t col;
  uint16_t srcX;
  uint16_t srcY;
  const uint8_t *src;

  if ((Image == 0) || (Width == 0U) || (Height == 0U) ||
      (X >= LCD_Info.Width) || (Y >= LCD_Info.Height))
  {
    return;
  }

  stride = (uint32_t)Width * 2U;
  drawWidth = (uint16_t)(Width * 2U);
  drawHeight = (uint16_t)(Height * 2U);

  if ((uint32_t)X + drawWidth > LCD_Info.Width)
  {
    drawWidth = (uint16_t)(LCD_Info.Width - X);
  }
  if ((uint32_t)Y + drawHeight > LCD_Info.Height)
  {
    drawHeight = (uint16_t)(LCD_Info.Height - Y);
  }
  if (drawWidth > LCD_MAX_WIDTH)
  {
    drawWidth = LCD_MAX_WIDTH;
  }

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, (uint16_t)(X + drawWidth - 1U), (uint16_t)(Y + drawHeight - 1U));

  for (row = 0U; row < drawHeight; row++)
  {
    srcY = (uint16_t)(row >> 1U);
    for (col = 0U; col < drawWidth; col++)
    {
      srcX = (uint16_t)(col >> 1U);
      src = Image + ((uint32_t)srcY * stride) + ((uint32_t)srcX * 2U);
      s_lineBuffer[2U * col] = src[0];
      s_lineBuffer[2U * col + 1U] = src[1];
    }
    LCD_Data(s_lineBuffer, (uint32_t)drawWidth * 2U);
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Draw one character using the ST Font8 (5x8) font.
  */
void LCD_DrawChar(uint16_t X, uint16_t Y, char Character, uint16_t Color, uint16_t BackColor)
{
  const uint8_t *charData;
  const uint8_t *rowData;
  uint8_t  width;
  uint8_t  height;
  uint8_t  bytesPerRow;
  uint8_t  line[64];
  uint8_t  row;
  uint8_t  col;
  uint8_t  pixel;
  uint16_t pixelColor;

  if ((uint8_t)Character < 0x20U || (uint8_t)Character > 0x7EU)
  {
    Character = (char)'?';
  }

  width       = (uint8_t)Font8.Width;
  height      = (uint8_t)Font8.Height;
  bytesPerRow = (uint8_t)((width + 7U) / 8U);

  if ((X + width > LCD_Info.Width) || (Y + height > LCD_Info.Height))
  {
    return;
  }

  charData = &Font8.table[((uint8_t)Character - 0x20U) * height * bytesPerRow];

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, (uint16_t)(X + width - 1U), (uint16_t)(Y + height - 1U));

  for (row = 0U; row < height; row++)
  {
    rowData = &charData[(uint16_t)row * bytesPerRow];
    for (col = 0U; col < width; col++)
    {
      pixel = (uint8_t)((rowData[col >> 3] >> (7U - (col & 7U))) & 0x01U);
      pixelColor = (pixel != 0U) ? Color : BackColor;
      line[2U * col]      = (uint8_t)(pixelColor >> 8);
      line[2U * col + 1U] = (uint8_t)(pixelColor & 0xFFU);
    }
    LCD_Data(line, (uint32_t)width * 2U);
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Draw a string.
  */
void LCD_DrawString(uint16_t X, uint16_t Y, const char *String, uint16_t Color, uint16_t BackColor)
{
  uint16_t startX = X;

  while (*String != '\0')
  {
    if (*String == '\n')
    {
      X = startX;
      Y = (uint16_t)(Y + Font8.Height);
    }
    else if (*String == '\r')
    {
      /* ignore */
    }
    else
    {
      LCD_DrawChar(X, Y, *String, Color, BackColor);
      X = (uint16_t)(X + Font8.Width);
      if ((uint16_t)(X + Font8.Width) > LCD_Info.Width)
      {
        X = startX;
        Y = (uint16_t)(Y + Font8.Height);
      }
    }
    String++;
  }
}

/**
  * @brief  Draw one character using Font8 scaled 2x (10x16).
  */
static void LCD_DrawChar2x(uint16_t X, uint16_t Y, char Character, uint16_t Color, uint16_t BackColor)
{
  const uint8_t *charData;
  const uint8_t *rowData;
  uint8_t  width;
  uint8_t  height;
  uint8_t  bytesPerRow;
  uint8_t  drawWidth;
  uint8_t  line[64];
  uint8_t  row;
  uint8_t  col;
  uint8_t  pixel;
  uint16_t pixelColor;

  if ((uint8_t)Character < 0x20U || (uint8_t)Character > 0x7EU)
  {
    Character = (char)'?';
  }

  width       = (uint8_t)Font8.Width;
  height      = (uint8_t)Font8.Height;
  drawWidth   = (uint8_t)(width * 2U);
  bytesPerRow = (uint8_t)((width + 7U) / 8U);

  if ((X + drawWidth > LCD_Info.Width) || (Y + (height * 2U) > LCD_Info.Height))
  {
    return;
  }

  charData = &Font8.table[((uint8_t)Character - 0x20U) * height * bytesPerRow];

  LCD_CS_LOW();
  LCD_SetWindowInternal(X, Y, (uint16_t)(X + drawWidth - 1U), (uint16_t)(Y + (height * 2U) - 1U));

  for (row = 0U; row < height; row++)
  {
    rowData = &charData[(uint16_t)row * bytesPerRow];
    for (col = 0U; col < width; col++)
    {
      pixel = (uint8_t)((rowData[col >> 3] >> (7U - (col & 7U))) & 0x01U);
      pixelColor = (pixel != 0U) ? Color : BackColor;
      line[4U * col]      = (uint8_t)(pixelColor >> 8);
      line[4U * col + 1U] = (uint8_t)(pixelColor & 0xFFU);
      line[4U * col + 2U] = (uint8_t)(pixelColor >> 8);
      line[4U * col + 3U] = (uint8_t)(pixelColor & 0xFFU);
    }
    LCD_Data(line, (uint32_t)drawWidth * 2U);
    LCD_Data(line, (uint32_t)drawWidth * 2U);
  }
  LCD_CS_HIGH();
}

/**
  * @brief  Draw a string using Font8 scaled 2x.
  */
void LCD_DrawString2x(uint16_t X, uint16_t Y, const char *String, uint16_t Color, uint16_t BackColor)
{
  uint16_t startX = X;
  uint16_t charWidth = (uint16_t)(Font8.Width * 2U);
  uint16_t charHeight = (uint16_t)(Font8.Height * 2U);

  while (*String != '\0')
  {
    if (*String == '\n')
    {
      X = startX;
      Y = (uint16_t)(Y + charHeight);
    }
    else if (*String == '\r')
    {
      /* ignore */
    }
    else
    {
      LCD_DrawChar2x(X, Y, *String, Color, BackColor);
      X = (uint16_t)(X + charWidth);
      if ((uint16_t)(X + charWidth) > LCD_Info.Width)
      {
        X = startX;
        Y = (uint16_t)(Y + charHeight);
      }
    }
    String++;
  }
}

/**
  * @brief  Try to read the ILI9341 ID (0x9341). Requires MISO connected.
  * @retval Controller ID or 0 if it cannot be read.
  */
uint16_t LCD_ReadID(void)
{
  uint8_t command = (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341) ? LCD_CMD_RDID4 : LCD_CMD_RDID_ST7789;
  uint8_t dummy[4] = {0U, 0U, 0U, 0U};
  uint8_t id[4] = {0U, 0U, 0U, 0U};
  uint16_t result = 0U;
  HAL_StatusTypeDef status;

  LCD_CS_LOW();
  LCD_Cmd(command);
  LCD_DC_HIGH();
  status = SPI_Bus_TransmitReceive(dummy, id, 4U, 1000U);
  LCD_CS_HIGH();

  LCD_Debug.LastHalStatus = (int32_t)status;
  LCD_Debug.SpiTxCount++;
  if (status != HAL_OK)
  {
    LCD_Debug.SpiErrorCount++;
    LCD_ErrorHandler(LCD_ERROR_SPI_RX);
  }

  LCD_Debug.ReadIDRaw[0] = id[0];
  LCD_Debug.ReadIDRaw[1] = id[1];
  LCD_Debug.ReadIDRaw[2] = id[2];
  LCD_Debug.ReadIDRaw[3] = id[3];

  if ((id[2] == 0x93U) && (id[3] == 0x41U))
  {
    result = 0x9341U;
  }
  else
  {
    result = (uint16_t)((id[2] << 8) | id[3]);
  }

  LCD_Debug.ReadID = result;
  return result;
}
