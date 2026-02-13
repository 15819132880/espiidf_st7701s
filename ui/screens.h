#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H
extern void show_wifi_config_info(void *arg);
extern void cleanup_wifi_check_timer();
#include <lvgl.h>
#ifdef __cplusplus
extern "C" {


#endif
typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *function;
    lv_obj_t *p1;
    lv_obj_t *dormancy;
    lv_obj_t *brightness;
    lv_obj_t *volume;
    lv_obj_t *button1;
    lv_obj_t *obj0;
    lv_obj_t *button2;
    lv_obj_t *obj1;
    lv_obj_t *button3;
    lv_obj_t *obj2;
    lv_obj_t *button4;
    lv_obj_t *obj3;
    lv_obj_t *wifi;
    lv_obj_t *obj4;
    lv_obj_t *when;
    lv_obj_t *obj5;
    lv_obj_t *points;
    lv_obj_t *years;
    lv_obj_t *obj6;
    lv_obj_t *month;
    lv_obj_t *obj7;
    lv_obj_t *day;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *hum;
    lv_obj_t *obj12;
    lv_obj_t *temp2;
    lv_obj_t *obj13;
    lv_obj_t *temp1;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *hum1;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *weather_picture;
    lv_obj_t *weather_picture1;
    lv_obj_t *weather;
    lv_obj_t *weather_picture2;
    lv_obj_t *obj20;
    lv_obj_t *ultraviolet;
    lv_obj_t *wifi_t;
    lv_obj_t *weather1;
    lv_obj_t *tom_temp;

    lv_obj_t *weather2;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *back_temp;

    lv_obj_t *obj23;
    lv_obj_t *obj25;
    lv_obj_t *obj27;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
} objects_t;

extern objects_t objects;
// extern int loading_screen;
enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_DORMANCY = 2,
    SCREEN_ID_P1 = 3,
    SCREEN_ID_FUNCTION = 4,
};

void create_screen_function();
void tick_screen_function();
void create_screen_dormancy();
void tick_screen_dormancy();

void create_screen_main();
void tick_screen_main();

void create_screen_p1();
void tick_screen_p1();
void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);
void create_screens();
void create_boot_screen(void);
void update_wifi_icon_strength(int screen_index, int percent);
void reset_sleep_timer_in_screens();
void cleanup_wifi_check_timer(void);
#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/