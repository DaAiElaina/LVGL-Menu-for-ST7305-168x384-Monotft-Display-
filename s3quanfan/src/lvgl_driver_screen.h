#ifndef LVGL_DRIVER_SCREEN_H
#define LVGL_DRIVER_SCREEN_H

#include <lvgl.h>

// 物理屏幕尺寸（固定，不要改）
#define PHY_W   168
#define PHY_H   384

// LVGL 逻辑尺寸（根据旋转方向自动交换）
// 旋转 0 或 180 度时：LV_W = PHY_W, LV_H = PHY_H
// 旋转 90 或 270 度时：LV_W = PHY_H, LV_H = PHY_W

// ========== 在这里修改旋转方向 ==========
#define LVGL_ROTATION 2   // 0=竖屏, 1=顺时针90°, 2=逆时针270°, 3=180°
// =====================================

#if LVGL_ROTATION == 0 || LVGL_ROTATION == 3
    #define LV_W    PHY_W
    #define LV_H    PHY_H
#else
    #define LV_W    PHY_H
    #define LV_H    PHY_W
#endif

void lvgl_screen_init();
void lvgl_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

#endif