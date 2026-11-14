/**
  ******************************************************************************
  * @file    esp01_pins.h
  * @brief   ESP-01 / ESP-01S (8-pin ESP8266) pin reservation.
  *
  *          WIFI LOGIC IS NOT IMPLEMENTED YET.
  *          This file only reserves USART3, two DMA streams and four GPIOs so
  *          that the LCD, OV7670 and future AT-command driver do not conflict.
  *
  *          ESP-01 8-pin header signals (verify the module silkscreen; common
  *          order is GND, GPIO2, GPIO0, RXD / TXD, CH_PD, RST, VCC):
  *            VCC, GND, TX, RX, CH_PD/EN, RST, GPIO0, GPIO2
  ******************************************************************************
  */

#ifndef ESP01_PINS_H
#define ESP01_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* UART: USART3 on PD8/PD9, AF7.
 * PD8 = STM32 TX -> ESP RX
 * PD9 = STM32 RX <- ESP TX
 * Both pins are free on the STM32F4-Discovery P1 header. */
#define ESP01_UART                      USART3
#define ESP01_UART_CLK_ENABLE()         __HAL_RCC_USART3_CLK_ENABLE()
#define ESP01_UART_IRQn                 USART3_IRQn
#define ESP01_UART_IRQHandler           USART3_IRQHandler
#define ESP01_UART_GPIO_PORT            GPIOD
#define ESP01_UART_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOD_CLK_ENABLE()
#define ESP01_UART_GPIO_AF              GPIO_AF7_USART3
#define ESP01_TX_PIN                    GPIO_PIN_8
#define ESP01_RX_PIN                    GPIO_PIN_9

/* DMA: USART3 uses DMA1, leaving DMA2 free for DCMI.
 * TX = DMA1 Stream3 Channel4, RX = DMA1 Stream1 Channel4.
 * SPI2 TX for the LCD only needs DMA1 Stream4, so there is no conflict. */
#define ESP01_TX_DMA_STREAM             DMA1_Stream3
#define ESP01_TX_DMA_CHANNEL            DMA_CHANNEL_4
#define ESP01_TX_DMA_IRQn               DMA1_Stream3_IRQn
#define ESP01_TX_DMA_IRQHandler         DMA1_Stream3_IRQHandler
#define ESP01_RX_DMA_STREAM             DMA1_Stream1
#define ESP01_RX_DMA_CHANNEL            DMA_CHANNEL_4
#define ESP01_RX_DMA_IRQn               DMA1_Stream1_IRQn
#define ESP01_RX_DMA_IRQHandler         DMA1_Stream1_IRQHandler

/* Control pins on free GPIOE header pins PE8..PE11. */
#define ESP01_CTRL_GPIO_PORT            GPIOE
#define ESP01_CTRL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOE_CLK_ENABLE()

#define ESP01_EN_GPIO_PORT              GPIOE
#define ESP01_EN_PIN                    GPIO_PIN_8
#define ESP01_RST_GPIO_PORT             GPIOE
#define ESP01_RST_PIN                   GPIO_PIN_9
#define ESP01_GPIO0_GPIO_PORT           GPIOE
#define ESP01_GPIO0_PIN                 GPIO_PIN_10
#define ESP01_GPIO2_GPIO_PORT           GPIOE
#define ESP01_GPIO2_PIN                 GPIO_PIN_11

#define ESP01_EN_HIGH()                 HAL_GPIO_WritePin(ESP01_EN_GPIO_PORT, ESP01_EN_PIN, GPIO_PIN_SET)
#define ESP01_EN_LOW()                  HAL_GPIO_WritePin(ESP01_EN_GPIO_PORT, ESP01_EN_PIN, GPIO_PIN_RESET)
#define ESP01_RST_HIGH()                HAL_GPIO_WritePin(ESP01_RST_GPIO_PORT, ESP01_RST_PIN, GPIO_PIN_SET)
#define ESP01_RST_LOW()                 HAL_GPIO_WritePin(ESP01_RST_GPIO_PORT, ESP01_RST_PIN, GPIO_PIN_RESET)
#define ESP01_GPIO0_HIGH()              HAL_GPIO_WritePin(ESP01_GPIO0_GPIO_PORT, ESP01_GPIO0_PIN, GPIO_PIN_SET)
#define ESP01_GPIO0_LOW()               HAL_GPIO_WritePin(ESP01_GPIO0_GPIO_PORT, ESP01_GPIO0_PIN, GPIO_PIN_RESET)
#define ESP01_GPIO2_HIGH()              HAL_GPIO_WritePin(ESP01_GPIO2_GPIO_PORT, ESP01_GPIO2_PIN, GPIO_PIN_SET)
#define ESP01_GPIO2_LOW()               HAL_GPIO_WritePin(ESP01_GPIO2_GPIO_PORT, ESP01_GPIO2_PIN, GPIO_PIN_RESET)

/* Normal boot: EN=1, RST=1, GPIO0=1, GPIO2=1.
 * Flash boot: GPIO0=0 at reset.
 * Add external 10k pull-ups to 3.3 V on EN, RST, GPIO0 and GPIO2 so the module
 * boots correctly before the STM32 configures these pins. */
#define ESP01_BOOT_NORMAL()             do { \
                                          ESP01_EN_HIGH(); \
                                          ESP01_RST_HIGH(); \
                                          ESP01_GPIO0_HIGH(); \
                                          ESP01_GPIO2_HIGH(); \
                                        } while (0)

/* AT firmware default is 115200 8N1. */
#define ESP01_AT_BAUDRATE               115200U

/* Power notes:
 * - ESP-01 is 3.3 V only and is not 5 V tolerant.
 * - Peak current can reach ~300 mA. Use an external 3.3 V supply (>=500 mA)
 *   with a common ground; do not rely on the Discovery 3V rail.
 * - Place 100 uF + 0.1 uF decoupling capacitors near the module.
 * - When implementing: enable HAL_UART_MODULE_ENABLED and add
 *   stm32f4xx_hal_uart.c to the Keil project. Use the USART3 IDLE interrupt
 *   with DMA RX for variable-length AT responses. */

#ifdef __cplusplus
}
#endif

#endif /* ESP01_PINS_H */
