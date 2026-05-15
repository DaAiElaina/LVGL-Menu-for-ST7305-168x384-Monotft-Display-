#include "lvgl_driver_screen.h"
#include <SPI.h>
#include <ST7305_2p9_BW_DisplayDriver.h>
#include <ST73xxPins.h>

#define PIN_DC      38
#define PIN_RST     0
#define PIN_CS      45
#define PIN_SCLK    36
#define PIN_SDIN    37

const ST73xxPins PINS{PIN_DC, PIN_CS, PIN_SCLK, PIN_SDIN, PIN_RST};
static ST7305_2p9_BW_DisplayDriver display(PINS, SPI);

#define DRAW_BUF_SIZE (LV_W * 40)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DRAW_BUF_SIZE];

// 坐标旋转函数
static void rotate_coord(int32_t x, int32_t y, int32_t *out_x, int32_t *out_y) {
#if LVGL_ROTATION == 1
    // 顺时针 90 度
    *out_x = y;
    *out_y = LV_W - 1 - x;
#elif LVGL_ROTATION == 2
    // 逆时针 90 度
    *out_x = LV_H - 1 - y;
    *out_y = x;
#elif LVGL_ROTATION == 3
    // 180 度
    *out_x = LV_W - 1 - x;
    *out_y = LV_H - 1 - y;
#else
    // 不旋转
    *out_x = x;
    *out_y = y;
#endif
}

// 显示刷新回调（支持软件横屏）
void lvgl_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    int32_t idx = 0;

    for (int32_t y = 0; y < h; y++) {
        for (int32_t x = 0; x < w; x++) {
            // LVGL 输出的原始坐标
            int32_t lv_x = area->x1 + x;
            int32_t lv_y = area->y1 + y;
            
            // 旋转后的物理坐标
            int32_t phys_x, phys_y;
            rotate_coord(lv_x, lv_y, &phys_x, &phys_y);
            
            // 边界检查
            if (phys_x >= 0 && phys_x < PHY_W && phys_y >= 0 && phys_y < PHY_H) {
                display.writePoint(phys_x, phys_y, (bool)!color_map[idx].full);
            }
            idx++;
        }
    }
    display.display();
    lv_disp_flush_ready(drv);
}

// 初始化显示驱动
void lvgl_screen_init() {
    SPI.begin(PIN_SCLK, -1, PIN_SDIN, PIN_CS);
    display.initialize();
    display.High_Power_Mode();
    display.display_on(true);
    display.display_Inversion(false);
    display.clearDisplay();
    display.display();

    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, DRAW_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_W;      // LVGL 逻辑宽度（横屏）
    disp_drv.ver_res = LV_H;      // LVGL 逻辑高度（横屏）
    disp_drv.flush_cb = lvgl_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}