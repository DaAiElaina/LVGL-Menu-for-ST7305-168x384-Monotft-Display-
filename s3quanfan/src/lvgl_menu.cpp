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

static lv_obj_t *focus_indicator = NULL;
static lv_obj_t *text_layer = NULL;

static int current_focus = 0;
static int saved_focus = 0;
static bool in_sub_menu = false;
static lv_group_t *menu_group = NULL;

// 前向声明
static void show_temp_popup(const char *title, const char *message, int duration_ms);
static void create_main_menu();
static void create_sub_menu();

// ========== 动画回调 ==========
static void anim_set_x(void *var, int32_t value) {
    lv_obj_set_x((lv_obj_t *)var, value);
}

static void anim_set_y(void *var, int32_t value) {
    lv_obj_set_y((lv_obj_t *)var, value);
}

// ========== 移动方块并更新文字颜色 ==========
static void move_indicator_to(lv_obj_t *target_btn) {
    if (!focus_indicator || !target_btn) return;
    
    lv_coord_t target_x = lv_obj_get_x(target_btn);
    lv_coord_t target_y = lv_obj_get_y(target_btn);
    lv_coord_t target_w = lv_obj_get_width(target_btn);
    lv_coord_t target_h = lv_obj_get_height(target_btn);
    
    lv_coord_t current_x = lv_obj_get_x(focus_indicator);
    lv_coord_t current_y = lv_obj_get_y(focus_indicator);
    
    lv_obj_set_size(focus_indicator, target_w, target_h);
    
    if (target_btn == btn_enter) {
        lv_obj_set_style_text_color(label_enter, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_settings, lv_color_black(), 0);
    } else {
        lv_obj_set_style_text_color(label_enter, lv_color_black(), 0);
        lv_obj_set_style_text_color(label_settings, lv_color_white(), 0);
    }
    
    if (target_x != current_x) {
        lv_anim_t a_x;
        lv_anim_init(&a_x);
        lv_anim_set_var(&a_x, focus_indicator);
        lv_anim_set_exec_cb(&a_x, anim_set_x);
        lv_anim_set_values(&a_x, current_x, target_x);
        lv_anim_set_time(&a_x, 200);
        lv_anim_set_path_cb(&a_x, lv_anim_path_ease_out);
        lv_anim_start(&a_x);
    }
    
    if (target_y != current_y) {
        lv_anim_t a_y;
        lv_anim_init(&a_y);
        lv_anim_set_var(&a_y, focus_indicator);
        lv_anim_set_exec_cb(&a_y, anim_set_y);
        lv_anim_set_values(&a_y, current_y, target_y);
        lv_anim_set_time(&a_y, 200);
        lv_anim_set_path_cb(&a_y, lv_anim_path_ease_out);
        lv_anim_start(&a_y);
    }
}

// ========== 刷新指示器 ==========
static void refresh_indicator() {
    if (!focus_indicator) return;
    
    lv_obj_t *target_btn = (current_focus == 0) ? btn_enter : btn_settings;
    if (!target_btn) return;
    
    lv_obj_set_pos(focus_indicator, lv_obj_get_x(target_btn), lv_obj_get_y(target_btn));
    lv_obj_set_size(focus_indicator, lv_obj_get_width(target_btn), lv_obj_get_height(target_btn));
    
    if (current_focus == 0) {
        lv_obj_set_style_text_color(label_enter, lv_color_white(), 0);
        lv_obj_set_style_text_color(label_settings, lv_color_black(), 0);
    } else {
        lv_obj_set_style_text_color(label_enter, lv_color_black(), 0);
        lv_obj_set_style_text_color(label_settings, lv_color_white(), 0);
    }
}

// ========== 获取焦点组 ==========
lv_group_t* lvgl_menu_get_group() {
    if (!menu_group) {
        menu_group = lv_group_create();
        lv_group_set_default(menu_group);
    }
    return menu_group;
}

int lvgl_menu_get_focus() {
    return current_focus;
}

bool lvgl_menu_is_in_sub() {
    return in_sub_menu;
}

void lvgl_menu_save_focus() {
    saved_focus = current_focus;
}

void lvgl_menu_restore_focus() {
    if (!in_sub_menu) {
        current_focus = saved_focus;
        refresh_indicator();
        
        if (menu_group) {
            lv_obj_t *target_btn = (current_focus == 0) ? btn_enter : btn_settings;
            if (target_btn) lv_group_focus_obj(target_btn);
        }
    }
}

// ========== 切换焦点 ==========
void lvgl_menu_switch_focus(int delta) {
    if (in_sub_menu) return;
    
    int num_buttons = 2;
    int new_focus = current_focus + delta;
    
    if (new_focus < 0) new_focus = num_buttons - 1;
    if (new_focus >= num_buttons) new_focus = 0;
    
    if (new_focus != current_focus) {
        lv_obj_t *target_btn = (new_focus == 0) ? btn_enter : btn_settings;
        move_indicator_to(target_btn);
        current_focus = new_focus;
        
        if (menu_group && target_btn) {
            lv_group_focus_obj(target_btn);
        }
    }
}

// ========== 弹出窗口 ==========
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

// ========== 设置按钮样式 ==========
static void setup_button_style(lv_obj_t *btn) {
    lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_black(), 0);
}

// ========== 创建黑色方块 ==========
static void create_focus_indicator() {
    if (focus_indicator) {
        lv_obj_del(focus_indicator);
    }
    
    if (!main_menu) return;
    
    lv_obj_t *target_btn = (current_focus == 0) ? btn_enter : btn_settings;
    if (!target_btn) return;
    
    focus_indicator = lv_obj_create(main_menu);
    lv_obj_set_size(focus_indicator, lv_obj_get_width(target_btn), lv_obj_get_height(target_btn));
    lv_obj_set_pos(focus_indicator, lv_obj_get_x(target_btn), lv_obj_get_y(target_btn));
    lv_obj_set_style_bg_color(focus_indicator, lv_color_black(), 0);
    lv_obj_set_style_border_width(focus_indicator, 0, 0);
    lv_obj_set_style_radius(focus_indicator, 4, 0);
    lv_obj_set_style_pad_all(focus_indicator, 0, 0);
    lv_obj_clear_flag(focus_indicator, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(focus_indicator, LV_OBJ_FLAG_CLICK_FOCUSABLE);
}

// ========== 创建文字图层 ==========
static void create_text_layer() {
    if (text_layer) {
        lv_obj_del(text_layer);
    }
    
    text_layer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(text_layer, DISPLAY_W, DISPLAY_H);
    lv_obj_set_style_bg_opa(text_layer, LV_OPA_0, 0);
    lv_obj_set_style_border_width(text_layer, 0, 0);
    lv_obj_set_style_pad_all(text_layer, 0, 0);
    lv_obj_clear_flag(text_layer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(text_layer, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_move_foreground(text_layer);
}

// ========== 更新文字位置（LVGL v8 兼容）==========
static void update_label_positions() {
    // 获取按钮的位置和大小
    lv_coord_t enter_x = lv_obj_get_x(btn_enter);
    lv_coord_t enter_y = lv_obj_get_y(btn_enter);
    lv_coord_t enter_w = lv_obj_get_width(btn_enter);
    lv_coord_t enter_h = lv_obj_get_height(btn_enter);
    
    lv_coord_t settings_x = lv_obj_get_x(btn_settings);
    lv_coord_t settings_y = lv_obj_get_y(btn_settings);
    lv_coord_t settings_w = lv_obj_get_width(btn_settings);
    lv_coord_t settings_h = lv_obj_get_height(btn_settings);
    
    // 获取文字的实际宽度和高度（通过 lv_label_get_* 函数）
    lv_coord_t enter_w_txt = lv_obj_get_width(label_enter);
    lv_coord_t enter_h_txt = lv_obj_get_height(label_enter);
    lv_coord_t settings_w_txt = lv_obj_get_width(label_settings);
    lv_coord_t settings_h_txt = lv_obj_get_height(label_settings);
    
    // 计算居中位置
    lv_coord_t enter_label_x = enter_x + (enter_w - enter_w_txt) / 2;
    lv_coord_t enter_label_y = enter_y + (enter_h - enter_h_txt) / 2;
    lv_coord_t settings_label_x = settings_x + (settings_w - settings_w_txt) / 2;
    lv_coord_t settings_label_y = settings_y + (settings_h - settings_h_txt) / 2;
    
    lv_obj_set_pos(label_enter, enter_label_x, enter_label_y);
    lv_obj_set_pos(label_settings, settings_label_x, settings_label_y);
}

// ========== 创建主菜单 ==========
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
    lv_label_set_text(title, "LVGL Menu Test");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // 署名
    lv_obj_t *subtitle = lv_label_create(main_menu);
    lv_label_set_text(subtitle, "by qingxian2233  v0.0.1");
    lv_obj_set_style_text_color(subtitle, lv_color_black(), 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // 按钮横向排列
    int btn_w = 120;
    int btn_h = 40;
    int btn_spacing = 20;
    int total_width = btn_w * 2 + btn_spacing;
    int start_x = (DISPLAY_W - total_width) / 2;
    int center_y = DISPLAY_H / 2;
    
    // Enter 按钮
    btn_enter = lv_btn_create(main_menu);
    lv_obj_set_size(btn_enter, btn_w, btn_h);
    lv_obj_set_pos(btn_enter, start_x, center_y - btn_h/2);
    setup_button_style(btn_enter);
    
    lv_obj_add_event_cb(btn_enter, [](lv_event_t *e) {
        Serial.println("Enter clicked - switching to sub menu");
        lvgl_menu_save_focus();
        create_sub_menu();
    }, LV_EVENT_CLICKED, NULL);

    // Settings 按钮
    btn_settings = lv_btn_create(main_menu);
    lv_obj_set_size(btn_settings, btn_w, btn_h);
    lv_obj_set_pos(btn_settings, start_x + btn_w + btn_spacing, center_y - btn_h/2);
    setup_button_style(btn_settings);
    
    lv_obj_add_event_cb(btn_settings, [](lv_event_t *e) {
        Serial.println("Settings clicked - showing popup");
        show_temp_popup("Settings", "Coming Soon!", 1500);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_update_layout(main_menu);
    
    // 创建文字图层
    create_text_layer();
    
    // 创建文字标签
    label_enter = lv_label_create(text_layer);
    lv_label_set_text(label_enter, "Enter");
    lv_obj_set_style_text_color(label_enter, lv_color_white(), 0);
    
    label_settings = lv_label_create(text_layer);
    lv_label_set_text(label_settings, "Settings");
    lv_obj_set_style_text_color(label_settings, lv_color_black(), 0);
    
    // 先更新一次布局，让标签获得实际大小
    lv_obj_update_layout(text_layer);
    
    // 计算并设置文字位置
    update_label_positions();
    
    // 创建黑色方块
    create_focus_indicator();
    
    // 焦点组
    lv_group_t *group = lvgl_menu_get_group();
    if (group) {
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, btn_enter);
        lv_group_add_obj(group, btn_settings);
        lv_group_focus_obj(btn_enter);
    }
    
    Serial.println("Main menu created");
}

// ========== 创建子菜单 ==========
static void create_sub_menu() {
    Serial.println("Creating sub menu...");
    in_sub_menu = true;
    current_focus = 0;
    
    if (main_menu) {
        lv_obj_add_flag(main_menu, LV_OBJ_FLAG_HIDDEN);
    }
    if (text_layer) {
        lv_obj_add_flag(text_layer, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (sub_menu) {
        lv_obj_del(sub_menu);
        sub_menu = NULL;
    }

    sub_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(sub_menu, DISPLAY_W, DISPLAY_H);
    lv_obj_set_style_bg_color(sub_menu, lv_color_white(), 0);
    lv_obj_set_style_border_width(sub_menu, 0, 0);

    lv_obj_t *title = lv_label_create(sub_menu);
    lv_label_set_text(title, "Sub Menu");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *info = lv_label_create(sub_menu);
    lv_label_set_text(info, "This is the sub menu page\n\nPress to go back");
    lv_obj_set_style_text_color(info, lv_color_black(), 0);
    lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);

    back_btn = lv_btn_create(sub_menu);
    lv_obj_set_size(back_btn, 100, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(back_btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        if (sub_menu) {
            lv_obj_del(sub_menu);
            sub_menu = NULL;
            back_btn = NULL;
        }
        
        if (main_menu) {
            lv_obj_clear_flag(main_menu, LV_OBJ_FLAG_HIDDEN);
        }
        if (text_layer) {
            lv_obj_clear_flag(text_layer, LV_OBJ_FLAG_HIDDEN);
            update_label_positions();
            lv_obj_move_foreground(text_layer);
        }
        
        refresh_indicator();
        in_sub_menu = false;
        lvgl_menu_restore_focus();
        
        lv_group_t *group = lvgl_menu_get_group();
        if (group) {
            lv_group_remove_all_objs(group);
            lv_group_add_obj(group, btn_enter);
            lv_group_add_obj(group, btn_settings);
            lv_obj_t *target = (current_focus == 0) ? btn_enter : btn_settings;
            if (target) lv_group_focus_obj(target);
        }
    }, LV_EVENT_CLICKED, NULL);

    label_back = lv_label_create(back_btn);
    lv_label_set_text(label_back, "Back");
    lv_obj_set_style_text_color(label_back, lv_color_white(), 0);
    lv_obj_center(label_back);
    
    lv_group_t *group = lvgl_menu_get_group();
    if (group) {
        lv_group_remove_all_objs(group);
        lv_group_add_obj(group, back_btn);
        lv_group_focus_obj(back_btn);
    }
}

// ========== 初始化 ==========
void lvgl_menu_init() {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    
    lvgl_menu_get_group();
    create_main_menu();
}