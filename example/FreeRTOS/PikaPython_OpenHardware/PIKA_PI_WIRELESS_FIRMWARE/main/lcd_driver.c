#include "lcd_driver.h"
#include "pika_hal.h"
#include <stdint.h>

#define LCD_SCK "P12"
#define LCD_SDA "P11"
#define LCD_CS "P10"
#define LCD_RS "P9"
#define LCD_RST "P48"
#define LCD_LEDK "P38"

typedef struct LCD_dev {
  pika_dev *spi;
  pika_dev *rst;
  pika_dev *rs;
  pika_dev *ledk;
} LCD_dev;

LCD_dev g_lcd_dev = {0};

static inline void _pika_hal_GPIO_write(pika_dev *dev, uint32_t val) {
  pika_hal_write(dev, &val, sizeof(val));
}

void LCD_RS_CLR(void) { _pika_hal_GPIO_write(g_lcd_dev.rs, 0); }
void LCD_RS_SET(void) { _pika_hal_GPIO_write(g_lcd_dev.rs, 1); }
void LCD_RST_CLR(void) { _pika_hal_GPIO_write(g_lcd_dev.rst, 0); }
void LCD_RST_SET(void) { _pika_hal_GPIO_write(g_lcd_dev.rst, 1); }

void SPI_WriteData(uint8_t *pData, uint32_t size) {
  pika_hal_write(g_lcd_dev.spi, pData, size);
}

// 向液晶屏写一个8位指令
void LCD_WriteIndex(uint8_t Index) {
  // SPI 写命令时序开始
  LCD_RS_CLR();
  SPI_WriteData(&Index, 1);
}

// 向液晶屏写一个8位数据
void LCD_WriteData(uint8_t Data) {
  LCD_RS_SET();
  SPI_WriteData(&Data, 1);
}

// 向液晶屏写一个16位数据
void LCD_Write_u16(uint16_t Data) {
  LCD_RS_SET();
  uint8_t buf[2] = {0};
  buf[0] = Data >> 8;
  buf[1] = Data;
  SPI_WriteData(buf, 2);
}

void LCD_WriteReg(uint8_t Index, uint8_t Data) {
  LCD_WriteIndex(Index);
  LCD_WriteData(Data);
}

void LCD_Reset(void) {
  LCD_RST_CLR();
  pika_sleep_ms(100); // delay 100 ms
  LCD_RST_SET();
  pika_sleep_ms(50); // delay 50 ms
}

pika_dev *LCD_SPI_Init(void) {
  pika_dev *spi = pika_hal_open(PIKA_HAL_SOFT_SPI, "spi");
  /* config spi */
  pika_hal_SOFT_SPI_config cfg_spi = {0};
  cfg_spi.SCK = pika_hal_open(PIKA_HAL_GPIO, LCD_SCK);
  cfg_spi.CS = pika_hal_open(PIKA_HAL_GPIO, LCD_CS);
  cfg_spi.MOSI = pika_hal_open(PIKA_HAL_GPIO, LCD_SDA);
  pika_hal_ioctl(spi, PIKA_HAL_IOCTL_CONFIG, &cfg_spi);
  pika_hal_ioctl(spi, PIKA_HAL_IOCTL_ENABLE);
  return spi;
}

// LCD Init For 1.44Inch LCD Panel with ST7735R.
void LCD_Init(void) {
  pika_dev *ledk = pika_hal_open(PIKA_HAL_GPIO, LCD_LEDK);
  pika_dev *rst = pika_hal_open(PIKA_HAL_GPIO, LCD_RST);
  pika_dev *rs = pika_hal_open(PIKA_HAL_GPIO, LCD_RS);

  /* config io */
  pika_hal_GPIO_config cfg_io = {0};
  cfg_io.dir = PIKA_HAL_GPIO_DIR_OUT;
  cfg_io.pull = PIKA_HAL_GPIO_PULL_UP;
  pika_hal_ioctl(rst, PIKA_HAL_IOCTL_CONFIG, &cfg_io);
  pika_hal_ioctl(rs, PIKA_HAL_IOCTL_CONFIG, &cfg_io);
  pika_hal_ioctl(ledk, PIKA_HAL_IOCTL_CONFIG, &cfg_io);

  /* enable */
  pika_hal_ioctl(rst, PIKA_HAL_IOCTL_ENABLE);
  pika_hal_ioctl(rs, PIKA_HAL_IOCTL_ENABLE);
  pika_hal_ioctl(ledk, PIKA_HAL_IOCTL_ENABLE);

  g_lcd_dev.rst = rst;
  g_lcd_dev.rs = rs;
  g_lcd_dev.ledk = ledk;
  g_lcd_dev.spi = LCD_SPI_Init();

  _pika_hal_GPIO_write(g_lcd_dev.ledk, 1);
  LCD_Reset(); // Reset before LCD Init.
  // LCD Init For 1.8Inch LCD Panel with ST7735S.
  LCD_WriteIndex(0x11); // Sleep exit
  pika_sleep_ms(120);

  LCD_WriteIndex(0xB1);
  LCD_WriteData(0x05);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x3C);
  LCD_WriteIndex(0xB2);
  LCD_WriteData(0x05);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x3C);
  LCD_WriteIndex(0xB3);
  LCD_WriteData(0x05);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x05);
  LCD_WriteData(0x3C);
  LCD_WriteData(0x3C);
  //------------------------------------End ST7735S Frame
  // Rate---------------------------------//
  LCD_WriteIndex(0xB4); // Dot inversion
  LCD_WriteData(0x03);
  //------------------------------------ST7735S Power
  // Sequence---------------------------------//
  LCD_WriteIndex(0xC0);
  LCD_WriteData(0x28);
  LCD_WriteData(0x08);
  LCD_WriteData(0x04);
  LCD_WriteIndex(0xC1);
  LCD_WriteData(0XC0);
  LCD_WriteIndex(0xC2);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x00);
  LCD_WriteIndex(0xC3);
  LCD_WriteData(0x8D);
  LCD_WriteData(0x2A);
  LCD_WriteIndex(0xC4);
  LCD_WriteData(0x8D);
  LCD_WriteData(0xEE);
  //---------------------------------End ST7735S Power
  // Sequence-------------------------------------//
  LCD_WriteIndex(0xC5); // VCOM
  LCD_WriteData(0x1A);
  LCD_WriteIndex(0x36); // MX, MY, RGB mode
  LCD_WriteData(0xC0);
  //------------------------------------ST7735S Gamma
  // Sequence---------------------------------//
  LCD_WriteIndex(0xE0);
  LCD_WriteData(0x04);
  LCD_WriteData(0x22);
  LCD_WriteData(0x07);
  LCD_WriteData(0x0A);
  LCD_WriteData(0x2E);
  LCD_WriteData(0x30);
  LCD_WriteData(0x25);
  LCD_WriteData(0x2A);
  LCD_WriteData(0x28);
  LCD_WriteData(0x26);
  LCD_WriteData(0x2E);
  LCD_WriteData(0x3A);
  LCD_WriteData(0x00);
  LCD_WriteData(0x01);
  LCD_WriteData(0x03);
  LCD_WriteData(0x13);
  LCD_WriteIndex(0xE1);
  LCD_WriteData(0x04);
  LCD_WriteData(0x16);
  LCD_WriteData(0x06);
  LCD_WriteData(0x0D);
  LCD_WriteData(0x2D);
  LCD_WriteData(0x26);
  LCD_WriteData(0x23);
  LCD_WriteData(0x27);
  LCD_WriteData(0x27);
  LCD_WriteData(0x25);
  LCD_WriteData(0x2D);
  LCD_WriteData(0x3B);
  LCD_WriteData(0x00);
  LCD_WriteData(0x01);
  LCD_WriteData(0x04);
  LCD_WriteData(0x13);
  //------------------------------------End ST7735S Gamma
  // Sequence-----------------------------//
  LCD_WriteIndex(0x3A); // 65k mode
  LCD_WriteData(0x05);
  LCD_WriteIndex(0x29); // Display on
}

/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                   uint16_t y_end) {
  LCD_WriteIndex(0x2a);
  uint8_t data[4];
  data[0] = (X_OFFSET + x_start) >> 8;
  data[1] = (X_OFFSET + x_start) & 0XFF;
  data[2] = (X_OFFSET + x_end) >> 8;
  data[3] = (X_OFFSET + x_end) & 0XFF;
  LCD_RS_SET();
  SPI_WriteData(data, sizeof(data));
  LCD_WriteIndex(0x2b);
  data[0] = (Y_OFFSET + y_start) >> 8;
  data[1] = (Y_OFFSET + y_start) & 0XFF;
  data[2] = (Y_OFFSET + y_end) >> 8;
  data[3] = (Y_OFFSET + y_end) & 0XFF;
  LCD_RS_SET();
  SPI_WriteData(data, sizeof(data));
  LCD_WriteIndex(0x2c);
}

/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t Data) {
  LCD_SetRegion(x, y, x + 1, y + 1);
  LCD_Write_u16(Data);
}

void LCD_DrawRegin(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                   uint16_t y_end, uint16_t *pData) {
  uint32_t size = (x_end - x_start) * (y_end - y_start) * 2;
  LCD_SetRegion(x_start, y_start, x_end - 1, y_end - 1);
  LCD_RS_SET();
  SPI_WriteData((uint8_t *)pData, size);
}

#define BUFF_SIZE (X_MAX_PIXEL)
void LCD_Clear(uint16_t Color) {
  unsigned int i, m;
  uint16_t data[BUFF_SIZE];
  for (i = 0; i < BUFF_SIZE; i++) {
    data[i] = Color;
  }
  int m_max = Y_MAX_PIXEL;
  for (m = 0; m < m_max; m++) {
    LCD_DrawRegin(0, m, X_MAX_PIXEL, m + 1, data);
  }
}

void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t hight, uint16_t wight,
              uint16_t color) {
  unsigned int i, y;
  uint16_t data[BUFF_SIZE];
  for (i = 0; i < BUFF_SIZE; i++) {
    data[i] = color;
  }
  int y_end = y0 + hight;
  for (y = y0; y < y_end; y++) {
    LCD_DrawRegin(x0, y, x0 + wight, y + 1, data);
  }
}
