#include "t_ime.h"

volatile bool need_update_time = false; // 是否需要更新时间的标志

void update_time_variables() { // 更新时间变量的函数

    time_t now; // 时间变量

    struct tm timeinfo; // 时间信息结构体

    time(&now); // 获取当前时间

    localtime_r(&now, &timeinfo); // 将时间转换为本地时间


    // 更新日期变量（通过头文件中的 set_var_* 函数）

    char buffer[64]; // 缓冲区用于格式化时间字符串

    strftime(buffer, sizeof(buffer), "%Y", &timeinfo); // 格式化年份

    set_var_years_t(buffer); // 年


    strftime(buffer, sizeof(buffer), "%m", &timeinfo); // 格式化月份

    set_var_month_t(buffer); // 月


    strftime(buffer, sizeof(buffer), "%d", &timeinfo); // 格式化日期

    set_var_day_t(buffer);   // 日


    // 更新时间变量

    strftime(buffer, sizeof(buffer), "%H", &timeinfo); // 格式化小时

    set_var_when_t(buffer);  // 时


    strftime(buffer, sizeof(buffer), "%M", &timeinfo); // 格式化分钟

    set_var_points_t(buffer); // 分

}


void initialize_sntp() { // 初始化SNTP（Simple Network Time Protocol）

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL); // 设置SNTP操作模式为轮询模式

    esp_sntp_setservername(0, "pool.ntp.org");   // 设置第一个NTP服务器

    esp_sntp_setservername(1, "ntp.ntsc.ac.cn"); // 设置第二个NTP服务器

    esp_sntp_init(); // 初始化SNTP服务

    setenv("TZ", "CST-8", 1); // 设置时区为CST-8（北京时间）

    tzset(); // 更新C库的时间相关环境变量

    int retry = 0; // 重试计数器

    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && retry++ < 10) { // 等待SNTP同步完成，最多重试10次

        vTaskDelay(2000 / portTICK_PERIOD_MS); // 延时2秒

    }


    if (retry < 10) { // 如果在重试次数内同步成功

        update_time_variables(); // 更新时间变量
 // 更新时间变量

    }
}



// // 定时器回调（每秒触发一次）
// static bool IRAM_ATTR timer_callback(void *arg) {
//     static uint32_t count = 0;
//     time_t now;
//     time(&now); // 更新系统时间
//     // need_update_time = true;
//     if (++count % 3600 == 0) { // 每1小时检查一次 SNTP 同步
//         if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
//             update_time_variables();
//         }
//     }
//     return true;
// }

// void setup_rtc_timer() {
//     timer_config_t config = {
//         .divider = 80,           // 80 MHz / 80 = 1 MHz
//         .counter_dir = TIMER_COUNT_UP,
//         .alarm_en = TIMER_ALARM_EN,
//         .auto_reload = true
//     };

//     timer_init(TIMER_GROUP_0, TIMER_0, &config);
//     timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
//     timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000000); // 1秒触发
//     timer_enable_intr(TIMER_GROUP_0, TIMER_0);
//     timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_callback, NULL, 0);
//     timer_start(TIMER_GROUP_0, TIMER_0);
// }
static void timer_callback(void* arg) { // 定时器回调函数

    update_time_variables();
}

void setup_time_update_timer() { // 设置时间更新定时器

    esp_timer_create_args_t timer_args = { // 定时器创建参数

        .callback = &timer_callback, // 设置回调函数

        .name = "time_update" // 定时器名称

    };
    esp_timer_handle_t timer; // 定时器句柄

    esp_timer_create(&timer_args, &timer); // 创建定时器

    esp_timer_start_periodic(timer, 1000000); // 每秒更新一次（1000000微秒）

}