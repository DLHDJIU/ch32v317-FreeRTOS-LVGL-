#ifndef UI_ANYUI5_H
#define UI_ANYUI5_H

#include "lvgl.h"
#include <stdint.h>

/* Environment monitor UI.  The legacy file name is kept so the MounRiver
 * project does not need its source list rewritten. */
lv_obj_t *ui_environment_main_create(void);
lv_obj_t *ui_environment_detail_create(void);
lv_obj_t *ui_environment_menu_create(void);
lv_obj_t *ui_environment_dynamic_create(void);

void ui_environment_set_readings(int32_t temperature_tenths,
                                 uint32_t humidity_percent,
                                 uint32_t co2_ppm,
                                 uint32_t pm25_ug_m3);

/* Programmatic navigation, also used by the UART 1/2 commands. */
void ui_environment_show_main(void);
void ui_environment_show_detail(void);

/* Single-key navigation: short press selects/advances, long press enters or
 * returns to the menu.  Connect these to the board key scanner. */
void ui_navigation_short_press(void);
void ui_navigation_long_press(void);

/* Update the dynamic-data page from the application once per second. */
void ui_dynamic_data_update(uint32_t uptime_seconds,
                            uint32_t refresh_count,
                            uint32_t link_up);

#endif /* UI_ANYUI5_H */
