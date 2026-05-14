#include <Arduino.h>
#include <lvgl.h>
#include "lvgl_driver_screen.h"
#include "lvgl_driver_encoder.h"
#include "lvgl_menu.h"

void setup() {
    Serial.begin(115200);
    delay(100);

    // 初始化显示驱动
    lvgl_screen_init();

    // 初始化摇杆输入驱动
    lvgl_encoder_init();

    // 初始化菜单系统
    lvgl_menu_init();

    Serial.println("Ready");
}

void loop() {
    lv_tick_inc(5);
    lv_timer_handler();
    delay(5);
}