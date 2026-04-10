#include "ziwei_ui.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "lvgl.h"

#include "ziwei_chart.h"
#include "ziwei_font.h"

typedef struct {
    lv_obj_t *tabview;
    lv_obj_t *year_spin;
    lv_obj_t *month_spin;
    lv_obj_t *day_spin;
    lv_obj_t *hour_spin;
    lv_obj_t *minute_spin;
    lv_obj_t *gender_btn_female;
    lv_obj_t *gender_btn_male;
    lv_obj_t *status_label;
    lv_obj_t *summary_label;
    lv_obj_t *mode_btn[5];
    lv_obj_t *quick_row;
    lv_obj_t *quick_btn_prev;
    lv_obj_t *quick_btn_next;
    lv_obj_t *center_panel;
    lv_obj_t *center_label;
    lv_obj_t *detail_overlay;
    lv_obj_t *detail_title;
    lv_obj_t *detail_body;
    lv_obj_t *selector_overlay;
    lv_obj_t *selector_title;
    lv_obj_t *selector_value;
    lv_obj_t *selector_info;
    lv_obj_t *palace_btn[ZIWEI_PALACE_COUNT];
    lv_obj_t *palace_label[ZIWEI_PALACE_COUNT];
    ziwei_chart_result_t chart;
    int selected_palace;
    ziwei_gender_t selected_gender;
    int flow_year;
    int flow_month;
    int flow_day;
    int selected_decade_step;
    int selector_value_temp;
} ziwei_ui_state_t;

typedef enum {
    ZIWEI_VIEW_NATAL = 0,
    ZIWEI_VIEW_DECADE,
    ZIWEI_VIEW_FLOW_YEAR,
    ZIWEI_VIEW_FLOW_MONTH,
    ZIWEI_VIEW_FLOW_DAY
} ziwei_view_mode_t;

typedef enum {
    ZIWEI_SELECTOR_NONE = 0,
    ZIWEI_SELECTOR_DECADE,
    ZIWEI_SELECTOR_FLOW_YEAR,
    ZIWEI_SELECTOR_FLOW_MONTH,
    ZIWEI_SELECTOR_FLOW_DAY
} ziwei_selector_kind_t;

static ziwei_ui_state_t g_ui;
static const char *TAG_ZIWEI_UI = "ZIWEI_UI";
static char g_detail_title_buffer[64];
static char g_detail_body_buffer[12288];
static char g_chart_summary_buffer[768];
static char g_selector_title_buffer[32];
static char g_selector_value_buffer[32];
static char g_selector_info_buffer[64];
static ziwei_view_mode_t g_view_mode = ZIWEI_VIEW_NATAL;
static ziwei_selector_kind_t g_selector_kind = ZIWEI_SELECTOR_NONE;

#define ZIWEI_FONT_TEXT (&ziwei_font_16)

static void refresh_chart_content(void);

static void style_input_button(lv_obj_t *btn)
{
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x243447), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x4C6684), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
}

static void style_input_field(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16202A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x4C6684), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_right(obj, 4, LV_PART_MAIN);
}

static int wrap_palace_ui(int palace)
{
    while (palace < 0) {
        palace += ZIWEI_PALACE_COUNT;
    }
    return palace % ZIWEI_PALACE_COUNT;
}

static int selected_decade_palace(const ziwei_chart_result_t *chart)
{
    if (chart == NULL || !chart->valid) {
        return -1;
    }

    return wrap_palace_ui(chart->ming_palace + (chart->decade_forward ? g_ui.selected_decade_step : -g_ui.selected_decade_step));
}

static int decade_palace_by_step(const ziwei_chart_result_t *chart, int step)
{
    if (chart == NULL || !chart->valid) {
        return -1;
    }

    return wrap_palace_ui(chart->ming_palace + (chart->decade_forward ? step : -step));
}

static int current_mode_palace(const ziwei_chart_result_t *chart)
{
    if (chart == NULL || !chart->valid) {
        return -1;
    }

    switch (g_view_mode) {
    case ZIWEI_VIEW_DECADE:
        return selected_decade_palace(chart);
    case ZIWEI_VIEW_FLOW_YEAR:
        return chart->flow_year_palace;
    case ZIWEI_VIEW_FLOW_MONTH:
        return chart->flow_month_palace;
    case ZIWEI_VIEW_FLOW_DAY:
        return chart->flow_day_palace;
    default:
        return -1;
    }
}

static const char *current_mode_marker(void)
{
    switch (g_view_mode) {
    case ZIWEI_VIEW_FLOW_YEAR:
        return "年宫";
    case ZIWEI_VIEW_FLOW_MONTH:
        return "月宫";
    case ZIWEI_VIEW_FLOW_DAY:
        return "日宫";
    default:
        return "";
    }
}

static void build_palace_display_text(const ziwei_chart_result_t *chart,
                                      int palace_index,
                                      char *buffer,
                                      size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    buffer[0] = '\0';

    if (chart == NULL || !chart->valid) {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    switch (g_view_mode) {
    case ZIWEI_VIEW_DECADE:
        snprintf(buffer, buffer_size, "%s\n%d",
                 ziwei_get_palace_role_short_name(chart->palace_role_by_index[palace_index]),
                 chart->decade_age_start_by_palace[palace_index]);
        break;
    case ZIWEI_VIEW_FLOW_YEAR:
    case ZIWEI_VIEW_FLOW_MONTH:
    case ZIWEI_VIEW_FLOW_DAY:
        if (current_mode_palace(chart) == palace_index) {
            snprintf(buffer, buffer_size, "%s\n%s",
                     ziwei_get_palace_role_short_name(chart->palace_role_by_index[palace_index]),
                     current_mode_marker());
        } else {
            snprintf(buffer, buffer_size, "%s",
                     ziwei_get_palace_role_short_name(chart->palace_role_by_index[palace_index]));
        }
        break;
    default:
        ziwei_build_palace_grid_text(chart, palace_index, buffer, buffer_size);
        break;
    }
}

static bool is_leap_year(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static int days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) {
        return 31;
    }

    if (month == 2 && is_leap_year(year)) {
        return 29;
    }

    return days[month - 1];
}

static void sync_day_range(void)
{
    const int year = lv_spinbox_get_value(g_ui.year_spin);
    const int month = lv_spinbox_get_value(g_ui.month_spin);
    const int max_day = days_in_month(year, month);
    const int current_day = lv_spinbox_get_value(g_ui.day_spin);

    lv_spinbox_set_range(g_ui.day_spin, 1, max_day);
    if (current_day > max_day) {
        lv_spinbox_set_value(g_ui.day_spin, max_day);
    }
}

static bool spinbox_increment_with_wrap(lv_obj_t *spinbox)
{
    int min_value = 0;
    int max_value = 0;
    int current_value;

    if (spinbox == g_ui.month_spin) {
        min_value = 1;
        max_value = 12;
    } else if (spinbox == g_ui.day_spin) {
        min_value = 1;
        max_value = days_in_month(lv_spinbox_get_value(g_ui.year_spin), lv_spinbox_get_value(g_ui.month_spin));
    } else if (spinbox == g_ui.hour_spin) {
        min_value = 0;
        max_value = 23;
    } else if (spinbox == g_ui.minute_spin) {
        min_value = 0;
        max_value = 55;
    } else {
        return false;
    }

    current_value = lv_spinbox_get_value(spinbox);
    if (current_value >= max_value) {
        lv_spinbox_set_value(spinbox, min_value);
    } else {
        lv_spinbox_increment(spinbox);
    }

    return true;
}

static bool spinbox_decrement_with_wrap(lv_obj_t *spinbox)
{
    int min_value = 0;
    int max_value = 0;
    int current_value;

    if (spinbox == g_ui.month_spin) {
        min_value = 1;
        max_value = 12;
    } else if (spinbox == g_ui.day_spin) {
        min_value = 1;
        max_value = days_in_month(lv_spinbox_get_value(g_ui.year_spin), lv_spinbox_get_value(g_ui.month_spin));
    } else if (spinbox == g_ui.hour_spin) {
        min_value = 0;
        max_value = 23;
    } else if (spinbox == g_ui.minute_spin) {
        min_value = 0;
        max_value = 55;
    } else {
        return false;
    }

    current_value = lv_spinbox_get_value(spinbox);
    if (current_value <= min_value) {
        lv_spinbox_set_value(spinbox, max_value);
    } else {
        lv_spinbox_decrement(spinbox);
    }

    return true;
}

static void spinbox_inc_event(lv_event_t *e)
{
    lv_obj_t *spinbox = lv_event_get_user_data(e);

    if (!spinbox_increment_with_wrap(spinbox)) {
        lv_spinbox_increment(spinbox);
    }
    if (spinbox == g_ui.year_spin || spinbox == g_ui.month_spin) {
        sync_day_range();
    }
}

static void spinbox_dec_event(lv_event_t *e)
{
    lv_obj_t *spinbox = lv_event_get_user_data(e);

    if (!spinbox_decrement_with_wrap(spinbox)) {
        lv_spinbox_decrement(spinbox);
    }
    if (spinbox == g_ui.year_spin || spinbox == g_ui.month_spin) {
        sync_day_range();
    }
}

static void spinbox_changed_event(lv_event_t *e)
{
    lv_obj_t *spinbox = lv_event_get_target(e);

    if (spinbox == g_ui.year_spin || spinbox == g_ui.month_spin) {
        sync_day_range();
    }
}

static void refresh_gender_buttons(void)
{
    lv_color_t selected_bg = lv_palette_main(LV_PALETTE_BLUE);
    lv_color_t normal_bg = lv_color_hex(0x243447);
    lv_obj_t *buttons[2] = {g_ui.gender_btn_female, g_ui.gender_btn_male};
    ziwei_gender_t values[2] = {ZIWEI_GENDER_FEMALE, ZIWEI_GENDER_MALE};
    int i;

    for (i = 0; i < 2; ++i) {
        if (buttons[i] == NULL) {
            continue;
        }

        lv_obj_set_style_bg_color(buttons[i],
                                  g_ui.selected_gender == values[i] ? selected_bg : normal_bg,
                                  LV_PART_MAIN);
        lv_obj_set_style_bg_opa(buttons[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(buttons[i], lv_color_hex(0x4C6684), LV_PART_MAIN);
        lv_obj_set_style_border_width(buttons[i], 1, LV_PART_MAIN);
        lv_obj_set_style_text_color(buttons[i], lv_color_white(), LV_PART_MAIN);
    }
}

static void refresh_mode_buttons(void)
{
    lv_color_t selected_bg = lv_palette_main(LV_PALETTE_BLUE_GREY);
    lv_color_t normal_bg = lv_color_hex(0x243447);
    int i;

    for (i = 0; i < 5; ++i) {
        if (g_ui.mode_btn[i] == NULL) {
            continue;
        }

        lv_obj_set_style_bg_color(g_ui.mode_btn[i], i == (int)g_view_mode ? selected_bg : normal_bg, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_ui.mode_btn[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(g_ui.mode_btn[i], lv_color_hex(0x4C6684), LV_PART_MAIN);
        lv_obj_set_style_border_width(g_ui.mode_btn[i], 1, LV_PART_MAIN);
    }
}

static void refresh_quick_switch_buttons(void)
{
    bool show_buttons = g_ui.chart.valid && g_view_mode != ZIWEI_VIEW_NATAL;

    if (g_ui.quick_btn_prev == NULL || g_ui.quick_btn_next == NULL) {
        return;
    }

    if (show_buttons) {
        lv_obj_clear_flag(g_ui.quick_btn_prev, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_ui.quick_btn_next, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_ui.quick_btn_prev, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_ui.quick_btn_next, LV_OBJ_FLAG_HIDDEN);
    }
}

static void sync_flow_day(void)
{
    int max_day;

    max_day = days_in_month(g_ui.flow_year, g_ui.flow_month);
    if (g_ui.flow_day > max_day) {
        g_ui.flow_day = max_day;
    }
}

static void refresh_flow_label(void)
{
    LV_UNUSED(g_ui.flow_year);
    LV_UNUSED(g_ui.flow_month);
    LV_UNUSED(g_ui.flow_day);
}

static void apply_flow_date(void)
{
    if (!g_ui.chart.valid) {
        return;
    }

    if (ziwei_chart_set_flow_date(&g_ui.chart, g_ui.flow_year, g_ui.flow_month, g_ui.flow_day)) {
        refresh_flow_label();
        refresh_chart_content();
    }
}

static void shift_flow_year(int delta)
{
    g_ui.flow_year += delta;
    if (g_ui.flow_year < 1901) {
        g_ui.flow_year = 1901;
    }
    if (g_ui.flow_year > 2100) {
        g_ui.flow_year = 2100;
    }
    sync_flow_day();
}

static void shift_flow_month(int delta)
{
    g_ui.flow_month += delta;

    while (g_ui.flow_month < 1) {
        g_ui.flow_month += 12;
        if (g_ui.flow_year > 1901) {
            g_ui.flow_year--;
        } else {
            g_ui.flow_month = 1;
            break;
        }
    }

    while (g_ui.flow_month > 12) {
        g_ui.flow_month -= 12;
        if (g_ui.flow_year < 2100) {
            g_ui.flow_year++;
        } else {
            g_ui.flow_month = 12;
            break;
        }
    }

    sync_flow_day();
}

static void shift_flow_day(int delta)
{
    int max_day;

    if (delta < 0 && g_ui.flow_year == 1901 && g_ui.flow_month == 1 && g_ui.flow_day == 1) {
        return;
    }
    if (delta > 0 && g_ui.flow_year == 2100 && g_ui.flow_month == 12 && g_ui.flow_day == 31) {
        return;
    }

    g_ui.flow_day += delta;
    max_day = days_in_month(g_ui.flow_year, g_ui.flow_month);

    while (g_ui.flow_day < 1) {
        shift_flow_month(-1);
        g_ui.flow_day += days_in_month(g_ui.flow_year, g_ui.flow_month);
    }

    max_day = days_in_month(g_ui.flow_year, g_ui.flow_month);
    while (g_ui.flow_day > max_day) {
        g_ui.flow_day -= max_day;
        shift_flow_month(1);
        max_day = days_in_month(g_ui.flow_year, g_ui.flow_month);
    }
}

static void gender_select_event(lv_event_t *e)
{
    g_ui.selected_gender = (ziwei_gender_t)(intptr_t)lv_event_get_user_data(e);
    refresh_gender_buttons();
}

static void hide_selector_overlay(void)
{
    g_selector_kind = ZIWEI_SELECTOR_NONE;
    if (g_ui.selector_overlay != NULL) {
        lv_obj_add_flag(g_ui.selector_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

static void refresh_selector_overlay(void)
{
    int palace;

    if (g_ui.selector_overlay == NULL || g_selector_kind == ZIWEI_SELECTOR_NONE) {
        return;
    }

    switch (g_selector_kind) {
    case ZIWEI_SELECTOR_DECADE:
        palace = decade_palace_by_step(&g_ui.chart, g_ui.selector_value_temp);
        snprintf(g_selector_title_buffer, sizeof(g_selector_title_buffer), "选择大限");
        snprintf(g_selector_value_buffer, sizeof(g_selector_value_buffer), "%d-%d岁",
                 g_ui.chart.decade_age_start_by_palace[palace],
                 g_ui.chart.decade_age_end_by_palace[palace]);
        snprintf(g_selector_info_buffer, sizeof(g_selector_info_buffer), "%s%s宫",
                 ziwei_get_palace_role_name(g_ui.chart.palace_role_by_index[palace]),
                 ziwei_get_branch_name(palace));
        break;
    case ZIWEI_SELECTOR_FLOW_YEAR:
        snprintf(g_selector_title_buffer, sizeof(g_selector_title_buffer), "选择流年");
        snprintf(g_selector_value_buffer, sizeof(g_selector_value_buffer), "%04d", g_ui.selector_value_temp);
        snprintf(g_selector_info_buffer, sizeof(g_selector_info_buffer), "当前流运年份");
        break;
    case ZIWEI_SELECTOR_FLOW_MONTH:
        snprintf(g_selector_title_buffer, sizeof(g_selector_title_buffer), "选择流月");
        snprintf(g_selector_value_buffer, sizeof(g_selector_value_buffer), "%02d", g_ui.selector_value_temp);
        snprintf(g_selector_info_buffer, sizeof(g_selector_info_buffer), "当前流运月份");
        break;
    case ZIWEI_SELECTOR_FLOW_DAY:
        snprintf(g_selector_title_buffer, sizeof(g_selector_title_buffer), "选择流日");
        snprintf(g_selector_value_buffer, sizeof(g_selector_value_buffer), "%02d", g_ui.selector_value_temp);
        snprintf(g_selector_info_buffer, sizeof(g_selector_info_buffer), "当前流运日期");
        break;
    default:
        g_selector_title_buffer[0] = '\0';
        g_selector_value_buffer[0] = '\0';
        g_selector_info_buffer[0] = '\0';
        break;
    }

    lv_label_set_text(g_ui.selector_title, g_selector_title_buffer);
    lv_label_set_text(g_ui.selector_value, g_selector_value_buffer);
    lv_label_set_text(g_ui.selector_info, g_selector_info_buffer);
}

static void open_selector_overlay(ziwei_selector_kind_t kind)
{
    if (!g_ui.chart.valid || g_ui.selector_overlay == NULL) {
        return;
    }

    g_selector_kind = kind;
    switch (kind) {
    case ZIWEI_SELECTOR_DECADE:
        g_ui.selector_value_temp = g_ui.selected_decade_step;
        break;
    case ZIWEI_SELECTOR_FLOW_YEAR:
        g_ui.selector_value_temp = g_ui.flow_year;
        break;
    case ZIWEI_SELECTOR_FLOW_MONTH:
        g_ui.selector_value_temp = g_ui.flow_month;
        break;
    case ZIWEI_SELECTOR_FLOW_DAY:
        g_ui.selector_value_temp = g_ui.flow_day;
        break;
    default:
        break;
    }

    refresh_selector_overlay();
    lv_obj_clear_flag(g_ui.selector_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_ui.selector_overlay);
}

static void selector_close_event(lv_event_t *e)
{
    LV_UNUSED(e);
    hide_selector_overlay();
}

static void selector_adjust_event(lv_event_t *e)
{
    int delta = (int)(intptr_t)lv_event_get_user_data(e);

    switch (g_selector_kind) {
    case ZIWEI_SELECTOR_DECADE:
        g_ui.selector_value_temp += delta;
        if (g_ui.selector_value_temp < 0) {
            g_ui.selector_value_temp = 0;
        }
        if (g_ui.selector_value_temp > 11) {
            g_ui.selector_value_temp = 11;
        }
        break;
    case ZIWEI_SELECTOR_FLOW_YEAR:
        g_ui.selector_value_temp += delta;
        if (g_ui.selector_value_temp < 1901) {
            g_ui.selector_value_temp = 1901;
        }
        if (g_ui.selector_value_temp > 2100) {
            g_ui.selector_value_temp = 2100;
        }
        break;
    case ZIWEI_SELECTOR_FLOW_MONTH:
        g_ui.selector_value_temp += delta;
        if (g_ui.selector_value_temp < 1) {
            g_ui.selector_value_temp = 12;
        }
        if (g_ui.selector_value_temp > 12) {
            g_ui.selector_value_temp = 1;
        }
        break;
    case ZIWEI_SELECTOR_FLOW_DAY:
        g_ui.selector_value_temp += delta;
        if (g_ui.selector_value_temp < 1) {
            g_ui.selector_value_temp = days_in_month(g_ui.flow_year, g_ui.flow_month);
        }
        if (g_ui.selector_value_temp > days_in_month(g_ui.flow_year, g_ui.flow_month)) {
            g_ui.selector_value_temp = 1;
        }
        break;
    default:
        break;
    }

    refresh_selector_overlay();
}

static void selector_apply_event(lv_event_t *e)
{
    LV_UNUSED(e);

    switch (g_selector_kind) {
    case ZIWEI_SELECTOR_DECADE:
        g_ui.selected_decade_step = g_ui.selector_value_temp;
        g_view_mode = ZIWEI_VIEW_DECADE;
        break;
    case ZIWEI_SELECTOR_FLOW_YEAR:
        g_view_mode = ZIWEI_VIEW_FLOW_YEAR;
        g_ui.flow_year = g_ui.selector_value_temp;
        sync_flow_day();
        apply_flow_date();
        break;
    case ZIWEI_SELECTOR_FLOW_MONTH:
        g_view_mode = ZIWEI_VIEW_FLOW_MONTH;
        g_ui.flow_month = g_ui.selector_value_temp;
        sync_flow_day();
        apply_flow_date();
        break;
    case ZIWEI_SELECTOR_FLOW_DAY:
        g_view_mode = ZIWEI_VIEW_FLOW_DAY;
        g_ui.flow_day = g_ui.selector_value_temp;
        apply_flow_date();
        break;
    default:
        break;
    }

    refresh_mode_buttons();
    refresh_quick_switch_buttons();
    refresh_chart_content();
    hide_selector_overlay();
}

static void view_mode_event(lv_event_t *e)
{
    ziwei_view_mode_t mode = (ziwei_view_mode_t)(intptr_t)lv_event_get_user_data(e);

    switch (mode) {
    case ZIWEI_VIEW_NATAL:
        g_view_mode = ZIWEI_VIEW_NATAL;
        refresh_mode_buttons();
        refresh_quick_switch_buttons();
        refresh_chart_content();
        break;
    case ZIWEI_VIEW_DECADE:
        open_selector_overlay(ZIWEI_SELECTOR_DECADE);
        break;
    case ZIWEI_VIEW_FLOW_YEAR:
        open_selector_overlay(ZIWEI_SELECTOR_FLOW_YEAR);
        break;
    case ZIWEI_VIEW_FLOW_MONTH:
        open_selector_overlay(ZIWEI_SELECTOR_FLOW_MONTH);
        break;
    case ZIWEI_VIEW_FLOW_DAY:
        open_selector_overlay(ZIWEI_SELECTOR_FLOW_DAY);
        break;
    default:
        break;
    }
}

static void quick_switch_event(lv_event_t *e)
{
    int delta = (int)(intptr_t)lv_event_get_user_data(e);

    if (!g_ui.chart.valid || delta == 0) {
        return;
    }

    switch (g_view_mode) {
    case ZIWEI_VIEW_DECADE:
        g_ui.selected_decade_step += delta;
        if (g_ui.selected_decade_step < 0) {
            g_ui.selected_decade_step = 0;
        }
        if (g_ui.selected_decade_step > 11) {
            g_ui.selected_decade_step = 11;
        }
        refresh_chart_content();
        break;
    case ZIWEI_VIEW_FLOW_YEAR:
        shift_flow_year(delta);
        apply_flow_date();
        break;
    case ZIWEI_VIEW_FLOW_MONTH:
        shift_flow_month(delta);
        apply_flow_date();
        break;
    case ZIWEI_VIEW_FLOW_DAY:
        shift_flow_day(delta);
        apply_flow_date();
        break;
    default:
        break;
    }
}

static lv_obj_t *create_spinbox_row(lv_obj_t *parent,
                                    const char *title,
                                    int min_value,
                                    int max_value,
                                    int digits,
                                    int default_value,
                                    int step)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_t *title_label;
    lv_obj_t *minus_btn;
    lv_obj_t *plus_btn;
    lv_obj_t *minus_label;
    lv_obj_t *plus_label;
    lv_obj_t *spinbox;

    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 26);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 3, 0);
    lv_obj_set_style_pad_top(row, 0, 0);
    lv_obj_set_style_pad_bottom(row, 0, 0);

    title_label = lv_label_create(row);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_font(title_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_width(title_label, 34);
    lv_obj_center(title_label);

    minus_btn = lv_btn_create(row);
    lv_obj_set_size(minus_btn, 22, 22);
    style_input_button(minus_btn);
    minus_label = lv_label_create(minus_btn);
    lv_label_set_text(minus_label, "-");
    lv_obj_set_style_text_font(minus_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(minus_label, lv_color_white(), 0);
    lv_obj_center(minus_label);

    spinbox = lv_spinbox_create(row);
    lv_spinbox_set_range(spinbox, min_value, max_value);
    lv_spinbox_set_digit_format(spinbox, digits, 0);
    lv_spinbox_set_step(spinbox, step);
    lv_spinbox_set_value(spinbox, default_value);
    lv_obj_set_size(spinbox, 56, 26);
    lv_obj_set_style_text_font(spinbox, ZIWEI_FONT_TEXT, 0);
    style_input_field(spinbox);
    lv_obj_set_style_pad_left(spinbox, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(spinbox, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(spinbox, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(spinbox, 1, LV_PART_MAIN);
    lv_obj_set_style_text_align(spinbox, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_event_cb(spinbox, spinbox_changed_event, LV_EVENT_VALUE_CHANGED, NULL);

    plus_btn = lv_btn_create(row);
    lv_obj_set_size(plus_btn, 22, 22);
    style_input_button(plus_btn);
    plus_label = lv_label_create(plus_btn);
    lv_label_set_text(plus_label, "+");
    lv_obj_set_style_text_font(plus_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(plus_label, lv_color_white(), 0);
    lv_obj_center(plus_label);

    lv_obj_add_event_cb(minus_btn, spinbox_dec_event, LV_EVENT_CLICKED, spinbox);
    lv_obj_add_event_cb(plus_btn, spinbox_inc_event, LV_EVENT_CLICKED, spinbox);

    return spinbox;
}

static void hide_detail_overlay(void)
{
    if (g_ui.detail_overlay != NULL) {
        lv_obj_add_flag(g_ui.detail_overlay, LV_OBJ_FLAG_HIDDEN);
    }
}

static void close_detail_event(lv_event_t *e)
{
    LV_UNUSED(e);
    hide_detail_overlay();
}

static void extract_summary_preview(const char *source, char *dest, size_t dest_size)
{
    const char *cursor;
    int newline_count = 0;
    size_t copy_len;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (source == NULL) {
        return;
    }

    cursor = source;
    while (*cursor != '\0') {
        if (*cursor == '\n') {
            newline_count++;
            if (newline_count >= 2) {
                cursor++;
                break;
            }
        }
        cursor++;
    }

    copy_len = (newline_count >= 2) ? (size_t)(cursor - source - 1) : strlen(source);
    if (copy_len >= dest_size) {
        copy_len = dest_size - 1;
    }

    memcpy(dest, source, copy_len);
    dest[copy_len] = '\0';
}

static void extract_summary_detail(const char *source, char *dest, size_t dest_size)
{
    const char *cursor;
    int newline_count = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (source == NULL || source[0] == '\0') {
        return;
    }

    cursor = source;
    while (*cursor != '\0' && newline_count < 2) {
        if (*cursor == '\n') {
            newline_count++;
        }
        cursor++;
    }

    if (*cursor == '\0') {
        strncpy(dest, "请点击查看详情。", dest_size - 1);
        dest[dest_size - 1] = '\0';
        return;
    }

    strncpy(dest, cursor, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

static void show_detail_overlay(int palace_index)
{
    if (!g_ui.chart.valid || g_ui.detail_overlay == NULL) {
        return;
    }

    snprintf(g_detail_title_buffer, sizeof(g_detail_title_buffer),
             "%s%s宫",
             ziwei_get_palace_role_name(g_ui.chart.palace_role_by_index[palace_index]),
             ziwei_get_branch_name(palace_index));
    ziwei_build_palace_detail(&g_ui.chart, palace_index, g_detail_body_buffer, sizeof(g_detail_body_buffer));

    lv_label_set_text(g_ui.detail_title, g_detail_title_buffer);
    lv_label_set_text(g_ui.detail_body, g_detail_body_buffer);
    lv_obj_scroll_to_y(lv_obj_get_parent(g_ui.detail_body), 0, LV_ANIM_OFF);
    lv_obj_clear_flag(g_ui.detail_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_ui.detail_overlay);
}

static void center_click_event(lv_event_t *e)
{
    char preview[256];

    LV_UNUSED(e);

    if (!g_ui.chart.valid || g_ui.detail_overlay == NULL) {
        return;
    }

    snprintf(g_detail_title_buffer, sizeof(g_detail_title_buffer), "命盘详情");
    extract_summary_detail(g_chart_summary_buffer, g_detail_body_buffer, sizeof(g_detail_body_buffer));
    extract_summary_preview(g_chart_summary_buffer, preview, sizeof(preview));

    if (g_detail_body_buffer[0] == '\0') {
        strncpy(g_detail_body_buffer, preview, sizeof(g_detail_body_buffer) - 1);
        g_detail_body_buffer[sizeof(g_detail_body_buffer) - 1] = '\0';
    }

    lv_label_set_text(g_ui.detail_title, g_detail_title_buffer);
    lv_label_set_text(g_ui.detail_body, g_detail_body_buffer);
    lv_obj_scroll_to_y(lv_obj_get_parent(g_ui.detail_body), 0, LV_ANIM_OFF);
    lv_obj_clear_flag(g_ui.detail_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_ui.detail_overlay);
}

static void refresh_selection(void)
{
    int i;
    const int mode_palace = current_mode_palace(&g_ui.chart);

    for (i = 0; i < ZIWEI_PALACE_COUNT; ++i) {
        lv_color_t bg = lv_color_hex(0x2F3A48);
        lv_color_t border = lv_color_hex(0x5A748F);
        if (g_ui.chart.valid && i == g_ui.selected_palace) {
            bg = lv_palette_main(LV_PALETTE_BLUE);
        } else if (g_ui.chart.valid && i == mode_palace) {
            bg = lv_palette_main(LV_PALETTE_DEEP_ORANGE);
            border = lv_palette_lighten(LV_PALETTE_DEEP_ORANGE, 2);
        }
        lv_obj_set_style_bg_color(g_ui.palace_btn[i], bg, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(g_ui.palace_btn[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(g_ui.palace_btn[i], border, LV_PART_MAIN);
    }
}

static void refresh_chart_content(void)
{
    char buffer[512];
    char preview[256];
    int i;

    ziwei_build_chart_summary(&g_ui.chart, g_chart_summary_buffer, sizeof(g_chart_summary_buffer));
    extract_summary_preview(g_chart_summary_buffer, preview, sizeof(preview));
    lv_label_set_text(g_ui.summary_label, preview);

    if (!g_ui.chart.valid) {
        lv_label_set_text(g_ui.center_label, "命盘\n未生成");
        for (i = 0; i < ZIWEI_PALACE_COUNT; ++i) {
            lv_label_set_text(g_ui.palace_label[i], "--");
        }
        refresh_mode_buttons();
        refresh_selection();
        return;
    }

    switch (g_view_mode) {
    case ZIWEI_VIEW_DECADE:
        snprintf(buffer, sizeof(buffer),
                 "命:%s\n限:%d",
                 ziwei_get_branch_name(g_ui.chart.ming_palace),
                 g_ui.chart.decade_start_age);
        break;
    case ZIWEI_VIEW_FLOW_YEAR:
        snprintf(buffer, sizeof(buffer),
                 "年:%s%s\n%04d-%02d-%02d",
                 ziwei_get_stem_name(g_ui.chart.flow_calendar.year_stem),
                 ziwei_get_branch_name(g_ui.chart.flow_calendar.year_branch),
                 g_ui.flow_year,
                 g_ui.flow_month,
                 g_ui.flow_day);
        break;
    case ZIWEI_VIEW_FLOW_MONTH:
        snprintf(buffer, sizeof(buffer),
                 "月:%d\n%04d-%02d-%02d",
                 g_ui.chart.flow_calendar.lunar_month,
                 g_ui.flow_year,
                 g_ui.flow_month,
                 g_ui.flow_day);
        break;
    case ZIWEI_VIEW_FLOW_DAY:
        snprintf(buffer, sizeof(buffer),
                 "日:%d\n%04d-%02d-%02d",
                 g_ui.chart.flow_calendar.lunar_day,
                 g_ui.flow_year,
                 g_ui.flow_month,
                 g_ui.flow_day);
        break;
    default:
        snprintf(buffer, sizeof(buffer),
                 "命:%s\n身:%s",
                 ziwei_get_branch_name(g_ui.chart.ming_palace),
                 ziwei_get_branch_name(g_ui.chart.shen_palace));
        break;
    }
    lv_label_set_text(g_ui.center_label, buffer);

    for (i = 0; i < ZIWEI_PALACE_COUNT; ++i) {
        build_palace_display_text(&g_ui.chart, i, buffer, sizeof(buffer));
        lv_label_set_text(g_ui.palace_label[i], buffer);
    }

    refresh_mode_buttons();
    refresh_selection();
}

static void palace_click_event(lv_event_t *e)
{
    int palace_index = (int)(intptr_t)lv_event_get_user_data(e);

    if (!g_ui.chart.valid) {
        return;
    }

    g_ui.selected_palace = palace_index;
    refresh_selection();
    show_detail_overlay(palace_index);
}

static ziwei_input_t collect_input(void)
{
    ziwei_input_t input;

    input.year = lv_spinbox_get_value(g_ui.year_spin);
    input.month = lv_spinbox_get_value(g_ui.month_spin);
    input.day = lv_spinbox_get_value(g_ui.day_spin);
    input.hour = lv_spinbox_get_value(g_ui.hour_spin);
    input.minute = lv_spinbox_get_value(g_ui.minute_spin);
    input.gender = g_ui.selected_gender;

    return input;
}

static void generate_chart_event(lv_event_t *e)
{
    ziwei_input_t input = collect_input();

    LV_UNUSED(e);

    if (!ziwei_chart_generate(&input, &g_ui.chart)) {
        refresh_chart_content();
        refresh_quick_switch_buttons();
        hide_detail_overlay();
        return;
    }

    g_view_mode = ZIWEI_VIEW_NATAL;
    g_ui.selected_decade_step = 0;
    g_ui.flow_year = input.year;
    g_ui.flow_month = input.month;
    g_ui.flow_day = input.day;
    refresh_flow_label();
    g_ui.selected_palace = g_ui.chart.ming_palace;
    refresh_chart_content();
    refresh_quick_switch_buttons();
    hide_detail_overlay();
    lv_tabview_set_act(g_ui.tabview, 1, LV_ANIM_OFF);
}

static void create_input_tab(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_t *title;
    lv_obj_t *hint;
    lv_obj_t *gender_row;
    lv_obj_t *gender_label;
    lv_obj_t *gender_label_btn;
    lv_obj_t *generate_btn;
    lv_obj_t *generate_label;

    lv_obj_set_size(page, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(page, 6, 0);
    lv_obj_set_layout(page, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, 3, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101820), 0);
    lv_obj_set_style_text_color(page, lv_color_white(), 0);

    title = lv_label_create(page);
    lv_label_set_text(title, "紫薇斗数");
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_font(title, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_line_space(title, -2, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    hint = lv_label_create(page);
    lv_label_set_text(hint, "公历时间");
    lv_obj_set_style_text_font(hint, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(hint, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_line_space(hint, -2, 0);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(hint, lv_pct(100));
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);

    g_ui.year_spin = create_spinbox_row(page, "年份", 1901, 2100, 4, 2000, 1);
    g_ui.month_spin = create_spinbox_row(page, "月份", 1, 12, 2, 6, 1);
    g_ui.day_spin = create_spinbox_row(page, "日期", 1, 31, 2, 23, 1);
    g_ui.hour_spin = create_spinbox_row(page, "小时", 0, 23, 2, 10, 1);
    g_ui.minute_spin = create_spinbox_row(page, "分钟", 0, 55, 2, 0, 5);

    gender_row = lv_obj_create(page);
    lv_obj_remove_style_all(gender_row);
    lv_obj_set_width(gender_row, lv_pct(100));
    lv_obj_set_height(gender_row, 26);
    lv_obj_set_layout(gender_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(gender_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(gender_row, 3, 0);

    gender_label = lv_label_create(gender_row);
    lv_label_set_text(gender_label, "性别");
    lv_obj_set_style_text_font(gender_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(gender_label, lv_color_white(), 0);
    lv_obj_set_width(gender_label, 34);
    lv_obj_center(gender_label);

    g_ui.gender_btn_female = lv_btn_create(gender_row);
    lv_obj_set_size(g_ui.gender_btn_female, 32, 24);
    style_input_button(g_ui.gender_btn_female);
    lv_obj_add_event_cb(g_ui.gender_btn_female, gender_select_event, LV_EVENT_CLICKED, (void *)(intptr_t)ZIWEI_GENDER_FEMALE);
    gender_label_btn = lv_label_create(g_ui.gender_btn_female);
    lv_label_set_text(gender_label_btn, "女");
    lv_obj_set_style_text_font(gender_label_btn, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(gender_label_btn, lv_color_white(), 0);
    lv_obj_center(gender_label_btn);

    g_ui.gender_btn_male = lv_btn_create(gender_row);
    lv_obj_set_size(g_ui.gender_btn_male, 32, 24);
    style_input_button(g_ui.gender_btn_male);
    lv_obj_add_event_cb(g_ui.gender_btn_male, gender_select_event, LV_EVENT_CLICKED, (void *)(intptr_t)ZIWEI_GENDER_MALE);
    gender_label_btn = lv_label_create(g_ui.gender_btn_male);
    lv_label_set_text(gender_label_btn, "男");
    lv_obj_set_style_text_font(gender_label_btn, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(gender_label_btn, lv_color_white(), 0);
    lv_obj_center(gender_label_btn);

    refresh_gender_buttons();

    generate_btn = lv_btn_create(page);
    lv_obj_set_width(generate_btn, lv_pct(100));
    lv_obj_set_height(generate_btn, 28);
    lv_obj_set_style_bg_color(generate_btn, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_text_color(generate_btn, lv_color_white(), 0);
    lv_obj_set_style_radius(generate_btn, 4, LV_PART_MAIN);
    lv_obj_add_event_cb(generate_btn, generate_chart_event, LV_EVENT_CLICKED, NULL);
    generate_label = lv_label_create(generate_btn);
    lv_label_set_text(generate_label, "开始排盘");
    lv_obj_set_style_text_font(generate_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(generate_label, lv_color_white(), 0);
    lv_obj_center(generate_label);

    g_ui.status_label = NULL;

    sync_day_range();
}

static void create_detail_overlay(void)
{
    lv_obj_t *panel;
    lv_obj_t *title_row;
    lv_obj_t *close_btn;
    lv_obj_t *close_label;
    lv_obj_t *body_card;

    g_ui.detail_overlay = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(g_ui.detail_overlay);
    lv_obj_set_size(g_ui.detail_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_ui.detail_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_ui.detail_overlay, LV_OPA_50, 0);
    lv_obj_add_flag(g_ui.detail_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_ui.detail_overlay, LV_OBJ_FLAG_CLICKABLE);

    panel = lv_obj_create(g_ui.detail_overlay);
    lv_obj_set_size(panel, 212, 214);
    lv_obj_center(panel);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 6, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x18222E), 0);
    lv_obj_set_style_text_color(panel, lv_color_white(), 0);

    title_row = lv_obj_create(panel);
    lv_obj_remove_style_all(title_row);
    lv_obj_set_width(title_row, lv_pct(100));
    lv_obj_set_height(title_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(title_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(title_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(title_row, 4, 0);

    g_ui.detail_title = lv_label_create(title_row);
    lv_label_set_text(g_ui.detail_title, "宫位详情");
    lv_obj_set_style_text_font(g_ui.detail_title, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.detail_title, lv_color_white(), 0);
    lv_obj_set_width(g_ui.detail_title, 156);

    close_btn = lv_btn_create(title_row);
    lv_obj_set_size(close_btn, 32, 28);
    style_input_button(close_btn);
    lv_obj_add_event_cb(close_btn, close_detail_event, LV_EVENT_CLICKED, NULL);
    close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "关");
    lv_obj_set_style_text_font(close_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(close_label, lv_color_white(), 0);
    lv_obj_center(close_label);

    body_card = lv_obj_create(panel);
    lv_obj_set_width(body_card, lv_pct(100));
    lv_obj_set_flex_grow(body_card, 1);
    lv_obj_set_scroll_dir(body_card, LV_DIR_VER);
    lv_obj_set_style_pad_all(body_card, 6, 0);
    lv_obj_set_style_bg_color(body_card, lv_color_hex(0x101820), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(body_card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(body_card, lv_color_hex(0x4C6684), LV_PART_MAIN);
    lv_obj_set_style_text_color(body_card, lv_color_white(), LV_PART_MAIN);

    g_ui.detail_body = lv_label_create(body_card);
    lv_obj_set_width(g_ui.detail_body, lv_pct(100));
    lv_obj_set_style_text_font(g_ui.detail_body, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.detail_body, lv_color_white(), 0);
    lv_label_set_long_mode(g_ui.detail_body, LV_LABEL_LONG_WRAP);
    lv_label_set_text(g_ui.detail_body, "请先生成命盘。");
}

static void create_selector_overlay(void)
{
    lv_obj_t *panel;
    lv_obj_t *title_row;
    lv_obj_t *close_btn;
    lv_obj_t *close_label;
    lv_obj_t *value_row;
    lv_obj_t *minus_btn;
    lv_obj_t *minus_label;
    lv_obj_t *plus_btn;
    lv_obj_t *plus_label;
    lv_obj_t *action_row;
    lv_obj_t *apply_btn;
    lv_obj_t *apply_label;

    g_ui.selector_overlay = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(g_ui.selector_overlay);
    lv_obj_set_size(g_ui.selector_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_ui.selector_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_ui.selector_overlay, LV_OPA_50, 0);
    lv_obj_add_flag(g_ui.selector_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_ui.selector_overlay, LV_OBJ_FLAG_CLICKABLE);

    panel = lv_obj_create(g_ui.selector_overlay);
    lv_obj_set_size(panel, 192, 146);
    lv_obj_center(panel);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 6, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x18222E), 0);
    lv_obj_set_style_text_color(panel, lv_color_white(), 0);

    title_row = lv_obj_create(panel);
    lv_obj_remove_style_all(title_row);
    lv_obj_set_width(title_row, lv_pct(100));
    lv_obj_set_height(title_row, LV_SIZE_CONTENT);
    lv_obj_set_layout(title_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(title_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(title_row, 4, 0);

    g_ui.selector_title = lv_label_create(title_row);
    lv_label_set_text(g_ui.selector_title, "选择");
    lv_obj_set_style_text_font(g_ui.selector_title, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.selector_title, lv_color_white(), 0);
    lv_obj_set_width(g_ui.selector_title, 136);

    close_btn = lv_btn_create(title_row);
    lv_obj_set_size(close_btn, 32, 28);
    style_input_button(close_btn);
    lv_obj_add_event_cb(close_btn, selector_close_event, LV_EVENT_CLICKED, NULL);
    close_label = lv_label_create(close_btn);
    lv_label_set_text(close_label, "关");
    lv_obj_set_style_text_font(close_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(close_label, lv_color_white(), 0);
    lv_obj_center(close_label);

    g_ui.selector_info = lv_label_create(panel);
    lv_obj_set_width(g_ui.selector_info, lv_pct(100));
    lv_obj_set_style_text_font(g_ui.selector_info, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.selector_info, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_align(g_ui.selector_info, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(g_ui.selector_info, "");

    value_row = lv_obj_create(panel);
    lv_obj_remove_style_all(value_row);
    lv_obj_set_width(value_row, lv_pct(100));
    lv_obj_set_height(value_row, 34);
    lv_obj_set_layout(value_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(value_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(value_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    minus_btn = lv_btn_create(value_row);
    lv_obj_set_size(minus_btn, 32, 28);
    style_input_button(minus_btn);
    lv_obj_add_event_cb(minus_btn, selector_adjust_event, LV_EVENT_CLICKED, (void *)(intptr_t)-1);
    minus_label = lv_label_create(minus_btn);
    lv_label_set_text(minus_label, "-");
    lv_obj_set_style_text_font(minus_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(minus_label, lv_color_white(), 0);
    lv_obj_center(minus_label);

    g_ui.selector_value = lv_label_create(value_row);
    lv_obj_set_width(g_ui.selector_value, 100);
    lv_obj_set_style_text_font(g_ui.selector_value, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.selector_value, lv_color_white(), 0);
    lv_obj_set_style_text_align(g_ui.selector_value, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(g_ui.selector_value, "");

    plus_btn = lv_btn_create(value_row);
    lv_obj_set_size(plus_btn, 32, 28);
    style_input_button(plus_btn);
    lv_obj_add_event_cb(plus_btn, selector_adjust_event, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    plus_label = lv_label_create(plus_btn);
    lv_label_set_text(plus_label, "+");
    lv_obj_set_style_text_font(plus_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(plus_label, lv_color_white(), 0);
    lv_obj_center(plus_label);

    action_row = lv_obj_create(panel);
    lv_obj_remove_style_all(action_row);
    lv_obj_set_width(action_row, lv_pct(100));
    lv_obj_set_height(action_row, 30);
    lv_obj_set_layout(action_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(action_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    apply_btn = lv_btn_create(action_row);
    lv_obj_set_size(apply_btn, 64, 28);
    style_input_button(apply_btn);
    lv_obj_set_style_bg_color(apply_btn, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_add_event_cb(apply_btn, selector_apply_event, LV_EVENT_CLICKED, NULL);
    apply_label = lv_label_create(apply_btn);
    lv_label_set_text(apply_label, "应用");
    lv_obj_set_style_text_font(apply_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(apply_label, lv_color_white(), 0);
    lv_obj_center(apply_label);
}

static void create_chart_tab(lv_obj_t *parent)
{
    static const lv_coord_t col_dsc[] = {40, 40, 40, 40, LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t row_dsc[] = {40, 40, 40, 40, LV_GRID_TEMPLATE_LAST};
    static const uint8_t ring_positions[ZIWEI_PALACE_COUNT][2] = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0},
        {3, 1}, {3, 2}, {3, 3}, {2, 3},
        {1, 3}, {0, 3}, {0, 2}, {0, 1}
    };

    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_t *grid;
    lv_obj_t *center_panel;
    lv_obj_t *mode_row;
    lv_obj_t *quick_label;
    lv_obj_t *mode_label;
    static const char *mode_names[5] = {"命", "限", "年", "月", "日"};
    int i;

    lv_obj_set_size(page, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_left(page, 2, 0);
    lv_obj_set_style_pad_right(page, 2, 0);
    lv_obj_set_style_pad_top(page, 6, 0);
    lv_obj_set_style_pad_bottom(page, 4, 0);
    lv_obj_set_layout(page, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(page, 4, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101820), 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_text_color(page, lv_color_white(), 0);

    g_ui.summary_label = lv_label_create(page);
    lv_obj_set_width(g_ui.summary_label, lv_pct(100));
    lv_obj_set_style_text_font(g_ui.summary_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.summary_label, lv_color_white(), 0);
    lv_obj_set_style_text_line_space(g_ui.summary_label, -2, 0);
    lv_label_set_long_mode(g_ui.summary_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(g_ui.summary_label, "尚未生成命盘。");

    grid = lv_obj_create(page);
    lv_obj_set_size(grid, 168, 168);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(grid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(grid, 2, 0);
    lv_obj_set_style_pad_row(grid, 2, 0);
    lv_obj_set_style_pad_column(grid, 2, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);

    for (i = 0; i < ZIWEI_PALACE_COUNT; ++i) {
        g_ui.palace_btn[i] = lv_btn_create(grid);
        lv_obj_set_grid_cell(g_ui.palace_btn[i],
                             LV_GRID_ALIGN_STRETCH, ring_positions[i][0], 1,
                             LV_GRID_ALIGN_STRETCH, ring_positions[i][1], 1);
        lv_obj_set_style_radius(g_ui.palace_btn[i], 8, LV_PART_MAIN);
        lv_obj_set_style_pad_all(g_ui.palace_btn[i], 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(g_ui.palace_btn[i], lv_color_hex(0x5A748F), LV_PART_MAIN);
        lv_obj_set_style_border_width(g_ui.palace_btn[i], 1, LV_PART_MAIN);
        lv_obj_set_style_text_color(g_ui.palace_btn[i], lv_color_white(), LV_PART_MAIN);
        lv_obj_add_event_cb(g_ui.palace_btn[i], palace_click_event, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        g_ui.palace_label[i] = lv_label_create(g_ui.palace_btn[i]);
        lv_obj_set_width(g_ui.palace_label[i], lv_pct(100));
        lv_label_set_long_mode(g_ui.palace_label[i], LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(g_ui.palace_label[i], ZIWEI_FONT_TEXT, 0);
        lv_obj_set_style_text_color(g_ui.palace_label[i], lv_color_white(), 0);
        lv_obj_set_style_text_line_space(g_ui.palace_label[i], -3, 0);
        lv_obj_set_style_text_align(g_ui.palace_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(g_ui.palace_label[i], "--");
        lv_obj_center(g_ui.palace_label[i]);
    }

    center_panel = lv_btn_create(grid);
    g_ui.center_panel = center_panel;
    lv_obj_clear_flag(center_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(center_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_grid_cell(center_panel, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_STRETCH, 1, 2);
    lv_obj_set_style_radius(center_panel, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(center_panel, lv_color_hex(0x16202A), LV_PART_MAIN);
    lv_obj_set_style_border_color(center_panel, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_MAIN);
    lv_obj_set_style_border_width(center_panel, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(center_panel, 3, 0);
    lv_obj_set_style_text_color(center_panel, lv_color_white(), 0);
    lv_obj_add_event_cb(center_panel, center_click_event, LV_EVENT_CLICKED, NULL);

    g_ui.center_label = lv_label_create(center_panel);
    lv_obj_set_style_text_font(g_ui.center_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(g_ui.center_label, lv_color_white(), 0);
    lv_obj_set_style_text_line_space(g_ui.center_label, -2, 0);
    lv_obj_set_style_text_align(g_ui.center_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(g_ui.center_label, "命盘\n未生成");
    lv_obj_center(g_ui.center_label);

    mode_row = lv_obj_create(page);
    lv_obj_remove_style_all(mode_row);
    lv_obj_set_width(mode_row, lv_pct(100));
    lv_obj_set_height(mode_row, 28);
    lv_obj_clear_flag(mode_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(mode_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_layout(mode_row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mode_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(mode_row, 4, 0);
    lv_obj_set_style_pad_right(mode_row, 4, 0);
    lv_obj_set_style_pad_column(mode_row, 2, 0);

    g_ui.quick_btn_prev = lv_btn_create(mode_row);
    lv_obj_set_size(g_ui.quick_btn_prev, 24, 24);
    style_input_button(g_ui.quick_btn_prev);
    lv_obj_add_event_cb(g_ui.quick_btn_prev, quick_switch_event, LV_EVENT_CLICKED, (void *)(intptr_t)-1);
    quick_label = lv_label_create(g_ui.quick_btn_prev);
    lv_label_set_text(quick_label, "<");
    lv_obj_set_style_text_font(quick_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(quick_label, lv_color_white(), 0);
    lv_obj_center(quick_label);

    for (i = 0; i < 5; ++i) {
        g_ui.mode_btn[i] = lv_btn_create(mode_row);
        lv_obj_set_size(g_ui.mode_btn[i], 30, 24);
        style_input_button(g_ui.mode_btn[i]);
        lv_obj_add_event_cb(g_ui.mode_btn[i], view_mode_event, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        mode_label = lv_label_create(g_ui.mode_btn[i]);
        lv_label_set_text(mode_label, mode_names[i]);
        lv_obj_set_style_text_font(mode_label, ZIWEI_FONT_TEXT, 0);
        lv_obj_set_style_text_color(mode_label, lv_color_white(), 0);
        lv_obj_center(mode_label);
    }

    g_ui.quick_btn_next = lv_btn_create(mode_row);
    lv_obj_set_size(g_ui.quick_btn_next, 24, 24);
    style_input_button(g_ui.quick_btn_next);
    lv_obj_add_event_cb(g_ui.quick_btn_next, quick_switch_event, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    quick_label = lv_label_create(g_ui.quick_btn_next);
    lv_label_set_text(quick_label, ">");
    lv_obj_set_style_text_font(quick_label, ZIWEI_FONT_TEXT, 0);
    lv_obj_set_style_text_color(quick_label, lv_color_white(), 0);
    lv_obj_center(quick_label);

    g_ui.quick_row = NULL;
    refresh_mode_buttons();
    refresh_quick_switch_buttons();

    create_selector_overlay();
    create_detail_overlay();
}

void Ziwei_App_Init(void)
{
    lv_obj_t *tab_input;
    lv_obj_t *tab_chart;
    lv_obj_t *tab_btns;
    lv_obj_t *tab_content;

    ESP_LOGI(TAG_ZIWEI_UI, "Ziwei_App_Init enter");
    memset(&g_ui, 0, sizeof(g_ui));
    g_ui.selected_palace = 0;
    g_ui.selected_gender = ZIWEI_GENDER_MALE;
    g_ui.flow_year = 2000;
    g_ui.flow_month = 6;
    g_ui.flow_day = 23;

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0B1218), 0);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_white(), 0);

    g_ui.tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 30);
    lv_obj_set_style_bg_color(g_ui.tabview, lv_color_hex(0x0B1218), LV_PART_MAIN);
    tab_btns = lv_tabview_get_tab_btns(g_ui.tabview);
    tab_content = lv_tabview_get_content(g_ui.tabview);
    lv_obj_set_style_text_font(tab_btns, ZIWEI_FONT_TEXT, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(tab_btns, ZIWEI_FONT_TEXT, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, lv_color_white(), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(tab_btns, lv_color_white(), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x1B2836), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_main(LV_PALETTE_BLUE_GREY), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(tab_btns, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(tab_btns, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(tab_content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(tab_content, LV_DIR_NONE);
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    tab_input = lv_tabview_add_tab(g_ui.tabview, "时间");
    tab_chart = lv_tabview_add_tab(g_ui.tabview, "命盘");

    create_input_tab(tab_input);
    create_chart_tab(tab_chart);

    refresh_chart_content();
    ESP_LOGI(TAG_ZIWEI_UI, "Ziwei_App_Init complete");
}
