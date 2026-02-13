#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_WHEN_T = 0,
    FLOW_GLOBAL_VARIABLE_POINTS_T = 1,
    FLOW_GLOBAL_VARIABLE_YEARS_T = 2,
    FLOW_GLOBAL_VARIABLE_MONTH_T = 3,
    FLOW_GLOBAL_VARIABLE_DAY_T = 4,
    FLOW_GLOBAL_VARIABLE_TEMP1_T = 5,
    FLOW_GLOBAL_VARIABLE_TEMP2_T = 6,
    FLOW_GLOBAL_VARIABLE_HUM_T = 7,
    FLOW_GLOBAL_VARIABLE_HUM1_T = 8,
    FLOW_GLOBAL_VARIABLE_ULTRAVIOLET_T = 9,
    FLOW_GLOBAL_VARIABLE_WEATHER_T = 10,
    FLOW_GLOBAL_VARIABLE_WEATHER1_T = 11,
    FLOW_GLOBAL_VARIABLE_WEATHER2_T = 12,
    FLOW_GLOBAL_VARIABLE_TOM_TEMP_T = 13,
    FLOW_GLOBAL_VARIABLE_BACK_TEMP_T = 15,
    FLOW_GLOBAL_VARIABLE_WEATHER_PICTURE_T = 17,
    FLOW_GLOBAL_VARIABLE_WEATHER_PICTURE1_T = 18,
    FLOW_GLOBAL_VARIABLE_WEATHER_PICTURE2_T = 19
};

// Native global variables

extern const char *get_var_when_t();
extern void set_var_when_t(const char *value);
extern const char *get_var_points_t();
extern void set_var_points_t(const char *value);
extern const char *get_var_years_t();
extern void set_var_years_t(const char *value);
extern const char *get_var_month_t();
extern void set_var_month_t(const char *value);
extern const char *get_var_day_t();
extern void set_var_day_t(const char *value);
extern const char *get_var_temp1_t();
extern void set_var_temp1_t(const char *value);
extern const char *get_var_temp2_t();
extern void set_var_temp2_t(const char *value);
extern const char *get_var_hum_t();
extern void set_var_hum_t(const char *value);
extern const char *get_var_hum1_t();
extern void set_var_hum1_t(const char *value);
extern const char *get_var_ultraviolet_t();
extern void set_var_ultraviolet_t(const char *value);
extern const char *get_var_weather_t();
extern void set_var_weather_t(const char *value);
extern const char *get_var_weather1_t();
extern void set_var_weather1_t(const char *value);
extern const char *get_var_weather2_t();
extern void set_var_weather2_t(const char *value);
extern const char *get_var_tom_temp_t();
extern void set_var_tom_temp_t(const char *value);
extern const char *get_var_back_temp_t();
extern void set_var_back_temp_t(const char *value);

extern const char *get_var_weather_picture_t();
extern void set_var_weather_picture_t(const char *value);

extern const char *get_var_weather_picture1_t();
extern void set_var_weather_picture1_t(const char *value);

extern const char *get_var_weather_picture2_t();
extern void set_var_weather_picture2_t(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/