#include "lvgl_driver_encoder.h"
#include "lvgl_menu.h"  // 需要引入菜单头文件来调用焦点切换

#define JOY_X_PIN   11
#define JOY_Y_PIN   12
#define JOY_SW_PIN  48

static lv_indev_t *indev = NULL;
static unsigned long last_key_time = 0;
static const unsigned long key_delay = 200;

// 摇杆读取回调
static void encoder_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    int x = analogRead(JOY_X_PIN);
    int y = analogRead(JOY_Y_PIN);
    bool pressed = (digitalRead(JOY_SW_PIN) == LOW);

    const int center = 2048;
    const int threshold = 600;
    
    // 防重复
    if (millis() - last_key_time < key_delay) {
        data->state = LV_INDEV_STATE_RELEASED;
        data->key = 0;
        return;
    }
    
    uint32_t act_key = 0;
    bool key_handled = false;
    
    // 获取当前是否在子菜单中
    bool in_sub_menu = lvgl_menu_is_in_sub();
    
    if (in_sub_menu) {
        // ========== 子菜单模式 ==========
        if (pressed && !key_handled) {
            act_key = LV_KEY_ENTER;
            last_key_time = millis();
            key_handled = true;
            
            // 触发 Back 按钮点击（通过 LVGL 事件）
            lv_group_t *group = lv_group_get_default();
            if (group) {
                lv_obj_t *focused = lv_group_get_focused(group);
                if (focused) {
                    lv_event_send(focused, LV_EVENT_CLICKED, NULL);
                }
            }
        }
    } else {
        // ========== 主菜单模式 ==========
        // 上下左右切换焦点
        if (y < center - threshold) {
            act_key = LV_KEY_UP;
            last_key_time = millis();
            key_handled = true;
            lvgl_menu_switch_focus(-1);  // 向上切换
            
        } else if (y > center + threshold) {
            act_key = LV_KEY_DOWN;
            last_key_time = millis();
            key_handled = true;
            lvgl_menu_switch_focus(1);   // 向下切换
        }
        
        // 左右也可以切换
        if (!key_handled && x < center - threshold) {
            act_key = LV_KEY_LEFT;
            last_key_time = millis();
            key_handled = true;
            lvgl_menu_switch_focus(-1);
            
        } else if (!key_handled && x > center + threshold) {
            act_key = LV_KEY_RIGHT;
            last_key_time = millis();
            key_handled = true;
            lvgl_menu_switch_focus(1);
        }
        
        // 按键确认
        static bool last_pressed = false;
        if (pressed && !last_pressed && !key_handled) {
            act_key = LV_KEY_ENTER;
            last_key_time = millis();
            key_handled = true;
            
            // 触发当前焦点按钮的点击事件
            lv_group_t *group = lv_group_get_default();
            if (group) {
                lv_obj_t *focused = lv_group_get_focused(group);
                if (focused) {
                    lv_event_send(focused, LV_EVENT_CLICKED, NULL);
                }
            }
        }
        last_pressed = pressed;
    }
    
    if (act_key != 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = act_key;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// 初始化摇杆输入驱动
void lvgl_encoder_init() {
    pinMode(JOY_X_PIN, INPUT);
    pinMode(JOY_Y_PIN, INPUT);
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = encoder_read;
    indev = lv_indev_drv_register(&indev_drv);
    
    // 注意：焦点组由菜单模块创建，这里不重复创建
    // 获取菜单模块创建的焦点组
    lv_group_t *group = lv_group_get_default();
    if (group) {
        lv_indev_set_group(indev, group);
    }
}

// 获取输入设备指针
lv_indev_t* lvgl_get_indev() {
    return indev;
}