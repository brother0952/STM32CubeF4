/**
  ******************************************************************************
  * @file    sdcard_pins.h
  * @brief   microSD socket on the LCD module (SPI mode) pin reservation.
  *
  *          SD PROTOCOL / FATFS IS NOT IMPLEMENTED YET.
  *          This file only defines the chip select and documents the shared
  *          SPI2 wiring so the LCD and SD card do not conflict.
  *
  *          The SD socket has four signals and is NOT connected to the LCD
  *          controller on the module. Wire it to the shared SPI2 bus:
  *
  *            SD_SCK   -> PB13  (SPI2_SCK)
  *            SD_MOSI  -> PB15  (SPI2_MOSI)
  *            SD_MISO  -> PB14  (SPI2_MISO)
  *            SD_CS    -> PE12  (GPIO output, active low)
  *
  *          Power the socket from 3.3 V and GND. Add a 10k pull-up from
  *          SD_CS to 3.3 V and 10 uF + 0.1 uF decoupling near the socket.
  ******************************************************************************
  */

#ifndef SDCARD_PINS_H
#define SDCARD_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define SDCARD_CS_GPIO_PORT             GPIOE
#define SDCARD_CS_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOE_CLK_ENABLE()
#define SDCARD_CS_PIN                   GPIO_PIN_12

#define SDCARD_CS_LOW()                 HAL_GPIO_WritePin(SDCARD_CS_GPIO_PORT, SDCARD_CS_PIN, GPIO_PIN_RESET)
#define SDCARD_CS_HIGH()                HAL_GPIO_WritePin(SDCARD_CS_GPIO_PORT, SDCARD_CS_PIN, GPIO_PIN_SET)

/* Documentation only; the actual bus configuration lives in spi_bus.c. */
#define SDCARD_SPI_INSTANCE             SPI2
#define SDCARD_SCK_PIN                  GPIO_PIN_13
#define SDCARD_MISO_PIN                 GPIO_PIN_14
#define SDCARD_MOSI_PIN                 GPIO_PIN_15

/* Notes:
 * - SPI mode 0, 8-bit, MSB first.
 * - SD initialization must use <= 400 kHz; use SPI_BUS_PRESCALER_SD_INIT.
 * - After initialization, SPI_BUS_PRESCALER_SD_FAST (10.5 MHz) is safe.
 * - Keep LCD_CS and SD_CS high when the bus is idle.
 * - Only one device may drive MISO. If the LCD SDO is not tri-stated while
 *   LCD_CS is high, leave LCD MISO unconnected or add a 0-ohm jumper.
 */

#ifdef __cplusplus
}
#endif

#endif /* SDCARD_PINS_H */
