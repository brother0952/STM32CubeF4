/**
  ******************************************************************************
  * @file    spi_bus.h
  * @brief   Shared SPI2 bus for the LCD and the SD card socket.
  *
  *          The LCD and the SD socket on the display module are two separate
  *          loads on the same SPI bus. They share SCK/MOSI/MISO and each has
  *          its own active-low chip select.
  *
  *          Bus mode: master, mode 0, 8-bit, MSB first, software NSS.
  ******************************************************************************
  */

#ifndef SPI_BUS_H
#define SPI_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* SPI2 pins: PB13 = SCK, PB14 = MISO, PB15 = MOSI. */
#define SPI_BUS_INSTANCE                SPI2
#define SPI_BUS_CLK_ENABLE()            __HAL_RCC_SPI2_CLK_ENABLE()
#define SPI_BUS_GPIO_PORT               GPIOB
#define SPI_BUS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPI_BUS_GPIO_AF                 GPIO_AF5_SPI2
#define SPI_BUS_SCK_PIN                 GPIO_PIN_13
#define SPI_BUS_MISO_PIN                GPIO_PIN_14
#define SPI_BUS_MOSI_PIN                GPIO_PIN_15

/* APB1 = 42 MHz.
 * 2  -> 21 MHz    (fast LCD mode; use 4 if the panel is unstable)
 * 4  -> 10.5 MHz  (SD fast mode after initialization)
 * 256 -> 164 kHz  (SD initialization, must be <= 400 kHz)
 */
#define SPI_BUS_PRESCALER_LCD           SPI_BAUDRATEPRESCALER_2
#define SPI_BUS_PRESCALER_SD_INIT       SPI_BAUDRATEPRESCALER_256
#define SPI_BUS_PRESCALER_SD_FAST       SPI_BAUDRATEPRESCALER_4

/* Initialize GPIO and SPI2, and leave the SD card deselected.
 * Safe to call more than once; later calls return HAL_OK immediately. */
HAL_StatusTypeDef SPI_Bus_Init(void);

/* Shared handle. Do not call HAL_SPI_Init() directly from a device driver. */
SPI_HandleTypeDef *SPI_Bus_GetHandle(void);

/* Change the bus clock. Call only while both device CS lines are high. */
HAL_StatusTypeDef SPI_Bus_SetPrescaler(uint32_t Prescaler);

/* Wait until the SPI peripheral is no longer busy. */
HAL_StatusTypeDef SPI_Bus_WaitIdle(uint32_t Timeout);

/* Blocking transfer helpers. The caller owns the device CS line. */
HAL_StatusTypeDef SPI_Bus_Transmit(const uint8_t *Data, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef SPI_Bus_Receive(uint8_t *Data, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef SPI_Bus_TransmitReceive(const uint8_t *TxData, uint8_t *RxData,
                                          uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif /* SPI_BUS_H */
