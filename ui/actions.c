#include "actions.h"

static const char *TAG = "actions";

// 外部函数声明（来自vars.cpp的C接口）
extern void set_var_temp1_t(const char *value);
extern void set_var_hum_t(const char *value);
extern void set_var_temp2_t(const char *value);
extern void set_var_hum1_t(const char *value);
extern void set_var_tom_temp_t(const char *value);
extern void set_var_back_temp_t(const char *value);
extern void set_var_weather_picture_t(const char *value);
extern void set_var_weather_picture1_t(const char *value);
extern void set_var_weather_picture2_t(const char *value);
extern void set_var_ultraviolet_t(const char *value);
extern void set_var_weather_t(const char *value);
extern void set_var_weather1_t(const char *value);
extern void set_var_weather2_t(const char *value);

static button_style_t *btn1_style;
static button_style_t *btn2_style;
static button_style_t *btn3_style;
static button_style_t *btn4_style;

void loadScreenByIndex(int index);
extern esp_lcd_touch_handle_t tp_handle;

// MQTT相关变量和函数声明
#define MQTT_UI_TOPIC "homeassistant/weather/heweather_all"  // MQTT主题用于接收UI数据更新
#define MQTT_BUTTON_CONTROL_TOPIC "ui/button/control"

static void mqtt_ui_data_handler(esp_mqtt_event_handle_t event);
static void update_ui_label_text(lv_obj_t *label, const char *text);
static void parse_and_update_ui_data(const char *json_data);
#define GPIO_LCD_BL     (GPIO_NUM_38)
static lv_obj_t *g_video_container = NULL;
static av_render_handle_t g_av_render = NULL;
static esp_webrtc_handle_t g_webrtc_conn = NULL;
static lv_obj_t *brightness_slider = NULL;
static lv_obj_t *volume_slider = NULL;
static lv_obj_t *microphone_volume_slider = NULL;
static bool gesture_locked = false;
static int32_t current_brightness = 100;
static lv_obj_t *slider_mask = NULL;
static void slider_event_cb(lv_event_t *e);

static bool led_on = true;
static esp_timer_handle_t inactivity_timer;
static const int INACTIVITY_TIMEOUT_MS = 30000; // 30秒无操作关闭LED
static int currentScreenIndex = 0;
static bool just_woke_up = false;



// 定义所有屏幕的数组
#define SCREEN_COUNT 4
static const int screenList[SCREEN_COUNT] = {
    SCREEN_ID_MAIN,
    SCREEN_ID_DORMANCY,
    SCREEN_ID_P1,
    SCREEN_ID_FUNCTION,
};
extern void set_button_styles(
    button_style_t *s1, button_style_t *s2,
    button_style_t *s3, button_style_t *s4
);

void set_button_styles(button_style_t *s1, button_style_t *s2,
                       button_style_t *s3, button_style_t *s4) {
    btn1_style = s1;
    btn2_style = s2;
    btn3_style = s3;
    btn4_style = s4;
}


// void esp_button(esp_etm_event_handle_t event)
// {
//     char *data = strndup(event->data, event->data_len);
//     ESP_LOGI("MQTT", "收到自定义命令: %s", data);
//     free(data);
// }


int current_volume = 100;
int current_microphone_volume = 50; // 假设麦克风默认音量为50





void set_volume(int percent) {
    current_volume = percent;
    ESP_LOGI(TAG, "设置音量: %d", percent);
    int vol_set;
    bsp_codec_volume_set(percent, &vol_set);
}

void set_microphone_volume(int percent) {
    current_microphone_volume = percent;
    ESP_LOGI(TAG, "设置麦克风音量: %d", percent);
    bsp_codec_microphone_volume_set(percent);
}

int get_current_volume() {
    return current_volume;
}

int get_current_microphone_volume() {
    return current_microphone_volume;
}
void save_settings(int brightness, int volume, int microphone_volume) {
    //ESP_LOGI(TAG, "保存亮度: %d", brightness);
    nvs_handle_t handle;
    esp_err_t err = nvs_open("user_config", NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_i32(handle, "brightness", brightness);
        nvs_set_i32(handle, "volume", volume);
        nvs_set_i32(handle, "microphone_volume", microphone_volume);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

void load_settings(int32_t *brightness, int32_t *volume, int32_t *microphone_volume) {
    ESP_LOGI(TAG, "读取亮度: %ld", *brightness);
    nvs_handle_t handle;
    esp_err_t err = nvs_open("user_config", NVS_READONLY, &handle);
    if (err == ESP_OK) {
        nvs_get_i32(handle, "brightness", brightness);
        nvs_get_i32(handle, "volume", volume);
        nvs_get_i32(handle, "microphone_volume", microphone_volume);
        nvs_close(handle);
    } else {
        *brightness = 100;
        *volume = 100;
        *microphone_volume = 60;
    }
    current_volume = *volume;
    current_brightness = *brightness;
    current_microphone_volume = *microphone_volume;
}


static void go_home_cb(void *param) {
    loadScreenByIndex(0); // 回主页
}
static int32_t last_brightness = 100; // 默认值
static void inactivity_timer_callback(void* arg) {
    if (led_on) {
        last_brightness = current_brightness; // 保存当前亮度
        set_backlight_brightness(0);          // 关闭屏幕
        led_on = false;
        just_woke_up = true;                  // 标记下次触摸是唤醒
    }

    if (currentScreenIndex != 0) {
        ESP_LOGI(TAG, "超时未操作，切回主页");
        lv_async_call(go_home_cb, NULL);
    }
}



void reset_inactivity_timer() {
    esp_timer_stop(inactivity_timer);
    esp_timer_start_once(inactivity_timer, INACTIVITY_TIMEOUT_MS * 1000ULL);

    if (!led_on) {
        set_backlight_brightness(last_brightness);
        led_on = true;
        if (tp_handle) {
            ESP_LOGI(TAG, "唤醒");
            esp_lcd_touch_exit_sleep(tp_handle);
        } else {
            ESP_LOGW(TAG, "tp_handle为空,无法调用esp_lcd_touch_exit_sleep唤醒");
        }
    }
}



void backlight_pwm_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .gpio_num = GPIO_LCD_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 4095,
        .hpoint = 0,
        .flags.output_invert = 0
    };
    ledc_channel_config(&ledc_channel);
    esp_timer_create_args_t timer_args = {
        .callback = &inactivity_timer_callback,
        .name = "inactivity_timer"
    };
    esp_timer_create(&timer_args, &inactivity_timer);
    esp_timer_start_once(inactivity_timer, INACTIVITY_TIMEOUT_MS * 1000);
    int32_t brightness = 100;
    int32_t volume = 100;
    int32_t microphone_volume = 50;
    load_settings(&brightness, &volume, &microphone_volume);
    set_backlight_brightness(brightness);
    set_volume(volume);
    set_microphone_volume(microphone_volume);
}

void set_backlight_brightness(uint32_t percent) {
    current_brightness = percent;  // ✅ 保存当前亮度
    uint32_t duty = percent * 8191 / 100;
    printf("duty: %ld\n", duty);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
// WebRTC自定义数据回调
static int webrtc_data_callback(esp_webrtc_custom_data_via_t via, uint8_t *data, int size, void *ctx)
{
    if (via == ESP_WEBRTC_CUSTOM_DATA_VIA_DATA_CHANNEL && size > 0) {
        // 处理音视频数据
        if (data[0] == 0x00) { // 视频数据
            av_render_video_data_t video_data = {
                .data = data + 1,
                .size = size - 1,
                .pts = esp_timer_get_time() / 1000
            };
            av_render_add_video_data(g_av_render, &video_data);
        } else if (data[0] == 0x01) { // 音频数据
            av_render_audio_data_t audio_data = {
                .data = data + 1,
                .size = size - 1,
                .pts = esp_timer_get_time() / 1000
            };
            av_render_add_audio_data(g_av_render, &audio_data);
        }
    }
    return 0;
}



typedef struct {
    lv_obj_t *btn;
    bool on;
} btn_action_t;
static button_style_t default_style;
static void apply_button_state(void *p) {
    default_style.default_color = lv_color_hex(0xFFFFFF);
    default_style.checked_color = lv_color_hex(0x00FF00);
    default_style.default_img   = NULL;
    default_style.checked_img   = NULL;
    default_style.btn           = 0;
    btn_action_t *act = p;
    if (!act || !act->btn) {
        free(act);
        return;
    }

    if (act->on) {
        lv_obj_add_state(act->btn, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(act->btn, LV_STATE_CHECKED);
    }

    const button_style_t *style = NULL;
    if (act->btn == objects.button1) {
        style = btn1_style;
    } else if (act->btn == objects.button2) {
        style = btn2_style;
    } else if (act->btn == objects.button3) {
        style = btn3_style;
    } else if (act->btn == objects.button4) {
        style = btn4_style;
    }

    // 如果 style 还没初始化，给个默认值避免变粉
    // if (!style) {
    //     static button_style_t default_style = {

    //     };
    //     style = &default_style;
    // }

    update_button_state_ex(act->btn, style);

    free(act);
}




static void mqtt_button_control_handler(esp_mqtt_event_handle_t event) {
    if (event->data_len <= 0) return;

    char data[32];
    strncpy(data, event->data, event->data_len);
    data[event->data_len] = '\0';
    ESP_LOGI(TAG, "收到按钮控制命令: %s", data);

    lv_obj_t *target_btn = NULL;
    bool on = false;

    if      (strcmp(data, "btn1:on")  == 0) { target_btn = objects.button1; on = true;  }
    else if (strcmp(data, "btn1:off") == 0) { target_btn = objects.button1; on = false; }
    else if (strcmp(data, "btn2:on")  == 0) { target_btn = objects.button2; on = true;  }
    else if (strcmp(data, "btn2:off") == 0) { target_btn = objects.button2; on = false; }
    else if (strcmp(data, "btn3:on")  == 0) { target_btn = objects.button3; on = true;  }
    else if (strcmp(data, "btn3:off") == 0) { target_btn = objects.button3; on = false; }
    else if (strcmp(data, "btn4:on")  == 0) { target_btn = objects.button4; on = true;  }
    else if (strcmp(data, "btn4:off") == 0) { target_btn = objects.button4; on = false; }

    if (!target_btn) return;

    // 分配参数结构体，异步投递到 LVGL 任务执行
    btn_action_t *act = malloc(sizeof(btn_action_t));
    if (!act) return;
    act->btn = target_btn;
    act->on  = on;
    lv_async_call(apply_button_state, act);
}


// 初始化WebRTC连接
void init_webrtc_connection()
{
    esp_webrtc_cfg_t webrtc_cfg = {
        .signaling_impl = NULL,
        .signaling_cfg = {
            .signal_url = "wss://your-webrtc-server.com",
            .extra_cfg = NULL,
            .extra_size = 0,
            .ctx = NULL
        },
        .peer_impl = NULL,
        .peer_cfg = {
            .server_lists = NULL,
            .server_num = 0,
            .audio_info = {0},
            .video_info = {0},
            .audio_dir = ESP_PEER_MEDIA_DIR_SEND_RECV,
            .video_dir = ESP_PEER_MEDIA_DIR_SEND_RECV,
            .enable_data_channel = true,
            .video_over_data_channel = false,
            .no_auto_reconnect = false,
            .extra_cfg = NULL,
            .extra_size = 0,
            .on_custom_data = webrtc_data_callback,
            .ctx = NULL
        }
    };

    esp_err_t ret = esp_webrtc_open(&webrtc_cfg, &g_webrtc_conn);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WebRTC connection: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "WebRTC connection initialized successfully");
}

// 初始化WebRTC视频播放
void init_webrtc_video_player(lv_obj_t *parent)
{
    // 创建视频容器
    g_video_container = lv_obj_create(parent);
    lv_obj_set_size(g_video_container, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(g_video_container);

    // 初始化AV渲染器
    av_render_cfg_t render_cfg = {
        .video_render = (video_render_handle_t)g_video_container,
        .sync_mode = AV_RENDER_SYNC_NONE,
        .quit_when_eos = false,
        .allow_drop_data = true
    };
    g_av_render = av_render_open(&render_cfg);

    // 设置视频流参数
    av_render_video_info_t video_info = {
        .codec = AV_RENDER_VIDEO_CODEC_H264,
        .width = 640,
        .height = 480,
        .fps = 30
    };
    av_render_add_video_stream(g_av_render, &video_info);

    // 设置音频流参数
    av_render_audio_info_t audio_info = {
        .codec = AV_RENDER_AUDIO_CODEC_OPUS,
        .sample_rate = 48000,
        .channel = 2,
        .bits_per_sample = 16
    };
    av_render_add_audio_stream(g_av_render, &audio_info);

    // // 启动渲染
    // av_render_open(g_av_render);
}
// 屏幕切换函数
void loadScreenByIndex(int index) {
    // 清理WiFi检查定时器，防止强制页面切换
    // extern void cleanup_wifi_check_timer(void);
    // cleanup_wifi_check_timer();
    
    currentScreenIndex = (index % SCREEN_COUNT + SCREEN_COUNT) % SCREEN_COUNT;
    loadScreen(screenList[currentScreenIndex]);
    // update_wifi_icon_visibility();
}

// 手势事件处理
void action_action_slide(lv_event_t * e) {
    reset_inactivity_timer();

    // 如果是刚唤醒，则忽略这次滑动
    if (just_woke_up) {
        ESP_LOGI(TAG, "忽略唤醒后的第一次滑动");
        just_woke_up = false;
        return;
    }

    if (gesture_locked) return;
    lv_obj_t *target = lv_event_get_target(e);
    if (lv_obj_check_type(target, &lv_slider_class)) {
        return;
    }
    lv_indev_t * indev = lv_indev_get_act();
    lv_dir_t dir = lv_indev_get_gesture_dir(indev);

    vTaskDelay(pdMS_TO_TICKS(10));
    
    if (dir == LV_DIR_LEFT) {
        loadScreenByIndex(currentScreenIndex + 1);
    } else if (dir == LV_DIR_RIGHT) {
        loadScreenByIndex(currentScreenIndex - 1);
    }
}


// MQTT消息处理函数实现
static void mqtt_ui_data_handler(esp_mqtt_event_handle_t event) {
    if (event->data_len > 0) {
        char *data = malloc(event->data_len + 1);
        if (data) {
            strncpy(data, event->data, event->data_len);
            data[event->data_len] = '\0';
            
            ESP_LOGI(TAG, "收到MQTT UI数据: %s", data);
            parse_and_update_ui_data(data);
            
            free(data);
        }
    }
}

static void update_ui_label_text(lv_obj_t *label, const char *text) {
    if (label && text) {
        lv_label_set_text(label, text);
        lv_obj_invalidate(label);  // 强制刷新标签
    }
}



static void parse_and_update_ui_data(const char *json_data) {
    //printf("json_data: %s\n", json_data);

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        ESP_LOGE(TAG, "JSON解析失败");
        return;
    }

    cJSON *sub_item;

    // 第一种结构：有 "weather" 对象
    cJSON *weather_obj = cJSON_GetObjectItemCaseSensitive(root, "weather");
    if (weather_obj) {
        // 温度
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(weather_obj, "temperature"))) {

            if (cJSON_IsNumber(sub_item)) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%.0f", sub_item->valuedouble);
                // printf("temperature: %s\n", buf);
                set_var_temp2_t(buf);
            }
        }
        // 湿度
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(weather_obj, "humidity"))) {

            if (cJSON_IsNumber(sub_item)) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%.0f", sub_item->valuedouble);
                set_var_hum1_t(buf);

            }

        }

        // 天气状况
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(weather_obj, "condition"))) {
            const char *value = cJSON_GetStringValue(sub_item);
            if (value) {
                set_var_weather_t(value);
                set_var_weather_picture_t(value);
            }
        }
    } 
    else {
        // 第二种结构：字段在根对象
        // 温度
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(root, "temperature"))) {
            if (cJSON_IsNumber(sub_item)) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%.0f", sub_item->valuedouble);
                // printf("temperature: %s\n", buf);
                set_var_temp2_t(buf);
            }
        }

        // 湿度
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(root, "humidity"))) {
            if (cJSON_IsNumber(sub_item)) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%.0f", sub_item->valuedouble);
                set_var_hum1_t(buf);
            }
        }

        // 天气状况
        if ((sub_item = cJSON_GetObjectItemCaseSensitive(root, "condition"))) {
            const char *value = cJSON_GetStringValue(sub_item);
            if (value) {
                set_var_weather_t(value);
                set_var_weather_picture_t(value);
            }
        }
    }

    // 解析生活建议（仅第一种结构有）
    cJSON *life_suggestion = cJSON_GetObjectItemCaseSensitive(root, "life_suggestion");
    if (life_suggestion) {
        cJSON *suggestion_item = NULL;
        cJSON_ArrayForEach(suggestion_item, life_suggestion) {
            const char *key = suggestion_item->string;
            if (cJSON_IsArray(suggestion_item)) {
                cJSON *first_element = cJSON_GetArrayItem(suggestion_item, 0);
                if (first_element && cJSON_IsString(first_element)) {
                    if (strcmp(key, "uv") == 0) {
                        set_var_ultraviolet_t(first_element->valuestring);
                        // printf("uv: %s\n", first_element->valuestring);
                    }
                }
            }
        }
    }
    cJSON *forecast_daily = cJSON_GetObjectItemCaseSensitive(root, "forecast_daily");
    if (forecast_daily && cJSON_IsArray(forecast_daily)) {
        int days = cJSON_GetArraySize(forecast_daily);
        for (int i = 0; i < days; i++) {
            cJSON *day_item = cJSON_GetArrayItem(forecast_daily, i);
            if (day_item && cJSON_IsArray(day_item)) {
                // 依次取：天气图标、最高温、最低温、中文描述
                cJSON *icon_item = cJSON_GetArrayItem(day_item, 0);
                cJSON *max_temp  = cJSON_GetArrayItem(day_item, 1);
                cJSON *min_temp  = cJSON_GetArrayItem(day_item, 2);
                cJSON *desc_item = cJSON_GetArrayItem(day_item, 3);

                const char *icon = icon_item ? icon_item->valuestring : "";
                double max = max_temp ? max_temp->valuedouble : 0;
                double min = min_temp ? min_temp->valuedouble : 0;
                const char *desc = desc_item ? desc_item->valuestring : "";

                // printf("Day %d: icon=%s, max=%.1f, min=%.1f, desc=%s\n",
                //     i + 1, icon, max, min, desc);
                if (i == 1) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.0f-%.0f", min, max);
                    set_var_tom_temp_t(buf);
                    set_var_weather1_t(desc);
                    set_var_weather_picture1_t(desc);
                }
                if (i == 2) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.0f-%.0f", min, max);
                    set_var_back_temp_t(buf);
                    set_var_weather2_t(desc);
                    set_var_weather_picture2_t(desc);
                }

                // 这里可以调用 UI 更新函数
                // set_var_forecast_icon(i, icon);
                // set_var_forecast_max(i, max);
                // set_var_forecast_min(i, min);
                // set_var_forecast_desc(i, desc);
            }
        }
    }

    cJSON_Delete(root);
}



// 初始化MQTT UI数据订阅
void mqtt_ui_init(void) {
    mqtt_register_topic(MQTT_UI_TOPIC, mqtt_ui_data_handler);
    mqtt_register_topic("homeassistant/heweather_all", mqtt_ui_data_handler);
    mqtt_register_topic(MQTT_BUTTON_CONTROL_TOPIC, mqtt_button_control_handler);
    ESP_LOGI(TAG, "MQTT UI数据订阅已注册: %s", MQTT_UI_TOPIC);

}




// 按钮状态更新
void update_button_state_ex(lv_obj_t *button, const button_style_t *style) {
    if (button && style) {
        if (lv_obj_has_state(button, LV_STATE_CHECKED)) {
            ESP_LOGE(TAG, "按钮按下\n");
            ESP_LOGE(TAG, "按钮样式地址: %d\n", style->btn);
            lv_obj_set_style_bg_color(button, style->checked_color, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_bg_image_src(button, style->checked_img, LV_PART_MAIN | LV_STATE_CHECKED);
            
            // MQTT发布按钮按下状态
            mqtt_publish_topic("ui/button/state", "pressed");
        } else {
            // 设置默认状态的样式
            ESP_LOGE(TAG, "取消按下\n");
            ESP_LOGE(TAG, "按钮样式地址: %d\n", style->btn);
            lv_obj_set_style_bg_color(button, style->default_color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_image_src(button, style->default_img, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            // MQTT发布按钮取消按下状态
            mqtt_publish_topic("ui/button/state", "released");
        }
    }
}

// 按钮事件回调
void button_event_cb(lv_event_t *e) {
    reset_inactivity_timer();
    lv_obj_t *btn = lv_event_get_target(e);
    button_style_t *style = lv_event_get_user_data(e);
    update_button_state_ex(btn, style);
}

static void close_slider_and_mask() {
    if (brightness_slider) {
        lv_obj_add_flag(brightness_slider, LV_OBJ_FLAG_HIDDEN);
    }
    if (volume_slider) {
        lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
    }
    if (microphone_volume_slider) {
        lv_obj_add_flag(microphone_volume_slider, LV_OBJ_FLAG_HIDDEN);
    }
    if (slider_mask) {
        lv_obj_del(slider_mask);
        slider_mask = NULL;
    }
    gesture_locked = false;
}

static void mask_event_cb(lv_event_t *e) {
    close_slider_and_mask();
}


int get_current_brightness() {
    return current_brightness;
}
static void show_slider_with_mask(lv_obj_t **slider, int align_x, int align_y, const char *type) {
    // 先销毁已有 slider（避免多个同时存在）
    if (*slider) {
        lv_obj_del(*slider);
        *slider = NULL;
    }

    *slider = lv_slider_create(lv_layer_top());

    // 设置范围
    if (strcmp(type, "brightness") == 0) {
        lv_slider_set_range(*slider, 90, 100);
    } else if (strcmp(type, "volume") == 0) {
        lv_slider_set_range(*slider, 0, 100);
    } else if (strcmp(type, "microphone_volume") == 0) {
        lv_slider_set_range(*slider, 0, 100);
    }

    lv_obj_align(*slider, LV_ALIGN_BOTTOM_MID, align_x, align_y);
    lv_obj_set_width(*slider, 150);
    lv_obj_add_event_cb(*slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)type);

    // 设置当前值
    if (strcmp(type, "brightness") == 0) {
        lv_slider_set_value(*slider, get_current_brightness(), LV_ANIM_OFF);
    } else if (strcmp(type, "volume") == 0) {
        lv_slider_set_value(*slider, get_current_volume(), LV_ANIM_OFF);
    } else if (strcmp(type, "microphone_volume") == 0) {
        lv_slider_set_value(*slider, get_current_microphone_volume(), LV_ANIM_OFF);
    }

    // 遮罩层
    if (!slider_mask) {
        slider_mask = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(slider_mask);
        lv_obj_set_size(slider_mask, LV_HOR_RES, LV_VER_RES);
        lv_obj_clear_flag(slider_mask, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_background(slider_mask);
        lv_obj_add_event_cb(slider_mask, mask_event_cb, LV_EVENT_CLICKED, NULL);
    }

    gesture_locked = true;
}





static void slider_event_cb(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    const char *type = (const char *)lv_event_get_user_data(e);

    // 只处理当前活跃的 slider，避免隐藏的也触发
    if ((target != brightness_slider) &&
        (target != volume_slider) &&
        (target != microphone_volume_slider)) {
        return;
    }

    int value = lv_slider_get_value(target);

    if (strcmp(type, "brightness") == 0) {
        ESP_LOGI(TAG, "亮度调节值: %d", value);
        set_backlight_brightness(value);
        save_settings(value, get_current_volume(), get_current_microphone_volume());
    } else if (strcmp(type, "volume") == 0) {
        ESP_LOGI(TAG, "音量调节值: %d", value);
        set_volume(value);
        bsp_codec_mute_set(value == 0);
        save_settings(get_current_brightness(), value, get_current_microphone_volume());
    } else if (strcmp(type, "microphone_volume") == 0) {
        ESP_LOGI(TAG, "麦克风音量调节值: %d", value);
        set_microphone_volume(value);
        save_settings(get_current_brightness(), get_current_volume(), value);
    }
}




// 修改亮度/音量点击事件
void on_brightness_click(lv_event_t *e) {
    ESP_LOGE(TAG, "按下亮度按钮\n");
    show_slider_with_mask(&brightness_slider, 10, -450, "brightness");
    if (volume_slider) lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
}

void on_volume_click(lv_event_t *e) {
    ESP_LOGE(TAG, "按下声音按钮\n");
    show_slider_with_mask(&volume_slider, 10, -450, "volume");
    if (brightness_slider) lv_obj_add_flag(brightness_slider, LV_OBJ_FLAG_HIDDEN);
    if (microphone_volume_slider) lv_obj_add_flag(microphone_volume_slider, LV_OBJ_FLAG_HIDDEN);
}

void on_microphone_volume_click(lv_event_t *e) {
    ESP_LOGE(TAG, "按下麦克风声音按钮\n");
    show_slider_with_mask(&microphone_volume_slider, 10, -450, "microphone_volume");
    if (brightness_slider) lv_obj_add_flag(brightness_slider, LV_OBJ_FLAG_HIDDEN);
    if (volume_slider) lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN);
}






// ================= 设置页面 =================
static lv_obj_t *settings_screen = NULL;

static void on_back_click(lv_event_t *e) {
    // 返回功能页面
    loadScreenByIndex(2); // SCREEN_ID_FUNCTION
}



/* 显示音量/亮度/麦克风 slider */
static void on_brightness_setting(lv_event_t *e) {
    show_slider_with_mask(&brightness_slider, 0, -200, "brightness");
}
static void on_volume_setting(lv_event_t *e) {
    show_slider_with_mask(&volume_slider, 0, -200, "volume");
}
static void on_microphone_setting(lv_event_t *e) {
    show_slider_with_mask(&microphone_volume_slider, 0, -200, "microphone_volume");
}

/* ---- 创建设置页面 ---- */
void create_screen_settings(void) {
    settings_screen = lv_obj_create(NULL);
    lv_obj_set_size(settings_screen, 480, 480);
    lv_obj_set_style_bg_color(settings_screen, lv_color_black(), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(settings_screen);
    lv_label_set_text(title, "设置");
    lv_obj_set_style_text_font(title, &ui_font_chinese_32, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);


    // 麦克风音量设置
    lv_obj_t *btn_mic = lv_btn_create(settings_screen);
    lv_obj_set_size(btn_mic, 180, 50);
    lv_obj_align(btn_mic, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_t *lbl3 = lv_label_create(btn_mic);
    lv_label_set_text(lbl3, "麦克风音量");
    lv_obj_center(lbl3);
    lv_obj_add_event_cb(btn_mic, on_microphone_setting, LV_EVENT_CLICKED, NULL);

    // 系统音量设置
    lv_obj_t *btn_vol = lv_btn_create(settings_screen);
    lv_obj_set_size(btn_vol, 180, 50);
    lv_obj_align(btn_vol, LV_ALIGN_TOP_MID, 0, 240);
    lv_obj_t *lbl4 = lv_label_create(btn_vol);
    lv_label_set_text(lbl4, "系统音量");
    lv_obj_center(lbl4);
    lv_obj_add_event_cb(btn_vol, on_volume_setting, LV_EVENT_CLICKED, NULL);

    // 亮度设置
    lv_obj_t *btn_bright = lv_btn_create(settings_screen);
    lv_obj_set_size(btn_bright, 180, 50);
    lv_obj_align(btn_bright, LV_ALIGN_TOP_MID, 0, 300);
    lv_obj_t *lbl5 = lv_label_create(btn_bright);
    lv_label_set_text(lbl5, "屏幕亮度");
    lv_obj_center(lbl5);
    lv_obj_add_event_cb(btn_bright, on_brightness_setting, LV_EVENT_CLICKED, NULL);

    // 返回按钮
    lv_obj_t *back_btn = lv_btn_create(settings_screen);
    lv_obj_set_size(back_btn, 100, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "返回");
    lv_obj_center(back_label);
    lv_obj_add_event_cb(back_btn, on_back_click, LV_EVENT_CLICKED, NULL);

    lv_scr_load(settings_screen);
}
