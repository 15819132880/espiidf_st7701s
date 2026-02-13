#include "freertos/FreeRTOS.h" // FreeRTOS操作系统头文件
#include "freertos/task.h"     // FreeRTOS任务管理头文件
#include "esp_err.h"           // ESP-IDF错误处理头文件
#include "esp_log.h"           // ESP-IDF日志系统头文件
#include "esp_check.h"         // ESP-IDF检查宏头文件
// #include "driver/i2c.h"     // I2C驱动头文件（注释掉）
#include "driver/gpio.h"       // GPIO驱动头文件
#include "driver/spi_master.h" // SPI主设备驱动头文件
#include "esp_lcd_panel_io.h"  // ESP LCD面板IO接口头文件
#include "esp_lcd_panel_rgb.h" // ESP LCD RGB面板驱动头文件
#include "esp_lcd_panel_vendor.h" // ESP LCD面板厂商特定头文件
#include "esp_lcd_panel_ops.h" // ESP LCD面板操作头文件
#include "hal/gpio_types.h"    // GPIO类型定义头文件
// #include "button.h"         // 按钮驱动头文件（注释掉）
#include "nvs_flash.h"         // NVS闪存头文件
// #include "user_uart.h"      // 用户UART头文件（注释掉）

// #include "esp_lcd_touch_gt1151.h" // GT1151触摸屏驱动头文件（注释掉）
#ifdef __cplusplus
extern "C" { // 兼容C++的C语言代码块
#endif
// #include "msm261s.h"        // MSM261S驱动头文件（注释掉）
#include "audio.h"             // 音频头文件
#include "esp32_s3_szp.h"      // ESP32-S3特定音频头文件
#include "esp_lcd_touch_gt911.h" // GT911触摸屏驱动头文件
// #include "max98357.h"       // MAX98357音频放大器头文件（注释掉）
#include "lcd_st7701.h"        // ST7701 LCD驱动头文件
#include "lvgl.h"              // LVGL图形库头文件
#include "esp_timer.h"         // ESP定时器头文件
#include "esp_lvgl_port.h"     // ESP-LVGL端口头文件
#include "lv_demos.h"          // LVGL示例头文件
#include "soc/gpio_num.h"      // SOC GPIO编号定义头文件
#include "ota_start.h"         // OTA更新启动头文件
#include "esp_psram.h"         // ESP PSRAM头文件
#include "t_ime.h"             // 时间相关头文件
#include "ui/actions.h"        // UI动作头文件
#include "ui/screens.h"        // UI屏幕头文件
#include "wifi_provisioning_api.h" // WiFi配网API头文件
#include "esp_log.h"           // ESP-IDF日志系统头文件

void app_ui_init(void); // 应用程序UI初始化函数声明

#ifdef __cplusplus
} // 兼容C++的C语言代码块结束
#endif