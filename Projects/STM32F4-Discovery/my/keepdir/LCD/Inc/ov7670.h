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
#define OV7670_CAPTURE_LINE_BYTES (OV7670_WIDTH * 2U - 1U)

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

typedef struct
{
  volatile uint32_t Stage;
  volatile uint32_t FrameCount;
  volatile uint32_t PclkEdges;
  volatile uint32_t HrefEdges;
  volatile uint32_t VsyncEdges;
  volatile uint32_t DcmiSr;
  volatile uint32_t DcmiCr;
  volatile uint32_t DmaRemaining;
  volatile uint32_t FramePeriodMs;
  volatile uint16_t Aec;
  volatile uint16_t DummyRows;
  volatile uint16_t AdvRows;
  volatile uint8_t  Clkrc;
  volatile uint8_t  Dblv;
  volatile uint8_t  Com8;
  volatile uint8_t  Com9;
  volatile uint8_t  Com11;
  volatile uint8_t  Gain;
  volatile uint8_t  Vref;
  volatile uint8_t  NtCtrl;
  volatile uint32_t FrameOr;
  volatile uint32_t DataOr;
  volatile uint32_t DataChanges;
  volatile uint32_t LineBytes;
  volatile uint32_t LineCount;
  volatile uint8_t  RowShift;
  volatile uint8_t  Pid;
  volatile uint8_t  Ver;
  volatile uint8_t  PidReadOk;
  volatile uint8_t  SccbMode;
  volatile uint8_t  FirstByte;
  volatile uint8_t  LastByte;
  volatile uint8_t  CaptureStatus;
} OV7670_DebugTypeDef;

extern volatile OV7670_DebugTypeDef OV7670_Debug;

OV7670_StatusTypeDef OV7670_Init(void);
OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout);
OV7670_StatusTypeDef OV7670_StartCapture(uint8_t *Buffer);
OV7670_StatusTypeDef OV7670_WaitCapture(uint8_t *Buffer, uint32_t Timeout);
uint8_t *OV7670_GetScratchBuffer(void);
void     OV7670_AlignRows(uint8_t *Buffer);
OV7670_StatusTypeDef OV7670_SetColorBar(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetMirror(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetFlip(uint8_t Enable);

uint16_t OV7670_GetWidth(void);
uint16_t OV7670_GetHeight(void);
uint8_t  OV7670_IsInitialized(void);
void     OV7670_SetPolarityCombo(uint8_t Combo);
uint8_t  OV7670_GetPolarityCombo(void);
void     OV7670_SetFastCapture(uint8_t Enable);
void     OV7670_ResetFrameStats(void);

void OV7670_DCMI_IRQHandler(void);
void OV7670_DMA_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_H */
