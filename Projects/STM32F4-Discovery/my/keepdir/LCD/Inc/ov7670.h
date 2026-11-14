#ifndef OV7670_H
#define OV7670_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#ifndef OV7670_ENABLE
#define OV7670_ENABLE             1
#endif

#define OV7670_WIDTH              160U
#define OV7670_HEIGHT             120U
#define OV7670_FRAME_BYTES        (OV7670_WIDTH * OV7670_HEIGHT * 2U)

#ifndef OV7670_MIRROR
#define OV7670_MIRROR             0
#endif

#ifndef OV7670_VFLIP
#define OV7670_VFLIP              0
#endif

#ifndef OV7670_COLOR_BAR
#define OV7670_COLOR_BAR          0
#endif

#ifndef OV7670_RGB565_SWAP
#define OV7670_RGB565_SWAP        0
#endif

typedef enum
{
  OV7670_OK = 0,
  OV7670_ERROR,
  OV7670_NOT_FOUND,
  OV7670_TIMEOUT,
  OV7670_NOT_INITIALIZED
} OV7670_StatusTypeDef;

OV7670_StatusTypeDef OV7670_Init(void);
OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout);
OV7670_StatusTypeDef OV7670_SetColorBar(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetMirror(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetFlip(uint8_t Enable);

uint16_t OV7670_GetWidth(void);
uint16_t OV7670_GetHeight(void);
uint8_t  OV7670_IsInitialized(void);

void OV7670_DCMI_IRQHandler(void);
void OV7670_DMA_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_H */
