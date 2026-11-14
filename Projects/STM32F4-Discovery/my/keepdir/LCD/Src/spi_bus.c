/**
  ******************************************************************************
  * @file    spi_bus.c
  * @brief   Shared SPI2 bus for the LCD and the SD card socket.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi_bus.h"
#include "sdcard_pins.h"

/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef s_hspi;
static uint8_t s_initialized;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configure the shared SPI2 GPIO pins.
  */
static void SPI_Bus_GpioInit(void)
{
  GPIO_InitTypeDef gpio;

  SPI_BUS_GPIO_CLK_ENABLE();

  gpio.Pin       = SPI_BUS_SCK_PIN | SPI_BUS_MISO_PIN | SPI_BUS_MOSI_PIN;
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = SPI_BUS_GPIO_AF;
  HAL_GPIO_Init(SPI_BUS_GPIO_PORT, &gpio);
}

/**
  * @brief  Configure SD_CS as an output and leave the card deselected.
  */
static void SDCard_CsInit(void)
{
  GPIO_InitTypeDef gpio;

  SDCARD_CS_GPIO_CLK_ENABLE();

  /* Set the output latch high before enabling the pin to avoid a low glitch. */
  SDCARD_CS_HIGH();

  gpio.Pin   = SDCARD_CS_PIN;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SDCARD_CS_GPIO_PORT, &gpio);

  SDCARD_CS_HIGH();
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the shared SPI2 bus and deselect the SD card.
  * @retval HAL status
  */
HAL_StatusTypeDef SPI_Bus_Init(void)
{
  if (s_initialized != 0U)
  {
    return HAL_OK;
  }

  SPI_Bus_GpioInit();
  SDCard_CsInit();
  SPI_BUS_CLK_ENABLE();

  s_hspi.Instance               = SPI_BUS_INSTANCE;
  s_hspi.Init.Mode              = SPI_MODE_MASTER;
  s_hspi.Init.Direction         = SPI_DIRECTION_2LINES;
  s_hspi.Init.DataSize          = SPI_DATASIZE_8BIT;
  s_hspi.Init.CLKPolarity       = SPI_POLARITY_LOW;
  s_hspi.Init.CLKPhase          = SPI_PHASE_1EDGE;
  s_hspi.Init.NSS               = SPI_NSS_SOFT;
  s_hspi.Init.BaudRatePrescaler = SPI_BUS_PRESCALER_LCD;
  s_hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  s_hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
  s_hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  s_hspi.Init.CRCPolynomial     = 7U;

  if (HAL_SPI_Init(&s_hspi) != HAL_OK)
  {
    return HAL_ERROR;
  }

  s_initialized = 1U;
  return HAL_OK;
}

/**
  * @brief  Return the shared SPI handle.
  */
SPI_HandleTypeDef *SPI_Bus_GetHandle(void)
{
  return &s_hspi;
}

/**
  * @brief  Change the shared bus clock.
  * @param  Prescaler: SPI_BAUDRATEPRESCALER_x
  * @retval HAL status
  */
HAL_StatusTypeDef SPI_Bus_SetPrescaler(uint32_t Prescaler)
{
  if (s_initialized == 0U)
  {
    return HAL_ERROR;
  }

  if (s_hspi.Init.BaudRatePrescaler == Prescaler)
  {
    return HAL_OK;
  }

  if (SPI_Bus_WaitIdle(100U) != HAL_OK)
  {
    return HAL_TIMEOUT;
  }

  __HAL_SPI_DISABLE(&s_hspi);
  s_hspi.Init.BaudRatePrescaler = Prescaler;

  if (HAL_SPI_Init(&s_hspi) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Wait until the SPI peripheral is no longer busy.
  * @param  Timeout: timeout in milliseconds, or HAL_MAX_DELAY
  * @retval HAL status
  */
HAL_StatusTypeDef SPI_Bus_WaitIdle(uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  if (s_initialized == 0U)
  {
    return HAL_ERROR;
  }

  while ((s_hspi.Instance->SR & SPI_SR_BSY) != 0U)
  {
    if (Timeout != HAL_MAX_DELAY)
    {
      if ((HAL_GetTick() - tickstart) > Timeout)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Blocking transmit on the shared bus.
  */
HAL_StatusTypeDef SPI_Bus_Transmit(const uint8_t *Data, uint16_t Size, uint32_t Timeout)
{
  HAL_StatusTypeDef status;

  if ((s_initialized == 0U) || ((Data == NULL) && (Size > 0U)))
  {
    return HAL_ERROR;
  }

  status = HAL_SPI_Transmit(&s_hspi, (uint8_t *)Data, Size, Timeout);
  if (status != HAL_OK)
  {
    return status;
  }

  return SPI_Bus_WaitIdle(Timeout);
}

/**
  * @brief  Blocking receive on the shared bus.
  */
HAL_StatusTypeDef SPI_Bus_Receive(uint8_t *Data, uint16_t Size, uint32_t Timeout)
{
  HAL_StatusTypeDef status;

  if ((s_initialized == 0U) || ((Data == NULL) && (Size > 0U)))
  {
    return HAL_ERROR;
  }

  status = HAL_SPI_Receive(&s_hspi, Data, Size, Timeout);
  if (status != HAL_OK)
  {
    return status;
  }

  return SPI_Bus_WaitIdle(Timeout);
}

/**
  * @brief  Blocking full-duplex transfer on the shared bus.
  */
HAL_StatusTypeDef SPI_Bus_TransmitReceive(const uint8_t *TxData, uint8_t *RxData,
                                          uint16_t Size, uint32_t Timeout)
{
  HAL_StatusTypeDef status;

  if ((s_initialized == 0U) || ((TxData == NULL) && (Size > 0U)) ||
      ((RxData == NULL) && (Size > 0U)))
  {
    return HAL_ERROR;
  }

  status = HAL_SPI_TransmitReceive(&s_hspi, (uint8_t *)TxData, RxData, Size, Timeout);
  if (status != HAL_OK)
  {
    return status;
  }

  return SPI_Bus_WaitIdle(Timeout);
}
