#include <stdio.h> // 标准输入输出库
#include "main_a.h" // 主应用程序头文件
#include "wifi_provisioning_api.h"
#include "lvgl.h"
#include "ui/actions.h" // 包含UI动作头文件，用于MQTT UI初始化
extern void show_wifi_config_info(void *arg);

#define LCD_WIDTH       (480) // LCD宽度定义
#define LCD_HEIGHT      (480) // LCD高度定义

// #define GPIO_LCD_BL     (GPIO_NUM_38) // LCD背光GPIO引脚（注释掉）
#define GPIO_LCD_RST    (GPIO_NUM_NC) // LCD复位GPIO引脚（未连接）
#define GPIO_LCD_DE     (GPIO_NUM_18) // LCD数据使能GPIO引脚
#define GPIO_LCD_VSYNC  (GPIO_NUM_17) // LCD垂直同步GPIO引脚
#define GPIO_LCD_HSYNC  (GPIO_NUM_16) // LCD水平同步GPIO引脚
#define GPIO_LCD_PCLK   (GPIO_NUM_21) // LCD像素时钟GPIO引脚

#define GPIO_LCD_R0    (GPIO_NUM_11) // LCD红色数据线0
#define GPIO_LCD_R1    (GPIO_NUM_12) // LCD红色数据线1
#define GPIO_LCD_R2    (GPIO_NUM_13) // LCD红色数据线2
#define GPIO_LCD_R3    (GPIO_NUM_14) // LCD红色数据线3
#define GPIO_LCD_R4    (GPIO_NUM_0)  // LCD红色数据线4

#define GPIO_LCD_G0    (GPIO_NUM_8)  // LCD绿色数据线0
#define GPIO_LCD_G1    (GPIO_NUM_20) // LCD绿色数据线1
#define GPIO_LCD_G2    (GPIO_NUM_3)  // LCD绿色数据线2
#define GPIO_LCD_G3    (GPIO_NUM_46) // LCD绿色数据线3
#define GPIO_LCD_G4    (GPIO_NUM_9)  // LCD绿色数据线4
#define GPIO_LCD_G5    (GPIO_NUM_10) // LCD绿色数据线5

#define GPIO_LCD_B0    (GPIO_NUM_4)  // LCD蓝色数据线0
#define GPIO_LCD_B1    (GPIO_NUM_5)  // LCD蓝色数据线1
#define GPIO_LCD_B2    (GPIO_NUM_6)  // LCD蓝色数据线2
#define GPIO_LCD_B3    (GPIO_NUM_7)  // LCD蓝色数据线3
#define GPIO_LCD_B4    (GPIO_NUM_15) // LCD蓝色数据线4

/* 触摸屏设置 */
// #define EXAMPLE_TOUCH_I2C_NUM       (0) // 触摸屏I2C总线号（注释掉）
// #define EXAMPLE_TOUCH_I2C_CLK_HZ    (200000) // 触摸屏I2C时钟频率（注释掉）
esp_lcd_touch_handle_t tp_handle = NULL; // 触摸屏句柄

/* LCD触摸引脚 */
// #define EXAMPLE_TOUCH_I2C_SCL       (GPIO_NUM_45) // 触摸屏I2C SCL引脚（注释掉）
// #define EXAMPLE_TOUCH_I2C_SDA       (GPIO_NUM_19) // 触摸屏I2C SDA引脚（注释掉）
static const char *TAG = "EXAMPLE"; // 日志标签

/* LCD IO和面板 */
static esp_lcd_panel_io_handle_t lcd_io = NULL; // LCD面板IO句柄
static esp_lcd_panel_handle_t lcd_panel = NULL; // LCD面板句柄
static esp_lcd_touch_handle_t touch_handle = NULL; // 触摸屏句柄
esp_lcd_panel_handle_t panel_handle = NULL; // LCD面板句柄

/* LVGL显示和触摸 */
static lv_display_t *lvgl_disp = NULL; // LVGL显示器对象
static lv_indev_t *lvgl_touch_indev = NULL; // LVGL触摸输入设备对象

// LCD初始化函数
static esp_err_t app_lcd_init(void)
{

    backlight_pwm_init(); // 背光PWM初始化
    st7701_reg_init();      // ST7701寄存器配置

    ESP_LOGI(TAG, "Install RGB panel driver"); // 打印日志：安装RGB面板驱动
    esp_lcd_rgb_panel_config_t panel_config = { // RGB面板配置结构体
        .data_width = 16, // RGB565并行模式，16位数据宽度
        .psram_trans_align = 64, // PSRAM传输对齐
#if 1
        .bounce_buffer_size_px = 10 * LCD_WIDTH, // 弹跳缓冲区大小
#endif   
        .clk_src = LCD_CLK_SRC_PLL240M, // 时钟源为PLL240M
        .disp_gpio_num = GPIO_NUM_NC, // 显示使能GPIO引脚（未连接）
        .pclk_gpio_num = GPIO_LCD_PCLK, // 像素时钟GPIO引脚
        .vsync_gpio_num = GPIO_LCD_VSYNC, // 垂直同步GPIO引脚
        .hsync_gpio_num = GPIO_LCD_HSYNC, // 水平同步GPIO引脚
        .de_gpio_num = GPIO_LCD_DE, // 数据使能GPIO引脚
        .data_gpio_nums = { // 数据GPIO引脚数组
            GPIO_LCD_B0, GPIO_LCD_B1, GPIO_LCD_B2, GPIO_LCD_B3, GPIO_LCD_B4,         
            GPIO_LCD_G0, GPIO_LCD_G1, GPIO_LCD_G2, GPIO_LCD_G3, GPIO_LCD_G4, GPIO_LCD_G5,
            GPIO_LCD_R0, GPIO_LCD_R1, GPIO_LCD_R2, GPIO_LCD_R3, GPIO_LCD_R4,
        },
        .timings = { // 时序配置
            .pclk_hz = 16 * 1000 * 1000, // 像素时钟频率
            .h_res = LCD_WIDTH, // 水平分辨率
            .v_res = LCD_HEIGHT, // 垂直分辨率
            // 以下参数应参考LCD规格书
            .hsync_back_porch = 50, // 水平同步后沿
            .hsync_front_porch = 10, // 水平同步前沿
            .hsync_pulse_width = 8, // 水平同步脉冲宽度
            .vsync_back_porch = 20, // 垂直同步后沿
            .vsync_front_porch = 10, // 垂直同步前沿
            .vsync_pulse_width = 8, // 垂直同步脉冲宽度
            .flags.pclk_active_neg = 0, // 像素时钟负沿有效
            // .flags.hsync_idle_low = true, // 水平同步空闲低电平（注释掉）
        },
        .flags.fb_in_psram = 1, // 帧缓冲区分配在PSRAM中
        // .on_frame_trans_done = notify_lvgl_flush_ready, // 帧传输完成回调函数（注释掉）
        // .user_ctx = &disp_drv, // 用户上下文（注释掉）
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle)); // 创建新的RGB面板

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle)); // 复位LCD面板
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle)); // 初始化LCD面板

    // ESP_LOGI(TAG, "Turn on LCD backlight"); // 打印日志：打开LCD背光（注释掉）
    // gpio_set_level(GPIO_LCD_BL, 1); // 设置背光GPIO电平（注释掉）
    return ESP_OK; // 返回成功

}

// 触摸屏初始化函数
static esp_err_t app_touch_init(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus"); // 打印日志：初始化I2C总线

    bsp_i2c_init(); // 初始化BSP I2C
    // 由于没有硬件复位引脚，这里添加一个延时等待芯片内部复位完成
    ESP_LOGI(TAG, "No reset pin defined, adding delay for GT911 reset"); // 打印日志：未定义复位引脚，为GT911复位添加延时
    vTaskDelay(pdMS_TO_TICKS(500));  // 延时500ms，延时值可根据实际情况调整

    esp_lcd_panel_io_handle_t tp_io_handle = NULL; // 触摸屏IO句柄
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG(); // 触摸屏I2C配置
    tp_io_config.scl_speed_hz = 400000; // 设置I2C时钟速度为400kHz

    ESP_LOGI(TAG, "Initialize I2C panel IO"); // 打印日志：初始化I2C面板IO
    extern i2c_master_bus_handle_t i2c_bus;  // 声明外部I2C主总线句柄

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle)); // 创建新的I2C面板IO

    ESP_LOGI(TAG, "Initialize touch controller GT911"); // 打印日志：初始化GT911触摸控制器
    const esp_lcd_touch_config_t tp_cfg = { // 触摸屏配置结构体
        .x_max = LCD_WIDTH, // X轴最大值
        .y_max = LCD_HEIGHT, // Y轴最大值
        .int_gpio_num = -1,  // 未使用中断引脚时设为 -1
        .rst_gpio_num = -1,  // 没有硬件复位，此处设为 -1
        .levels = { // 电平配置
            .reset = 0,
            .interrupt = 0,
        },
        .flags = { // 标志位
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    // 增加重试机制，防止GT911初始化时偶然失败
    int retry = 0; // 重试计数
    const int max_retry = 3; // 最大重试次数
    esp_err_t ret = ESP_OK; // 返回值
    do { // 循环重试
        ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp_handle); // 创建GT911触摸屏
        if (ret == ESP_OK) { // 如果成功
            break; // 跳出循环
        }
        retry++; // 重试次数加1
        ESP_LOGW(TAG, "GT911 initialization failed, retry %d/%d", retry, max_retry); // 打印警告日志
        vTaskDelay(pdMS_TO_TICKS(100)); // 延时100ms
    } while (retry < max_retry); // 当重试次数小于最大重试次数时继续

    if (ret != ESP_OK) { // 如果初始化失败
        ESP_LOGE(TAG, "Error (0x%x)! Touch controller GT911 initialization failed!", ret); // 打印错误日志
        tp_handle = NULL; // 确保tp_handle为NULL
    }

    if (ret == ESP_OK) { // 如果初始化成功
        ESP_LOGI(TAG, "GT911 initialization succeeded"); // 打印成功日志
    } else { // 否则
        ESP_LOGE(TAG, "GT911 initialization failed after retries."); // 打印失败日志
    }
    return ret; // 返回结果
}

// LVGL初始化函数
static esp_err_t app_lvgl_init(void)
{
    /* 初始化LVGL */
    const lvgl_port_cfg_t lvgl_cfg = { // LVGL端口配置结构体
        .task_priority = 5,         /* LVGL任务优先级 */
        .task_stack = 16384,         /* LVGL任务栈大小 */
        .task_affinity = -1,        /* LVGL任务绑定核心（-1表示不绑定） */
        .task_max_sleep_ms = 500,   /* LVGL任务最大睡眠时间 */
        .timer_period_ms = 5        /* LVGL定时器周期 */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed"); // 初始化LVGL端口，如果失败则返回错误
    uint32_t buff_size = LCD_WIDTH * 50; // LVGL显示缓冲区大小
#if EXAMPLE_LCD_LVGL_FULL_REFRESH || EXAMPLE_LCD_LVGL_DIRECT_MODE
    buff_size = LCD_WIDTH * LCD_HEIGHT; // 如果是全刷新或直接模式，缓冲区大小为LCD全屏大小
#endif

    /* 添加LCD屏幕 */
    ESP_LOGD(TAG, "Add LCD screen"); // 打印日志：添加LCD屏幕
    const lvgl_port_display_cfg_t disp_cfg = { // LVGL端口显示配置结构体
        .panel_handle = panel_handle, // LCD面板句柄
        .buffer_size = buff_size, // 缓冲区大小
        .double_buffer = 1, // 启用双缓冲
        .hres = LCD_WIDTH, // 水平分辨率
        .vres = LCD_HEIGHT, // 垂直分辨率
        .monochrome = false, // 非单色显示
        .color_format = LV_COLOR_FORMAT_RGB565, // 颜色格式为RGB565

        .rotation = { // 旋转配置
            .swap_xy = false, // 不交换X和Y轴
            .mirror_x = false, // 不镜像X轴
            .mirror_y = false, // 不镜像Y轴
        },
        .flags = { // 标志位
            .buff_dma = false, // 不使用DMA缓冲
            .buff_spiram = false, // 不使用SPIRAM缓冲
#if EXAMPLE_LCD_LVGL_FULL_REFRESH
            .full_refresh = true, // 启用全刷新
#elif EXAMPLE_LCD_LVGL_DIRECT_MODE
            .direct_mode = true, // 启用直接模式
#endif
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false, // 不交换字节
#endif
        }
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = { // LVGL端口RGB显示配置结构体
        .flags = { // 标志位
#if 1
            .bb_mode = true, // 启用BB模式
#else
            .bb_mode = false, // 禁用BB模式
#endif
#if EXAMPLE_LCD_LVGL_AVOID_TEAR
            .avoid_tearing = true, // 避免撕裂
#else
            .avoid_tearing = false, // 不避免撕裂
#endif
        }
    };
    lvgl_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg); // 添加LVGL RGB显示器
     /* 添加触摸输入（针对选定的屏幕） */
    const lvgl_port_touch_cfg_t touch_cfg = { // 触摸配置结构体
        .disp = lvgl_disp, // LVGL显示器对象
        .handle = tp_handle, // 触摸屏句柄
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg); // 添加LVGL触摸输入设备
    return ESP_OK; // 返回成功
}

// OTA更新函数
void ota_updates(void)
{
    if (secure_esp_init() != ESP_OK) // 如果安全ESP初始化不成功
    {
        ESP_LOGE("secure_esp", "更新完成"); // 打印错误日志：更新完成
    }
}

// 初始化网络和时间任务函数
static void init_network_and_time(void *pvParameters)
{
    ota_updates(); // 执行OTA更新
    initialize_sntp(); // 初始化SNTP时间同步
    setup_time_update_timer(); // 设置时间更新定时器

    vTaskDelete(NULL); // 删除当前任务
}

// 应用程序UI初始化函数
void app_ui_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }



    ESP_LOGI(TAG, "启动屏幕"); // 打印日志：启动屏幕
    ESP_ERROR_CHECK(app_touch_init()); // 初始化触摸屏
    ESP_ERROR_CHECK(app_lcd_init()); // 初始化LCD

    ESP_ERROR_CHECK(app_lvgl_init()); // 初始化LVGL
    xTaskCreate(init_network_and_time, "init_network", 4096, NULL, 5, NULL); // 创建初始化网络和时间任务
    app_audio(); // 启动音频应用
    mqtt_ui_init(); // 初始化MQTT UI数据订阅
    // i2c_scanner(); // I2C扫描器（注释掉）
    // ota_updates(); // OTA更新（注释掉）
    // initialize_sntp(); // 初始化SNTP（注释掉）
    // setup_time_update_timer(); // 设置时间更新定时器（注释掉）
    // setup_rtc_timer(); // 设置RTC定时器（注释掉）
    /* 显示LVGL对象 */
    // lvgl_port_lock(0); // 锁定LVGL端口（注释掉）
    // // lv_demo_music(); // LVGL音乐演示（注释掉）
    // lv_demo_widgets(); // LVGL小部件演示（注释掉）
    // lvgl_port_unlock(); // 解锁LVGL端口（注释掉）
    // i2c_test(); // I2C测试（注释掉）
    // ESP_LOGI(TAG, "启动麦克风"); // 打印日志：启动麦克风（注释掉）
    // musicians(); // 音乐家（注释掉）
    // app_audio(); // 音频应用（注释掉）

    // esp_event_handler_register(WIFI_PROVISIONING_EVENT, WIFI_PROVISIONING_EVENT_SHOW_CONFIG_INFO, show_wifi_config_info, NULL);
}
