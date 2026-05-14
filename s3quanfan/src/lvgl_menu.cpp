#include "lvgl_menu.h"
#include "lvgl_driver_encoder.h"

static lv_obj_t *main_menu = NULL;
static lv_obj_t *sub_menu = NULL;
static lv_obj_t *btn_enter = NULL;
static lv_obj_t *btn_settings = NULL;
static lv_obj_t *back_btn = NULL;
static lv_obj_t *popup_window = NULL;

static lv_obj_t *label_enter = NULL;
static lv_obj_t *label_settings = NULL;
static lv_obj_t *label_back = NULL;

static int current_focus = 0;
static bool in_sub_menu = false;
static lv_group_t *menu_group = NULL;

// 前向声明
static void update_focus_style();
static void show_temp_popup(const char *title, const char *message, int duration_ms);
static void create_main_menu();
static void create_sub_menu();

// 获取焦点组
lv_group_t* lvgl_menu_get_group() {
    if (!menu_group) {
        menu_group = lv_group_create();
        lv_group_set_default(menu_group);
    }
    return menu_group;
}

// 获取当前焦点索引
int lvgl_menu_get_focus() {
    return current_focus;
}

// 手动触发焦点切换
void lvgl_menu_switch_focus(int delta) {
    if (in_sub_menu) return;
    
    int num_buttons = 2;
    int new_focus = current_focus + delta;
    
    if (new_focus < 0) {
        new_focus = num_buttons - 1;
    } else if (new_focus >= num_buttons) {
        new_focus = 0;
    }
    
    if (new_focus != current_focus) {
        current_focus = new_focus;
        update_focus_style();
        
        // 同时更新 LVGL 焦点组的焦点
        if (menu_group) {
            if (current_focus == 0 && btn_enter) {
                lv_group_focus_obj(btn_enter);
            } else if (current_focus == 1 && btn_settings) {
                lv_group_focus_obj(btn_settings);
            }
        }
        
        Serial.printf("Focus switched to %d\n", current_focus);
    }
}

// 获取当前是否在子菜单中
bool lvgl_menu_is_in_sub() {
    return in_sub_menu;
}

// 更新焦点样式
static void update_focus_style() {
    if (in_sub_menu) {
        if (!back_btn) return;
        
        if (current_focus == 0) {
            lv_obj_set_style_bg_color(back_btn, lv_color_black(), 0);
            lv_obj_set_style_border_width(back_btn, 2, 0);
            lv_obj_set_style_border_color(back_btn, lv_color_black(), 0);
            if (label_back) {
                lv_obj_set_style_text_color(label_back, lv_color_white(), 0);
            }
        }
    } else {
        if (!btn_enter || !btn_settings) return;
        
        // Enter 按钮
        if (current_focus == 0) {
            lv_obj_set_style_bg_color(btn_enter, lv_color_black(), 0);
            lv_obj_set_style_border_width(btn_enter, 2, 0);
            if (label_enter) {
                lv_obj_set_style_text_color(label_enter, lv_color_white(), 0);
            }
        } else {
            lv_obj_set_style_bg_color(btn_enter, lv_color_white(), 0);
            lv_obj_set_style_border_width(btn_enter, 1, 0);
            if (label_enter) {
                lv_obj_set_style_text_color(label_enter, lv_color_black(), 0);
            }
        }
        
        // Settings 按钮
        if (current_focus == 1) {
            lv_obj_set_style_bg_color(btn_settings, lv_color_black(), 0);
            lv_obj_set_style_border_width(btn_settings, 2, 0);
            if (label_settings) {
                lv_obj_set_style_text_color(label_settings, lv_color_white(), 0);
            }
        } else {
            lv_obj_set_style_bg_color(btn_settings, lv_color_white(), 0);
            lv_obj_set_style_border_width(btn_settings, 1, 0);
            if (label_settings) {
                lv_obj_set_style_text_color(label_settings, lv_color_black(), 0);
            }
        }
        
        lv_obj_set_style_border_color(btn_enter, lv_color_black(), 0);
        lv_obj_set_style_border_color(btn_settings, lv_color_black(), 0);
    }
    
    lv_obj_invalidate(lv_scr_act());
}

// 显示临时弹出窗口
static void show_temp_popup(const char *title, const char *message, int duration_ms) {
    if (popup_window) {
        lv_obj_del(popup_window);
        popup_window = NULL;
    }
    
    popup_window = lv_obj_create(lv_scr_act());
    lv_obj_set_size(popup_window, 140, 80);
    lv_obj_center(popup_window);
    lv_obj_set_style_bg_color(popup_window, lv_color_white(), 0);
    lv_obj_set_style_border_color(popup_window, lv_color_black(), 0);
    lv_obj_set_style_border_width(popup_window, 2, 0);
    
    lv_obj_t *popup_title = lv_label_create(popup_window);
    lv_label_set_text(popup_title, title);
    lv_obj_set_style_text_color(popup_title, lv_color_black(), 0);
    lv_obj_align(popup_title, LV_ALIGN_TOP_MID, 0, 10);
    
    lv_obj_t *popup_msg = lv_label_create(popup_window);
    lv_label_set_text(popup_msg, message);
    lv_obj_set_style_text_color(popup_msg, lv_color_black(), 0);
    lv_obj_align(popup_msg, LV_ALIGN_CENTER, 0, 0);
    
    lv_timer_t *timer = lv_timer_create([](lv_timer_t *t) {
        if (popup_window) {
            lv_obj_del(popup_window);
            popup_window = NULL;
        }
        lv_timer_del(t);
    }, duration_ms, NULL);
    lv_timer_set_repeat_count(timer, 1);
}

// 创建主菜单
static void create_main_menu() {
    Serial.println("Creating main menu...");
    in_sub_menu = false;
    current_focus = 0;
    
    if (main_menu) {
        lv_obj_del(main_menu);
        main_menu = NULL;
    }

    main_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_menu, DISPLAY_W, DISPLAY_H);
    lv_obj_set_style_bg_color(main_menu, lv_color_white(), 0);
    lv_obj_set_style_border_width(main_menu, 0, 0);
    lv_obj_set_style_pad_all(main_menu, 0, 0);

    // 标题
    lv_obj_t *title = lv_label_create(main_menu);
    lv_label_set_text(title, "LVGL menu test");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // 署名
    lv_obj_t *subtitle = lv_label_create(main_menu);
    lv_label_set_text(subtitle, "by qingxian2233  v0.0.1");
    lv_obj_set_style_text_color(subtitle, lv_color_black(), 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // Enter 按钮
    btn_enter = lv_btn_create(main_menu);
    lv_obj_set_size(btn_enter, 140, 40);
    lv_obj_set_pos(btn_enter, (DISPLAY_W - 140) / 2, 120);
    
    lv_obj_add_event_cb(btn_enter, [](lv_event_t *e) {
        Serial.println("Enter button clicked - switching to sub menu");
        create_sub_menu();
    }, LV_EVENT_CLICKED, NULL);

    label_enter = lv_label_create(btn_enter);
    lv_label_set_text(label_enter, "Enter");
    lv_obj_center(label_enter);

    // Settings 按钮
    btn_settings = lv_btn_create(main_menu);
    lv_obj_set_size(btn_settings, 140, 40);
    lv_obj_set_pos(btn_settings, (DISPLAY_W - 140) / 2, 180);
    
    lv_obj_add_event_cb(btn_settings, [](lv_event_t *e) {
        Serial.println("Settings button clicked - showing popup");
        show_temp_popup("Settings", "Coming Soon!", 1500);
    }, LV_EVENT_CLICKED, NULL);

    label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, "Settings");
    lv_obj_center(label_settings);

    // 更新焦点样式
    update_focus_style();
    
    // 将按钮添加到焦点组
    lv_group_t *group = lvgl_menu_get_group();
    if (group) {
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, btn_enter);
        lv_group_add_obj(group, btn_settings);
        lv_group_focus_obj(btn_enter);
    }
    
    Serial.println("Main menu created");
}

// 创建子菜单
static void create_sub_menu() {
    Serial.println("Creating sub menu...");
    in_sub_menu = true;
    current_focus = 0;
    
    if (main_menu) {
        lv_obj_add_flag(main_menu, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (sub_menu) {
        lv_obj_del(sub_menu);
        sub_menu = NULL;
    }

    sub_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(sub_menu, DISPLAY_W, DISPLAY_H);
    lv_obj_set_style_bg_color(sub_menu, lv_color_white(), 0);
    lv_obj_set_style_border_width(sub_menu, 0, 0);

    // 标题
    lv_obj_t *title = lv_label_create(sub_menu);
    lv_label_set_text(title, "Sub Menu test");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // 信息
    lv_obj_t *info = lv_label_create(sub_menu);
    lv_label_set_text(info, "This is the\nsub menu page\n\n\nPress to go back");
    lv_obj_set_style_text_color(info, lv_color_black(), 0);
    lv_obj_align(info, LV_ALIGN_CENTER, 0, -20);

    // Back 按钮
    back_btn = lv_btn_create(sub_menu);
    lv_obj_set_size(back_btn, 100, 40);
    lv_obj_set_pos(back_btn, (DISPLAY_W - 100) / 2, 300);
    
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        Serial.println("Back button clicked - returning to main menu");
        
        if (sub_menu) {
            lv_obj_del(sub_menu);
            sub_menu = NULL;
            back_btn = NULL;
        }
        
        if (main_menu) {
            lv_obj_clear_flag(main_menu, LV_OBJ_FLAG_HIDDEN);
        }
        
        in_sub_menu = false;
        current_focus = 0;
        update_focus_style();
        
        // 重新添加按钮到焦点组
        lv_group_t *group = lvgl_menu_get_group();
        if (group) {
            lv_group_remove_all_objs(group);
            lv_group_add_obj(group, btn_enter);
            lv_group_add_obj(group, btn_settings);
            lv_group_focus_obj(btn_enter);
        }
    }, LV_EVENT_CLICKED, NULL);

    label_back = lv_label_create(back_btn);
    lv_label_set_text(label_back, "Back");
    lv_obj_center(label_back);

    lv_obj_set_style_bg_color(back_btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(back_btn, 2, 0);
    lv_obj_set_style_border_color(back_btn, lv_color_black(), 0);
    lv_obj_set_style_text_color(label_back, lv_color_white(), 0);
    
    // 添加到焦点组
    lv_group_t *group = lvgl_menu_get_group();
    if (group) {
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, back_btn);
        lv_group_focus_obj(back_btn);
    }
    
    Serial.println("Sub menu created");
}

// 初始化菜单系统
void lvgl_menu_init() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    
    // 创建默认焦点组
    lvgl_menu_get_group();
    
    create_main_menu();
    Serial.println("Menu system initialized");
}