#include "main_a.h" // 包含主应用程序的头文件
#include "ui.h"       // 包含UI界面的头文件

// 应用程序主入口函数
extern "C" void app_main(void)
{
        app_ui_init(); // 初始化应用程序UI
    lvgl_port_lock(0); // 锁定LVGL端口，防止多线程访问冲突
    ui_init(); // 初始化UI界面

    lvgl_port_unlock(); // 解锁LVGL端口

    // 主循环
    while(1){
        lvgl_port_lock(0); // 锁定LVGL端口
        ui_tick(); // 执行UI任务，刷新界面
        // update_wifi_icon_strength(0, get_wifi_rssi()); // 更新WiFi信号强度图标（注释掉的代码）
        lvgl_port_unlock(); // 解锁LVGL端口
        vTaskDelay(pdMS_TO_TICKS(10)); // 延时10毫秒，让出CPU时间片
    }
}