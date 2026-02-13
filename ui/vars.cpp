#include <string>
#include "vars.h"
#include "t_ime.h"
std::string when_t = "12";
std::string points_t = "12";
std::string years_t = "2025";
std::string month_t = "12";
std::string day_t = "12";
std::string temp1_t = "20";
std::string temp2_t = "20";
std::string hum_t   = "50";
std::string hum1_t  = "50";
std::string ultraviolet_t = "强"; ;
std::string weather_t = "晴";
std::string weather1_t = "晴";
std::string weather2_t = "晴";
std::string tom_temp_t = "20-30";
std::string back_temp_t = "20-30";
std::string weather_picture_t = "雾";
std::string weather_picture1_t = "晴";
std::string weather_picture2_t = "多云";

extern "C" const char *get_var_when_t() {
    // printf(when_t.c_str());
    // update_time_variables();
    return when_t.c_str();
}

extern "C" void set_var_when_t(const char *value) {
    when_t = value;
}


extern "C" const char *get_var_points_t() {
    return points_t.c_str();
}

extern "C" void set_var_points_t(const char *value) {
    points_t = value;
}



extern "C" const char *get_var_years_t() {
    return years_t.c_str();
}

extern "C" void set_var_years_t(const char *value) {
    years_t = value;
}



extern "C" const char *get_var_month_t() {
    return month_t.c_str();
}

extern "C" void set_var_month_t(const char *value) {
    month_t = value;
}



extern "C" const char *get_var_day_t() {
    return day_t.c_str();
}

extern "C" void set_var_day_t(const char *value) {
    day_t = value;
}



extern "C" const char *get_var_temp1_t() {
    return temp1_t.c_str();
}

extern "C" void set_var_temp1_t(const char *value) {
    temp1_t = value;
}


extern "C" const char *get_var_temp2_t() {
    return temp2_t.c_str();
}

extern "C" void set_var_temp2_t(const char *value) {
    temp2_t = value;
}



extern "C" const char *get_var_hum_t() {
    return hum_t.c_str();
}

extern "C" void set_var_hum_t(const char *value) {
    hum_t = value;
}




extern "C" const char *get_var_hum1_t() {
    return hum1_t.c_str();
}

extern "C" void set_var_hum1_t(const char *value) {
    hum1_t = value;
}



extern "C" const char *get_var_ultraviolet_t() {
    return ultraviolet_t.c_str();
}

extern "C" void set_var_ultraviolet_t(const char *value) {
    ultraviolet_t = value;
}




extern "C" const char *get_var_weather_t() {
    return weather_t.c_str();
}

extern "C" void set_var_weather_t(const char *value) {
    weather_t = value;
}



extern "C" const char *get_var_weather1_t() {
    return weather1_t.c_str();
}

extern "C" void set_var_weather1_t(const char *value) {
    weather1_t = value;
}




extern "C" const char *get_var_weather2_t() {
    return weather2_t.c_str();
}

extern "C" void set_var_weather2_t(const char *value) {
    weather2_t = value;
}




extern "C" const char *get_var_tom_temp_t() {
    return tom_temp_t.c_str();
}

extern "C" void set_var_tom_temp_t(const char *value) {
    tom_temp_t = value;
}




extern "C" const char *get_var_back_temp_t() {
    return back_temp_t.c_str();
}

extern "C" void set_var_back_temp_t(const char *value) {
    back_temp_t = value;
}

extern "C" const char *get_var_weather_picture_t() {
    return weather_picture_t.c_str();
}

extern "C" void set_var_weather_picture_t(const char *value) {
    weather_picture_t = value;
}

extern "C" const char *get_var_weather_picture1_t() {
    return weather_picture1_t.c_str();
}

extern "C" void set_var_weather_picture1_t(const char *value) {
    weather_picture1_t = value;
}

extern "C" const char *get_var_weather_picture2_t() {
    return weather_picture2_t.c_str();
}

extern "C" void set_var_weather_picture2_t(const char *value) {
    weather_picture2_t = value;
}
