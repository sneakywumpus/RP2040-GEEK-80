/*****************************************************************************
* | File	:   LCD_1IN14_V2.C
* | Function	:   test Demo
* | Info	:
*----------------
* | This version:   V1.0
* | Date	:   2021-03-16
* | Info	:
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/

#include "LCD_1in14_V2.h"
#include "DEV_Config.h"

LCD_1IN14_V2_ATTRIBUTES LCD_1IN14_V2;

/******************************************************************************
function :	Hardware reset
parameter:
******************************************************************************/
static void LCD_1IN14_V2_Reset(void)
{
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_RST_PIN, 1);
	DEV_Delay_ms(100);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_RST_PIN, 0);
	DEV_Delay_ms(100);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_RST_PIN, 1);
	DEV_Delay_ms(100);
}

/******************************************************************************
function :	send command
parameter:
		Reg : Command register
******************************************************************************/
static void __not_in_flash_func(LCD_1IN14_V2_SendCommand)(uint8_t Reg)
{
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 0);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	DEV_SPI_WriteByte(Reg);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
		Data : Write data
******************************************************************************/
static void __not_in_flash_func(LCD_1IN14_V2_SendData_8Bit)(uint8_t Data)
{
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	DEV_SPI_WriteByte(Data);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
		Data : Write data
******************************************************************************/
static void __not_in_flash_func(LCD_1IN14_V2_SendData_16Bit)(uint16_t Data)
{
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	DEV_SPI_WriteByte((Data >> 8) & 0xff);
	DEV_SPI_WriteByte(Data & 0xff);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Initialize the LCD register
parameter:
******************************************************************************/
static void LCD_1IN14_V2_InitReg(void)
{
	LCD_1IN14_V2_SendCommand(0x3a);	/* Interface pixel format */
	LCD_1IN14_V2_SendData_8Bit(0x05); /* 16-bit */

	LCD_1IN14_V2_SendCommand(0xb2);	/* Porch Setting */
	LCD_1IN14_V2_SendData_8Bit(0x0c); /* Back porch in normal mode (12) */
	LCD_1IN14_V2_SendData_8Bit(0x0c); /* Front porch in normal mode (12) */
	LCD_1IN14_V2_SendData_8Bit(0x00); /* Disable separate porch control */
	LCD_1IN14_V2_SendData_8Bit(0x33); /* Back/Front porch idle (3, 3) */
	LCD_1IN14_V2_SendData_8Bit(0x33); /* Back/Front porch partial (3, 3) */

	LCD_1IN14_V2_SendCommand(0xb7); /* Gate Control */
	LCD_1IN14_V2_SendData_8Bit(0x35);/* VGH = 13.26V, VGL = -10.43V */

	LCD_1IN14_V2_SendCommand(0xbb); /* VCOM Setting */
	LCD_1IN14_V2_SendData_8Bit(0x19); /* VCOM = 0.725V */

	LCD_1IN14_V2_SendCommand(0xc0); /* LCM Control */
	LCD_1IN14_V2_SendData_8Bit(0x2c); /* XBGR, XMX, XMH (default) */

	LCD_1IN14_V2_SendCommand(0xc2); /* VDV and VRH Command Enable */
	LCD_1IN14_V2_SendData_8Bit(0x01); /* VDV & VRH from command write */
	LCD_1IN14_V2_SendCommand(0xc3); /* VRH Set */
	LCD_1IN14_V2_SendData_8Bit(0x12); /* VRH = 4.45V */
	LCD_1IN14_V2_SendCommand(0xc4); /* VDV Set */
	LCD_1IN14_V2_SendData_8Bit(0x20); /* VDV = 0V (default) */
	LCD_1IN14_V2_SendCommand(0xc5); /* VCOM Offset Set */
	LCD_1IN14_V2_SendData_8Bit(0x20); /* VCOM Offset = 0V (default) */

	LCD_1IN14_V2_SendCommand(0xc6); /* Frame Rate Control in Normal Mode */
	LCD_1IN14_V2_SendData_8Bit(0x0f); /* 60Hz (default) */

	LCD_1IN14_V2_SendCommand(0xd0); /* Power Control 1 */
	LCD_1IN14_V2_SendData_8Bit(0xa4); /* fixed parameter */
	LCD_1IN14_V2_SendData_8Bit(0xa1); /* AVDD = 6.8V, AVCL = -4.8V,
					     VDDS = 2.3V */

	LCD_1IN14_V2_SendCommand(0xe0); /* Positive Voltage Gamma Control */
	LCD_1IN14_V2_SendData_8Bit(0xd0); /* V63P = 13, V0P = 0 */
	LCD_1IN14_V2_SendData_8Bit(0x04); /* V1P = 4 */
	LCD_1IN14_V2_SendData_8Bit(0x0d); /* V2P = 13 */
	LCD_1IN14_V2_SendData_8Bit(0x11); /* V4P = 17 */
	LCD_1IN14_V2_SendData_8Bit(0x13); /* V6P = 19 */
	LCD_1IN14_V2_SendData_8Bit(0x2b); /* J0P = 2, V13P = 11 */
	LCD_1IN14_V2_SendData_8Bit(0x3f); /* V20P = 63 */
	LCD_1IN14_V2_SendData_8Bit(0x54); /* V36P = 5, V27P = 4 */
	LCD_1IN14_V2_SendData_8Bit(0x4c); /* V43P = 76 */
	LCD_1IN14_V2_SendData_8Bit(0x18); /* J1P = 1, V50P = 8 */
	LCD_1IN14_V2_SendData_8Bit(0x0d); /* V57P = 13 */
	LCD_1IN14_V2_SendData_8Bit(0x0b); /* V59P = 11 */
	LCD_1IN14_V2_SendData_8Bit(0x1f); /* V61P = 31 */
	LCD_1IN14_V2_SendData_8Bit(0x23); /* V62P = 35 */

	LCD_1IN14_V2_SendCommand(0xe1); /* Negative Voltage Gamma Control */
	LCD_1IN14_V2_SendData_8Bit(0xd0); /* V63N = 13, V0N = 0 */
	LCD_1IN14_V2_SendData_8Bit(0x04); /* V1N = 4 */
	LCD_1IN14_V2_SendData_8Bit(0x0c); /* V2N = 12 */
	LCD_1IN14_V2_SendData_8Bit(0x11); /* V4N = 17 */
	LCD_1IN14_V2_SendData_8Bit(0x13); /* V6N = 19 */
	LCD_1IN14_V2_SendData_8Bit(0x2c); /* J0N = 2, V13N = 12 */
	LCD_1IN14_V2_SendData_8Bit(0x3f); /* V20N = 63 */
	LCD_1IN14_V2_SendData_8Bit(0x44); /* V36N = 4, V27N = 4 */
	LCD_1IN14_V2_SendData_8Bit(0x51); /* V43N = 81 */
	LCD_1IN14_V2_SendData_8Bit(0x2f); /* J1N = 2, V50N = 15 */
	LCD_1IN14_V2_SendData_8Bit(0x1f); /* V57N = 31 */
	LCD_1IN14_V2_SendData_8Bit(0x1f); /* V59N = 31 */
	LCD_1IN14_V2_SendData_8Bit(0x20); /* V61N = 32 */
	LCD_1IN14_V2_SendData_8Bit(0x23); /* V62N = 35 */

	LCD_1IN14_V2_SendCommand(0x21); /* Display Inversion On */

	LCD_1IN14_V2_SendCommand(0x11); /* Sleep Out */

	LCD_1IN14_V2_SendCommand(0x29); /* Display On */
}

/********************************************************************************
function:	Set the resolution and scanning method of the screen
parameter:
		Scan_dir:   Scan direction
********************************************************************************/
static void LCD_1IN14_V2_SetAttributes(uint8_t Scan_dir)
{
	/* Get the screen scan direction */
	LCD_1IN14_V2.SCAN_DIR = Scan_dir;
	uint8_t MemoryAccessReg = 0x00;

	/* Get GRAM and LCD width and height */
	if (Scan_dir == LCD_HORIZONTAL) {
		LCD_1IN14_V2.HEIGHT = LCD_1IN14_V2_WIDTH;
		LCD_1IN14_V2.WIDTH  = LCD_1IN14_V2_HEIGHT;
		MemoryAccessReg = 0x70; /* MX=1, MV=1, ML=1 */
	} else {
		LCD_1IN14_V2.HEIGHT = LCD_1IN14_V2_HEIGHT;
		LCD_1IN14_V2.WIDTH  = LCD_1IN14_V2_WIDTH;
		MemoryAccessReg = 0x00;
	}

	/* Set the read / write scan direction of the frame memory */
	LCD_1IN14_V2_SendCommand(0x36); /* Memory Data Access Control */
	LCD_1IN14_V2_SendData_8Bit(MemoryAccessReg);
}

/********************************************************************************
function:	Set the display orientation
parameter:
********************************************************************************/
void __not_in_flash_func(LCD_1IN14_V2_SetRotated)(uint8_t Rotated)
{
	uint8_t MemoryAccessReg = 0x00;

	if (LCD_1IN14_V2.SCAN_DIR == LCD_HORIZONTAL)
		MemoryAccessReg = 0x70; /* MX=1, MV=1, ML=1 */
	else
		MemoryAccessReg = 0x00;
	if (Rotated) {
		MemoryAccessReg ^= 0xc0; /* MX=!MX, MY=!MY */
		LCD_1IN14_V2.ROTATED = 1;
	} else
		LCD_1IN14_V2.ROTATED = 0;

	/* Set the read / write scan direction of the frame memory */
	LCD_1IN14_V2_SendCommand(0x36); /* Memory Data Access Control */
	LCD_1IN14_V2_SendData_8Bit(MemoryAccessReg);
}

/********************************************************************************
function :	Initialize the LCD
parameter:
********************************************************************************/
void LCD_1IN14_V2_Init(uint8_t Scan_dir)
{
	/* initialize LCD device */
	DEV_Module_Init();

	/* Hardware reset */
	LCD_1IN14_V2_Reset();

	LCD_1IN14_V2_SetBacklight(90);

	/* Set the resolution and scanning method of the screen */
	LCD_1IN14_V2_SetAttributes(Scan_dir);

	/* Set the initialization register */
	LCD_1IN14_V2_InitReg();
}

void LCD_1IN14_V2_Exit(void)
{
	LCD_1IN14_V2_SetBacklight(0);

	DEV_Module_Exit();
}

/********************************************************************************
function:	Sets the start position and size of the display area
parameter:
		Xstart	:   X direction Start coordinates
		Ystart	:   Y direction Start coordinates
		Xend	:   X direction end coordinates
		Yend	:   Y direction end coordinates
********************************************************************************/
void __not_in_flash_func(LCD_1IN14_V2_SetWindows)(uint16_t Xstart,
						  uint16_t Ystart,
						  uint16_t Xend, uint16_t Yend)
{
	uint8_t x, y;

	if (LCD_1IN14_V2.SCAN_DIR == LCD_HORIZONTAL) {
		x = 40;
		y = LCD_1IN14_V2.ROTATED ? 52 : 53;
	} else {
		x = LCD_1IN14_V2.ROTATED ? 53 : 52;
		y = 40;
	}
	/* set the X coordinates */
	LCD_1IN14_V2_SendCommand(0x2a); /* Column Address Set */
	LCD_1IN14_V2_SendData_16Bit(Xstart + x);
	LCD_1IN14_V2_SendData_16Bit(Xend - 1 + x);
	/* set the Y coordinates */
	LCD_1IN14_V2_SendCommand(0x2b); /* Row Address Set */
	LCD_1IN14_V2_SendData_16Bit(Ystart + y);
	LCD_1IN14_V2_SendData_16Bit(Yend - 1 + y);

	LCD_1IN14_V2_SendCommand(0x2c); /* Memory Write */
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void LCD_1IN14_V2_Clear(uint16_t Color)
{
	uint16_t j, i;

	LCD_1IN14_V2_SetWindows(0, 0, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	for (j = 0; j < LCD_1IN14_V2.HEIGHT; j++) {
		for (i = 0; i < LCD_1IN14_V2.WIDTH; i++) {
			DEV_SPI_WriteByte(Color >> 8);
			DEV_SPI_WriteByte(Color & 0xff);
		}
	}
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Sends the 16-bit image buffer in RAM to displays
parameter:
******************************************************************************/
void __not_in_flash_func(LCD_1IN14_V2_Display)(uint16_t *Image)
{
	uint16_t j;

	LCD_1IN14_V2_SetWindows(0, 0, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	for (j = 0; j < LCD_1IN14_V2.HEIGHT; j++)
		DEV_SPI_Write_nByte((uint8_t *) &Image[j * LCD_1IN14_V2.WIDTH],
				    LCD_1IN14_V2.WIDTH * 2);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
	LCD_1IN14_V2_SendCommand(0x29); /* Display On */
}

/******************************************************************************
function :	Sends the 12-bit image buffer in RAM to displays
parameter:
******************************************************************************/
void __not_in_flash_func(LCD_1IN14_V2_Display12)(uint8_t *Image)
{
	uint16_t j;
	const uint16_t n = ((LCD_1IN14_V2.WIDTH + 1) / 2) * 3;

	LCD_1IN14_V2_SendCommand(0x3a); /* Interface Pixel Format */
	LCD_1IN14_V2_SendData_8Bit(0x03); /* 12-bit */
	LCD_1IN14_V2_SetWindows(0, 0, LCD_1IN14_V2.WIDTH, LCD_1IN14_V2.HEIGHT);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	for (j = 0; j < LCD_1IN14_V2.HEIGHT; j++)
		DEV_SPI_Write_nByte(&Image[j * n], n);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
	LCD_1IN14_V2_SendCommand(0x29); /* Display On */
	LCD_1IN14_V2_SendCommand(0x3a); /* Interface Pixel Format */
	LCD_1IN14_V2_SendData_8Bit(0x05); /* 16-bit */
}

/******************************************************************************
function :	Sends the 16-bit image buffer in RAM to displays
parameter:
		Xstart	:   X direction Start coordinates
		Ystart	:   Y direction Start coordinates
		Xend	:   X direction end coordinates
		Yend	:   Y direction end coordinates
		Image	:   Written content
******************************************************************************/
void LCD_1IN14_V2_DisplayWindows(uint16_t Xstart, uint16_t Ystart,
				 uint16_t Xend, uint16_t Yend, uint16_t *Image)
{
	uint32_t Addr = 0;
	uint16_t j;

	LCD_1IN14_V2_SetWindows(Xstart, Ystart, Xend, Yend);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_DC_PIN, 1);
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 0);
	Addr = Xstart + Ystart * LCD_1IN14_V2.WIDTH;
	for (j = Ystart; j < Yend - 1; j++) {
		DEV_SPI_Write_nByte((uint8_t *) &Image[Addr],
				    (Xend - Xstart) * 2);
		Addr += LCD_1IN14_V2.WIDTH;
	}
	DEV_Digital_Write(WAVESHARE_GEEK_LCD_CS_PIN, 1);
}

/******************************************************************************
function :	Change the color of a point
parameter:
		X	:   X coordinates
		Y	:   Y coordinates
		Color	:   Color
******************************************************************************/
void LCD_1IN14_V2_DisplayPoint(uint16_t X, uint16_t Y, uint16_t Color)
{
	LCD_1IN14_V2_SetWindows(X, Y, X, Y);
	LCD_1IN14_V2_SendData_16Bit(Color);
}

/******************************************************************************
function :	Set backlight brightness of LCD
parameter:
		Value	:   Brightness
******************************************************************************/
void LCD_1IN14_V2_SetBacklight(uint8_t Value)
{
	DEV_Set_PWM(Value);
}
