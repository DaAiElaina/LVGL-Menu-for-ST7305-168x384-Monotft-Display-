#ifndef LVGL_MENU_H
#define LVGL_MENU_H

#include <lvgl.h>
#include <Arduino.h>

void lvgl_encoder_init() ;
// 初始化菜单系统
void lvgl_menu_init();

// 获取当前焦点索引
int lvgl_menu_get_focus();

// 手动触发焦点切换（供外部使用）
void lvgl_menu_switch_focus(int delta);

// 获取当前是否在子菜单中
bool lvgl_menu_is_in_sub();

// 获取焦点组
lv_group_t* lvgl_menu_get_group();

#endif