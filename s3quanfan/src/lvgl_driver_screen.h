#ifndef LVGL_DRIVER_SCREEN_H
#define LVGL_DRIVER_SCREEN_H

#include <lvgl.h>

// 屏幕尺寸
#define DISPLAY_W   168
#define DISPLAY_H   384

// 初始化显示驱动
void lvgl_screen_init();

// 获取显示刷新回调
void lvgl_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

#endif