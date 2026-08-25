#include "aml070wxii4006.h"

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

#define panel_reset(val) sunxi_lcd_gpio_set_value(sel, 0, val)

static void lcd_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		/* {input value, corrected value} */
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
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl)/2;
	for (i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for (j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 lcd_open_flow(u32 sel)
{
	/* open lcd power, and delay 50ms */
	LCD_OPEN_FUNC(sel, lcd_power_on, 50);
	/* open lcd power, than delay 50ms */
	LCD_OPEN_FUNC(sel, lcd_panel_init, 50);
	/* open lcd controller, and delay 100ms */
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);
	/* open lcd backlight, and delay 0ms */
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);

	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	/* close lcd backlight, and delay 0ms */
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 0);
	/* close lcd controller, and delay 0ms */
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	/* open lcd power, than delay 200ms */
	LCD_CLOSE_FUNC(sel, lcd_panel_exit, 200);
	/* close lcd power, and delay 500ms */
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);

	return 0;
}

static void lcd_power_on(u32 sel)
{
	panel_reset(0);
	/* config lcd_power pin to open lcd power */
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_pin_cfg(sel, 1);
	sunxi_lcd_delay_ms(30);
	panel_reset(1);
	sunxi_lcd_delay_ms(30);
	panel_reset(0);
	sunxi_lcd_delay_ms(30);
	panel_reset(1);
	sunxi_lcd_delay_ms(30);
}

static void lcd_power_off(u32 sel)
{
	panel_reset(0);
	sunxi_lcd_delay_ms(10);
	sunxi_lcd_pin_cfg(sel, 0);
	/* config lcd_power pin to open lcd power */
	sunxi_lcd_power_disable(sel, 0);
	sunxi_lcd_dsi_clk_disable(sel);
}

static void lcd_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
}

static void lcd_bl_close(u32 sel)
{
	sunxi_lcd_pwm_disable(sel);
}

static void lcd_panel_init(u32 sel)
{
	sunxi_lcd_dsi_clk_enable(sel);
	sunxi_lcd_delay_ms(50);

	/* reset lcd panel */
	sunxi_lcd_dsi_dcs_write_0para(sel, DSI_DCS_SOFT_RESET);
	sunxi_lcd_delay_ms(50);

	/* Set gamma curve related setting */
	/* enter page1 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xee, 0x50);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xea, 0x85);
	/* write enable */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xeb, 0x55);
	/* bist=1 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x30, 0x00);
	/* bist=1 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x31, 0x00);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x90, 0x80);
	/* ss_tp location */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x91, 0x00);
	/* mirror te */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x24, 0x20);
	/* ss_tp de ndg */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x99, 0x00);
	/* zigzag */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x79, 0x00);
	/* column invertion */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x95, 0x74);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x7a, 0x20);
	/* sm gip */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x97, 0x09);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x7d, 0x08);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x56, 0x83);

	/* enter page2 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xee, 0x60);
	/* 4 LANE */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x30, 0x01);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x27, 0x22);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x31, 0x0f);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x32, 0xd9);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x33, 0xc0);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x34, 0x1f);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x35, 0x22);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x36, 0x00);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x37, 0x00);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x3a, 0x24);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x3b, 0x00);
	/* VCOM SET 29 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x3c, 0x1a);

	/* vgl */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x3d, 0x11);
	/* vgh */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x3e, 0x93);
	/* vspr */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x42, 0x64);
	/* vsnr */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x43, 0x64);
	/* vgh */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x44, 0x0b);
	/* vgl */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x46, 0x4e);
	/* blkh,1 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x8b, 0x90);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x8d, 0x45);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x91, 0x11);
	/* frq_cp1_clk[2:0] */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x92, 0x11);
	/* fp7721 power 9f */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x93, 0x9f);
	/* s_out=800 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x9a, 0x00);
	/* vlength=1280 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0x9c, 0x80);

	/* gamma 2.2  2021/01/19 */
	/* gamma P0.4.8.12.20 0X1E */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x47, 0x0f, 0x24, 0x2c, 0x39, 0x36);
	/* gamma n 0.4.8.12.20 0X1E */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x5A, 0x0f, 0x24, 0x2c, 0x39, 0x36);
	/* 28.44.64.96.128 */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x4C, 0x4a, 0x40, 0x51, 0x31, 0x2f);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x5F, 0x4a, 0x40, 0x51, 0x31, 0x2f);
	/* 159.191.211.227.235 */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x51, 0x2d, 0x10, 0x25, 0x1f, 0x30);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x64, 0x2d, 0x10, 0x25, 0x1f, 0x30);
	/* 243.247.251.255 */
	sunxi_lcd_dsi_dcs_write_4para(sel, 0x56, 0x37, 0x46, 0x5b, 0x7F);
	sunxi_lcd_dsi_dcs_write_4para(sel, 0x69, 0x37, 0x46, 0x5b, 0x7F);
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xee, 0x70);

	/* STV0   stv1 */
	sunxi_lcd_dsi_dcs_write_4para(sel, 0x00, 0x03, 0x07, 0x00, 0x01);
	sunxi_lcd_dsi_dcs_write_4para(sel, 0x04, 0x08, 0x0c, 0x55, 0x01);
	sunxi_lcd_dsi_dcs_write_2para(sel, 0x0c, 0x05, 0x3d);

	/* CYC0 */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x10, 0x05, 0x08, 0x00, 0x01, 0x05);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x15, 0x00, 0x15, 0x0d, 0x08, 0x00);
	sunxi_lcd_dsi_dcs_write_2para(sel, 0x29, 0x05, 0x3d);

	/* forward scan */
	/* gip22-gip43=gipl1-gipl22 */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x60, 0x3c, 0x3c, 0x07, 0x05, 0x17);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x65, 0x15, 0x13, 0x11, 0x01, 0x03);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x6a, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x6f, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c);
	sunxi_lcd_dsi_dcs_write_2para(sel, 0x74, 0x3C, 0x3c);

	/* gip0-gip21=gipr1-gipr22 */
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x80, 0x3c, 0x3c, 0x06, 0x04, 0x16);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x85, 0x14, 0x12, 0x10, 0x00, 0x02);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x8a, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c);
	sunxi_lcd_dsi_dcs_write_5para(sel, 0x8f, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c);
	sunxi_lcd_dsi_dcs_write_2para(sel, 0x94, 0x3c, 0x3c);

	/* write enable */
	sunxi_lcd_dsi_dcs_write_2para(sel, 0xea, 0x00, 0x00);
	/* ENTER PAGE0 */
	sunxi_lcd_dsi_dcs_write_1para(sel, 0xee, 0x00);

	/*exit sleep mode and set display on*/
	sunxi_lcd_dsi_dcs_write_0para(sel, DSI_DCS_EXIT_SLEEP_MODE);
	sunxi_lcd_delay_ms(120);
	sunxi_lcd_dsi_dcs_write_0para(sel, DSI_DCS_SET_DISPLAY_ON);
	sunxi_lcd_delay_ms(10);
}

static void lcd_panel_exit(u32 sel)
{
	sunxi_lcd_dsi_dcs_write_0para(sel, DSI_DCS_SET_DISPLAY_OFF);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_dsi_dcs_write_0para(sel, DSI_DCS_ENTER_SLEEP_MODE);
	sunxi_lcd_delay_ms(20);
}

/* sel: 0:lcd0; 1:lcd1 */
static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

static s32 lcd_set_bright(u32 sel, u32 bright)
{
	lcd_bl_open(sel);
	return 0;
}

__lcd_panel_t aml070wxii4006_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "panel-aml070wxii4006",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
		.set_bright = lcd_set_bright,
	},
};
