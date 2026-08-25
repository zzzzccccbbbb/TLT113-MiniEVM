/* drivers/video/sunxi/disp2/disp/lcd/sc7705_boe686_480x1280.c
 *
 * Copyright (c) 2017 Allwinnertech Co., Ltd.
 * Author: xixinle <xixinle@allwinnertech.com>
 *
 * sc7705_boe686_480x1280 mipi dsi panel driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#include "sc7705_boe686_480x1280.h"

/*
	to use this driver, modify your dts with the following content,
	modify fb0_width and fb0_height, modify lcd_gpio_x pin config,
	modify bootlogo.bmp to suitable size,
	and open driver in u-boot/kernel defconfig.

&lcd0 {
	lcd_used            = <1>;

	lcd_driver_name     = "sc7705_boe686_480x1280";
	lcd_backlight       = <50>;
	lcd_if              = <4>;

	lcd_x               = <480>;
	lcd_y               = <1280>;
	lcd_width           = <480>;
	lcd_height          = <1280>;
	lcd_dclk_freq       = <64>;

	lcd_pwm_used        = <1>;
	lcd_pwm_ch          = <7>;
	lcd_pwm_freq        = <50000>;
	lcd_pwm_pol         = <1>;
	lcd_pwm_max_limit   = <255>;

	lcd_hbp             = <220>;
	lcd_ht              = <810>;
	lcd_hspw            = <110>;
	lcd_vbp             = <13>;
	lcd_vt              = <1310>;
	lcd_vspw            = <3>;

	lcd_dsi_lane        = <4>;
	lcd_dsi_if          = <0>;
	lcd_dsi_format      = <0>;
	lcd_dsi_te          = <0>;
	lcd_frm             = <0>;
	lcd_io_phase        = <0>;
	lcd_gamma_en        = <0>;
	lcd_bright_curve_en = <0>;
	lcd_cmap_en         = <0>;

	deu_mode            = <0>;
	lcdgamma4iep        = <22>;
	smart_color         = <90>;

	lcd_gpio_0 = <&pio PD 18 GPIO_ACTIVE_HIGH>; // lcd reset
	lcd_gpio_1 = <&pio PB 7 GPIO_ACTIVE_HIGH>; // lcd power enable
	pinctrl-0 = <&dsi4lane_pins_a>;
	pinctrl-1 = <&dsi4lane_pins_b>;
};
*/

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

#define panel_reset(sel, val) sunxi_lcd_gpio_set_value(sel, 0, val) /* lcd_gpio_0 */
#define panel_power(sel, val) sunxi_lcd_gpio_set_value(sel, 1, val) /* lcd_gpio_1 */

static void lcd_cfg_panel_info(panel_extend_para *info)
{
	u32 i = 0, j = 0;
	u32 items;
	u8 lcd_gamma_tbl[][2] = {
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
		{LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
		{LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
		{LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
		{LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] +
				((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1])
				* j) / num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] =
							(value<<16)
							+ (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) +
					(lcd_gamma_tbl[items-1][1]<<8)
					+ lcd_gamma_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 lcd_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, lcd_power_on, 10);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 10);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 50);
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);

	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 0);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit, 200);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);

	return 0;
}

static void lcd_power_on(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 1);
	panel_power(sel, 0);
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_delay_ms(10);
	panel_power(sel, 1);
	sunxi_lcd_power_enable(sel, 1);
	sunxi_lcd_delay_ms(10);

	sunxi_lcd_delay_ms(50);
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(100);
	panel_reset(sel, 0);
	sunxi_lcd_delay_ms(100);
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(100);

}

static void lcd_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_delay_ms(20);
	panel_reset(sel, 0);
	panel_power(sel, 0);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 1);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 0);
}

static void lcd_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);
}

static void lcd_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}

#define REGFLAG_DELAY         0XFE
#define REGFLAG_END_OF_TABLE  0xFC   /* END OF REGISTERS MARKER */

struct LCM_setting_table {
	u8 cmd;
	u32 count;
	u8 para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xB9, 3, {0xF1, 0x12, 0x84} },

	{0xB1, 10, {0x12, 0x25, 0x23, 0x32, 0x32, 0x33, 0x77, 0x04,
				0xDB, 0x0C} },
	{0xB2, 2, {0x40, 0x0C} },
	{0xB3, 8, {0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28, 0x28} },
	{0xB4, 1, {0x80} },
	{0xB5, 2, {0x0A, 0x0A} },
	{0xB6, 2, {0x58, 0x58} },
	{0xB8, 2, {0xA6, 0x03} },
	{0xBA, 27, {0x33, 0x81, 0x05, 0xF9, 0x0E, 0x0E, 0x20, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x25,
				0x00, 0x90, 0x0A, 0x00, 0x00, 0x02, 0x4F, 0x01,
				0x00, 0x00, 0x37} },
	{0xBC, 1, {0x46} },
	{0xBF, 3, {0x00, 0x10, 0x82} },
	{0xC0, 9, { 0x73, 0x73, 0x50, 0x50, 0x00, 0x00, 0x08, 0xF0,
				0x00} },
	{0xCC, 1, {0x07} },
	{0xE0, 34, {0x00, 0x12, 0x1C, 0x2B, 0x3B, 0x3F, 0x4D, 0x3E,
				0x06, 0x0B, 0x0D, 0x12, 0x14, 0x11, 0x12, 0x11,
				0x19, 0x00, 0x12, 0x1C, 0x2B, 0x3B, 0x3F, 0x4D,
				0x3E, 0x06, 0x0B, 0x0D, 0x12, 0x14, 0x11, 0x12,
				0x11, 0x19} },
	{0xE3, 11, {0x03, 0x03, 0x03, 0x03, 0x00, 0x03, 0x00, 0x00,
				0x00, 0xC0, 0x00} },
	{0xE9, 63, {0x02, 0x00, 0x05, 0x05, 0x12, 0x80, 0x28, 0x12,
				0x31, 0x23, 0x4F, 0x07, 0x80, 0x28, 0x47, 0x08,
				0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C,
				0x00, 0x00, 0x00, 0x00, 0x88, 0xAB, 0x46, 0x02,
				0x8F, 0x02, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
				0xAB, 0x57, 0x13, 0x8F, 0x13, 0x88, 0x88, 0x88,
				0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0xEA, 63, {0x07, 0x1A, 0x01, 0x01, 0x02, 0x3C, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0xF8, 0xAB, 0x31, 0x75,
				0x88, 0x31, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF8,
				0xAB, 0x20, 0x64, 0x88, 0x20, 0x88, 0x88, 0x88,
				0x88, 0x88, 0x23, 0x10, 0x00, 0x00, 0x01, 0x5F,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80,
				0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0x11, 0, {0x00} },
	{REGFLAG_DELAY, 150, {} },
	{0x29, 0, {0x00} },
	{REGFLAG_DELAY, 80, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void lcd_panel_init(u32 sel)
{
	u32 i = 0;
/*
	int ret;
	u32 read_num;
	u8 read_result[3];
	for(i=0;i<3;i++)read_result[i]=0;
*/
	sunxi_lcd_dsi_clk_enable(sel);
	sunxi_lcd_delay_ms(10);

	/* fixme: read panel reg will caused DSI error, no irq, no HS signal */
/*
	sunxi_lcd_dsi_set_max_ret_size(sel, 3);
	sunxi_lcd_delay_ms(5);
	ret = sunxi_lcd_dsi_dcs_read(sel, 0x04, read_result, &read_num);
	//command 0x04 read display id, id: 0x48 0x21 0x1f
	printf("sc7705_boe686 read id ret=%d read_num=%d id:0x%x,0x%x,0x%x\n", ret, read_num,read_result[0], read_result[1],read_result[2]);
*/

	/*========== Internal setting ==========*/
	for (i = 0;; i++) {
		if (lcm_initialization_setting[i].cmd == REGFLAG_END_OF_TABLE)
			break;
		else if (lcm_initialization_setting[i].cmd == REGFLAG_DELAY)
			sunxi_lcd_delay_ms(lcm_initialization_setting[i].count);
#ifdef SUPPORT_DSI
		else {
			dsi_dcs_wr(sel, lcm_initialization_setting[i].cmd,
				   lcm_initialization_setting[i].para_list,
				   lcm_initialization_setting[i].count);
		}
#endif
	}
}

static void lcd_panel_exit(u32 sel)
{
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x10);
	sunxi_lcd_delay_ms(80);
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x28);
	sunxi_lcd_delay_ms(50);
}

/*sel: 0:lcd0; 1:lcd1*/
static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t sc7705_boe686_480x1280_mipi_panel = {
	/* panel driver name, must mach the name of
	 * lcd_drv_name in sys_config.fex/board.dts
	 */
	.name = "sc7705_boe686_480x1280",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
			.cfg_open_flow = lcd_open_flow,
			.cfg_close_flow = lcd_close_flow,
			.lcd_user_defined_func = lcd_user_defined_func,
	},
};
