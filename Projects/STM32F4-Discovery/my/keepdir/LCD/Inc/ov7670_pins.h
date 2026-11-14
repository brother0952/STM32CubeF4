/**
  ******************************************************************************
  * @file    ov7670_pins.h
  * @brief   OV7670 (18-pin module) pin reservation for STM32F4-Discovery.
  *
  *          CAMERA LOGIC IS NOT IMPLEMENTED YET.
  *          This file only reserves the pins so that the LCD driver and the
  *          future DCMI/SCCB driver do not conflict.
  ******************************************************************************
  */

#ifndef OV7670_PINS_H
#define OV7670_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* DCMI data bus, 8-bit ------------------------------------------------*/
#define OV7670_D0_GPIO_PORT        GPIOC
#define OV7670_D0_PIN              GPIO_PIN_6
#define OV7670_D0_AF               GPIO_AF13_DCMI

#define OV7670_D1_GPIO_PORT        GPIOC
#define OV7670_D1_PIN              GPIO_PIN_7
#define OV7670_D1_AF               GPIO_AF13_DCMI

#define OV7670_D2_GPIO_PORT        GPIOC
#define OV7670_D2_PIN              GPIO_PIN_8
#define OV7670_D2_AF               GPIO_AF13_DCMI

#define OV7670_D3_GPIO_PORT        GPIOC
#define OV7670_D3_PIN              GPIO_PIN_9
#define OV7670_D3_AF               GPIO_AF13_DCMI

#define OV7670_D4_GPIO_PORT        GPIOC
#define OV7670_D4_PIN              GPIO_PIN_11
#define OV7670_D4_AF               GPIO_AF13_DCMI

#define OV7670_D5_GPIO_PORT        GPIOD
#define OV7670_D5_PIN              GPIO_PIN_3
#define OV7670_D5_AF               GPIO_AF13_DCMI

#define OV7670_D6_GPIO_PORT        GPIOB
#define OV7670_D6_PIN              GPIO_PIN_8
#define OV7670_D6_AF               GPIO_AF13_DCMI

#define OV7670_D7_GPIO_PORT        GPIOE
#define OV7670_D7_PIN              GPIO_PIN_6
#define OV7670_D7_AF               GPIO_AF13_DCMI

/* DCMI synchronization and pixel clock -------------------------------*/
#define OV7670_VSYNC_GPIO_PORT     GPIOB
#define OV7670_VSYNC_PIN           GPIO_PIN_7
#define OV7670_VSYNC_AF            GPIO_AF13_DCMI

#define OV7670_HREF_GPIO_PORT      GPIOA
#define OV7670_HREF_PIN            GPIO_PIN_4
#define OV7670_HREF_AF             GPIO_AF13_DCMI

#define OV7670_PCLK_GPIO_PORT      GPIOA
#define OV7670_PCLK_PIN            GPIO_PIN_6
#define OV7670_PCLK_AF             GPIO_AF13_DCMI

/* Camera master clock: PA8 / MCO1 (recommended 16 MHz from HSI) -----*/
#define OV7670_XCLK_GPIO_PORT      GPIOA
#define OV7670_XCLK_PIN            GPIO_PIN_8
#define OV7670_XCLK_AF             GPIO_AF0_MCO

/* SCCB (I2C-like control bus): I2C2 -------------------------------*/
#define OV7670_SCL_GPIO_PORT       GPIOB
#define OV7670_SCL_PIN             GPIO_PIN_10
#define OV7670_SCL_AF              GPIO_AF4_I2C2

#define OV7670_SDA_GPIO_PORT       GPIOB
#define OV7670_SDA_PIN             GPIO_PIN_11
#define OV7670_SDA_AF              GPIO_AF4_I2C2

/* Camera control GPIOs --------------------------------------------*/
#define OV7670_RESET_GPIO_PORT     GPIOD
#define OV7670_RESET_PIN           GPIO_PIN_0

#define OV7670_PWDN_GPIO_PORT      GPIOD
#define OV7670_PWDN_PIN            GPIO_PIN_1

#ifdef __cplusplus
}
#endif

#endif /* OV7670_PINS_H */
