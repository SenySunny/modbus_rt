#ifndef __LCD_DRIVER__H
#define __LCD_DRIVER__H

#include <stdint.h>
#define X_MAX_PIXEL 128
#define Y_MAX_PIXEL 160
#define X_OFFSET 2
#define Y_OFFSET 1

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09
#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13
#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E
#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5
#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD
#define ST7735_PWCTR6 0xFC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define RED 0x00f8
#define GREEN 0xe007
#define BLUE 0x1f00
#define WHITE 0xffff
#define BLACK 0x0000
#define YELLOW 0xE0FF
#define GRAY0 0x7DEF // 灰色0 3165 00110 001011 00101
#define GRAY1 0x1084 // 灰色1      00000 000000 00000
#define GRAY2 0x0842 // 灰色2  1111111111011111

void LCD_WriteIndex(uint8_t Index);
void LCD_WriteData(uint8_t Data);
void LCD_WriteReg(uint8_t Index, uint8_t Data);
void LCD_Reset(void);
void LCD_Init(void);
void LCD_Clear(uint16_t Color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t Data);
void LCD_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                   uint16_t y_end);
void LCD_DrawRegin(uint16_t x_start, uint16_t y_start, uint16_t x_end,
                   uint16_t y_end, uint16_t *pData);
void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t hight, uint16_t wight,
              uint16_t color);

#endif