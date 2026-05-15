#ifndef LVGL_MENU_H
#define LVGL_MENU_H

#include <lvgl.h>
#include "lvgl_driver_screen.h"
#include <Arduino.h>

#define DISPLAY_W   LV_W
#define DISPLAY_H   LV_H

void lvgl_menu_init();
void lvgl_menu_switch_focus(int delta);
bool lvgl_menu_is_in_sub();
lv_group_t* lvgl_menu_get_group();
void lvgl_menu_save_focus();
void lvgl_menu_restore_focus();

#endif