/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk & Thomas Eberhardt
 *
 * This module configures the machine appropriate for the
 * Z80/8080 software we want to run on it.
 *
 * History:
 * 20-APR-2024 dummy, no configuration implemented yet
 * 12-MAY-2024 implemented configuration dialog
 * 27-MAY-2024 implemented load file
 * 28-MAY-2024 implemented mount/unmount of disk images
 * 03-JUN-2024 added directory list for code files and disk images
 * 31-AUG-2024 read date/time from an optional I2C battery backed RTC
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hardware/rtc.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "ff.h"
#include "ds3231.h"

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simutil.h"
#include "simport.h"
#include "simio.h"
#include "simcfg.h"

#include "disks.h"
#include "lcd.h"
#include "picosim.h"
#if LIB_STDIO_MSC_USB
#include "stdio_msc_usb.h"
#endif

/*
 * prompt for a filename
 */
static void prompt_fn(char *s, const char *ext)
{
	printf("Filename (without .%s): ", ext);
	get_cmdline(s, 9);
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}
}

/*
 * get an integer with range check
 */
static int get_int(const char *prompt, const char *hint,
		   int min_val, int max_val)
{
	int i;
	char s[6];

	for (;;) {
		printf("Enter %s%s: ", prompt, hint);
		get_cmdline(s, 5);
		if (s[0] == '\0')
			return -1;
		i = atoi(s);
		if (i < min_val || i > max_val) {
			printf("Invalid %s: range %d - %d\n",
			       prompt, min_val, max_val);
		} else
			return i;
	}
}

/*
 * Configuration dialog for the machine
 */
void config(void)
{
	const char *cfg = "/CONF80/GEEK80.DAT";
	const char *cpath = "/CODE80";
	const char *cext = "*.BIN";
	const char *dpath = "/DISKS80";
	const char *dext = "*.DSK";
	char s[10];
	unsigned int br;
	int go_flag = 0, brightness = 90, rotated = 0;
	int i, n, menu;
	datetime_t t = { .year = 2024, .month = 6, .day = 13, .dotw = 4,
			.hour = 20, .min = 27, .sec = 53 };
	static const char *dotw[7] = { "Sun", "Mon", "Tue", "Wed",
				       "Thu", "Fri", "Sat" };
	UNUSED(DS3231_MONTHS);
	UNUSED(DS3231_WDAYS);

	/* try to read config file */
	sd_res = f_open(&sd_file, cfg, FA_READ);
	if (sd_res == FR_OK) {
		f_read(&sd_file, &cpu, sizeof(cpu), &br);
		f_read(&sd_file, &speed, sizeof(speed), &br);
		f_read(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_read(&sd_file, &brightness, sizeof(brightness), &br);
		f_read(&sd_file, &rotated, sizeof(rotated), &br);
		f_read(&sd_file, &initial_lcd, sizeof(initial_lcd), &br);
		f_read(&sd_file, &t, sizeof(datetime_t), &br);
		f_read(&sd_file, &disks[0], DISKLEN, &br);
		f_read(&sd_file, &disks[1], DISKLEN, &br);
		f_read(&sd_file, &disks[2], DISKLEN, &br);
		f_read(&sd_file, &disks[3], DISKLEN, &br);
		f_close(&sd_file);
		switch (initial_lcd) {
		case LCD_STATUS_REGISTERS:
#ifdef SIMPLEPANEL
		case LCD_STATUS_PANEL:
#endif
		case LCD_STATUS_MEMORY:
			break;
		default:
			initial_lcd = LCD_STATUS_REGISTERS;
		}
	}

	/* Create a real-time clock structure and initiate this */
	struct ds3231_rtc rtc;
	ds3231_init(i2c_default, PICO_DEFAULT_I2C_SDA_PIN,
		    PICO_DEFAULT_I2C_SCL_PIN, &rtc);

	/* A `ds3231_datetime_t` structure holds date and time values. It is */
	/* used first to set the initial (user-defined) date/time. After */
	/* these initial values are set, then this same structure will be */
	/* updated by the ds3231 functions as needed with the most current */
	/* time and date values. */
	ds3231_datetime_t dt = {
		.year = 2000,	/* Year (2000-2099) */
		.month = 1,	/* Month (1-12, 1=January, 12=December) */
		.day = 1,	/* Day (1-31) */
		.hour = 0,	/* Hour, in 24-h format (0-23) */
		.minutes = 0,	/* Minutes (0-59) */
		.seconds = 0,	/* Seconds (0-59) */
		.dotw = 1,	/* Day of the week (1-7, 1=Monday, 7=Sunday) */
	};

	/* Read the date and time from the DS3231 RTC */
        ds3231_get_datetime(&dt, &rtc);

	/* if we read something take it over */
	if (dt.year != 2000) {
		t.year = dt.year;
		t.month = dt.month;
		t.day = dt.day;
		t.hour = dt.hour;
		t.min = dt.minutes;
		t.sec = dt.seconds;
		if (dt.dotw < 7)
			t.dotw = dt.dotw;
		else
			t.dotw = 0;
	}

	rtc_set_datetime(&t);
	sleep_us(64);

	lcd_brightness(brightness);
	lcd_set_rotated(rotated);

	menu = 1;

	while (!go_flag) {
		if (menu) {
			rtc_get_datetime(&t);
			printf("Current time: %s %04d-%02d-%02d "
			       "%02d:%02d:%02d\n", dotw[t.dotw],
			       t.year, t.month, t.day,
			       t.hour, t.min, t.sec);
			printf("b - LCD brightness: %d\n", brightness);
			printf("m - rotate LCD\n");
			printf("l - LCD status display: ");
			switch (initial_lcd) {
			case LCD_STATUS_MEMORY:
				printf("memory pixmap\n");
				break;
#ifdef SIMPLEPANEL
			case LCD_STATUS_PANEL:
				printf("classic front panel\n");
				break;
#endif
			case LCD_STATUS_REGISTERS:
				printf("CPU registers\n");
				break;
			default:
				printf("unknown!\n");
				break;
			}
			printf("a - set date\n");
			printf("t - set time\n");
#if LIB_STDIO_MSC_USB
			printf("u - enable USB mass storage access\n");
#endif
			printf("c - switch CPU, currently %s\n",
			       (cpu == Z80) ? "Z80" : "8080");
			printf("s - CPU speed: ");
			if (speed == 0)
				puts("unlimited");
			else
				printf("%d MHz\n", speed);
			printf("p - Port 255 value: %02XH\n", fp_value);
			printf("f - list files\n");
			printf("r - load file\n");
			printf("d - list disks\n");
			printf("0 - Disk 0: %s\n", disks[0]);
			printf("1 - Disk 1: %s\n", disks[1]);
			printf("2 - Disk 2: %s\n", disks[2]);
			printf("3 - Disk 3: %s\n", disks[3]);
			printf("g - run machine\n\n");
		} else
			menu = 1;
		printf("Command: ");
		get_cmdline(s, 2);
		putchar('\n');

		switch (tolower((unsigned char) s[0])) {
		case 'b':
			if ((i = get_int("brightness", "", 0, 100)) >= 0) {
				brightness = i;
				lcd_brightness((uint8_t) brightness);
			}
			putchar('\n');
			break;

		case 'm':
			rotated = !rotated;
			lcd_set_rotated(rotated);
			break;

		case 'l':
			if (initial_lcd == LCD_STATUS_REGISTERS)
#ifdef SIMPLEPANEL
				initial_lcd = LCD_STATUS_PANEL;
			else if (initial_lcd == LCD_STATUS_PANEL)
#endif
				initial_lcd = LCD_STATUS_MEMORY;
			else
				initial_lcd = LCD_STATUS_REGISTERS;
			break;

		case 'a':
			n = 0;
			rtc_get_datetime(&t);
			ds3231_get_datetime(&dt, &rtc);
			if ((i = get_int("weekday", " (0=Sun)", 0, 6)) >= 0) {
				t.dotw = i;
				if (i == 0)
					dt.dotw = 7;
				else
					dt.dotw = i;
				n++;
			}
			if ((i = get_int("year", "", 0, 4095)) >= 0) {
				t.year = i;
				dt.year = i;
				n++;
			}
			if ((i = get_int("month", "", 1, 12)) >= 0) {
				t.month = i;
				dt.month = i;
				n++;
			}
			if ((i = get_int("day", "", 1, 31)) >= 0) {
				t.day = i;
				dt.day = i;
				n++;
			}
			if (n > 0) {
				ds3231_set_datetime(&dt, &rtc);
				rtc_set_datetime(&t);
				sleep_us(64);
			}
			putchar('\n');
			break;

		case 't':
			n = 0;
			rtc_get_datetime(&t);
			ds3231_get_datetime(&dt, &rtc);
			if ((i = get_int("hour", "", 0, 23)) >= 0) {
				t.hour = i;
				dt.hour = i;
				n++;
			}
			if ((i = get_int("minute", "", 0, 59)) >= 0) {
				t.min = i;
				dt.minutes = i;
				n++;
			}
			if ((i = get_int("second", "", 0, 59)) >= 0) {
				t.sec = i;
				dt.seconds = i;
				n++;
			}
			if (n > 0) {
				ds3231_set_datetime(&dt, &rtc);
				rtc_set_datetime(&t);
				sleep_us(64);
			}
			putchar('\n');
			break;

#if LIB_STDIO_MSC_USB
		case 'u':
			exit_disks();
			puts("Waiting for disk to be ejected");
			stdio_msc_usb_do_msc();
			puts("Disk ejected\n");
			init_disks();
			check_disks();
			break;
#endif

		case 'c':
			if (cpu == Z80)
				switch_cpu(I8080);
			else
				switch_cpu(Z80);
			break;

		case 's':
			i = get_int("speed", " in MHz (0=unlimited)", 0, 40);
			putchar('\n');
			if (i >= 0)
				speed = i;
			break;

		case 'p':
again:
			printf("Enter value in Hex: ");
			get_cmdline(s, 3);
			if (s[0]) {
				if (!isxdigit((unsigned char) s[0]) ||
				    !isxdigit((unsigned char) s[1])) {
					puts("Invalid value: range 00 - FF");
					goto again;
				}
				fp_value = (s[0] <= '9' ? s[0] - '0' :
					    toupper((unsigned char) s[0]) -
					    'A' + 10) << 4;
				fp_value += (s[1] <= '9' ? s[1] - '0' :
					     toupper((unsigned char) s[1]) -
					     'A' + 10);
			}
			putchar('\n');
			break;

		case 'f':
			list_files(cpath, cext);
			putchar('\n');
			menu = 0;
			break;

		case 'r':
			prompt_fn(s, "bin");
			if (s[0])
				load_file(s);
			putchar('\n');
			menu = 0;
			break;

		case 'd':
			list_files(dpath, dext);
			putchar('\n');
			menu = 0;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
			i = s[0] - '0';
			prompt_fn(s, "dsk");
			if (s[0])
				mount_disk(i, s);
			else {
				disks[i][0] = '\0';
				putchar('\n');
			}
			break;

		case 'g':
			go_flag = 1;
			break;

		default:
			break;
		}
	}

	/* try to save config file */
	sd_res = f_open(&sd_file, cfg, FA_WRITE | FA_CREATE_ALWAYS);
	if (sd_res == FR_OK) {
		f_write(&sd_file, &cpu, sizeof(cpu), &br);
		f_write(&sd_file, &speed, sizeof(speed), &br);
		f_write(&sd_file, &fp_value, sizeof(fp_value), &br);
		f_write(&sd_file, &brightness, sizeof(brightness), &br);
		f_write(&sd_file, &rotated, sizeof(rotated), &br);
		f_write(&sd_file, &initial_lcd, sizeof(initial_lcd), &br);
		f_write(&sd_file, &t, sizeof(datetime_t), &br);
		f_write(&sd_file, &disks[0], DISKLEN, &br);
		f_write(&sd_file, &disks[1], DISKLEN, &br);
		f_write(&sd_file, &disks[2], DISKLEN, &br);
		f_write(&sd_file, &disks[3], DISKLEN, &br);
		f_close(&sd_file);
	}
}
