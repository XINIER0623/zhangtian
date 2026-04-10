#include "ziwei_calendar.h"

#include <stdio.h>
#include <string.h>

#include "ziwei_lunar_years.h"

static bool is_gregorian_leap_year(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static int days_in_gregorian_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && is_gregorian_leap_year(year)) {
        return 29;
    }

    return days[month - 1];
}

static int64_t days_from_civil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = (unsigned)(year - era * 400);
    const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097LL + (int64_t)doe - 719468LL;
}

static const ziwei_lunar_year_info_t *find_lunar_year_info(int lunar_year)
{
    if (lunar_year < 1901 || lunar_year > 2100) {
        return NULL;
    }

    return &ZIWEI_LUNAR_YEAR_TABLE[lunar_year - 1901];
}

bool ziwei_is_valid_solar_date(int year, int month, int day)
{
    if (year < 1901 || year > 2100) {
        return false;
    }

    const int max_day = days_in_gregorian_month(year, month);
    return max_day > 0 && day >= 1 && day <= max_day;
}

int ziwei_hour_to_branch(int hour)
{
    if (hour < 0 || hour > 23) {
        return -1;
    }

    if (hour == 23 || hour == 0) {
        return ZIWEI_BRANCH_ZI;
    }

    return (hour + 1) / 2;
}

bool ziwei_calendar_convert(const ziwei_input_t *input, ziwei_calendar_result_t *out)
{
    int lunar_year;
    int seq;
    int offset;
    int date_days;
    int new_year_days;
    const ziwei_lunar_year_info_t *info;

    if (out == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));

    if (input == NULL) {
        snprintf(out->error, sizeof(out->error), "缺少输入参数");
        return false;
    }

    if (!ziwei_is_valid_solar_date(input->year, input->month, input->day)) {
        snprintf(out->error, sizeof(out->error), "公历日期无效");
        return false;
    }

    if (input->minute < 0 || input->minute > 59 || (input->minute % 5) != 0) {
        snprintf(out->error, sizeof(out->error), "分钟只支持 5 分钟步进");
        return false;
    }

    out->hour_branch = ziwei_hour_to_branch(input->hour);
    if (out->hour_branch < 0) {
        snprintf(out->error, sizeof(out->error), "小时必须在 0 到 23 之间");
        return false;
    }

    date_days = (int)days_from_civil(input->year, (unsigned)input->month, (unsigned)input->day);

    lunar_year = input->year;
    info = find_lunar_year_info(lunar_year);
    if (info == NULL) {
        snprintf(out->error, sizeof(out->error), "当前只支持 1901 到 2100 年");
        return false;
    }

    new_year_days = (int)days_from_civil(lunar_year,
                                         (unsigned)info->new_year_month,
                                         (unsigned)info->new_year_day);
    if (date_days < new_year_days) {
        lunar_year--;
        info = find_lunar_year_info(lunar_year);
        if (info == NULL) {
            snprintf(out->error, sizeof(out->error), "日期超出当前农历换算范围");
            return false;
        }
        new_year_days = (int)days_from_civil(lunar_year,
                                             (unsigned)info->new_year_month,
                                             (unsigned)info->new_year_day);
    }

    offset = date_days - new_year_days;
    if (offset < 0) {
        snprintf(out->error, sizeof(out->error), "农历换算失败");
        return false;
    }

    for (seq = 0; seq < info->month_count; ++seq) {
        const int month_days = 29 + ((info->month_bits >> seq) & 0x1);
        if (offset < month_days) {
            if (info->leap_month > 0) {
                if (seq < info->leap_month) {
                    out->lunar_month = seq + 1;
                    out->lunar_is_leap = false;
                } else if (seq == info->leap_month) {
                    out->lunar_month = info->leap_month;
                    out->lunar_is_leap = true;
                } else {
                    out->lunar_month = seq;
                    out->lunar_is_leap = false;
                }
            } else {
                out->lunar_month = seq + 1;
                out->lunar_is_leap = false;
            }
            out->lunar_day = offset + 1;
            break;
        }
        offset -= month_days;
    }

    if (seq >= info->month_count) {
        snprintf(out->error, sizeof(out->error), "农历月份计算溢出");
        return false;
    }

    out->lunar_year = lunar_year;
    out->year_stem = (lunar_year - 4) % 10;
    out->year_branch = (lunar_year - 4) % 12;
    out->valid = true;
    out->error[0] = '\0';

    return true;
}
