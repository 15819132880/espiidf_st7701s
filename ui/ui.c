#if defined(EEZ_FOR_LVGL)
#include <eez/core/vars.h>
#endif

#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"
#include "esp_lcd_touch.h"






#if defined(EEZ_FOR_LVGL)

void ui_init() {
    eez_flow_init(assets, sizeof(assets), (lv_obj_t **)&objects, sizeof(objects), images, sizeof(images), actions);
}

void ui_tick() {
    eez_flow_tick();
    tick_screen(g_currentScreen);
    reset_sleep_timer();
}

void reset_sleep_timer() {
    reset_sleep_timer_in_screens();
}

#else

static int16_t currentScreen = -1;
static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId) {
    // 清理WiFi检查定时器，防止强制页面切换
    extern void cleanup_wifi_check_timer(void);
    cleanup_wifi_check_timer();
    
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    extern esp_lcd_touch_handle_t tp_handle;
    if (screenId != SCREEN_ID_DORMANCY && tp_handle) {
        esp_lcd_touch_exit_sleep(tp_handle);
    }
}

void ui_init() {
    create_boot_screen();
}

void ui_tick() {
    if (currentScreen >= 0) {
        tick_screen(currentScreen);
    }
    reset_sleep_timer();
}

void reset_sleep_timer() {
    reset_sleep_timer_in_screens();
}
#endif
