/**
  ******************************************************************************
  * @file    main.c
  * @brief   OV7670 live preview on a 240x320 SPI LCD (STM32F4-Discovery)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd.h"
#include "ov7670.h"

/* 1 = 160x120 (fast), 2 = 320x240 (full screen, 4x more SPI traffic). */
#define CAMERA_PREVIEW_SCALE      1U

/* 1 = show the camera image, 0 = show frame-rate statistics instead.
 * The image drawing path is kept below and can be re-enabled by setting this to 1. */
#define CAMERA_DISPLAY_IMAGE      0U

/* Show CAM/MCU/CAP/DRAW measurements. */
#define CAMERA_SHOW_STATS         1U

/* Low light can stretch the OV7670 exposure to about one second. */
#define CAMERA_CAPTURE_TIMEOUT_MS 2000U

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

#if (OV7670_ENABLE != 0)
static void Camera_Preview(void);
#if (CAMERA_SHOW_STATS != 0)
static void Camera_EnableCycleCounter(void);
static void DebugUart_Init(void);
static void DebugUart_Write(const char *Text);
static void DebugUart_WriteN(const char *Text, uint8_t Length);
static void Camera_LogStats(uint32_t DisplayFps10, uint32_t CaptureMs);
static void Camera_DrawStats(uint32_t CameraFps10, uint32_t DisplayFps10,
                             uint32_t CaptureMs, uint32_t DrawMs10);
#endif
#endif

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program: initialize the LCD and run the camera preview.
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

#if (OV7670_ENABLE != 0) && (CAMERA_SHOW_STATS != 0)
  Camera_EnableCycleCounter();
  DebugUart_Init();
#endif

  BSP_LED_Init(LED3);
  BSP_LED_On(LED3);

  LCD_Init();
  LCD_SetRotation(1U);
  LCD_FillScreen(LCD_COLOR_BLACK);

#if (OV7670_ENABLE != 0)
  if (OV7670_Init() == OV7670_OK)
  {
    OV7670_SetPolarityCombo(2U);
    OV7670_SetFastCapture(1U);
    OV7670_ResetFrameStats();
    Camera_Preview();
  }
  else
  {
    LCD_DrawString2x(8U, 8U, "CAM INIT ERR", LCD_COLOR_RED, LCD_COLOR_BLACK);
  }
#else
  LCD_DrawString2x(8U, 8U, "CAM DISABLED", LCD_COLOR_GRAY, LCD_COLOR_BLACK);
#endif

  while (1)
  {
    BSP_LED_Toggle(LED3);
    HAL_Delay(500U);
  }
}

#if (OV7670_ENABLE != 0)
static uint32_t s_cameraFrame[OV7670_FRAME_BYTES / 4U];

#if (CAMERA_SHOW_STATS != 0)
static void Camera_EnableCycleCounter(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void DebugUart_Init(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_USART3_CLK_ENABLE();

  gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &gpio);

  USART3->CR1 = 0U;
  USART3->BRR = (HAL_RCC_GetPCLK1Freq() + 57600U) / 115200U;
  USART3->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

  DebugUart_Write("DBG USART3 PD8/PD9 115200\r\n");
}

static void DebugUart_PutChar(char Ch)
{
  while ((USART3->SR & USART_SR_TXE) == 0U)
  {
  }
  USART3->DR = (uint8_t)Ch;
}

static void DebugUart_Write(const char *Text)
{
  while (*Text != '\0')
  {
    DebugUart_PutChar(*Text);
    Text++;
  }
}

static void DebugUart_WriteN(const char *Text, uint8_t Length)
{
  while (Length > 0U)
  {
    DebugUart_PutChar(*Text);
    Text++;
    Length--;
  }
}

static uint8_t Camera_AppendString(char *Dst, uint8_t Pos, const char *Src)
{
  while (*Src != '\0')
  {
    Dst[Pos] = *Src;
    Pos++;
    Src++;
  }
  return Pos;
}

static uint8_t Camera_AppendUInt(char *Dst, uint8_t Pos, uint32_t Value)
{
  char tmp[10];
  uint8_t n = 0U;

  do
  {
    tmp[n] = (char)('0' + (Value % 10U));
    n++;
    Value /= 10U;
  } while ((Value != 0U) && (n < sizeof(tmp)));

  while (n > 0U)
  {
    n--;
    Dst[Pos] = tmp[n];
    Pos++;
  }
  return Pos;
}

static uint8_t Camera_AppendTenths(char *Dst, uint8_t Pos, uint32_t Value10)
{
  Pos = Camera_AppendUInt(Dst, Pos, Value10 / 10U);
  Dst[Pos] = '.';
  Pos++;
  Dst[Pos] = (char)('0' + (Value10 % 10U));
  Pos++;
  return Pos;
}

static void Camera_LogStats(uint32_t DisplayFps10, uint32_t CaptureMs)
{
  char line[160];
  uint8_t p = 0U;

  p = Camera_AppendString(line, p, "FPS ");
  p = Camera_AppendTenths(line, p, DisplayFps10);
  p = Camera_AppendString(line, p, " CAP ");
  p = Camera_AppendUInt(line, p, CaptureMs);
  p = Camera_AppendString(line, p, " FC ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.FrameCount);
  p = Camera_AppendString(line, p, " ST ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.CaptureStatus);
  p = Camera_AppendString(line, p, " VS ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.VsyncEdges);
  p = Camera_AppendString(line, p, " PL ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.PclkEdges / 1000U);
  p = Camera_AppendString(line, p, " CL ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Clkrc);
  p = Camera_AppendString(line, p, " DB ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Dblv);
  p = Camera_AppendString(line, p, " AE ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Aec);
  p = Camera_AppendString(line, p, " GN ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Gain);
  p = Camera_AppendString(line, p, " DL ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.DummyRows);
  p = Camera_AppendString(line, p, " AD ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.AdvRows);
  p = Camera_AppendString(line, p, " NT ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.NtCtrl);
  p = Camera_AppendString(line, p, " C11 ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Com11);
  p = Camera_AppendString(line, p, " C8 ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Com8);
  p = Camera_AppendString(line, p, " C9 ");
  p = Camera_AppendUInt(line, p, OV7670_Debug.Com9);
  line[p] = '\r';
  p++;
  line[p] = '\n';
  p++;
  DebugUart_WriteN(line, p);
}

#if (CAMERA_DISPLAY_IMAGE == 0)
static void Camera_PadLine(char *Dst, uint8_t Pos)
{
  while (Pos < 19U)
  {
    Dst[Pos] = ' ';
    Pos++;
  }
  Dst[Pos] = '\0';
}

static void Camera_BuildFpsLine(char *Dst, const char *Label, uint32_t Fps10)
{
  uint8_t p = 0U;

  p = Camera_AppendString(Dst, p, Label);
  Dst[p] = ' ';
  p++;
  p = Camera_AppendTenths(Dst, p, Fps10);
  p = Camera_AppendString(Dst, p, " FPS");
  Camera_PadLine(Dst, p);
}

static void Camera_BuildIntLine(char *Dst, const char *Label, uint32_t Value,
                                const char *Unit)
{
  uint8_t p = 0U;

  p = Camera_AppendString(Dst, p, Label);
  Dst[p] = ' ';
  p++;
  p = Camera_AppendUInt(Dst, p, Value);
  p = Camera_AppendString(Dst, p, Unit);
  Camera_PadLine(Dst, p);
}

#endif

static void Camera_DrawStats(uint32_t CameraFps10, uint32_t DisplayFps10,
                             uint32_t CaptureMs, uint32_t DrawMs10)
{
#if (CAMERA_DISPLAY_IMAGE == 0)
  char line[24];

  (void)CameraFps10;

  Camera_BuildFpsLine(line, "FPS", DisplayFps10);
  LCD_DrawString2x(20U, 50U, line, LCD_COLOR_GREEN, LCD_COLOR_BLACK);

  Camera_BuildIntLine(line, "CAP", CaptureMs, " ms");
  LCD_DrawString2x(20U, 90U, line, LCD_COLOR_YELLOW, LCD_COLOR_BLACK);

  (void)DrawMs10;

  Camera_BuildIntLine(line, "FC", OV7670_Debug.FrameCount, "");
  LCD_DrawString2x(20U, 130U, line, LCD_COLOR_CYAN, LCD_COLOR_BLACK);

  Camera_BuildIntLine(line, "ST", OV7670_Debug.CaptureStatus, "");
  LCD_DrawString2x(20U, 170U, line, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

  {
    char dbg[40];
    uint8_t q = 0U;

    q = Camera_AppendString(dbg, q, "VS ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.VsyncEdges);
    q = Camera_AppendString(dbg, q, " PL ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.PclkEdges / 1000U);
    q = Camera_AppendString(dbg, q, " CL ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Clkrc);
    q = Camera_AppendString(dbg, q, " DB ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Dblv);
    dbg[q] = '\0';
    LCD_DrawString(2U, 200U, dbg, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

    q = 0U;
    q = Camera_AppendString(dbg, q, "AE ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Aec);
    q = Camera_AppendString(dbg, q, " GN ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Gain);
    q = Camera_AppendString(dbg, q, " DL ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.DummyRows);
    q = Camera_AppendString(dbg, q, " AD ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.AdvRows);
    dbg[q] = '\0';
    LCD_DrawString(2U, 210U, dbg, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

    q = 0U;
    q = Camera_AppendString(dbg, q, "NT ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.NtCtrl);
    q = Camera_AppendString(dbg, q, " C11 ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Com11);
    q = Camera_AppendString(dbg, q, " C8 ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Com8);
    q = Camera_AppendString(dbg, q, " C9 ");
    q = Camera_AppendUInt(dbg, q, OV7670_Debug.Com9);
    dbg[q] = '\0';
    LCD_DrawString(2U, 220U, dbg, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
  }
#else
  char line[48];
  uint8_t p = 0U;

  p = Camera_AppendString(line, p, "CAM ");
  p = Camera_AppendTenths(line, p, CameraFps10);
  p = Camera_AppendString(line, p, " LCD ");
  p = Camera_AppendTenths(line, p, DisplayFps10);
  p = Camera_AppendString(line, p, " CAP ");
  p = Camera_AppendUInt(line, p, CaptureMs);
  p = Camera_AppendString(line, p, "ms DRAW ");
  p = Camera_AppendTenths(line, p, DrawMs10);
  p = Camera_AppendString(line, p, "ms");
  line[p] = '\0';

  LCD_FillRect(0U, 0U, LCD_Info.Width, 10U, LCD_COLOR_BLACK);
  LCD_DrawString(2U, 1U, line, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
#endif
}
#endif

static void Camera_Preview(void)
{
  uint8_t *frame = (uint8_t *)s_cameraFrame;
#if (CAMERA_SHOW_STATS != 0)
  uint32_t statStart = HAL_GetTick();
  uint32_t statFrames = 0U;
#endif

#if (CAMERA_DISPLAY_IMAGE == 0)
  while (1)
  {
    if (OV7670_Capture(frame, CAMERA_CAPTURE_TIMEOUT_MS) == OV7670_OK)
    {
#if (CAMERA_SHOW_STATS != 0)
      statFrames++;
#endif
      BSP_LED_Toggle(LED3);
    }

#if (CAMERA_SHOW_STATS != 0)
    {
      uint32_t now = HAL_GetTick();
      uint32_t elapsed = now - statStart;

      if (elapsed >= 1000U)
      {
        uint32_t displayFps10 = (statFrames * 10000U) / elapsed;

        Camera_DrawStats(0U, displayFps10,
                         OV7670_Debug.FramePeriodMs, 0U);
        Camera_LogStats(displayFps10, OV7670_Debug.FramePeriodMs);

        statStart = now;
        statFrames = 0U;
      }
    }
#endif
  }
#else
  uint16_t x;
  uint16_t y;
  uint8_t *next = OV7670_GetScratchBuffer();
#if (CAMERA_SHOW_STATS != 0)
  uint32_t statDrawCycles = 0U;
#endif

  x = (LCD_Info.Width > (OV7670_WIDTH * CAMERA_PREVIEW_SCALE)) ?
      (uint16_t)((LCD_Info.Width - (OV7670_WIDTH * CAMERA_PREVIEW_SCALE)) / 2U) : 0U;
  y = (LCD_Info.Height > (OV7670_HEIGHT * CAMERA_PREVIEW_SCALE)) ?
      (uint16_t)((LCD_Info.Height - (OV7670_HEIGHT * CAMERA_PREVIEW_SCALE)) / 2U) : 0U;

  (void)OV7670_Capture(frame, CAMERA_CAPTURE_TIMEOUT_MS);

  while (1)
  {
    uint32_t drawStart;
    uint32_t drawCycles;

    if (OV7670_StartCapture(next) != OV7670_OK)
    {
      continue;
    }

    drawStart = DWT->CYCCNT;

#if (CAMERA_PREVIEW_SCALE == 2U)
    LCD_DrawImageScaled2x(x, y, OV7670_WIDTH, OV7670_HEIGHT, frame);
#else
    LCD_DrawImage(x, y, OV7670_WIDTH, OV7670_HEIGHT, frame);
#endif

    drawCycles = DWT->CYCCNT - drawStart;

    if (OV7670_WaitCapture(next, CAMERA_CAPTURE_TIMEOUT_MS) == OV7670_OK)
    {
      uint8_t *swap = frame;

      frame = next;
      next  = swap;
#if (CAMERA_SHOW_STATS != 0)
      statFrames++;
      statDrawCycles += drawCycles;
#endif
      BSP_LED_Toggle(LED3);
    }

#if (CAMERA_SHOW_STATS != 0)
    {
      uint32_t now = HAL_GetTick();
      uint32_t elapsed = now - statStart;

      if (elapsed >= 1000U)
      {
        uint32_t cameraFps10 = (OV7670_Debug.FramePeriodMs != 0U) ?
                               (10000U / OV7670_Debug.FramePeriodMs) : 0U;
        uint32_t displayFps10 = (statFrames * 10000U) / elapsed;
        uint32_t drawMs10 = (statFrames != 0U) ?
                            ((statDrawCycles / statFrames) / 16800U) : 0U;

        Camera_DrawStats(cameraFps10, displayFps10,
                         OV7670_Debug.FramePeriodMs, drawMs10);
        Camera_LogStats(displayFps10, OV7670_Debug.FramePeriodMs);

        statStart = now;
        statFrames = 0U;
        statDrawCycles = 0U;
      }
    }
#endif
  }
#endif
}
#endif

/**
  * @brief  System Clock Configuration
  *         SYSCLK = 168 MHz from HSE (8 MHz) through PLL.
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8U;
  RCC_OscInitStruct.PLL.PLLN = 336U;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7U;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                 RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_GetREVID() >= 0x1001U)
  {
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  Error handler.
  */
static void Error_Handler(void)
{
  BSP_LED_On(LED5);
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
  while (1)
  {
  }
}
#endif
