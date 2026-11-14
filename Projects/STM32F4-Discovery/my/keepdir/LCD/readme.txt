240x320 SPI LCD 驱动（STM32F4-Discovery / STM32F407VGT6）
==========================================================

一、LCD 接线（管脚分配）
----------------------------------------------------------
LCD 模块引脚     STM32F4-Discovery 引脚     说明
VCC              3V                          3.3V 电源
GND              GND                         地
SCK              PB13                        SPI2_SCK
SDI (MOSI)       PB15                        SPI2_MOSI
SDO (MISO)       PB14                        SPI2_MISO（只写屏可不接，读 ID 时需要）
CS               PE2                         片选，低有效
D/C              PE4                         命令/数据：0=命令，1=数据
RESET            PE5                         复位，低有效
LED (背光)       PE7                         高电平点亮（PE6 已预留给 OV7670 D7）

说明：
1. 使用 SPI2（PB13/PB14/PB15），没有占用板载 LIS3DSH 的 SPI1。
2. PE2/PE4/PE5/PE7 是扩展排针上的空闲 IO。
3. 背光 LED 如果电流较大（超过 GPIO 的驱动能力），请用三极管/MOS 管驱动，
   GPIO 只做控制信号，不要直接给背光供电。

二、OV7670 摄像头（QQVGA 160x120 RGB565）
----------------------------------------------------------
已实现 OV7670 驱动、SCCB 配置和 DCMI 快照预览。
默认 OV7670_ENABLE=1；把 Inc/ov7670.h 中的 OV7670_ENABLE 改为 0
即可快速关闭摄像头，恢复纯 LCD 测试程序。

OV7670 信号      STM32 引脚    说明
3.3V             3V            摄像头电源（请确认模块工作电压）
GND              GND
SCL              PB10          SCCB 时钟（I2C2_SCL，也可软件模拟）
SDA              PB11          SCCB 数据（I2C2_SDA，也可软件模拟）
VSYNC            PB7           DCMI_VSYNC
HREF             PA4           DCMI_HSYNC
PCLK             PA6           DCMI_PIXCLK
XCLK             PA8           MCO1，建议输出 16 MHz
D0               PC6           DCMI_D0
D1               PC7           DCMI_D1
D2               PC8           DCMI_D2
D3               PC9           DCMI_D3
D4               PC11          DCMI_D4
D5               PD3           DCMI_D5
D6               PB8           DCMI_D6
D7               PE6           DCMI_D7
RESET            PD0           GPIO 输出，低有效
PWDN             PD1           GPIO 输出，高有效

注意事项：
1. 18-pin OV7670 模块的引脚顺序可能不同，请以模块丝印为准。
2. PA6/PCLK 与板载 LIS3DSH 的 SPI1_MISO 共用。以后使用摄像头前，
   需要把 LIS3DSH 片选 PE3 拉高（取消选中），避免传感器驱动 PA6。
3. PA4/HREF 与板载 CS43L22 音频 DAC 的 I2S3_WS 共用；PC7/D1 与
   I2S3_MCK 共用。使用摄像头时音频输出不可用（可将 PD4 拉低让
   CS43L22 保持复位）。
4. PB10/SCL 与板载 MP45DT02 麦克风的 I2S2_CK 共用；使用 SCCB 时
   麦克风不可用。
5. SCCB 需要上拉电阻（多数模块已自带，若没有请在 SCL/SDA 上加 4.7k 到 3.3V）。
6. OV7670 与 LCD 背光同时工作时可能超过板载 3V 稳压器的电流能力，
   建议摄像头单独用 3.3V 供电，并与开发板共地。
7. 摄像头驱动在 Inc/ov7670.h / Src/ov7670.c，引脚定义在 Inc/ov7670_pins.h。
8. DCMI 使用 8-bit 硬件同步、DMA2 Stream1 Channel1、DCMI_MODE_SNAPSHOT；
   QQVGA RGB565 一帧 38400 字节，DMA 目标缓冲区位于 SRAM。
9. 预览时 LCD 临时切到旋转 1（320x240），160x120 图像 2x 放大后显示；
   退出预览后恢复旋转 2。
10. 如果颜色/字节顺序明显异常，把 Inc/ov7670.h 中的 OV7670_RGB565_SWAP
    改为 1；如果图像左右或上下反了，调 OV7670_MIRROR / OV7670_VFLIP。
11. 如果摄像头未接或 ID 读不到，程序显示 "OV7670 not found" 后回到 LCD 测试；
    调试摄像头时可把 OV7670_COLOR_BAR 设为 1 输出彩条。

三、ESP-01 / ESP8266 WiFi 模块引脚预留（8 pin，暂未实现）
----------------------------------------------------------
当前只预留引脚，不初始化 USART3，也不影响 LCD / OV7670。
模块 8 pin 信号：VCC、GND、TX、RX、CH_PD/EN、RST、GPIO0、GPIO2
（不同批次引脚顺序可能不同，请以模块丝印为准）。

ESP-01 信号      STM32 引脚    说明
VCC              3.3V          必须 3.3V；峰值约 300mA，建议外部 3.3V/≥500mA 供电
GND              GND           与开发板共地
TX               PD9           ESP TX -> STM32 USART3_RX
RX               PD8           STM32 USART3_TX -> ESP RX
CH_PD/EN         PE8           高电平使能；建议外部 10k 上拉到 3.3V
RST              PE9           低电平复位；建议外部 10k 上拉到 3.3V
GPIO0            PE10          正常启动为高；拉低复位进入 Flash 下载模式
GPIO2            PE11          正常启动为高；建议外部 10k 上拉到 3.3V

说明：
1. USART3 使用 PD8/PD9（AF7），默认 115200 8N1；PD8/PD9 在 P1 排针上为空闲 IO。
2. USART3 使用 DMA1：TX = DMA1 Stream3 Channel4，RX = DMA1 Stream1 Channel4；
   DCMI 使用 DMA2 Stream1，因此不冲突。
3. PE8..PE11 在 P1 排针上为空闲 IO，与 LCD（PE2/PE4/PE5/PE7）、
   OV7670（PE6）均不冲突。
4. 建议在 EN/RST/GPIO0/GPIO2 上加 10k 上拉，保证 STM32 初始化前模块正常启动。
5. ESP-01 为 3.3V 器件，不兼容 5V；VCC 附近加 100uF + 0.1uF 去耦。
6. 引脚定义在 Inc/esp01_pins.h，当前仅作预留，不参与编译逻辑。
7. 实现 AT 指令时：使能 HAL_UART_MODULE_ENABLED，并把 stm32f4xx_hal_uart.c
   加入 Keil 工程；建议 USART3 RX 使用 DMA + IDLE 中断接收不定长 AT 响应。

四、支持的 LCD 控制器
----------------------------------------------------------
常见 240x320 SPI 屏一般是 ILI9341 或 ST7789。
当前工程默认使用 ST7789。如果初始化后屏幕一直白屏/黑屏/花屏，
可以把 Inc/lcd.h 中的：

    #define LCD_CONTROLLER           LCD_CONTROLLER_ST7789

改成 ILI9341，然后重新编译下载。

如果颜色反了（红蓝互换），可以在 lcd.c 中调整 LCD_MADCTL_BGR：
    ILI9341 和 ST7789 都使用 0x08。ST7789 原来默认 0x00，会导致红蓝互换，现已修正。
当前 ST7789 默认旋转 2，并启用了 lcd.h 中的 LCD_PANEL_MIRROR_X（水平镜像修正），
最终 MADCTL=0xC8 ^ 0x40 = 0x88。
如果图像方向不对，可调用 LCD_SetRotation(0..3) 切换方向。
如果文字左右镜像反了，把 LCD_PANEL_MIRROR_X 设为 0；如果仍然镜像，设为 1。

五、文件说明
----------------------------------------------------------
Inc/lcd.h           LCD 控制脚定义、颜色定义、API 声明
Inc/spi_bus.h       LCD/SD 共用 SPI2 总线接口
Inc/sdcard_pins.h   microSD 卡座引脚预留（仅定义，暂不实现 SD/FatFS 逻辑）
Inc/ov7670.h        OV7670 开关宏、尺寸、配置宏和 API
Inc/ov7670_pins.h   OV7670 DCMI/SCCB/控制引脚定义
Inc/esp01_pins.h    ESP-01 引脚预留（仅定义，暂不实现 WiFi 逻辑）
Src/lcd.c           SPI2 + ILI9341/ST7789 驱动（使用 spi_bus）
Src/spi_bus.c       SPI2 总线初始化、分频切换、阻塞收发
Src/ov7670.c        OV7670 SCCB 初始化、DCMI/DMA 快照采集
Src/main.c          LCD 测试程序 + 摄像头预览（由 OV7670_ENABLE 控制）
Utilities/Fonts/font8.c   5x8 ASCII 字体（来自 STM32CubeF4）

六、主要 API
----------------------------------------------------------
void LCD_Init(void);
void LCD_SetRotation(uint8_t Rotation);           // 0..3
void LCD_FillScreen(uint16_t Color);
void LCD_FillRect(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t Color);
void LCD_DrawPixel(uint16_t X, uint16_t Y, uint16_t Color);
void LCD_DrawLine(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t Color);
void LCD_DrawRect(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t Color);
void LCD_DrawString(uint16_t X, uint16_t Y, const char *String,
                    uint16_t Color, uint16_t BackColor);

void LCD_DrawImage(uint16_t X, uint16_t Y, uint16_t W, uint16_t H,
                   const uint8_t *Image);
void LCD_DrawImageScaled2x(uint16_t X, uint16_t Y, uint16_t W, uint16_t H,
                           const uint8_t *Image);

颜色使用 RGB565，可用 LCD_COLOR565(r, g, b) 或预定义颜色：
LCD_COLOR_BLACK / WHITE / RED / GREEN / BLUE / YELLOW / CYAN / MAGENTA / GRAY / ORANGE

OV7670 API（Inc/ov7670.h）：
OV7670_StatusTypeDef OV7670_Init(void);
OV7670_StatusTypeDef OV7670_Capture(uint8_t *Buffer, uint32_t Timeout);
OV7670_StatusTypeDef OV7670_SetColorBar(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetMirror(uint8_t Enable);
OV7670_StatusTypeDef OV7670_SetFlip(uint8_t Enable);

OV7670_Capture 的 Buffer 至少要有 OV7670_FRAME_BYTES（160*120*2）字节，
数据格式为 RGB565，高字节在前；LCD_DrawImage* 直接按这个格式显示。

七、调试建议
----------------------------------------------------------
1. 先确认背光 LED 已点亮（PE7 高电平）。
2. 测量 RESET 上电时序：先低 20ms，再高，延时 120ms。
3. 如果白屏：确认控制器型号，必要时在 ILI9341 / ST7789 之间切换。
4. 如果花屏/错位：降低 SPI 速度，把 spi_bus.h 中
   SPI_BUS_PRESCALER_LCD 改成 SPI_BAUDRATEPRESCALER_8。
5. 如果红蓝反了：调整 LCD_MADCTL_BGR。
6. 如果文字左右镜像：调整 lcd.h 中的 LCD_PANEL_MIRROR_X（当前为 1）。
7. 如果完全不亮：检查 VCC/GND、CS/DC/RESET 接线和背光供电。
8. 摄像头不显示：先确认 OV7670_ENABLE=1，再检查 PA8 是否有 16MHz XCLK、
   PB10/PB11 是否有上拉、PD0/PD1 电平和 3.3V 供电。
9. 摄像头有数据但花屏/错位：检查 DCMI 极性（VSYNC/HREF/PCLK）和
   SCCB 是否读到 PID=0x76；可先把 OV7670_COLOR_BAR 设为 1 验证数据通路。
10. 摄像头颜色异常：尝试 OV7670_RGB565_SWAP=1，或调整 OV7670_MIRROR /
    OV7670_VFLIP。

八、microSD 卡座（SPI 复用，暂未实现 FatFS）
----------------------------------------------------------
屏幕模块上的 SD 卡座只有 4 个信号，且与 LCD 的 SPI 在模块内部没有连通。
建议复用 LCD 的 SPI2，只给 SD 单独分配 CS：

SD 卡座信号      STM32 引脚    说明
SD_SCK           PB13          SPI2_SCK，与 LCD 共用
SD_MOSI          PB15          SPI2_MOSI，与 LCD 共用
SD_MISO          PB14          SPI2_MISO，与 LCD 共用
SD_CS            PE12          GPIO 输出，低有效；建议 10k 上拉到 3.3V
3.3V             3.3V          SD 卡为 3.3V 器件，不要接 5V
GND              GND           与开发板共地

说明：
1. SPI2 是总线，LCD 和 SD 共用 SCK/MOSI/MISO，靠各自的 CS 选择。
   LCD_CS=PE2，SD_CS=PE12；同一时刻只能有一个 CS 为低。
2. 总线模式为 mode 0、8-bit、MSB first，LCD 和 SD 都兼容。
   SD 初始化必须 <=400kHz，使用 SPI_BUS_PRESCALER_SD_INIT；
   初始化完成后可切到 SPI_BUS_PRESCALER_SD_FAST（10.5MHz）。
3. 切换分频前先等待 SPI 空闲，并确保 LCD_CS 和 SD_CS 都为高。
4. SD 卡座 VCC 附近建议加 10uF + 0.1uF 去耦；SCK/MOSI 可预留 22~33Ω 串联电阻。
5. 共用 MISO 时，若 LCD 的 SDO 在 CS 高时不是高阻，建议在 LCD_MISO 上留
   0Ω 跳线，必要时断开；LCD ID 读取本来就是可选的。
6. DMA 注意：SPI2_RX = DMA1 Stream3，而 ESP-01 的 USART3_TX 也用 DMA1 Stream3。
   如果 SD 读使用 DMA，ESP-01 发送就先用阻塞/中断；SPI2_TX = DMA1 Stream4 不冲突。
7. 当前只实现 `spi_bus.c` 和 `sdcard_pins.h` 的引脚/总线框架，
   SD 初始化、块读写和 FatFS 尚未实现。
