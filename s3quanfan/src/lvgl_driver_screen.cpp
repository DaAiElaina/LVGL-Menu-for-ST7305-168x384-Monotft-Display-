#include "lvgl_driver_screen.h"
#include <SPI.h>
#include <ST7305_2p9_BW_DisplayDriver.h>
#include <ST73xxPins.h>

// 引脚定义
#define PIN_DC      38
#define PIN_RST     0
#define PIN_CS      45
#define PIN_SCLK    36
#define PIN_SDIN    37

// 全局显示对象
static const ST73xxPins PINS{PIN_DC, PIN_CS, PIN_SCLK, PIN_SDIN, PIN_RST};
static ST7305_2p9_BW_DisplayDriver display(PINS, SPI);

// LVGL 缓冲区
#define DRAW_BUF_SIZE (DISPLAY_W * 40)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DRAW_BUF_SIZE];

// 显示刷新回调
void lvgl_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    int32_t idx = 0;

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint16_t px = area->x1 + x;
            uint16_t py = area->y1 + y;

            if (px < DISPLAY_W && py < DISPLAY_H) {
                display.writePoint(px, py, (bool)!color_map[idx].full);
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
    disp_drv.hor_res = DISPLAY_W;
    disp_drv.ver_res = DISPLAY_H;
    disp_drv.flush_cb = lvgl_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}