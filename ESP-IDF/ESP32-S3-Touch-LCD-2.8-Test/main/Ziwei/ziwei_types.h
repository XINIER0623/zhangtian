#pragma once

#include <stdbool.h>
#include <stdint.h>

#define ZIWEI_PALACE_COUNT 12
#define ZIWEI_MAJOR_STAR_COUNT 14
#define ZIWEI_MINOR_STAR_COUNT 14
#define ZIWEI_TRANSFORM_COUNT 4

typedef enum {
    ZIWEI_BRANCH_ZI = 0,
    ZIWEI_BRANCH_CHOU,
    ZIWEI_BRANCH_YIN,
    ZIWEI_BRANCH_MAO,
    ZIWEI_BRANCH_CHEN,
    ZIWEI_BRANCH_SI,
    ZIWEI_BRANCH_WU,
    ZIWEI_BRANCH_WEI,
    ZIWEI_BRANCH_SHEN,
    ZIWEI_BRANCH_YOU,
    ZIWEI_BRANCH_XU,
    ZIWEI_BRANCH_HAI
} ziwei_branch_t;

typedef enum {
    ZIWEI_STEM_JIA = 0,
    ZIWEI_STEM_YI,
    ZIWEI_STEM_BING,
    ZIWEI_STEM_DING,
    ZIWEI_STEM_WU,
    ZIWEI_STEM_JI,
    ZIWEI_STEM_GENG,
    ZIWEI_STEM_XIN,
    ZIWEI_STEM_REN,
    ZIWEI_STEM_GUI
} ziwei_stem_t;

typedef enum {
    ZIWEI_GENDER_FEMALE = 0,
    ZIWEI_GENDER_MALE = 1
} ziwei_gender_t;

typedef enum {
    ZIWEI_PALACE_ROLE_MING = 0,
    ZIWEI_PALACE_ROLE_BROTHERS,
    ZIWEI_PALACE_ROLE_SPOUSE,
    ZIWEI_PALACE_ROLE_CHILDREN,
    ZIWEI_PALACE_ROLE_WEALTH,
    ZIWEI_PALACE_ROLE_HEALTH,
    ZIWEI_PALACE_ROLE_TRAVEL,
    ZIWEI_PALACE_ROLE_FRIENDS,
    ZIWEI_PALACE_ROLE_CAREER,
    ZIWEI_PALACE_ROLE_ESTATE,
    ZIWEI_PALACE_ROLE_FORTUNE,
    ZIWEI_PALACE_ROLE_PARENTS
} ziwei_palace_role_t;

typedef enum {
    ZIWEI_STAR_REF_MAJOR = 0,
    ZIWEI_STAR_REF_MINOR = 1
} ziwei_star_ref_family_t;

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    ziwei_gender_t gender;
} ziwei_input_t;

typedef struct {
    bool valid;
    char error[64];
    int lunar_year;
    int lunar_month;
    int lunar_day;
    bool lunar_is_leap;
    int year_stem;
    int year_branch;
    int hour_branch;
} ziwei_calendar_result_t;

typedef struct {
    uint8_t family;
    uint8_t id;
} ziwei_star_ref_t;

typedef struct {
    bool valid;
    char error[64];
    ziwei_input_t input;
    ziwei_calendar_result_t calendar;
    ziwei_calendar_result_t flow_calendar;
    int ming_palace;
    int shen_palace;
    int palace_role_by_index[ZIWEI_PALACE_COUNT];
    int ming_palace_stem;
    int bureau_result;
    int bureau_number;
    const char *bureau_name;
    int major_star_palace[ZIWEI_MAJOR_STAR_COUNT];
    int minor_star_palace[ZIWEI_MINOR_STAR_COUNT];
    ziwei_star_ref_t natal_transforms[ZIWEI_TRANSFORM_COUNT];
    bool decade_forward;
    int decade_start_age;
    int decade_age_start_by_palace[ZIWEI_PALACE_COUNT];
    int decade_age_end_by_palace[ZIWEI_PALACE_COUNT];
    int flow_year_palace;
    int flow_month_base_palace;
    int flow_month_palace;
    int flow_day_palace;
} ziwei_chart_result_t;
