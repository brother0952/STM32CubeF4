/**
  ******************************************************************************
  * @file    main.c
  * @brief   SPI LCD demo for STM32F4-Discovery
  *          240x320 ILI9341 / ST7789 panel on SPI2
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd.h"
#include "ov7670.h"

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void LCD_ShowTestPattern(void);

#if (OV7670_ENABLE != 0)
static void Camera_Preview(void);
#endif

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  */
int main(void)
{
  uint16_t x = 0U;
  uint16_t oldX = 0U;
  uint16_t y = 0U;
  uint8_t  dir = 1U;

  HAL_Init();

  SystemClock_Config();

  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);

  BSP_LED_On(LED3);

  LCD_Init();
  BSP_LED_On(LED4);

  /* Obvious color cycle: if the panel stays white, no SPI command is getting in. */
  LCD_FillScreen(LCD_COLOR_RED);
  HAL_Delay(500U);
  LCD_FillScreen(LCD_COLOR_GREEN);
  HAL_Delay(500U);
  LCD_FillScreen(LCD_COLOR_BLUE);
  HAL_Delay(500U);
  BSP_LED_On(LED5);

  LCD_ShowTestPattern();
  BSP_LED_On(LED6);

#if (OV7670_ENABLE != 0)
  if (OV7670_Init() == OV7670_OK)
  {
    LCD_SetRotation(1U);
    LCD_FillScreen(LCD_COLOR_BLACK);
    Camera_Preview();
    LCD_SetRotation(2U);
    LCD_FillScreen(LCD_COLOR_BLACK);
    LCD_DrawString(8U, 8U, "Camera stopped", LCD_COLOR_RED, LCD_COLOR_BLACK);
    HAL_Delay(1000U);
  }
  else
  {
    LCD_DrawString(8U, 8U, "OV7670 not found", LCD_COLOR_RED, LCD_COLOR_BLACK);
    HAL_Delay(1000U);
  }
#endif

  y = (uint16_t)(LCD_Info.Height - 24U);

  while (1)
  {
    /* Move a small square along the bottom of the screen */
    LCD_FillRect(oldX, y, 24U, 24U, LCD_COLOR_BLACK);
    LCD_FillRect(x, y, 24U, 24U, LCD_COLOR_RED);
    oldX = x;

    if (dir != 0U)
    {
      if ((uint16_t)(x + 8U + 24U) >= LCD_Info.Width)
      {
        dir = 0U;
      }
      else
      {
        x = (uint16_t)(x + 8U);
      }
    }
    else
    {
      if (x < 8U)
      {
        dir = 1U;
      }
      else
      {
        x = (uint16_t)(x - 8U);
      }
    }

    BSP_LED_Toggle(LED3);
    HAL_Delay(30U);
  }
}

#if (OV7670_ENABLE != 0)
static uint8_t s_cameraFrame[OV7670_FRAME_BYTES];

static void Camera_Preview(void)
{
  uint16_t x;
  uint16_t y;
  uint8_t errors = 0U;

  if (LCD_Info.Width > (OV7670_WIDTH * 2U))
  {
    x = (uint16_t)((LCD_Info.Width - (OV7670_WIDTH * 2U)) / 2U);
  }
  else
  {
    x = 0U;
  }

  if (LCD_Info.Height > (OV7670_HEIGHT * 2U))
  {
    y = (uint16_t)((LCD_Info.Height - (OV7670_HEIGHT * 2U)) / 2U);
  }
  else
  {
    y = 0U;
  }

  while (1)
  {
    if (OV7670_Capture(s_cameraFrame, 1000U) == OV7670_OK)
    {
      LCD_DrawImageScaled2x(x, y, OV7670_WIDTH, OV7670_HEIGHT, s_cameraFrame);
      errors = 0U;
    }
    else
    {
      errors++;
      if (errors >= 3U)
      {
        return;
      }
    }

    BSP_LED_Toggle(LED3);
  }
}
#endif

/**
  * @brief  Draw a test pattern to verify the display.
  */
static void LCD_ShowTestPattern(void)
{
  static const uint16_t colors[8] =
  {
    LCD_COLOR_RED,
    LCD_COLOR_GREEN,
    LCD_COLOR_BLUE,
    LCD_COLOR_YELLOW,
    LCD_COLOR_CYAN,
    LCD_COLOR_MAGENTA,
    LCD_COLOR_ORANGE,
    LCD_COLOR_WHITE
  };
  uint8_t i;

  LCD_FillScreen(LCD_COLOR_BLACK);

  LCD_DrawString(8U, 8U, "STM32F4-Discovery", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
  LCD_DrawString(8U, 20U, "240x320 SPI LCD", LCD_COLOR_CYAN, LCD_COLOR_BLACK);
#if (LCD_CONTROLLER == LCD_CONTROLLER_ILI9341)
  LCD_DrawString(8U, 32U, "Controller: ILI9341", LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
#else
  LCD_DrawString(8U, 32U, "Controller: ST7789", LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
#endif

  for (i = 0U; i < 8U; i++)
  {
    LCD_FillRect((uint16_t)(i * 30U), 52U, 30U, 40U, colors[i]);
  }

  LCD_DrawRect(0U, 0U, LCD_Info.Width, LCD_Info.Height, LCD_COLOR_GRAY);
  LCD_DrawRect(4U, 4U, (uint16_t)(LCD_Info.Width - 8U), (uint16_t)(LCD_Info.Height - 8U), LCD_COLOR_GRAY);

  LCD_DrawLine(8U, 110U, (uint16_t)(LCD_Info.Width - 9U), (uint16_t)(LCD_Info.Height - 40U), LCD_COLOR_BLUE);

  LCD_FillRect(8U, 120U, 100U, 60U, LCD_COLOR565(32U, 32U, 32U));
  LCD_DrawString(16U, 136U, "LCD OK", LCD_COLOR_GREEN, LCD_COLOR565(32U, 32U, 32U));

  LCD_DrawString(8U, (uint16_t)(LCD_Info.Height - 16U), "Hello, world!", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
}

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
