#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "driver/gptimer.h"
#include "esp_sleep.h"
#include "images.h"
#include "screens.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "esp_webrtc.h"
#include "esp_peer_types.h"
#include "av_render.h"
#include "esp_timer.h"
#include "esp_jpeg_dec.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "nvs.h"
#include "esp32_s3_szp.h"
#include "esp_lcd_touch.h"
#include "mqtt.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "fonts.h"

extern void action_action_slide(lv_event_t * e);
extern void reset_inactivity_timer();

typedef struct {
    lv_obj_t *video_screen;
    lv_obj_t *video_img;
    lv_obj_t *video_back_btn;
    lv_color_t default_color;    // 默认状态颜色
    const void *default_img;     // 默认状态图片
    lv_color_t checked_color;    // 选中状态颜色
    const void *checked_img;     // 选中状态图片
    int btn;                     // 按钮编号

} button_style_t;
void init_webrtc_connection();
void init_webrtc_video_player(lv_obj_t *parent);
// 根据传入的样式更新按钮外观（根据当前状态判断使用哪种样式）
void update_button_state_ex(lv_obj_t *button, const button_style_t *style);

// 按钮状态变化的回调函数（通过 LV_EVENT_VALUE_CHANGED 触发）
void button_event_cb(lv_event_t *e);
// 如果需要提供手动切换状态的函数，也可以保留
void toggle_button(lv_obj_t *button);
void video_receiver_init(void);
void on_volume_click(lv_event_t *e);
void on_brightness_click(lv_event_t *e);
void backlight_pwm_init(void);
void set_backlight_brightness(uint32_t percent);
void mqtt_ui_init(void);
void on_settings_click(lv_event_t *e);
#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/