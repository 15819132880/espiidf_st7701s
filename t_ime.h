#include "esp_err.h"
#include "esp_log.h"
// #include <time.h>
#include <stdbool.h>
// #include "button.h"
#include <stdio.h>
#include "nvs_flash.h"
// #include "user_uart.h"
#include "ui/vars.h"
// #include "driver/gptimer.h"
// #include "esp_lcd_touch_gt1151.h"
#ifdef __cplusplus
extern "C" {

#endif
// #include "msm261s.h"
// #include "audio.h"
// #include "esp_lcd_touch_gt911.h"
// // #include "max98357.h"
#include "esp_sntp.h"
#include <time.h>
// #include "lcd_st7701.h"
// #include "lvgl.h"
#include "esp_timer.h"
// #include "esp_lvgl_port.h"
// #include "lv_demos.h"
// #include "soc/gpio_num.h"
// #include "ota_start.h"
// #include "esp_psram.h"
void setup_time_update_timer(void);
void initialize_sntp(void);
void update_time_variables(void);
// void setup_rtc_timer(void);
#ifdef __cplusplus
}
#endif