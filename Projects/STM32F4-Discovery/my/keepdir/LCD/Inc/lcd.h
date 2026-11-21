/**
  ******************************************************************************
  * @file    lcd.h
  * @brief   SPI LCD driver for 240x320 modules (ILI9341 / ST7789)
  *          Target board: STM32F4-Discovery (STM32F407VGT6)
  ******************************************************************************
  * Pin assignment (can be changed below):
  *
  *   LCD pin     STM32F4-Discovery pin     Function
  *   ---------   ----------------------    ------------------------------
  *   VCC         3V                        Power (3.3 V)
  *   GND         GND                       Ground
  *   SCK         PB13                      SPI2_SCK
  *   SDI (MOSI)  PB15                      SPI2_MOSI
  *   SDO (MISO)  PB14                      SPI2_MISO (optional, used for ID read)
  *   CS          PE2                       GPIO output, active low
  *   D/C         PE4                       GPIO output (0 = command, 1 = data)
  *   RESET       PE5                       GPIO output, active low
  *   LED (BL)    PE7                       GPIO output, high = backlight on
  *
  * Note: PE2/PE4/PE5/PE7 are used by the LCD. PE6 is reserved for OV7670 D7.
  *       If the backlight draws more than a GPIO can source, drive it
  *       through a transistor instead of connecting it directly to PE7.
  ******************************************************************************
  */

#ifndef __LCD_H
#define __LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Controller selection ------------------------------------------------------*/
#define LCD_CONTROLLER_ILI9341   1
#define LCD_CONTROLLER_ST7789    2

/* Select the controller here. This project uses ST7789 by default.
 * If the panel stays white/blank after LCD_Init(), try ILI9341. */
#ifndef LCD_CONTROLLER
#define LCD_CONTROLLER           LCD_CONTROLLER_ST7789
#endif

/* Some panels are mounted with the opposite horizontal scan order.
 * Set to 1 if text/graphics look left-right mirrored; 0 for normal panels. */
#ifndef LCD_PANEL_MIRROR_X
#define LCD_PANEL_MIRROR_X       1
#endif

/* Panel geometry ------------------------------------------------------------*/
#define LCD_NATIVE_WIDTH         240
#define LCD_NATIVE_HEIGHT        320
#define LCD_MAX_WIDTH            320

/* Pin assignment ------------------------------------------------------------*/
/* SPI2: PB13 = SCK, PB14 = MISO, PB15 = MOSI.
 * The bus itself is owned by spi_bus.c; the LCD only owns its CS/DC/RST/BL.
 * See spi_bus.h for the shared bus API and clock prescalers. */

/* Control pins on GPIOE.
 * PE6 is reserved for the OV7670 D7 (DCMI_D7), so the LCD backlight uses PE7. */
#define LCD_CTRL_GPIO_PORT              GPIOE
#define LCD_CTRL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOE_CLK_ENABLE()
#define LCD_CS_PIN                      GPIO_PIN_2
#define LCD_DC_PIN                      GPIO_PIN_4
#define LCD_RST_PIN                     GPIO_PIN_5
#define LCD_BL_PIN                      GPIO_PIN_7

#define LCD_CS_LOW()                    HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_RESET)
#define LCD_CS_HIGH()                   HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_DC_LOW()                    HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_RESET)
#define LCD_DC_HIGH()                   HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_DC_PIN, GPIO_PIN_SET)
#define LCD_RST_LOW()                   HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_RST_PIN, GPIO_PIN_RESET)
#define LCD_RST_HIGH()                  HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_RST_PIN, GPIO_PIN_SET)
#define LCD_BL_LOW()                    HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_BL_PIN, GPIO_PIN_RESET)
#define LCD_BL_HIGH()                   HAL_GPIO_WritePin(LCD_CTRL_GPIO_PORT, LCD_BL_PIN, GPIO_PIN_SET)

/* Set to 1 to use the minimal init sequence (SWRESET/SLPOUT/COLMOD/MADCTL/DISPON).
 * Set to 0 after the panel responds, to use the full controller init. */
#define LCD_USE_MINIMAL_INIT            1

/* RGB565 colors -------------------------------------------------------------*/
#define LCD_COLOR_BLACK                 0x0000U
#define LCD_COLOR_WHITE                 0xFFFFU
#define LCD_COLOR_RED                   0xF800U
#define LCD_COLOR_GREEN                 0x07E0U
#define LCD_COLOR_BLUE                  0x001FU
#define LCD_COLOR_YELLOW                0xFFE0U
#define LCD_COLOR_CYAN                  0x07FFU
#define LCD_COLOR_MAGENTA               0xF81FU
#define LCD_COLOR_GRAY                  0x8410U
#define LCD_COLOR_ORANGE                0xFD20U

#define LCD_COLOR565(r, g, b) \
  ((uint16_t)((((uint16_t)(r) & 0xF8U) << 8) | \
              (((uint16_t)(g) & 0xFCU) << 3) | \
              (((uint16_t)(b) & 0xF8U) >> 3)))

/* Public types --------------------------------------------------------------*/
typedef struct
{
  uint16_t Width;
  uint16_t Height;
  uint16_t Controller;
  uint8_t  Rotation;
} LCD_InfoTypeDef;

extern LCD_InfoTypeDef LCD_Info;

/* Debug information for J-Link / debugger inspection. ----------------------*/
#define LCD_ERROR_NONE                  0U
#define LCD_ERROR_SPI_INIT              1U
#define LCD_ERROR_SPI_TX                2U
#define LCD_ERROR_SPI_RX                3U

typedef struct
{
  volatile uint32_t Stage;          /* 1..8 = init progress, 0xEE = error    */
  volatile uint32_t ErrorCode;      /* LCD_ERROR_xxx                         */
  volatile uint32_t SpiTxCount;     /* Number of SPI transmissions           */
  volatile uint32_t SpiErrorCount;  /* Number of failed SPI transfers        */
  volatile int32_t  LastHalStatus;  /* Last HAL_SPI_xxx return value         */
  volatile uint16_t ReadID;         /* Best-effort display ID (needs MISO)   */
  volatile uint8_t  ReadIDRaw[4];   /* Raw bytes returned by ID command       */
  volatile uint32_t SpiCR1;         /* SPI2->CR1 snapshot                    */
  volatile uint32_t SpiSR;          /* SPI2->SR snapshot                     */
  volatile uint32_t GpioBMode;      /* GPIOB->MODER snapshot                 */
  volatile uint32_t GpioBAfr1;      /* GPIOB->AFR[1] snapshot                */
  volatile uint32_t GpioEMode;      /* GPIOE->MODER snapshot                 */
} LCD_DebugTypeDef;

extern volatile LCD_DebugTypeDef LCD_Debug;

/* Public functions ----------------------------------------------------------*/
void     LCD_Init(void);
void     LCD_Reset(void);
void     LCD_Backlight(uint8_t On);
void     LCD_SetRotation(uint8_t Rotation);

void     LCD_WriteCommand(uint8_t Command);
void     LCD_WriteData(uint8_t Data);

void     LCD_SetWindow(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1);
void     LCD_DrawPixel(uint16_t X, uint16_t Y, uint16_t Color);
void     LCD_FillRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color);
void     LCD_FillScreen(uint16_t Color);
void     LCD_DrawHLine(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Color);
void     LCD_DrawVLine(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Color);
void     LCD_DrawRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Color);
void     LCD_DrawLine(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t Color);
void     LCD_DrawImage(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image);
void     LCD_DrawImageScaled2x(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, const uint8_t *Image);

void     LCD_DrawChar(uint16_t X, uint16_t Y, char Character, uint16_t Color, uint16_t BackColor);
void     LCD_DrawString(uint16_t X, uint16_t Y, const char *String, uint16_t Color, uint16_t BackColor);
void     LCD_DrawString2x(uint16_t X, uint16_t Y, const char *String, uint16_t Color, uint16_t BackColor);

uint16_t LCD_ReadID(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_H */
