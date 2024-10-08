/*****************************************************************************
* | File	:   DEV_Config.c
* | Author	:
* | Function	:   Hardware underlying interface
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
# copies of theex Software, and to permit persons to  whom the Software is
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

#include "DEV_Config.h"

uint DEV_pwm_slice_num;

void DEV_Module_Init(void)
{
	/* SPI Config */
	spi_init(DEV_SPI_PORT, 125 * 1000 * 1000 / 4); /* 31.25 MHz */
	gpio_set_function(WAVESHARE_RP2040_LCD_SCLK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(WAVESHARE_RP2040_LCD_TX_PIN, GPIO_FUNC_SPI);

	/* GPIO Config */
	gpio_init(WAVESHARE_RP2040_LCD_RST_PIN);
	gpio_set_dir(WAVESHARE_RP2040_LCD_RST_PIN, GPIO_OUT);
	gpio_init(WAVESHARE_RP2040_LCD_DC_PIN);
	gpio_set_dir(WAVESHARE_RP2040_LCD_DC_PIN, GPIO_OUT);
	gpio_init(WAVESHARE_RP2040_LCD_CS_PIN);
	gpio_set_dir(WAVESHARE_RP2040_LCD_CS_PIN, GPIO_OUT);
	gpio_init(WAVESHARE_RP2040_LCD_BL_PIN);
	gpio_set_dir(WAVESHARE_RP2040_LCD_BL_PIN, GPIO_OUT);
	DEV_Digital_Write(WAVESHARE_RP2040_LCD_CS_PIN, 1);
	DEV_Digital_Write(WAVESHARE_RP2040_LCD_DC_PIN, 0);
	DEV_Digital_Write(WAVESHARE_RP2040_LCD_BL_PIN, 1);

	/* PWM Config */
	gpio_set_function(WAVESHARE_RP2040_LCD_BL_PIN, GPIO_FUNC_PWM);
	DEV_pwm_slice_num = pwm_gpio_to_slice_num(WAVESHARE_RP2040_LCD_BL_PIN);
	pwm_set_wrap(DEV_pwm_slice_num, 100);
	pwm_set_chan_level(DEV_pwm_slice_num, PWM_CHAN_B, 1);
	pwm_set_clkdiv(DEV_pwm_slice_num, 50);
	pwm_set_enabled(DEV_pwm_slice_num, true);
}

void DEV_Module_Exit(void)
{
	pwm_set_enabled(DEV_pwm_slice_num, false);
	gpio_deinit(WAVESHARE_RP2040_LCD_BL_PIN);

	gpio_deinit(WAVESHARE_RP2040_LCD_DC_PIN);
	gpio_deinit(WAVESHARE_RP2040_LCD_CS_PIN);
	gpio_deinit(WAVESHARE_RP2040_LCD_RST_PIN);

	spi_deinit(DEV_SPI_PORT);
	gpio_deinit(WAVESHARE_RP2040_LCD_SCLK_PIN);
	gpio_deinit(WAVESHARE_RP2040_LCD_TX_PIN);
}
