#include "ov7670.h"
#include "ov7670_pins.h"

#if (OV7670_ENABLE != 0)

#define OV7670_SCCB_ADDR                 0x21U
#define OV7670_PID                       0x76U
#define OV7670_FRAME_WORDS               (OV7670_FRAME_BYTES / 4U)

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
#define OV7670_REG_COM16                 0x41U
#define OV7670_REG_COM17                 0x42U
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
#define OV7670_COM7_RGB565               0x14U
#define OV7670_COM8_VALUE                0xC7U
#define OV7670_COM9_VALUE                0x6AU
#define OV7670_COM10_VALUE               0x00U
#define OV7670_COM16_VALUE               0x28U
#define OV7670_COM17_CBAR                0x08U
#define OV7670_COM11_VALUE               0x12U
#define OV7670_COM13_VALUE               0xC3U
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
  {0x3FU,                            0x00U},
  {0x75U,                            0x05U},
  {0x76U,                            0xE1U},
  {0x4CU,                            0x00U},
  {0x77U,                            0x01U},
  {OV7670_REG_COM16,                 OV7670_COM16_VALUE},
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
  {OV7670_REG_HREF,                  0xB6U},
  {OV7670_REG_VSTART,                0x02U},
  {OV7670_REG_VSTOP,                 0x7AU},
  {OV7670_REG_VREF,                  0x0AU},
  {0xFFU,                            0xFFU}
};

volatile OV7670_DebugTypeDef OV7670_Debug;

static I2C_HandleTypeDef  s_i2c;
static DCMI_HandleTypeDef s_dcmi;
static DMA_HandleTypeDef  s_dma;
static uint32_t           s_tuneFrame[OV7670_FRAME_BYTES / 4U];
static volatile uint8_t   s_frameReady;
static volatile uint8_t   s_frameError;
static volatile uint8_t   s_initialized;
static volatile uint8_t   s_sccbBitBang;
static volatile uint8_t   s_polarityCombo;
static volatile uint8_t   s_fastCapture;
static volatile uint32_t  s_lastFrameTick;

static void OV7670_GPIO_Init(void);
static void OV7670_PowerOn(void);
static OV7670_StatusTypeDef OV7670_SCCB_Init(void);
static void OV7670_SCCB_BitBangInit(void);
static void OV7670_SCCB_BusRecovery(void);
static void OV7670_SCCB_Delay(void);
static void OV7670_SCCB_Start(void);
static void OV7670_SCCB_Stop(void);
static void OV7670_SCCB_WriteByte(uint8_t Value);
static uint8_t OV7670_SCCB_ReadByte(void);
static OV7670_StatusTypeDef OV7670_WriteRegBitBang(uint8_t Reg, uint8_t Value);
static OV7670_StatusTypeDef OV7670_ReadRegBitBang(uint8_t Reg, uint8_t *Value);
static OV7670_StatusTypeDef OV7670_WriteReg(uint8_t Reg, uint8_t Value);
static OV7670_StatusTypeDef OV7670_ReadReg(uint8_t Reg, uint8_t *Value);
static OV7670_StatusTypeDef OV7670_WriteArray(const OV7670_RegValTypeDef *Regs);
static OV7670_StatusTypeDef OV7670_Configure(void);
static OV7670_StatusTypeDef OV7670_DCMI_Init(void);
static OV7670_StatusTypeDef OV7670_AutoTune(void);
static uint32_t OV7670_MeasurePclkFrequency(void);
static uint8_t OV7670_ReadDataBus(void);
static void OV7670_MeasureDataBus(void);
static void OV7670_MeasureGeometry(void);
static uint8_t OV7670_MeasureRowShift(const uint8_t *Buffer);
static void OV7670_MeasureSignals(void);

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

static void OV7670_SCCB_BitBangInit(void)
{
  GPIO_InitTypeDef gpio;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN | OV7670_SDA_PIN, GPIO_PIN_SET);

  gpio.Pin       = OV7670_SCL_PIN | OV7670_SDA_PIN;
  gpio.Mode      = GPIO_MODE_OUTPUT_OD;
  gpio.Pull      = GPIO_PULLUP;
  gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = 0U;
  HAL_GPIO_Init(GPIOB, &gpio);

  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN | OV7670_SDA_PIN, GPIO_PIN_SET);
  s_sccbBitBang = 1U;
  OV7670_SCCB_Delay();
  OV7670_SCCB_BusRecovery();
}

static void OV7670_SCCB_BusRecovery(void)
{
  uint8_t i;

  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  for (i = 0U; i < 9U; i++)
  {
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
    OV7670_SCCB_Delay();
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
    OV7670_SCCB_Delay();
  }
  OV7670_SCCB_Stop();
}

static void OV7670_SCCB_Delay(void)
{
  volatile uint32_t i;

  for (i = 0U; i < 800U; i++)
  {
    __NOP();
  }
}

static void OV7670_SCCB_Start(void)
{
  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_RESET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
  OV7670_SCCB_Delay();
}

static void OV7670_SCCB_Stop(void)
{
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_RESET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
}

static void OV7670_SCCB_WriteByte(uint8_t Value)
{
  uint8_t i;

  for (i = 0U; i < 8U; i++)
  {
    if ((Value & 0x80U) != 0U)
    {
      HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
    }
    else
    {
      HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_RESET);
    }
    Value <<= 1;
    OV7670_SCCB_Delay();
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
    OV7670_SCCB_Delay();
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
    OV7670_SCCB_Delay();
  }

  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
  OV7670_SCCB_Delay();
}

static uint8_t OV7670_SCCB_ReadByte(void)
{
  uint8_t value = 0U;
  uint8_t i;

  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  for (i = 0U; i < 8U; i++)
  {
    value <<= 1;
    OV7670_SCCB_Delay();
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
    OV7670_SCCB_Delay();
    if (HAL_GPIO_ReadPin(GPIOB, OV7670_SDA_PIN) == GPIO_PIN_SET)
    {
      value |= 1U;
    }
    HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
    OV7670_SCCB_Delay();
  }

  HAL_GPIO_WritePin(GPIOB, OV7670_SDA_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_SET);
  OV7670_SCCB_Delay();
  HAL_GPIO_WritePin(GPIOB, OV7670_SCL_PIN, GPIO_PIN_RESET);
  OV7670_SCCB_Delay();

  return value;
}

static OV7670_StatusTypeDef OV7670_WriteRegBitBang(uint8_t Reg, uint8_t Value)
{
  OV7670_SCCB_Start();
  OV7670_SCCB_WriteByte((uint8_t)(OV7670_SCCB_ADDR << 1U));
  OV7670_SCCB_WriteByte(Reg);
  OV7670_SCCB_WriteByte(Value);
  OV7670_SCCB_Stop();
  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_ReadRegBitBang(uint8_t Reg, uint8_t *Value)
{
  OV7670_SCCB_Start();
  OV7670_SCCB_WriteByte((uint8_t)(OV7670_SCCB_ADDR << 1U));
  OV7670_SCCB_WriteByte(Reg);
  OV7670_SCCB_Stop();
  OV7670_SCCB_Delay();

  OV7670_SCCB_Start();
  OV7670_SCCB_WriteByte((uint8_t)((OV7670_SCCB_ADDR << 1U) | 1U));
  *Value = OV7670_SCCB_ReadByte();
  OV7670_SCCB_Stop();
  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_WriteReg(uint8_t Reg, uint8_t Value)
{
  if (s_sccbBitBang != 0U)
  {
    return OV7670_WriteRegBitBang(Reg, Value);
  }

  if (HAL_I2C_Mem_Write(&s_i2c, (uint16_t)(OV7670_SCCB_ADDR << 1U), Reg,
                        I2C_MEMADD_SIZE_8BIT, &Value, 1U, 100U) != HAL_OK)
  {
    return OV7670_ERROR;
  }

  return OV7670_OK;
}

static OV7670_StatusTypeDef OV7670_ReadReg(uint8_t Reg, uint8_t *Value)
{
  if (s_sccbBitBang != 0U)
  {
    return OV7670_ReadRegBitBang(Reg, Value);
  }

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
  uint8_t value;

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
  value &= 0x7FU;
  if (OV7670_WriteReg(OV7670_REG_SCALING_YSC, value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_ReadReg(OV7670_REG_COM17, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  value |= OV7670_COM17_CBAR;
  if (OV7670_WriteReg(OV7670_REG_COM17, value) != OV7670_OK)
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
  s_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  s_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
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
  s_dcmi.Init.PCKPolarity         = DCMI_PCKPOLARITY_FALLING;
  s_dcmi.Init.VSPolarity          = DCMI_VSPOLARITY_HIGH;
  s_dcmi.Init.HSPolarity          = DCMI_HSPOLARITY_HIGH;
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

static uint32_t OV7670_MeasurePclkFrequency(void)
{
  GPIO_InitTypeDef gpio;
  uint32_t timerClock;
  uint32_t timeout;
  uint32_t firstCapture;
  uint32_t secondCapture;
  uint32_t delta;
  uint32_t frequency = 0U;

  gpio.Pin = OV7670_PCLK_PIN;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &gpio);

  __HAL_RCC_TIM3_CLK_ENABLE();

  TIM3->CR1 = 0U;
  TIM3->PSC = 0U;
  TIM3->ARR = 0xFFFFU;
  TIM3->CCMR1 = 0x0001U;
  TIM3->CCER = 0U;
  TIM3->SR = 0U;
  TIM3->CCER = 0x0001U;
  TIM3->CR1 = 0x0001U;

  timeout = HAL_GetTick();
  while (((TIM3->SR & 0x0002U) == 0U) && ((HAL_GetTick() - timeout) < 20U))
  {
  }

  if ((TIM3->SR & 0x0002U) != 0U)
  {
    firstCapture = TIM3->CCR1;
    TIM3->SR &= ~0x0002U;

    timeout = HAL_GetTick();
    while (((TIM3->SR & 0x0002U) == 0U) && ((HAL_GetTick() - timeout) < 20U))
    {
    }

    if ((TIM3->SR & 0x0002U) != 0U)
    {
      secondCapture = TIM3->CCR1;
      delta = (secondCapture - firstCapture) & 0xFFFFU;

      timerClock = HAL_RCC_GetPCLK1Freq();
      if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
      {
        timerClock *= 2U;
      }

      if (delta != 0U)
      {
        frequency = timerClock / delta;
      }
    }
  }

  TIM3->CR1 = 0U;
  TIM3->CCER = 0U;
  TIM3->SR = 0U;
  __HAL_RCC_TIM3_CLK_DISABLE();

  gpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &gpio);

  return frequency;
}

static uint8_t OV7670_ReadDataBus(void)
{
  uint8_t value = 0U;

  if ((OV7670_D0_GPIO_PORT->IDR & OV7670_D0_PIN) != 0U) { value |= 0x01U; }
  if ((OV7670_D1_GPIO_PORT->IDR & OV7670_D1_PIN) != 0U) { value |= 0x02U; }
  if ((OV7670_D2_GPIO_PORT->IDR & OV7670_D2_PIN) != 0U) { value |= 0x04U; }
  if ((OV7670_D3_GPIO_PORT->IDR & OV7670_D3_PIN) != 0U) { value |= 0x08U; }
  if ((OV7670_D4_GPIO_PORT->IDR & OV7670_D4_PIN) != 0U) { value |= 0x10U; }
  if ((OV7670_D5_GPIO_PORT->IDR & OV7670_D5_PIN) != 0U) { value |= 0x20U; }
  if ((OV7670_D6_GPIO_PORT->IDR & OV7670_D6_PIN) != 0U) { value |= 0x40U; }
  if ((OV7670_D7_GPIO_PORT->IDR & OV7670_D7_PIN) != 0U) { value |= 0x80U; }

  return value;
}

static void OV7670_MeasureDataBus(void)
{
  GPIO_InitTypeDef gpio;
  uint32_t start;
  uint8_t last;
  uint8_t now;

  gpio.Mode      = GPIO_MODE_INPUT;
  gpio.Pull      = GPIO_PULLDOWN;
  gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = 0U;

  gpio.Pin = OV7670_D0_PIN | OV7670_D1_PIN | OV7670_D2_PIN |
             OV7670_D3_PIN | OV7670_D4_PIN;
  HAL_GPIO_Init(GPIOC, &gpio);

  gpio.Pin = OV7670_D5_PIN;
  HAL_GPIO_Init(GPIOD, &gpio);

  gpio.Pin = OV7670_D6_PIN;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = OV7670_D7_PIN;
  HAL_GPIO_Init(GPIOE, &gpio);

  OV7670_Debug.DataOr = 0U;
  OV7670_Debug.DataChanges = 0U;
  last = OV7670_ReadDataBus();

  start = HAL_GetTick();
  while ((HAL_GetTick() - start) < 50U)
  {
    if ((GPIOA->IDR & OV7670_HREF_PIN) != 0U)
    {
      now = OV7670_ReadDataBus();
      OV7670_Debug.DataOr |= now;
      if (now != last)
      {
        OV7670_Debug.DataChanges++;
        last = now;
      }
    }
  }

  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Alternate = GPIO_AF13_DCMI;

  gpio.Pin = OV7670_D0_PIN | OV7670_D1_PIN | OV7670_D2_PIN |
             OV7670_D3_PIN | OV7670_D4_PIN;
  HAL_GPIO_Init(GPIOC, &gpio);

  gpio.Pin = OV7670_D5_PIN;
  HAL_GPIO_Init(GPIOD, &gpio);

  gpio.Pin = OV7670_D6_PIN;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = OV7670_D7_PIN;
  HAL_GPIO_Init(GPIOE, &gpio);
}

static void OV7670_MeasureGeometry(void)
{
  GPIO_InitTypeDef gpio;
  uint32_t start;
  uint32_t bestBytes = 0U;
  uint32_t first;
  uint32_t second;
  uint8_t i;

  gpio.Pin       = OV7670_PCLK_PIN;
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(GPIOA, &gpio);

  __HAL_RCC_TIM3_CLK_ENABLE();

  TIM3->CR1 = 0U;
  TIM3->PSC = 0U;
  TIM3->ARR = 0xFFFFU;
  TIM3->CCMR1 = 0x0001U;
  TIM3->CCER = 0x0001U;
  TIM3->SMCR = 0x0057U;
  TIM3->CNT = 0U;
  TIM3->SR = 0U;
  TIM3->CR1 = 0x0001U;

  for (i = 0U; i < 10U; i++)
  {
    start = HAL_GetTick();
    while (((GPIOA->IDR & OV7670_HREF_PIN) != 0U) &&
           ((HAL_GetTick() - start) < 20U))
    {
    }

    start = HAL_GetTick();
    while (((GPIOA->IDR & OV7670_HREF_PIN) == 0U) &&
           ((HAL_GetTick() - start) < 20U))
    {
    }

    if ((GPIOA->IDR & OV7670_HREF_PIN) == 0U)
    {
      break;
    }

    first = TIM3->CNT;

    start = HAL_GetTick();
    while (((GPIOA->IDR & OV7670_HREF_PIN) != 0U) &&
           ((HAL_GetTick() - start) < 20U))
    {
    }

    second = TIM3->CNT;

    if ((second - first) > bestBytes)
    {
      bestBytes = second - first;
    }
  }

  TIM3->CR1 = 0U;
  TIM3->CCER = 0U;
  TIM3->SMCR = 0U;
  TIM3->SR = 0U;
  __HAL_RCC_TIM3_CLK_DISABLE();

  gpio.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &gpio);

  OV7670_Debug.LineBytes = bestBytes;
}

static void OV7670_MeasureSignals(void)
{
  uint32_t start;
  uint8_t href;
  uint8_t vsync;
  uint8_t lastHref;
  uint8_t lastVsync;

  OV7670_Debug.PclkEdges = 0U;
  OV7670_Debug.HrefEdges = 0U;
  OV7670_Debug.VsyncEdges = 0U;
  OV7670_Debug.LineBytes = 0U;
  OV7670_Debug.LineCount = 0U;

  OV7670_Debug.PclkEdges = OV7670_MeasurePclkFrequency();
  OV7670_MeasureDataBus();
  OV7670_MeasureGeometry();

  lastHref = ((GPIOA->IDR & OV7670_HREF_PIN) != 0U) ? 1U : 0U;
  lastVsync = ((GPIOB->IDR & OV7670_VSYNC_PIN) != 0U) ? 1U : 0U;

  start = HAL_GetTick();
  while ((HAL_GetTick() - start) < 500U)
  {
    href = ((GPIOA->IDR & OV7670_HREF_PIN) != 0U) ? 1U : 0U;
    if (href != lastHref)
    {
      OV7670_Debug.HrefEdges++;
      lastHref = href;
    }

    vsync = ((GPIOB->IDR & OV7670_VSYNC_PIN) != 0U) ? 1U : 0U;
    if (vsync != lastVsync)
    {
      OV7670_Debug.VsyncEdges++;
      lastVsync = vsync;
    }
  }

  if (OV7670_Debug.VsyncEdges != 0U)
  {
    OV7670_Debug.LineCount = OV7670_Debug.HrefEdges / OV7670_Debug.VsyncEdges;
  }
}

OV7670_StatusTypeDef OV7670_Init(void)
{
  uint8_t pid = 0U;
  uint8_t ver = 0U;

  OV7670_Debug.Stage = 0U;
  OV7670_Debug.FrameCount = 0U;
  OV7670_Debug.PclkEdges = 0U;
  OV7670_Debug.HrefEdges = 0U;
  OV7670_Debug.VsyncEdges = 0U;
  OV7670_Debug.DcmiSr = 0U;
  OV7670_Debug.DcmiCr = 0U;
  OV7670_Debug.DmaRemaining = 0U;
  OV7670_Debug.FramePeriodMs = 0U;
  OV7670_Debug.FrameOr = 0U;
  OV7670_Debug.DataOr = 0U;
  OV7670_Debug.DataChanges = 0U;
  OV7670_Debug.LineBytes = 0U;
  OV7670_Debug.LineCount = 0U;
  OV7670_Debug.RowShift = 8U;
  OV7670_Debug.Pid = 0U;
  OV7670_Debug.Ver = 0U;
  OV7670_Debug.PidReadOk = 0U;
  OV7670_Debug.SccbMode = 0U;
  OV7670_Debug.FirstByte = 0U;
  OV7670_Debug.LastByte = 0U;
  OV7670_Debug.CaptureStatus = 0U;
  s_sccbBitBang = 0U;
  s_lastFrameTick = 0U;

  if (s_initialized != 0U)
  {
    OV7670_Debug.Stage = 0xA5U;
    return OV7670_OK;
  }

  OV7670_Debug.Stage = 1U;
  OV7670_GPIO_Init();
  OV7670_PowerOn();

  OV7670_Debug.Stage = 2U;
  if (OV7670_SCCB_Init() != OV7670_OK)
  {
    OV7670_SCCB_BitBangInit();
    OV7670_Debug.SccbMode = 1U;
  }

  HAL_Delay(10U);

  OV7670_Debug.Stage = 3U;
  if (OV7670_ReadReg(OV7670_REG_PID, &pid) != OV7670_OK)
  {
    OV7670_SCCB_BitBangInit();
    OV7670_Debug.SccbMode = 1U;
    if (OV7670_ReadReg(OV7670_REG_PID, &pid) != OV7670_OK)
    {
      OV7670_Debug.Stage = 0xE3U;
      return OV7670_NOT_FOUND;
    }
  }

  OV7670_Debug.Pid = pid;
  OV7670_Debug.PidReadOk = 1U;
  (void)OV7670_ReadReg(OV7670_REG_VER, &ver);
  OV7670_Debug.Ver = ver;

  if (pid != OV7670_PID)
  {
    OV7670_Debug.Stage = 0xE4U;
    return OV7670_NOT_FOUND;
  }

  OV7670_Debug.Stage = 4U;
  if (OV7670_Configure() != OV7670_OK)
  {
    OV7670_Debug.Stage = 0xE5U;
    return OV7670_ERROR;
  }

  {
    uint8_t d = 0U;
    uint8_t aecH = 0U;
    uint8_t aecHH = 0U;
    uint8_t dmL = 0U;
    uint8_t dmH = 0U;
    uint8_t advL = 0U;
    uint8_t advH = 0U;

    if (OV7670_ReadReg(0x11U, &d) == OV7670_OK) { OV7670_Debug.Clkrc = d; }
    if (OV7670_ReadReg(0x6BU, &d) == OV7670_OK) { OV7670_Debug.Dblv = d; }
    if (OV7670_ReadReg(0x13U, &d) == OV7670_OK) { OV7670_Debug.Com8 = d; }
    if (OV7670_ReadReg(0x14U, &d) == OV7670_OK) { OV7670_Debug.Com9 = d; }
    if (OV7670_ReadReg(0x3BU, &d) == OV7670_OK) { OV7670_Debug.Com11 = d; }
    if (OV7670_ReadReg(0x00U, &d) == OV7670_OK) { OV7670_Debug.Gain = d; }
    if (OV7670_ReadReg(0x03U, &d) == OV7670_OK) { OV7670_Debug.Vref = d; }
    if (OV7670_ReadReg(0xA4U, &d) == OV7670_OK) { OV7670_Debug.NtCtrl = d; }
    (void)OV7670_ReadReg(0x10U, &aecH);
    (void)OV7670_ReadReg(0x07U, &aecHH);
    (void)OV7670_ReadReg(0x92U, &dmL);
    (void)OV7670_ReadReg(0x93U, &dmH);
    (void)OV7670_ReadReg(0x2DU, &advL);
    (void)OV7670_ReadReg(0x2EU, &advH);
    OV7670_Debug.Aec = (uint16_t)(((uint16_t)(aecHH & 0x3FU) << 8) | aecH);
    OV7670_Debug.DummyRows = (uint16_t)(((uint16_t)dmH << 8) | dmL);
    OV7670_Debug.AdvRows = (uint16_t)(((uint16_t)advH << 8) | advL);
  }

  OV7670_Debug.Stage = 5U;
  if (OV7670_DCMI_Init() != OV7670_OK)
  {
    OV7670_Debug.Stage = 0xE6U;
    return OV7670_ERROR;
  }

  OV7670_Debug.Stage = 6U;
  OV7670_MeasureSignals();

  s_initialized = 1U;
  OV7670_Debug.Stage = 7U;
  (void)OV7670_AutoTune();

  OV7670_Debug.Stage = 0xA5U;
  return OV7670_OK;
}

void OV7670_AlignRows(uint8_t *Buffer)
{
  static uint8_t line[OV7670_WIDTH * 2U];
  const uint16_t srcBytes = (uint16_t)OV7670_CAPTURE_LINE_BYTES;
  const uint16_t dstBytes = (uint16_t)(OV7670_WIDTH * 2U);
  int32_t y;
  uint16_t i;

  if ((Buffer == 0) || (srcBytes >= dstBytes))
  {
    return;
  }

  for (y = (int32_t)OV7670_HEIGHT - 1; y >= 0; y--)
  {
    const uint8_t *src = &Buffer[(uint32_t)y * srcBytes];
    uint8_t *dst = &Buffer[(uint32_t)y * dstBytes];

    for (i = 0U; i < srcBytes; i++)
    {
      line[i] = src[i];
    }
    line[srcBytes] = src[srcBytes - 1U];
    for (i = 0U; i < dstBytes; i++)
    {
      dst[i] = line[i];
    }
  }
}

OV7670_StatusTypeDef OV7670_StartCapture(uint8_t *Buffer)
{
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
  OV7670_Debug.CaptureStatus = 1U;
  OV7670_Debug.FrameOr = 0U;

  __HAL_DCMI_CLEAR_FLAG(&s_dcmi, DCMI_FLAG_ERRRI | DCMI_FLAG_OVRRI);
  __HAL_DCMI_ENABLE_IT(&s_dcmi, DCMI_IT_ERR | DCMI_IT_OVR);

  if (HAL_DCMI_Start_DMA(&s_dcmi, DCMI_MODE_SNAPSHOT, (uint32_t)Buffer,
                         OV7670_FRAME_WORDS) != HAL_OK)
  {
    OV7670_Debug.DcmiSr = s_dcmi.Instance->SR;
    OV7670_Debug.DcmiCr = s_dcmi.Instance->CR;
    OV7670_Debug.CaptureStatus = 2U;
    return OV7670_ERROR;
  }

  return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_WaitCapture(uint8_t *Buffer, uint32_t Timeout)
{
  uint32_t start;

  if ((Buffer == 0) || (s_initialized == 0U))
  {
    return OV7670_ERROR;
  }

  start = HAL_GetTick();
  while ((s_frameReady == 0U) && (s_frameError == 0U))
  {
    if (__HAL_DMA_GET_COUNTER(&s_dma) == 0U)
    {
      s_frameReady = 1U;
      break;
    }

    if ((HAL_GetTick() - start) > Timeout)
    {
      OV7670_Debug.DcmiSr = s_dcmi.Instance->SR;
      OV7670_Debug.DcmiCr = s_dcmi.Instance->CR;
      OV7670_Debug.DmaRemaining = __HAL_DMA_GET_COUNTER(&s_dma);
      (void)HAL_DCMI_Stop(&s_dcmi);
      OV7670_Debug.FramePeriodMs = HAL_GetTick() - start;
      s_lastFrameTick = 0U;
      OV7670_Debug.CaptureStatus = 3U;
      return OV7670_TIMEOUT;
    }
  }

  if (s_frameError != 0U)
  {
    OV7670_Debug.DcmiSr = s_dcmi.Instance->SR;
    OV7670_Debug.DcmiCr = s_dcmi.Instance->CR;
    OV7670_Debug.DmaRemaining = __HAL_DMA_GET_COUNTER(&s_dma);
    s_frameError = 0U;
    (void)HAL_DCMI_Stop(&s_dcmi);
    OV7670_Debug.CaptureStatus = 4U;
    return OV7670_ERROR;
  }

  s_frameReady = 0U;
  (void)HAL_DCMI_Stop(&s_dcmi);
  {
    uint32_t now = HAL_GetTick();

    if (s_lastFrameTick != 0U)
    {
      OV7670_Debug.FramePeriodMs = now - s_lastFrameTick;
    }
    s_lastFrameTick = now;
  }
  OV7670_Debug.FrameCount++;
  if (OV7670_Debug.LineBytes == OV7670_CAPTURE_LINE_BYTES)
  {
    OV7670_AlignRows(Buffer);
  }
  OV7670_Debug.FirstByte = Buffer[0];
  OV7670_Debug.LastByte = Buffer[OV7670_FRAME_BYTES - 1U];

  if (s_fastCapture == 0U)
  {
    uint32_t i;
    uint32_t orAll = 0U;
    for (i = 0U; i < OV7670_FRAME_BYTES; i++)
    {
      orAll |= Buffer[i];
    }
    OV7670_Debug.FrameOr = orAll;

    OV7670_Debug.RowShift = OV7670_MeasureRowShift(Buffer);
  }

  OV7670_Debug.CaptureStatus = 5U;
  return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout)
{
  OV7670_StatusTypeDef status = OV7670_StartCapture(Buffer);

  if (status != OV7670_OK)
  {
    return status;
  }

  return OV7670_WaitCapture(Buffer, Timeout);
}

static uint8_t OV7670_MeasureRowShift(const uint8_t *Buffer)
{
  const uint16_t lineBytes = (uint16_t)(OV7670_WIDTH * 2U);
  int16_t bestShift = 0;
  uint32_t bestSad = 0xFFFFFFFFU;
  int16_t shift;
  uint16_t i;
  int32_t j;
  uint32_t sad;
  uint8_t a;
  uint8_t b;

  for (shift = -4; shift <= 4; shift++)
  {
    sad = 0U;
    for (i = 0U; i < lineBytes; i++)
    {
      j = (int32_t)i + shift;
      if ((j < 0) || (j >= (int32_t)lineBytes))
      {
        continue;
      }
      a = Buffer[i];
      b = Buffer[lineBytes + (uint16_t)j];
      sad += (a > b) ? (uint32_t)(a - b) : (uint32_t)(b - a);
    }
    if (sad < bestSad)
    {
      bestSad = sad;
      bestShift = shift;
    }
  }

  return (uint8_t)(bestShift + 8);
}

void OV7670_SetPolarityCombo(uint8_t Combo)
{
  static const uint32_t pck[2] = {DCMI_PCKPOLARITY_RISING, DCMI_PCKPOLARITY_FALLING};
  static const uint32_t hsp[2] = {DCMI_HSPOLARITY_HIGH, DCMI_HSPOLARITY_LOW};
  static const uint32_t vsp[2] = {DCMI_VSPOLARITY_HIGH, DCMI_VSPOLARITY_LOW};
  uint32_t cr;

  if (s_initialized == 0U)
  {
    return;
  }

  Combo &= 7U;
  cr = s_dcmi.Instance->CR;
  cr &= ~(DCMI_CR_PCKPOL | DCMI_CR_HSPOL | DCMI_CR_VSPOL);
  cr |= pck[Combo & 1U] | hsp[(Combo >> 1U) & 1U] | vsp[(Combo >> 2U) & 1U];
  s_dcmi.Instance->CR = cr;
  s_dcmi.Init.PCKPolarity = pck[Combo & 1U];
  s_dcmi.Init.HSPolarity = hsp[(Combo >> 1U) & 1U];
  s_dcmi.Init.VSPolarity = vsp[(Combo >> 2U) & 1U];
  s_polarityCombo = Combo;
}

uint8_t OV7670_GetPolarityCombo(void)
{
  return s_polarityCombo;
}

void OV7670_SetFastCapture(uint8_t Enable)
{
  s_fastCapture = (Enable != 0U) ? 1U : 0U;
}

void OV7670_ResetFrameStats(void)
{
  s_lastFrameTick = 0U;
  OV7670_Debug.FramePeriodMs = 0U;
}

uint8_t *OV7670_GetScratchBuffer(void)
{
  return (uint8_t *)s_tuneFrame;
}

static OV7670_StatusTypeDef OV7670_AutoTune(void)
{
  static const uint32_t pck[2] = {DCMI_PCKPOLARITY_RISING, DCMI_PCKPOLARITY_FALLING};
  static const uint32_t hsp[2] = {DCMI_HSPOLARITY_HIGH, DCMI_HSPOLARITY_LOW};
  static const uint32_t vsp[2] = {DCMI_VSPOLARITY_HIGH, DCMI_VSPOLARITY_LOW};
  uint8_t p;
  uint8_t h;
  uint8_t v;

  if (s_initialized == 0U)
  {
    return OV7670_NOT_INITIALIZED;
  }

  for (v = 0U; v < 2U; v++)
  {
    for (h = 0U; h < 2U; h++)
    {
      for (p = 0U; p < 2U; p++)
      {
        uint32_t cr = s_dcmi.Instance->CR;

        cr &= ~(DCMI_CR_PCKPOL | DCMI_CR_HSPOL | DCMI_CR_VSPOL);
        cr |= pck[p] | hsp[h] | vsp[v];
        s_dcmi.Instance->CR = cr;
        s_dcmi.Init.PCKPolarity = pck[p];
        s_dcmi.Init.HSPolarity = hsp[h];
        s_dcmi.Init.VSPolarity = vsp[v];

        (void)OV7670_Capture((uint8_t *)s_tuneFrame, 100U);

        if ((OV7670_Debug.CaptureStatus == 5U) && (OV7670_Debug.FrameOr != 0U))
        {
          s_polarityCombo = (uint8_t)(p | (uint8_t)(h << 1U) | (uint8_t)(v << 2U));
          return OV7670_OK;
        }
      }
    }
  }

  OV7670_Debug.CaptureStatus = 7U;
  return OV7670_ERROR;
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
  value &= 0x7FU;
  if (OV7670_WriteReg(OV7670_REG_SCALING_YSC, value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }

  if (OV7670_ReadReg(OV7670_REG_COM17, &value) != OV7670_OK)
  {
    return OV7670_ERROR;
  }
  if (Enable != 0U)
  {
    value |= OV7670_COM17_CBAR;
  }
  else
  {
    value &= (uint8_t)~OV7670_COM17_CBAR;
  }

  return OV7670_WriteReg(OV7670_REG_COM17, value);
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

void OV7670_AlignRows(uint8_t *Buffer)
{
  (void)Buffer;
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
