#include "ui_anyui5.h"
#include <stdio.h>

#define C_BG_TOP       0x000000u
#define C_BG_BOTTOM    0x000000u
#define C_CARD         0x111C2Cu
#define C_HEADER       0x0B1726u
#define C_BORDER       0x29415Du
#define C_TEXT         0xF8FAFCu
#define C_MUTED        0x94A3B8u
#define C_BLUE         0x2563EBu
#define C_BLUE_DARK    0x1D4ED8u
#define C_BLUE_LIGHT   0x93C5FDu
#define C_GREEN        0x10B981u
#define C_ORANGE       0xEA580Cu
#define C_TRACK        0x22344Au

static lv_obj_t *s_main_screen;
static lv_obj_t *s_detail_screen;
static lv_obj_t *s_temp_value;
static lv_obj_t *s_humidity_value;
static lv_obj_t *s_co2_main;
static lv_obj_t *s_pm25_main;
static lv_obj_t *s_co2_detail;
static lv_obj_t *s_pm25_detail;
static lv_obj_t *s_humidity_bar;
static lv_obj_t *s_co2_bar;
static lv_obj_t *s_pm25_bar;
static lv_obj_t *s_menu_screen;
static lv_obj_t *s_dynamic_screen;
static lv_obj_t *s_pages[3];
static lv_obj_t *s_menu_items[3];
static lv_obj_t *s_dynamic_uptime;
static lv_obj_t *s_dynamic_refresh;
static lv_obj_t *s_dynamic_link;
static lv_obj_t *s_dynamic_sample;
static uint8_t s_menu_selected;
static uint8_t s_current_page;

static void back_button_cb(lv_event_t *event);

static lv_obj_t *make_label(lv_obj_t *parent, const char *text,
                            int32_t x, int32_t y, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    return label;
}

static void prepare_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(C_BG_TOP), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(C_BG_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
}

static lv_obj_t *make_panel(lv_obj_t *parent, int32_t x, int32_t y,
                            int32_t width, int32_t height)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, width, height);
    lv_obj_set_style_bg_color(panel, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(C_BORDER), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 8, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    return panel;
}

static lv_obj_t *make_track(lv_obj_t *parent, int32_t x, int32_t y,
                            int32_t width, uint32_t color, int32_t fill)
{
    lv_obj_t *track = lv_obj_create(parent);
    lv_obj_t *indicator;
    lv_obj_set_pos(track, x, y);
    lv_obj_set_size(track, width, 6);
    lv_obj_set_style_bg_color(track, lv_color_hex(C_TRACK), 0);
    lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(track, 0, 0);
    lv_obj_set_style_radius(track, 3, 0);
    lv_obj_set_style_pad_all(track, 0, 0);
    lv_obj_set_scrollbar_mode(track, LV_SCROLLBAR_MODE_OFF);

    indicator = lv_obj_create(track);
    lv_obj_set_pos(indicator, 0, 0);
    lv_obj_set_size(indicator, fill, 6);
    lv_obj_set_style_bg_color(indicator, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(indicator, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(indicator, 0, 0);
    lv_obj_set_style_radius(indicator, 3, 0);
    lv_obj_set_style_pad_all(indicator, 0, 0);
    return indicator;
}

static void make_header(lv_obj_t *screen, const char *title, bool back)
{
    lv_obj_t *header = lv_obj_create(screen);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 320, 42);
    lv_obj_set_style_bg_color(header, lv_color_hex(C_HEADER), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);

    if(back) {
        lv_obj_t *button = lv_button_create(header);
        lv_obj_set_pos(button, 7, 5);
        lv_obj_set_size(button, 60, 32);
        lv_obj_set_style_bg_color(button, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_bg_color(button, lv_color_hex(C_BLUE_LIGHT), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(button, lv_color_hex(C_BORDER), 0);
        lv_obj_set_style_border_color(button, lv_color_hex(C_BLUE), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(button, 1, 0);
        lv_obj_set_style_border_width(button, 2, LV_STATE_PRESSED);
        lv_obj_set_style_radius(button, 6, 0);
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, "< BACK");
        lv_obj_set_style_text_color(label, lv_color_hex(C_TEXT), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_center(label);
        lv_obj_add_event_cb(button, back_button_cb, LV_EVENT_CLICKED, NULL);
        make_label(header, title, 82, 12, C_TEXT);
    }
    else {
        lv_obj_t *dot;
        make_label(header, title, 12, 12, C_TEXT);
        dot = lv_obj_create(header);
        lv_obj_set_pos(dot, 270, 16);
        lv_obj_set_size(dot, 9, 9);
        lv_obj_set_style_bg_color(dot, lv_color_hex(C_GREEN), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_pad_all(dot, 0, 0);
        make_label(header, "LIVE", 284, 12, C_GREEN);
    }
}

static void anim_translate_y(void *obj, int32_t value)
{
    lv_obj_set_style_translate_y((lv_obj_t *)obj, value, 0);
}

static void animate_object(lv_obj_t *obj, uint32_t delay)
{
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, obj);
    lv_anim_set_exec_cb(&animation, anim_translate_y);
    lv_anim_set_values(&animation, 14, 0);
    lv_anim_set_duration(&animation, 260);
    lv_anim_set_delay(&animation, delay);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_start(&animation);
}

static void main_loaded_cb(lv_event_t *event)
{
    lv_obj_t *screen = lv_event_get_target(event);
    animate_object(lv_obj_get_child(screen, 1), 0);
    animate_object(lv_obj_get_child(screen, 2), 60);
    animate_object(lv_obj_get_child(screen, 3), 120);
    animate_object(lv_obj_get_child(screen, 4), 180);
}

static void detail_loaded_cb(lv_event_t *event)
{
    lv_obj_t *screen = lv_event_get_target(event);
    animate_object(lv_obj_get_child(screen, 1), 0);
    animate_object(lv_obj_get_child(screen, 2), 70);
    animate_object(lv_obj_get_child(screen, 3), 140);
    animate_object(lv_obj_get_child(screen, 4), 200);
}

static void detail_button_cb(lv_event_t *event)
{
    (void)event;
    ui_environment_show_detail();
}

static void back_button_cb(lv_event_t *event)
{
    (void)event;
    if(s_menu_screen != NULL)
        lv_screen_load_anim(s_menu_screen, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT, 280, 0, false);
}

static void update_menu_highlight(void)
{
    uint8_t i;
    for(i = 0; i < 3; i++) {
        lv_obj_set_style_bg_color(s_menu_items[i],
                                 lv_color_hex(i == s_menu_selected ? C_BLUE_DARK : C_CARD), 0);
        lv_obj_set_style_border_color(s_menu_items[i],
                                     lv_color_hex(i == s_menu_selected ? C_BLUE_LIGHT : C_BORDER), 0);
        lv_obj_set_style_border_width(s_menu_items[i], i == s_menu_selected ? 2 : 1, 0);
    }
}

static lv_obj_t *make_menu_item(lv_obj_t *parent, uint8_t index,
                                int32_t x, int32_t y,
                                const char *number, const char *title,
                                const char *subtitle)
{
    lv_obj_t *item = make_panel(parent, x, y, 148, 72);
    make_label(item, number, 9, 8, C_BLUE_LIGHT);
    make_label(item, title, 34, 8, C_TEXT);
    make_label(item, subtitle, 9, 42, C_MUTED);
    s_menu_items[index] = item;
    return item;
}

lv_obj_t *ui_environment_menu_create(void)
{
    s_menu_screen = lv_obj_create(NULL);
    prepare_screen(s_menu_screen);
    make_header(s_menu_screen, "PAGE SELECT", false);
    make_menu_item(s_menu_screen, 0, 8, 48, "01", "OVERVIEW", "Temp / humidity");
    make_menu_item(s_menu_screen, 1, 164, 48, "02", "DETAILS", "Air quality data");
    make_menu_item(s_menu_screen, 2, 86, 128, "03", "LIVE DATA", "Runtime monitor");
    make_label(s_menu_screen, "UART N: SELECT     E: ENTER", 47, 214, C_MUTED);
    s_menu_selected = 0;
    update_menu_highlight();
    return s_menu_screen;
}

lv_obj_t *ui_environment_main_create(void)
{
    lv_obj_t *card;
    lv_obj_t *button;
    lv_obj_t *label;

    s_main_screen = lv_obj_create(NULL);
    prepare_screen(s_main_screen);
    make_header(s_main_screen, "ENV MONITOR", false);

    card = make_panel(s_main_screen, 8, 50, 148, 76);
    make_label(card, "TEMPERATURE", 8, 8, C_MUTED);
    s_temp_value = make_label(card, "24.6 C", 8, 31, C_ORANGE);
    make_label(card, "COMFORT", 82, 52, C_GREEN);

    card = make_panel(s_main_screen, 164, 50, 148, 76);
    make_label(card, "HUMIDITY", 8, 8, C_MUTED);
    s_humidity_value = make_label(card, "58 %RH", 8, 31, C_BLUE);
    s_humidity_bar = make_track(card, 8, 59, 132, C_BLUE, 77);

    card = make_panel(s_main_screen, 8, 134, 304, 58);
    make_label(card, "AIR QUALITY", 8, 8, C_MUTED);
    make_label(card, "GOOD", 8, 32, C_GREEN);
    s_co2_main = make_label(card, "CO2  612 ppm", 105, 10, C_TEXT);
    s_pm25_main = make_label(card, "PM2.5  12 ug/m3", 105, 34, C_TEXT);

    button = lv_button_create(s_main_screen);
    lv_obj_set_pos(button, 8, 200);
    lv_obj_set_size(button, 304, 32);
    lv_obj_set_style_bg_color(button, lv_color_hex(C_BLUE), 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(C_BLUE_DARK), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, lv_color_hex(C_BLUE_LIGHT), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 2, LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 7, 0);
    label = lv_label_create(button);
    lv_label_set_text(label, "VIEW DETAILS  >");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(button, detail_button_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(s_main_screen, main_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    s_pages[0] = s_main_screen;
    return s_main_screen;
}

lv_obj_t *ui_environment_detail_create(void)
{
    lv_obj_t *card;
    lv_obj_t *tip;

    s_detail_screen = lv_obj_create(NULL);
    prepare_screen(s_detail_screen);
    make_header(s_detail_screen, "ENV DETAILS", true);

    card = make_panel(s_detail_screen, 8, 50, 304, 48);
    make_label(card, "CO2", 8, 8, C_TEXT);
    s_co2_detail = make_label(card, "612 ppm", 225, 7, C_GREEN);
    s_co2_bar = make_track(card, 8, 33, 288, C_GREEN, 94);

    card = make_panel(s_detail_screen, 8, 104, 304, 48);
    make_label(card, "PM2.5", 8, 8, C_TEXT);
    s_pm25_detail = make_label(card, "12 ug/m3", 218, 7, C_BLUE);
    s_pm25_bar = make_track(card, 8, 33, 288, C_BLUE, 69);

    card = make_panel(s_detail_screen, 8, 158, 148, 42);
    make_label(card, "PRESSURE", 8, 8, C_MUTED);
    make_label(card, "1013 hPa", 72, 8, C_TEXT);

    card = make_panel(s_detail_screen, 164, 158, 148, 42);
    make_label(card, "LIGHT", 8, 8, C_MUTED);
    make_label(card, "386 lux", 76, 8, C_TEXT);

    tip = lv_obj_create(s_detail_screen);
    lv_obj_set_pos(tip, 8, 207);
    lv_obj_set_size(tip, 304, 25);
    lv_obj_set_style_bg_color(tip, lv_color_hex(0x0B2A25), 0);
    lv_obj_set_style_bg_opa(tip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(tip, lv_color_hex(C_GREEN), 0);
    lv_obj_set_style_border_width(tip, 1, 0);
    lv_obj_set_style_radius(tip, 7, 0);
    lv_obj_set_style_pad_all(tip, 0, 0);
    make_label(tip, "All readings are within the healthy range", 15, 5, C_GREEN);

    lv_obj_add_event_cb(s_detail_screen, detail_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    s_pages[1] = s_detail_screen;
    return s_detail_screen;
}

lv_obj_t *ui_environment_dynamic_create(void)
{
    lv_obj_t *card;
    s_dynamic_screen = lv_obj_create(NULL);
    prepare_screen(s_dynamic_screen);
    make_header(s_dynamic_screen, "LIVE DATA", true);

    card = make_panel(s_dynamic_screen, 8, 50, 148, 72);
    make_label(card, "UPTIME", 9, 8, C_MUTED);
    s_dynamic_uptime = make_label(card, "0 s", 9, 38, C_GREEN);

    card = make_panel(s_dynamic_screen, 164, 50, 148, 72);
    make_label(card, "UI REFRESH", 9, 8, C_MUTED);
    s_dynamic_refresh = make_label(card, "0 frames", 9, 38, C_BLUE_LIGHT);

    card = make_panel(s_dynamic_screen, 8, 130, 148, 72);
    make_label(card, "PHY LINK", 9, 8, C_MUTED);
    s_dynamic_link = make_label(card, "CHECKING", 9, 38, C_ORANGE);

    card = make_panel(s_dynamic_screen, 164, 130, 148, 72);
    make_label(card, "SAMPLE", 9, 8, C_MUTED);
    s_dynamic_sample = make_label(card, "24.6 C", 9, 38, C_GREEN);

    make_label(s_dynamic_screen, "Values update once per second", 52, 216, C_MUTED);
    s_pages[2] = s_dynamic_screen;
    return s_dynamic_screen;
}

void ui_navigation_short_press(void)
{
    if(lv_screen_active() == s_menu_screen) {
        s_menu_selected = (uint8_t)((s_menu_selected + 1u) % 3u);
        update_menu_highlight();
    }
    else {
        s_current_page = (uint8_t)((s_current_page + 1u) % 3u);
        if(s_pages[s_current_page] != NULL)
            lv_screen_load_anim(s_pages[s_current_page], LV_SCREEN_LOAD_ANIM_MOVE_LEFT,
                                240, 0, false);
    }
}

void ui_navigation_long_press(void)
{
    if(lv_screen_active() == s_menu_screen) {
        s_current_page = s_menu_selected;
        if(s_pages[s_current_page] != NULL)
            lv_screen_load_anim(s_pages[s_current_page], LV_SCREEN_LOAD_ANIM_MOVE_LEFT,
                                280, 0, false);
    }
    else if(s_menu_screen != NULL) {
        lv_screen_load_anim(s_menu_screen, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT,
                            280, 0, false);
    }
}

void ui_dynamic_data_update(uint32_t uptime_seconds,
                            uint32_t refresh_count,
                            uint32_t link_up)
{
    char text[24];
    if(s_dynamic_uptime != NULL) {
        snprintf(text, sizeof(text), "%lu s", (unsigned long)uptime_seconds);
        lv_label_set_text(s_dynamic_uptime, text);
    }
    if(s_dynamic_refresh != NULL) {
        snprintf(text, sizeof(text), "%lu frames", (unsigned long)refresh_count);
        lv_label_set_text(s_dynamic_refresh, text);
    }
    if(s_dynamic_link != NULL) {
        lv_label_set_text(s_dynamic_link, link_up ? "LINK UP" : "LINK DOWN");
        lv_obj_set_style_text_color(s_dynamic_link,
                                    lv_color_hex(link_up ? C_GREEN : C_ORANGE), 0);
    }
    if(s_dynamic_sample != NULL) {
        snprintf(text, sizeof(text), "%lu.%lu C",
                 (unsigned long)(24u + ((uptime_seconds / 10u) % 3u)),
                 (unsigned long)(uptime_seconds % 10u));
        lv_label_set_text(s_dynamic_sample, text);
    }
}

void ui_environment_show_main(void)
{
    s_current_page = 0;
    if(s_main_screen != NULL)
        lv_screen_load_anim(s_main_screen, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT, 280, 0, false);
}

void ui_environment_show_detail(void)
{
    s_current_page = 1;
    if(s_detail_screen != NULL)
        lv_screen_load_anim(s_detail_screen, LV_SCREEN_LOAD_ANIM_MOVE_LEFT, 280, 0, false);
}

void ui_environment_set_readings(int32_t temperature_tenths,
                                 uint32_t humidity_percent,
                                 uint32_t co2_ppm,
                                 uint32_t pm25_ug_m3)
{
    char text[32];
    uint32_t width;

    if(s_temp_value != NULL) {
        snprintf(text, sizeof(text), "%ld.%ld C",
                 (long)(temperature_tenths / 10),
                 (long)(temperature_tenths < 0 ? -(temperature_tenths % 10) : temperature_tenths % 10));
        lv_label_set_text(s_temp_value, text);
    }
    if(s_humidity_value != NULL) {
        snprintf(text, sizeof(text), "%lu %%RH", (unsigned long)humidity_percent);
        lv_label_set_text(s_humidity_value, text);
    }
    if(s_humidity_bar != NULL) {
        if(humidity_percent > 100u) humidity_percent = 100u;
        width = humidity_percent * 132u / 100u;
        lv_obj_set_width(s_humidity_bar, (int32_t)width);
    }
    if(s_co2_main != NULL) {
        snprintf(text, sizeof(text), "CO2  %lu ppm", (unsigned long)co2_ppm);
        lv_label_set_text(s_co2_main, text);
    }
    if(s_co2_detail != NULL) {
        snprintf(text, sizeof(text), "%lu ppm", (unsigned long)co2_ppm);
        lv_label_set_text(s_co2_detail, text);
    }
    if(s_co2_bar != NULL) {
        width = co2_ppm >= 2000u ? 288u : co2_ppm * 288u / 2000u;
        lv_obj_set_width(s_co2_bar, (int32_t)width);
    }
    if(s_pm25_main != NULL) {
        snprintf(text, sizeof(text), "PM2.5  %lu ug/m3", (unsigned long)pm25_ug_m3);
        lv_label_set_text(s_pm25_main, text);
    }
    if(s_pm25_detail != NULL) {
        snprintf(text, sizeof(text), "%lu ug/m3", (unsigned long)pm25_ug_m3);
        lv_label_set_text(s_pm25_detail, text);
    }
    if(s_pm25_bar != NULL) {
        width = pm25_ug_m3 >= 100u ? 288u : pm25_ug_m3 * 288u / 100u;
        lv_obj_set_width(s_pm25_bar, (int32_t)width);
    }
}
