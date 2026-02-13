#include <string.h>
#include "esp_lvgl_port.h"
#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"
#include "wifi_provisioning_api.h"
#include <string.h>
#include "esp_task_wdt.h"
// #include "qrcodegen.h"
#include "lvgl__lvgl/src/libs/qrcode/lv_qrcode.h"

#define WIFI_BAR_WIDTH 4
#define WIFI_BAR_SPACING 3
objects_t objects;
lv_obj_t *tick_value_change_obj;
static button_style_t btn1_style;
static button_style_t btn2_style;
static button_style_t btn3_style;
static button_style_t btn4_style;
// int loading_screen= 0;
// static lv_timer_t *loading_timer = NULL;
static lv_timer_t *colon_timer = NULL;
static lv_timer_t *sleep_timer = NULL;
static bool colon_visible = true;
//static lv_obj_t *boot_screen;
void reset_sleep_timer_in_screens();
static lv_obj_t *boot_container = NULL;
extern char provisioned_service_name[12];
extern void cleanup_wifi_check_timer();

// static void sleep_timer_cb(lv_timer_t * timer);
typedef void (*tick_screen_func_t)();

lv_obj_t *wifi_bars[4][4];  // 4个界面，每个4格信号

void create_wifi_icon(lv_obj_t *parent, int screen_index) {
    int base_x = 430;
    int base_y = 1;
    for (int i = 0; i < 4; i++) {
        lv_obj_t *bar = lv_obj_create(parent);
        lv_obj_set_size(bar, WIFI_BAR_WIDTH, (i + 1) * 6);
        lv_obj_set_style_radius(bar, 2, 0);
        lv_obj_set_style_bg_color(bar, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(bar, 0, 0);
        lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(bar, base_x + i * (WIFI_BAR_WIDTH + WIFI_BAR_SPACING), base_y + (3 - i) * 6);
        wifi_bars[screen_index][i] = bar;
    }
}

void update_wifi_icon_strength(int screen_index, int rssi) {
    int level = 0;

    if (rssi >= -55) level = 4;
    else if (rssi >= -65) level = 3;
    else if (rssi >= -75) level = 2;
    else if (rssi >= -85) level = 1;
    else level = 0;

    for (int i = 0; i < 4; i++) {
        if (wifi_bars[screen_index][i]) {
            lv_obj_set_style_bg_opa(wifi_bars[screen_index][i], i < level && shared_state ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        }
    }
}



void create_common_icons(lv_obj_t *parent_obj) {
    // 亮度图标
    lv_obj_t *brightness_img = lv_img_create(parent_obj);
    lv_img_set_src(brightness_img, &img___2);  
    lv_obj_set_pos(brightness_img, 387, 1);
    lv_obj_add_flag(brightness_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(brightness_img, on_brightness_click, LV_EVENT_CLICKED, NULL);

    // 音量图标
    lv_obj_t *volume_img = lv_img_create(parent_obj);
    lv_img_set_src(volume_img, &img___3);  
    lv_obj_set_pos(volume_img, 341, 1);
    lv_obj_add_flag(volume_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(volume_img, on_volume_click, LV_EVENT_CLICKED, NULL);
}

extern void set_button_styles();

void init_btn_styles(void) {

    btn1_style.default_color = lv_color_hex(0x666464);
    btn1_style.checked_color = lv_color_hex(0xffffff);
    btn1_style.default_img   = &img___1;
    btn1_style.checked_img   = &img___;
    btn1_style.btn = 1; 
    btn2_style.default_color = lv_color_hex(0x666464);
    btn2_style.checked_color = lv_color_hex(0xffffff);
    btn2_style.default_img   = &img___1;
    btn2_style.checked_img   = &img___;
    btn2_style.btn = 2;
    btn3_style.default_color = lv_color_hex(0x666464);
    btn3_style.checked_color = lv_color_hex(0xffffff);
    btn3_style.default_img   = &img___1;
    btn3_style.checked_img   = &img___;
    btn3_style.btn = 3;
    btn4_style.default_color = lv_color_hex(0x666464);
    btn4_style.checked_color = lv_color_hex(0xffffff);
    btn4_style.default_img   = &img___1;
    btn4_style.checked_img   = &img___;
    btn4_style.btn = 4;
    set_button_styles(&btn1_style, &btn2_style, &btn3_style, &btn4_style);
}













void create_screen_dormancy() {
    init_btn_styles();
    // start_gradient_animation();
    lvgl_port_lock(1);
    lv_obj_t *obj = lv_obj_create(NULL);
    objects.dormancy = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_add_event_cb(obj, action_action_slide, LV_EVENT_GESTURE, NULL);
    lvgl_port_unlock();
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_CHECKED);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Button1
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button1 = obj;
            lv_obj_set_pos(obj, 98, 78);
            lv_obj_set_size(obj, 117, 117);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x666464), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_image_src(obj, &img___1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);  // 使其支持 Toggle
            lv_obj_add_event_cb(obj, button_event_cb, LV_EVENT_VALUE_CHANGED, &btn1_style);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj0 = obj;
                    lv_obj_set_pos(obj, 23, 83);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "灯光1");
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Button2
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button2 = obj;
            lv_obj_set_pos(obj, 283, 79);
            lv_obj_set_size(obj, 117, 117);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff666464), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_image_src(obj, &img___1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);  // 使其支持 Toggle
            lv_obj_add_event_cb(obj, button_event_cb, LV_EVENT_VALUE_CHANGED, &btn2_style);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj1 = obj;
                    lv_obj_set_pos(obj, 23, 83);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "灯光2");
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Button3
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button3 = obj;
            lv_obj_set_pos(obj, 98, 303);
            lv_obj_set_size(obj, 117, 117);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff666464), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_image_src(obj, &img___1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);  // 使其支持 Toggle
            lv_obj_add_event_cb(obj, button_event_cb, LV_EVENT_VALUE_CHANGED, &btn3_style);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj2 = obj;
                    lv_obj_set_pos(obj, 23, 83);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "灯光3");
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            // Button4
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.button4 = obj;
            lv_obj_set_pos(obj, 283, 303);
            lv_obj_set_size(obj, 117, 117);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff666464), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_image_src(obj, &img___1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);  // 使其支持 Toggle
            lv_obj_add_event_cb(obj, button_event_cb, LV_EVENT_VALUE_CHANGED, &btn4_style);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj3 = obj;
                    lv_obj_set_pos(obj, 23, 83);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "灯光4");
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }


        create_common_icons(obj);
        create_wifi_icon(obj, 0);

    }
    
    tick_screen_dormancy();
}

static void wifi_config_screen_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_PRESSED || code == LV_EVENT_CLICKED || code == LV_EVENT_GESTURE) {
        reset_inactivity_timer();
    }
}

// 屏幕删除时的回调函数
static void wifi_config_screen_delete_cb(lv_event_t *e) {
    // 清理WiFi检查定时器，防止内存泄漏
    cleanup_wifi_check_timer();
}

// 释放资源的回调函数（如果使用动态分配）
static void event_handler_free_qr(lv_event_t *e) {
    lv_obj_t *qr = lv_event_get_target(e);
    if (lv_obj_get_user_data(qr)) {
        free(lv_obj_get_user_data(qr)); // 释放可能的附加数据
    }
}

static lv_timer_t *wifi_check_timer = NULL;

// 清理WiFi检查定时器
void cleanup_wifi_check_timer(void) {
    if (wifi_check_timer) {
        printf("[定时器清理] wifi_check_timer=%p\n", wifi_check_timer);
        lv_timer_del(wifi_check_timer);
        wifi_check_timer = NULL;
    }
}
void switch_to_main_screen(void *arg) {
    lvgl_port_lock(0); // 如果你用锁，一定要加在这里
    cleanup_wifi_check_timer();
    create_screens(); // 其中包含 create_screen_main()
    loadScreen(SCREEN_ID_MAIN); // 调用 lv_scr_load()
    lvgl_port_unlock();
}



static void wifi_status_check_timer_cb(lv_timer_t *timer) {
    if (shared_state) {
        // ❌ 不能直接调用 create_screens()，会导致内存访问错误
        // ✅ 使用异步回调安全切换
        lv_async_call(switch_to_main_screen, NULL);
        return;
    }
}


void show_wifi_config_info(void *arg) {
    lvgl_port_lock(0);
    // 创建全屏黑色背景
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 480, 480);
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 标题（居中顶部）
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "请配置Wi-Fi信息");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    
    // // 检查WiFi是否已连接
    // if(shared_state) {
    //     // 如果已经连接，确保清理定时器，但不进行页面切换
    //     cleanup_wifi_check_timer();
    //     lvgl_port_unlock();
    //     return;
    // }

    // 获取服务名称，优先使用传入的参数
    const char *service_name = arg ? (const char *)arg : provisioned_service_name;

    // 动态生成Wi-Fi配置字符串（符合WIFI QR标准格式）
    char qr_data_buffer[128];
    snprintf(qr_data_buffer, sizeof(qr_data_buffer), 
        "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"softap\"}",
        service_name,
        PROOF_OF_POSSESION
    );

    // 使用LVGL内置二维码生成器（更简洁）
    lv_obj_t *qr = lv_qrcode_create(scr);
    lv_obj_set_size(qr, 200, 200);
    lv_obj_set_style_bg_color(qr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_color(qr, lv_color_white(), LV_PART_MAIN);
    if (qr) {
        // 设置用户数据用于后续释放（可选）
        lv_obj_set_user_data(qr, NULL);
        lv_obj_add_event_cb(qr, event_handler_free_qr, LV_EVENT_DELETE, NULL);
        
        // 更新二维码内容
        if (lv_qrcode_update(qr, qr_data_buffer, strlen(qr_data_buffer)) != LV_RES_OK) {
            lv_obj_del(qr); // 删除失败的二维码对象
            goto qr_error;
        }
        lv_obj_align(qr, LV_ALIGN_CENTER, 0, 0);
    } else {
        goto qr_error;
    }

    // 底部提示文字
    lv_obj_t *label_qr = lv_label_create(scr);
    lv_label_set_text(label_qr, "请扫描二维码进行配网");
    lv_obj_set_style_text_color(label_qr, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_qr, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_qr, LV_ALIGN_BOTTOM_MID, 0, -20);

    // 添加屏幕事件处理器
    lv_obj_add_event_cb(scr, wifi_config_screen_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(scr, wifi_config_screen_delete_cb, LV_EVENT_DELETE, NULL);
    
    // 确保屏幕可点击以接收触摸事件
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_disp_load_scr(scr);
    
    // 启动WiFi状态检查定时器
    if (wifi_check_timer == NULL) {
        wifi_check_timer = lv_timer_create(wifi_status_check_timer_cb, 1000, NULL);
    } else {
        // 如果定时器已存在，先删除再重新创建
        lv_timer_del(wifi_check_timer);
        wifi_check_timer = lv_timer_create(wifi_status_check_timer_cb, 1000, NULL);
    }
    
    lvgl_port_unlock();
    return;

qr_error:
    // 错误处理时也清理定时器
    cleanup_wifi_check_timer();
    // 错误处理（居中显示红色错误信息）
    lv_obj_t *error_label = lv_label_create(scr);
    lv_label_set_text(error_label, "二维码生成失败");
    lv_obj_set_style_text_color(error_label, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
    
    // 仍然显示底部提示（可选）
    lv_obj_t *label_qr_2 = lv_label_create(scr);
    lv_label_set_text(label_qr_2, "请手动连接Wi-Fi配置");
    lv_obj_set_style_text_color(label_qr_2, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_text_font(label_qr_2, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_align(label_qr_2, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_disp_load_scr(scr);
    lvgl_port_unlock();
    vTaskDelete(NULL);
}



void tick_screen_dormancy() {
    update_wifi_icon_strength(0, get_wifi_rssi());
}

static void colon_blink_timer_cb(lv_timer_t *timer) {
    if (objects.obj5 == NULL) return;

    lv_label_set_text(objects.obj5, colon_visible ? " " : ":");
    colon_visible = !colon_visible;
}

void create_screen_main() {
    lvgl_port_lock(1);
    lv_obj_t *obj = lv_obj_create(NULL);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_add_event_cb(obj, action_action_slide, LV_EVENT_GESTURE, NULL);
    lvgl_port_unlock();
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    if (colon_timer == NULL) {
        colon_timer = lv_timer_create(colon_blink_timer_cb, 500, NULL);
    }
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_list_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 57, 45);
            lv_obj_set_size(obj, 371, 148);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // when
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.when = obj;
            lv_obj_set_pos(obj, 155, 71);
            lv_obj_set_size(obj, 64, 52);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 238, 66);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, ":");
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }



        {
            // points
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.points = obj;
            lv_obj_set_pos(obj, 282, 73);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // years
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.years = obj;
            lv_obj_set_pos(obj, 150, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 195, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "年");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // month
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.month = obj;
            lv_obj_set_pos(obj, 229, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 264, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "月");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // day
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.day = obj;
            lv_obj_set_pos(obj, 298, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj8 = obj;
            lv_obj_set_pos(obj, 334, 149);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "日");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_list_create(parent_obj);
            objects.obj9 = obj;
            lv_obj_set_pos(obj, 55, 207);
            lv_obj_set_size(obj, 90, 246);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj10 = obj;
            lv_obj_set_pos(obj, 68, 254);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "室内温度");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj11 = obj;
            lv_obj_set_pos(obj, 104, 226);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "℃");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // hum
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hum = obj;
            lv_obj_set_pos(obj, 81, 285);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj12 = obj;
            lv_obj_set_pos(obj, 68, 310);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "室内湿度");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // temp2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.temp2 = obj;
            lv_obj_set_pos(obj, 81, 343);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj13 = obj;
            lv_obj_set_pos(obj, 68, 367);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "室外温度");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // temp1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.temp1 = obj;
            lv_obj_set_pos(obj, 81, 226);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj14 = obj;
            lv_obj_set_pos(obj, 104, 343);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "℃");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj15 = obj;
            lv_obj_set_pos(obj, 104, 285);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "%");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj16 = obj;
            lv_obj_set_pos(obj, 104, 396);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "%");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj17 = obj;
            lv_obj_set_pos(obj, 66, 421);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "室外湿度");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // hum1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.hum1 = obj;
            lv_obj_set_pos(obj, 81, 396);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_list_create(parent_obj);
            objects.obj18 = obj;
            lv_obj_set_pos(obj, 174, 207);
            lv_obj_set_size(obj, 252, 246);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj19 = obj;
            lv_obj_set_pos(obj, 275, 216);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "天气");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weather_picture
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.weather_picture = obj;
            lv_obj_set_pos(obj, 198, 235);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img__);
            lv_image_set_inner_align(obj, LV_IMAGE_ALIGN_DEFAULT);
        }
        {
            // weather_picture1
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.weather_picture1 = obj;
            lv_obj_set_pos(obj, 224, 319);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img____2);
        }
        {
            // weather
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weather = obj;
            lv_obj_set_pos(obj, 277, 258);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weather_picture2
            lv_obj_t *obj = lv_img_create(parent_obj);
            objects.weather_picture2 = obj;
            lv_obj_set_pos(obj, 224, 387);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img____2);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj20 = obj;
            lv_obj_set_pos(obj, 331, 246);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "紫外线");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ultraviolet
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ultraviolet = obj;
            lv_obj_set_pos(obj, 350, 265);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffb32afb), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 186, 310);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img____);
        }
        {
            lv_obj_t *obj = lv_img_create(parent_obj);
            lv_obj_set_pos(obj, 186, 375);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_img_set_src(obj, &img____);
        }

        create_wifi_icon(obj, 1);
        create_common_icons(obj);
        {
            // weather1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weather1 = obj;
            lv_obj_set_pos(obj, 282, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // tom_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.tom_temp = obj;
            lv_obj_set_pos(obj, 348, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        {
            // weather2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weather2 = obj;
            lv_obj_set_pos(obj, 282, 406);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj21 = obj;
            lv_obj_set_pos(obj, 188, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "明日");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj22 = obj;
            lv_obj_set_pos(obj, 188, 401);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "后日");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // back_temp
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.back_temp = obj;
            lv_obj_set_pos(obj, 348, 401);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj23 = obj;
            lv_obj_set_pos(obj, 394, 334);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "℃");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj25 = obj;
            lv_obj_set_pos(obj, 395, 401);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "℃");
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT);
        }

    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        update_wifi_icon_strength(1, get_wifi_rssi());
        const char *new_val = get_var_when_t();
        const char *cur_val = lv_label_get_text(objects.when);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.when;
            lv_label_set_text(objects.when, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_points_t();
        const char *cur_val = lv_label_get_text(objects.points);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.points;
            lv_label_set_text(objects.points, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_years_t();
        const char *cur_val = lv_label_get_text(objects.years);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.years;
            lv_label_set_text(objects.years, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_month_t();
        const char *cur_val = lv_label_get_text(objects.month);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.month;
            lv_label_set_text(objects.month, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_day_t();
        const char *cur_val = lv_label_get_text(objects.day);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.day;
            lv_label_set_text(objects.day, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_hum_t();
        const char *cur_val = lv_label_get_text(objects.hum);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.hum;
            lv_label_set_text(objects.hum, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_temp2_t();
        const char *cur_val = lv_label_get_text(objects.temp2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.temp2;
            lv_label_set_text(objects.temp2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_temp1_t();
        const char *cur_val = lv_label_get_text(objects.temp1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.temp1;
            lv_label_set_text(objects.temp1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_hum1_t();
        const char *cur_val = lv_label_get_text(objects.hum1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.hum1;
            lv_label_set_text(objects.hum1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_weather_t();
        const char *cur_val = lv_label_get_text(objects.weather);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.weather;
            lv_label_set_text(objects.weather, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_ultraviolet_t();
        const char *cur_val = lv_label_get_text(objects.ultraviolet);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.ultraviolet;
            lv_label_set_text(objects.ultraviolet, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_weather1_t();
        const char *cur_val = lv_label_get_text(objects.weather1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.weather1;
            lv_label_set_text(objects.weather1, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_tom_temp_t();
        const char *cur_val = lv_label_get_text(objects.tom_temp);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.tom_temp;
            lv_label_set_text(objects.tom_temp, new_val);
            tick_value_change_obj = NULL;
        }
    }

    {
        const char *new_val = get_var_weather2_t();
        const char *cur_val = lv_label_get_text(objects.weather2);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.weather2;
            lv_label_set_text(objects.weather2, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_back_temp_t();
        const char *cur_val = lv_label_get_text(objects.back_temp);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.back_temp;
            lv_label_set_text(objects.back_temp, new_val);
            tick_value_change_obj = NULL;
        }
    }

    {
        const char *new_val = get_var_weather_picture_t();
        const char *cur_img_name = lv_img_get_src(objects.weather_picture);
        bool found = false;
        for (int i = 0; i < 48; i++) {
            if (strcmp(new_val, images[i].name) == 0) {
                if (cur_img_name != images[i].img_dsc) {
                    lv_img_set_src(objects.weather_picture, images[i].img_dsc);
                }
                found = true;
                break;
            }
        }
        if (!found && cur_img_name != &img__) {
            lv_img_set_src(objects.weather_picture, &img__);
        }
    }
    {
        const char *new_val = get_var_weather_picture1_t();
        const char *cur_img_name = lv_img_get_src(objects.weather_picture1);
        bool found = false;
        for (int i = 0; i < 48; i++) {
            if (strcmp(new_val, images[i].name) == 0) {
                if (cur_img_name != images[i].img_dsc) {
                    lv_img_set_src(objects.weather_picture1, images[i].img_dsc);
                }
                found = true;
                break;
            }
        }
        if (!found && cur_img_name != &img____2) {
            lv_img_set_src(objects.weather_picture1, &img____2);
        }
    }
    {
        const char *new_val = get_var_weather_picture2_t();
        const char *cur_img_name = lv_img_get_src(objects.weather_picture2);
        bool found = false;
        for (int i = 0; i < 48; i++) {
            if (strcmp(new_val, images[i].name) == 0) {
                if (cur_img_name != images[i].img_dsc) {
                    lv_img_set_src(objects.weather_picture2, images[i].img_dsc);
                }
                found = true;
                break;
            }
        }
        if (!found && cur_img_name != &img_____2) {
            lv_img_set_src(objects.weather_picture2, &img_____2);
        }
    }
}

void create_screen_p1(){
    lv_obj_t *parent_obj = lv_scr_act();
    
    // 初始化WebRTC连接
    init_webrtc_connection();
    
    // 创建视频播放器
    init_webrtc_video_player(parent_obj);
    
    lv_obj_t *obj = lv_obj_create(NULL);
    objects.p1 = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_add_event_cb(obj, action_action_slide, LV_EVENT_GESTURE, NULL);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);

    create_wifi_icon(obj, 2);
    create_common_icons(obj);
    tick_screen_p1();
}

void tick_screen_p1() {
    update_wifi_icon_strength(2, get_wifi_rssi());
}
void create_screen_function() {
    lv_obj_t *obj = lv_obj_create(NULL);
    objects.function = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_add_event_cb(obj, action_action_slide, LV_EVENT_GESTURE, NULL);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj27 = obj;
            lv_obj_set_pos(obj, 75, 69);
            lv_obj_set_size(obj, 213, 163);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff7221f3), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "灯光");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj28 = obj;
            lv_obj_set_pos(obj, 79, 232);
            lv_obj_set_size(obj, 141, 139);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa29106), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "自定义\n  开关");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            objects.obj29 = obj;
            lv_obj_set_pos(obj, 284, 69);
            lv_obj_set_size(obj, 146, 163);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffc2d8fc), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "空调");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
        {
            lv_obj_t *obj = lv_btn_create(parent_obj);
            lv_obj_set_pos(obj, 220, 232);
            lv_obj_set_size(obj, 210, 139);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_label_set_text(obj, "设置");
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_chinese_32, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
           // lv_obj_add_event_cb(obj, on_settings_click, LV_EVENT_CLICKED, NULL);
        }

        create_wifi_icon(obj, 3);
        create_common_icons(obj);

    }

    tick_screen_function();
}

void tick_screen_function() {
    update_wifi_icon_strength(3, get_wifi_rssi());
}


tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_function,
    tick_screen_dormancy,
    tick_screen_p1,
};
void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_dormancy();
    create_screen_main();
    create_screen_p1();
    create_screen_function();

    // sleep_timer = lv_timer_create(sleep_timer_cb, 5000, NULL); // 5秒无操作进入休眠
    // lv_timer_set_repeat_count(sleep_timer, -1); // 无限重复

}
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}
static void anim_set_opa_cb(void *obj, int32_t opa) {
    lv_obj_set_style_opa(obj, opa, 0);  // 0 表示默认 selector
}

void boot_animation_cb(lv_anim_t *a) {
    // 启动动画播放完成，直接进入主页面
    create_screens();
    loadScreen(SCREEN_ID_MAIN);
    
    // 安全删除 boot_container
    if (boot_container && lv_obj_is_valid(boot_container)) {
        lv_obj_del(boot_container);
        boot_container = NULL;
    }
}

static const char *boot_msgs[] = {
    " 正在初始化系统.....",
    " 正在加载界面.......",
    " 正在连接WiFi.......",
    "         准 备 完 成"
};

static int msg_index = 0;
static lv_obj_t *boot_label = NULL;

static void boot_msg_fadein_cb(lv_anim_t *a);
static void boot_msg_fadeout_cb(lv_anim_t *a);

static void boot_msg_fadein_cb(lv_anim_t *a) {
    // 第三条是"正在连接WiFi..."
    if (msg_index == 2) {
        // 检查Wi-Fi连接状态
        if (!shared_state) {
            // 如果连接失败，销毁启动动画并显示Wi-Fi配置信息
            if (boot_container && lv_obj_is_valid(boot_container)) {
                lv_obj_del(boot_container);
                boot_container = NULL;
            }
            show_wifi_config_info(NULL);
            return;
        }
        // 如果连接成功，继续正常流程
    }

    // 继续正常流程：启动淡出动画
    lv_anim_t fade_out;
    lv_anim_init(&fade_out);
    lv_anim_set_var(&fade_out, boot_label);
    lv_anim_set_values(&fade_out, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&fade_out, 500);
    lv_anim_set_delay(&fade_out, 500); // 停留时间
    lv_anim_set_exec_cb(&fade_out, anim_set_opa_cb);

    if (msg_index < 3) {
        lv_anim_set_ready_cb(&fade_out, boot_msg_fadeout_cb);  // 下一条
    } else {
        lv_anim_set_ready_cb(&fade_out, boot_animation_cb);    // 最后一条后跳转
    }

    lv_anim_start(&fade_out);
}


static void boot_msg_fadeout_cb(lv_anim_t *a) {
    msg_index++;
    if (msg_index >= 4) return;

    lv_label_set_text(boot_label, boot_msgs[msg_index]); 

    lv_anim_t fade_in;
    lv_anim_init(&fade_in);
    lv_anim_set_var(&fade_in, boot_label);
    lv_anim_set_values(&fade_in, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&fade_in, 500);
    lv_anim_set_exec_cb(&fade_in, anim_set_opa_cb);
    lv_anim_set_ready_cb(&fade_in, boot_msg_fadein_cb);
    lv_anim_start(&fade_in);
}

void create_boot_screen(void) {
    // 无论WiFi是否连接，都先播放完整启动动画
    lv_obj_t *parent = lv_scr_act(); // 获取当前活动屏幕
    lv_obj_set_style_bg_color(parent, lv_color_white(), 0); // 设置背景颜色为白色
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0); // 设置背景不透明度为完全覆盖

    boot_container = lv_obj_create(parent); // 创建容器对象
    lv_obj_set_size(boot_container, lv_obj_get_width(parent), lv_obj_get_height(parent)); // 设置容器大小为屏幕大小
    lv_obj_center(boot_container); // 将容器居中
    lv_obj_set_style_bg_color(boot_container, lv_color_white(), 0); // 设置容器背景颜色为白色
    lv_obj_set_style_bg_opa(boot_container, LV_OPA_COVER, 0); // 设置容器背景不透明度为完全覆盖
    lv_obj_clear_flag(boot_container, LV_OBJ_FLAG_SCROLLABLE); // 清除容器的可滚动标志

    // 加载圆圈
    lv_obj_t *spinner = lv_spinner_create(boot_container); // 创建加载动画
    lv_obj_set_size(spinner, 120, 120); // 设置加载动画大小
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -60); // 将加载动画居中并向上偏移60像素

    // 创建消息 label（复用）
    msg_index = 0; // 初始化消息索引
    boot_label = lv_label_create(boot_container); // 创建标签对象
    lv_obj_set_style_text_font(boot_label, &ui_font_chinese, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置标签字体
    lv_obj_set_style_text_color(boot_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT); // 设置标签文本颜色为黑色
    lv_obj_set_size(boot_label, 200, 50); // 设置标签大小
    lv_label_set_text(boot_label, boot_msgs[msg_index]); // 设置标签文本内容
    lv_obj_align(boot_label, LV_ALIGN_CENTER, 20, 40); // 将标签居中并向下偏移40像素
    lv_obj_set_style_opa(boot_label, LV_OPA_TRANSP, 0); // 设置标签初始不透明度为透明

    // 启动第一条动画
    lv_anim_t a; // 定义动画对象
    lv_anim_init(&a); // 初始化动画
    lv_anim_set_var(&a, boot_label); // 设置动画目标为标签
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER); // 设置动画值从透明到不透明
    lv_anim_set_time(&a, 500); // 设置动画时间为500毫秒
    lv_anim_set_exec_cb(&a, anim_set_opa_cb); // 设置动画执行回调函数
    lv_anim_set_ready_cb(&a, boot_msg_fadein_cb); // 设置动画完成回调函数
    lv_anim_start(&a); // 启动动画
}
// static void sleep_timer_cb(lv_timer_t * timer) {
//     // 当计时器触发时，切换到休眠屏幕
//     loadScreen(SCREEN_ID_DORMANCY);
// }


void reset_sleep_timer_in_screens() {
    if (sleep_timer) {
        lv_timer_reset(sleep_timer);
        lv_timer_resume(sleep_timer);
    }
}