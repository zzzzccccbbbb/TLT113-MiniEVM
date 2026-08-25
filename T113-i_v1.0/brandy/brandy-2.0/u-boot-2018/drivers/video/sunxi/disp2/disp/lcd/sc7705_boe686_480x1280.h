/* drivers/video/sunxi/disp2/disp/lcd/sc7705_boe686_480x1280.h
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

#ifndef _SC7705_BOE686_480X1280_H
#define _SC7705_BOE686_480X1280_H

#include "panels.h"

extern __lcd_panel_t sc7705_boe686_480x1280_mipi_panel;

extern s32 bsp_disp_get_panel_info(u32 screen_id, disp_panel_para *info);

#endif /*End of file*/
