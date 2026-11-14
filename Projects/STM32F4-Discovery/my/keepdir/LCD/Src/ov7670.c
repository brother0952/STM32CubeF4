#include "ov7670.h"
#include "ov7670_pins.h"

#if (OV7670_ENABLE != 0)

#define OV7670_SCCB_ADDR                 0x21U
#define OV7670_PID                       0x76U

#define OV7670_REG_GAIN                  0x00U
#define OV7670_REG_VREF                  0x03U
#define OV7670_REG_COM1                  0x04U
#define OV7670_REG_PID                   0x0AU
#define OV7670_REG_VER                   0x0BU
#define OV7670_REG_COM3                  0x0CU
#define OV7670_REG_COM4                  0x0DU
#define OV7670_REG_COM7                  0x12U
#define OV7670_REG_COM8                  0x13U
#define OV7670_REG_COM9                  0x14U
#define OV7670_REG_COM10                 0x15U
#define OV7670_REG_HSTART                0x17U
#define OV7670_REG_HSTOP                 0x18U
#define OV7670_REG_VSTART                0x19U
#define OV7670_REG_VSTOP                 0x1AU
#define OV7670_REG_MVFP                  0x1EU
#define OV7670_REG_HSYST                 0x30U
#define OV7670_REG_HSYEN                 0x31U
#define OV7670_REG_HREF                  0x32U
#define OV7670_REG_TSLB                  0x3AU
#define OV7670_REG_COM11                 0x3BU
#define OV7670_REG_COM13                 0x3DU
#define OV7670_REG_COM14                 0x3EU
#define OV7670_REG_COM15                 0x40U
#define OV7670_REG_AWBC1                 0x43U
#define OV7670_REG_AWBC2                 0x44U
#define OV7670_REG_AWBC3                 0x45U
#define OV7670_REG_AWBC4                 0x46U
#define OV7670_REG_AWBC5                 0x47U
#define OV7670_REG_AWBC6                 0x48U
#define OV7670_REG_MTX1                  0x4FU
#define OV7670_REG_MTX2                  0x50U
#define OV7670_REG_MTX3                  0x51U
#define OV7670_REG_MTX4                  0x52U
#define OV7670_REG_MTX5                  0x53U
#define OV7670_REG_MTX6                  0x54U
#define OV7670_REG_DBLV                  0x6BU
#define OV7670_REG_AWBCTR3               0x6CU
#define OV7670_REG_AWBCTR2               0x6DU
#define OV7670_REG_AWBCTR1               0x6EU
#define OV7670_REG_AWBCTR0               0x6FU
#define OV7670_REG_SCALING_XSC           0x70U
#define OV7670_REG_SCALING_YSC           0x71U
#define OV7670_REG_SCALING_DCWCTR        0x72U
#define OV7670_REG_SCALING_PCLK_DIV      0x73U
#define OV7670_REG_RGB444                0x8CU
#define OV7670_REG_SCALING_PCLK_DELAY    0xA2U
#define OV7670_REG_RSVD_B0               0xB0U

#define OV7670_MVFP_SUN                  0x02U
#define OV7670_MVFP_MIRROR               0x20U
#define OV7670_MVFP_FLIP                 0x10U

#define OV7670_COM7_RESET                0x80U
#define OV7670_COM7_RGB565               0x04U
#define OV7670_COM8_VALUE                0xC7U
#define OV7670_COM9_VALUE                0x6AU
#define OV7670_COM10_VALUE               0x00U
#define OV7670_COM11_VALUE               0x12U
#define OV7670_COM13_VALUE               0xC0U
#define OV7670_COM15_VALUE               0xD0U
#define OV7670_CLKRC_RGB565              0x01U

#if (OV7670_RGB565_SWAP != 0)
#define OV7670_COM3_VALUE                0x44U
#else
#define OV7670_COM3_VALUE                0x04U
#endif

#if (OV7670_MIRROR != 0)
#define OV7670_MIRROR_BITS               OV7670_MVFP_MIRROR
#else
#define OV7670_MIRROR_BITS               0x00U
#endif

#if (OV7670_VFLIP != 0)
#define OV7670_FLIP_BITS                 OV7670_MVFP_FLIP
#else
#define OV7670_FLIP_BITS                 0x00U
#endif

#define OV7670_MVFP_VALUE                (uint8_t)(OV7670_MVFP_SUN | OV7670_MIRROR_BITS | OV7670_FLIP_BITS)

typedef struct
{
  uint8_t Reg;
  uint8_t Value;
} OV7670_RegValTypeDef;

static const OV7670_RegValTypeDef s_defaultRegs[] =
{
  {OV7670_REG_TSLB,                  0x04U},
  {0x11U,                            0x00U},
  {OV7670_REG_DBLV,                  0x4AU},
  {OV7670_REG_COM10,                 OV7670_COM10_VALUE},
  {OV7670_REG_COM4,                  0x40U},
  {OV7670_REG_RSVD_B0,               0x84U},
  {OV7670_REG_COM11,                 OV7670_COM11_VALUE},
  {OV7670_REG_HSYST,                 0x00U},
  {OV7670_REG_HSYEN,                 0x00U},
  {OV7670_REG_MVFP,                  OV7670_MVFP_SUN},
  {OV7670_REG_AWBC1,                 0x0AU},
  {OV7670_REG_AWBC2,                 0xF0U},
  {OV7670_REG_AWBC3,                 0x34U},
  {OV7670_REG_AWBC4,                 0x58U},
  {OV7670_REG_AWBC5,                 0x28U},
  {OV7670_REG_AWBC6,                 0x3AU},
  {OV7670_REG_AWBCTR3,               0x0AU},
  {OV7670_REG_AWBCTR2,               0x55U},
  {OV7670_REG_AWBCTR1,               0x11U},
  {OV7670_REG_AWBCTR0,               0x9EU},
  {OV7670_REG_COM8,                  OV7670_COM8_VALUE},
  {0xFFU,                            0xFFU}
};

static const OV7670_RegValTypeDef s_rgb565Regs[] =
{
  {OV7670_REG_COM7,                  OV7670_COM7_RGB565},
  {OV7670_REG_RGB444,                0x00U},
  {OV7670_REG_COM1,                  0x00U},
  {OV7670_REG_COM15,                 OV7670_COM15_VALUE},
  {OV7670_REG_MVFP,                  OV7670_MVFP_VALUE},
  {OV7670_REG_COM9,                  OV7670_COM9_VALUE},
  {OV7670_REG_MTX1,                  0xB3U},
  {OV7670_REG_MTX2,                  0xB3U},
  {OV7670_REG_MTX3,                  0x00U},
  {OV7670_REG_MTX4,                  0x3DU},
  {OV7670_REG_MTX5,                  0xA7U},
  {OV7670_REG_MTX6,                  0xE4U},
  {OV7670_REG_COM13,                 OV7670_COM13_VALUE},
  {0xFFU,                            0xFFU}
};

static const OV7670_RegValTypeDef s_qqvgaRegs[] =
{
  {OV7670_REG_COM3,                  OV7670_COM3_VALUE},
  {OV7670_REG_COM14,                 0x1AU},
  {OV7670_REG_SCALING_XSC,           0x3AU},
  {OV7670_REG_SCALING_YSC,           0x35U},
  {OV7670_REG_SCALING_DCWCTR,        0x22U},
  {OV7670_REG_SCALING_PCLK_DIV,      0xF2U},
  {OV7670_REG_SCALING_PCLK_DELAY,    0x02U},
  {0xFFU,                            0xFFU}
};

static const OV7670_RegValTypeDef s_frameRegs[] =
{
  {OV7670_REG_HSTART,                0x13U},
  {OV7670_REG_HSTOP,                 0x01U},
  {OV7670_REG_HREF,                  0x36U},
  {OV7670_REG_VSTART,                0x03U},
  {OV7670_REG_VSTOP,                 0x7AU},
  {OV7670_REG_VREF,                  0x08U},
  {0xFFU,                            0xFFU}
};

static I2C_HandleTypeDef  s_i2c;
static DCMI_HandleTypeDef s_dcmi;
static DMA_HandleTypeDef  s_dma;
static volatile uint8_t   s_frameReady;
static volatile uint8_t   s_frameError;
static volatile uint8_t   s_initialized;

static void OV7670_GPIO_Init(void);
static void OV7670_PowerOn(void);
static OV7670_StatusTypeDef OV7670_SCCB_Init(void);
static OV7670_StatusTypeDef OV7670_WriteReg(uint8_t Reg, uint8_t Value);
static OV7670_StatusTypeDef OV7670_ReadReg(uint8_t Reg, uint8_t *Value);
static OV7670_StatusTypeDef OV7670_WriteArray(const OV7670_RegValTypeDef *Regs);
static OV7670_StatusTypeDef OV7670_Configure(void);
static OV7670_StatusTypeDef OV7670_DCMI_Init(void);

static void OV7670_GPIO_Init(void)
{
  GPIO_InitTypeDef gpio;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = GPIO_AF13_DCMI;

  gpio.Pin = OV7670_D0_PIN | OV7670_D1_PIN | OV7670_D2_PIN | OV7670_D3_PIN | OV7670_D4_PIN;
  HAL_GPIO_Init(GPIOC, &gpio);

  gpio.Pin = OV7670_D5_PIN;
  HAL_GPIO_Init(GPIOD, &gpio);

  gpio.Pin = OV7670_D6_PIN | OV7670_VSYNC_PIN;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = OV7670_D7_PIN;
  HAL_GPIO_Init(GPIOE, &gpio);

  gpio.Pin = OV7670_HREF_PIN | OV7670_PCLK_PIN;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_LOW;
  gpio.Alternate = 0U;

  gpio.Pin = OV7670_RESET_PIN | OV7670_PWDN_PIN;
  HAL_GPIO_Init(GPIOD, &gpio);

  HAL_GPIO_WritePin(OV7670_PWDN_GPIO_PORT, OV7670_PWDN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_PORT, OV7670_RESET_PIN, GPIO_PIN_RESET);

  gpio.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOD, &gpio);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);

  gpio.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOE, &gpio);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

static void OV7670_PowerOn(void)
{
  HAL_GPIO_WritePin(OV7670_PWDN_GPIO_PORT, OV7670_PWDN_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_PORT, OV7670_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(1U);
  HAL_GPIO_WritePin(OV7670_RESET_GPIO_PORT, OV7670_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(10U);
}

static OV7670_StatusTypeDef OV7670_SCCB_Init(void)
{
  GPIO_InitTypeDef gpio;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_I2C2_CLK_ENABLE();

  gpio.Pin       = OV7670_SCL_PIN | OV7670_SDA_PIN;
  gpio.Mode      = GPIO_MODE_AF_OD;
  gpio.Pull      = GPIO_PULLUP;
  gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &gpio);

  s_i2c.Instance             = I2C2;
  s_i2c.Init.ClockSpeed      = 100000U;
  s_i2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
  s_i2c.Init.OwnAddress1     = 0U;
  s_i2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  s_i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  s_i2c.Init.OwnAddress2     = 0U;
  s_i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  s_i2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&s_i2c) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_WriteReg(uint8_t Reg, uint8_t Value)
{
  if (HAL_I2C_Mem_Write(&s_i2c, (uint16_t)(OV7670_SCCB_ADDR << 1U), Reg,
                        I2C_MEMADD_SIZE_8BIT, &Value, 1U, 100U) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_ReadReg(uint8_t Reg, uint8_t *Value)
{
  if (HAL_I2C_Mem_Read(&s_i2c, (uint16_t)(OV7670_SCCB_ADDR << 1U), Reg,
                       I2C_MEMADD_SIZE_8BIT, Value, 1U, 100U) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_WriteArray(const OV7670_RegValTypeDef *Regs)
{
  while ((Regs->Reg != 0xFFU) || (Regs->Value != 0xFFU))
  {
    if (OV7670_WriteReg(Regs->Reg, Regs->Value) != OV7670_OK)
    {
      return OV7670_ERROR;
    }
    Regs++;
  }

  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_Configure(void)
{
  if (OV7670_WriteReg(OV7670_REG_COM7, OV7670_COM7_RESET) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  HAL_Delay(10U);

  if (OV7670_WriteArray(s_defaultRegs) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  HAL_Delay(30U);

  if (OV7670_WriteArray(s_rgb565Regs) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  HAL_Delay(30U);

  if (OV7670_WriteArray(s_qqvgaRegs) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_WriteArray(s_frameRegs) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_WriteReg(0x11U, OV7670_CLKRC_RGB565) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

#if (OV7670_COLOR_BAR != 0)
  if (OV7670_SetColorBar(1U) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
#endif

  HAL_Delay(100U);
  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_DCMI_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DCMI_CLK_ENABLE();

  s_dma.Instance                 = DMA2_Stream1;
  s_dma.Init.Channel             = DMA_CHANNEL_1;
  s_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  s_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
  s_dma.Init.MemInc              = DMA_MINC_ENABLE;
  s_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  s_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  s_dma.Init.Mode                = DMA_NORMAL;
  s_dma.Init.Priority            = DMA_PRIORITY_HIGH;
  s_dma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  s_dma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_HALFFULL;
  s_dma.Init.MemBurst            = DMA_MBURST_SINGLE;
  s_dma.Init.PeriphBurst         = DMA_PBURST_SINGLE;

  if (HAL_DMA_Init(&s_dma) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  __HAL_LINKDMA(&s_dcmi, DMA_Handle, s_dma);

  s_dcmi.Instance                 = DCMI;
  s_dcmi.Init.SynchroMode         = DCMI_SYNCHRO_HARDWARE;
  s_dcmi.Init.PCKPolarity         = DCMI_PCKPOLARITY_RISING;
  s_dcmi.Init.VSPolarity          = DCMI_VSPOLARITY_HIGH;
  s_dcmi.Init.HSPolarity          = DCMI_HSPOLARITY_LOW;
  s_dcmi.Init.CaptureRate         = DCMI_CR_ALL_FRAME;
  s_dcmi.Init.ExtendedDataMode    = DCMI_EXTEND_DATA_8B;
  s_dcmi.Init.JPEGMode            = DCMI_JPEG_DISABLE;

  if (HAL_DCMI_Init(&s_dcmi) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  __HAL_DCMI_CLEAR_FLAG(&s_dcmi, DCMI_FLAG_ERRRI | DCMI_FLAG_OVRRI);
  __HAL_DCMI_ENABLE_IT(&s_dcmi, DCMI_IT_ERR | DCMI_IT_OVR);

  HAL_NVIC_SetPriority(DCMI_IRQn, 5U, 0U);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5U, 0U);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

  return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_Init(void)
{
  uint8_t pid = 0U;

  if (s_initialized != 0U)
  {
    return OV7670_OK;
  }

  OV7670_GPIO_Init();
  OV7670_PowerOn();

  if (OV7670_SCCB_Init() != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  HAL_Delay(10U);

  if (OV7670_ReadReg(OV7670_REG_PID, &pid) != OV7670_OK)
  {
    return OV7670_NOT_FOUND;
  }

  if (pid != OV7670_PID)
  {
    return OV7670_NOT_FOUND;
  }

  if (OV7670_Configure() != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_DCMI_Init() != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  s_initialized = 1U;
  return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout)
{
  uint32_t start;

  if (s_initialized == 0U)
  {
    return OV7670_NOT_INITIALIZED;
  }

  if (Buffer == 0)
  {
    return OV7670_ERROR;
  }

  s_frameReady = 0U;
  s_frameError = 0U;

  __HAL_DCMI_CLEAR_FLAG(&s_dcmi, DCMI_FLAG_ERRRI | DCMI_FLAG_OVRRI);
  __HAL_DCMI_ENABLE_IT(&s_dcmi, DCMI_IT_ERR | DCMI_IT_OVR);

  if (HAL_DCMI_Start_DMA(&s_dcmi, DCMI_MODE_SNAPSHOT, (uint32_t)Buffer,
                         OV7670_FRAME_BYTES) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  start = HAL_GetTick();
  while ((s_frameReady == 0U) && (s_frameError == 0U))
  {
    if ((HAL_GetTick() - start) > Timeout)
    {
      (void)HAL_DCMI_Stop(&s_dcmi);
      return OV7670_TIMEOUT;
    }
  }

  if (s_frameError != 0U)
  {
    s_frameError = 0U;
    (void)HAL_DCMI_Stop(&s_dcmi);
    return OV7670_ERROR;
  }

  s_frameReady = 0U;
  (void)HAL_DCMI_Stop(&s_dcmi);
  return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_SetColorBar(uint8_t Enable)
{
  uint8_t value;

  if (s_initialized == 0U)
  {
    return OV7670_NOT_INITIALIZED;
  }

  if (OV7670_ReadReg(OV7670_REG_SCALING_XSC, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  value &= 0x7FU;
  if (OV7670_WriteReg(OV7670_REG_SCALING_XSC, value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_ReadReg(OV7670_REG_SCALING_YSC, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  if (Enable != 0U)
  {
    value |= 0x80U;
  }
  else
  {
    value &= 0x7FU;
  }

  return OV7670_WriteReg(OV7670_REG_SCALING_YSC, value);
}

OV7670_StatusTypeDef OV7670_SetMirror(uint8_t Enable)
{
  uint8_t value;

  if (s_initialized == 0U)
  {
    return OV7670_NOT_INITIALIZED;
  }

  if (OV7670_ReadReg(OV7670_REG_MVFP, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (Enable != 0U)
  {
    value |= OV7670_MVFP_MIRROR;
  }
  else
  {
    value &= (uint8_t)~OV7670_MVFP_MIRROR;
  }

  return OV7670_WriteReg(OV7670_REG_MVFP, value);
}

OV7670_StatusTypeDef OV7670_SetFlip(uint8_t Enable)
{
  uint8_t value;

  if (s_initialized == 0U)
  {
    return OV7670_NOT_INITIALIZED;
  }

  if (OV7670_ReadReg(OV7670_REG_MVFP, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (Enable != 0U)
  {
    value |= OV7670_MVFP_FLIP;
  }
  else
  {
    value &= (uint8_t)~OV7670_MVFP_FLIP;
  }

  return OV7670_WriteReg(OV7670_REG_MVFP, value);
}

uint16_t OV7670_GetWidth(void)
{
  return OV7670_WIDTH;
}

uint16_t OV7670_GetHeight(void)
{
  return OV7670_HEIGHT;
}

uint8_t OV7670_IsInitialized(void)
{
  return s_initialized;
}

void OV7670_DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&s_dcmi);
}

void OV7670_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&s_dma);
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  if (hdcmi == &s_dcmi)
  {
    s_frameReady = 1U;
  }
}

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
  if (hdcmi == &s_dcmi)
  {
    s_frameError = 1U;
  }
}

#else

OV7670_StatusTypeDef OV7670_Init(void)
{
  return OV7670_NOT_INITIALIZED;
}

OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout)
{
  (void)Buffer;
  (void)Timeout;
  return OV7670_NOT_INITIALIZED;
}

OV7670_StatusTypeDef OV7670_SetColorBar(uint8_t Enable)
{
  (void)Enable;
  return OV7670_NOT_INITIALIZED;
}

OV7670_StatusTypeDef OV7670_SetMirror(uint8_t Enable)
{
  (void)Enable;
  return OV7670_NOT_INITIALIZED;
}

OV7670_StatusTypeDef OV7670_SetFlip(uint8_t Enable)
{
  (void)Enable;
  return OV7670_NOT_INITIALIZED;
}

uint16_t OV7670_GetWidth(void)
{
  return 0U;
}

uint16_t OV7670_GetHeight(void)
{
  return 0U;
}

uint8_t OV7670_IsInitialized(void)
{
  return 0U;
}

void OV7670_DCMI_IRQHandler(void)
{
}

void OV7670_DMA_IRQHandler(void)
{
}

#endif
