#include "ziwei_chart.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ziwei_calendar.h"

enum {
    ZIWEI_MAJOR_ZIWEI = 0,
    ZIWEI_MAJOR_TIANJI,
    ZIWEI_MAJOR_TAIYANG,
    ZIWEI_MAJOR_WUQU,
    ZIWEI_MAJOR_TIANTONG,
    ZIWEI_MAJOR_LIANZHEN,
    ZIWEI_MAJOR_TIANFU,
    ZIWEI_MAJOR_TAIYIN,
    ZIWEI_MAJOR_TANLANG,
    ZIWEI_MAJOR_JUMEN,
    ZIWEI_MAJOR_TIANXIANG,
    ZIWEI_MAJOR_TIANLIANG,
    ZIWEI_MAJOR_QISHA,
    ZIWEI_MAJOR_POJUN
};

enum {
    ZIWEI_MINOR_ZUOFU = 0,
    ZIWEI_MINOR_YOUBI,
    ZIWEI_MINOR_WENCHANG,
    ZIWEI_MINOR_WENQU,
    ZIWEI_MINOR_TIANKUI,
    ZIWEI_MINOR_TIANYUE,
    ZIWEI_MINOR_LUCUN,
    ZIWEI_MINOR_QINGYANG,
    ZIWEI_MINOR_TUOLUO,
    ZIWEI_MINOR_TIANMA,
    ZIWEI_MINOR_DIKONG,
    ZIWEI_MINOR_DIJIE,
    ZIWEI_MINOR_HUOXING,
    ZIWEI_MINOR_LINGXING
};

static const char *BRANCH_NAMES[ZIWEI_PALACE_COUNT] = {
    "子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"
};

static const char *STEM_NAMES[10] = {
    "甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"
};

static const char *PALACE_ROLE_NAMES[ZIWEI_PALACE_COUNT] = {
    "命宫", "兄弟", "夫妻", "子女", "财帛", "疾厄",
    "迁移", "交友", "官禄", "田宅", "福德", "父母"
};

static const char *PALACE_ROLE_SHORT_NAMES[ZIWEI_PALACE_COUNT] = {
    "命", "兄", "夫", "子", "财", "疾",
    "迁", "友", "官", "田", "福", "父"
};

static const char *MAJOR_STAR_NAMES[ZIWEI_MAJOR_STAR_COUNT] = {
    "紫微", "天机", "太阳", "武曲", "天同", "廉贞",
    "天府", "太阴", "贪狼", "巨门", "天相", "天梁",
    "七杀", "破军"
};

static const char *MAJOR_STAR_SHORT_NAMES[ZIWEI_MAJOR_STAR_COUNT] = {
    "紫微", "天机", "太阳", "武曲", "天同", "廉贞",
    "天府", "太阴", "贪狼", "巨门", "天相", "天梁",
    "七杀", "破军"
};

static const char *MINOR_STAR_NAMES[ZIWEI_MINOR_STAR_COUNT] = {
    "左辅", "右弼", "文昌", "文曲", "天魁", "天钺",
    "禄存", "擎羊", "陀罗", "天马", "地空", "地劫",
    "火星", "铃星"
};

static const char *TRANSFORM_NAMES[ZIWEI_TRANSFORM_COUNT] = {
    "禄", "权", "科", "忌"
};

static const int ZIWEI_GROUP_OFFSETS[] = {0, -1, -3, -4, -5, -8};
static const int TIANFU_GROUP_OFFSETS[] = {0, 1, 2, 3, 4, 5, 6, 10};
static const int TIANFU_MAPPING[ZIWEI_PALACE_COUNT] = {4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5};

#define ZIWEI_EXTRA_STAR_COUNT 39
#define ZIWEI_GROUP_STAR_COUNT 12

enum {
    ZIWEI_EXTRA_HONGLUAN = 0,
    ZIWEI_EXTRA_TIANXI,
    ZIWEI_EXTRA_TIANYAO,
    ZIWEI_EXTRA_XIANCHI,
    ZIWEI_EXTRA_JIESHEN,
    ZIWEI_EXTRA_SANTAI,
    ZIWEI_EXTRA_BAZUO,
    ZIWEI_EXTRA_ENGGUANG,
    ZIWEI_EXTRA_TIANGUI,
    ZIWEI_EXTRA_LONGCHI,
    ZIWEI_EXTRA_FENGGE,
    ZIWEI_EXTRA_TIANCAI,
    ZIWEI_EXTRA_TIANSHOU,
    ZIWEI_EXTRA_TAIFU,
    ZIWEI_EXTRA_FENGGAO,
    ZIWEI_EXTRA_TIANWU,
    ZIWEI_EXTRA_HUAGAI,
    ZIWEI_EXTRA_TIANGUAN,
    ZIWEI_EXTRA_TIANFU_ADJ,
    ZIWEI_EXTRA_TIANCHU,
    ZIWEI_EXTRA_TIANYUE,
    ZIWEI_EXTRA_TIANDE,
    ZIWEI_EXTRA_YUEDE,
    ZIWEI_EXTRA_TIANKONG,
    ZIWEI_EXTRA_XUNKONG,
    ZIWEI_EXTRA_JIELU,
    ZIWEI_EXTRA_KONGWANG,
    ZIWEI_EXTRA_GUCHEN,
    ZIWEI_EXTRA_GUASU,
    ZIWEI_EXTRA_FEILIAN,
    ZIWEI_EXTRA_POSUI,
    ZIWEI_EXTRA_TIANXING,
    ZIWEI_EXTRA_YINSHA,
    ZIWEI_EXTRA_TIANKU,
    ZIWEI_EXTRA_TIANXU,
    ZIWEI_EXTRA_TIANSHI,
    ZIWEI_EXTRA_TIANSHANG,
    ZIWEI_EXTRA_NIANJIE,
    ZIWEI_EXTRA_DAHAO_ADJ
};

static const char *EXTRA_STAR_NAMES[ZIWEI_EXTRA_STAR_COUNT] = {
    "红鸾", "天喜", "天姚", "咸池", "解神", "三台", "八座", "恩光", "天贵",
    "龙池", "凤阁", "天才", "天寿", "台辅", "封诰", "天巫", "华盖", "天官",
    "天福", "天厨", "天月", "天德", "月德", "天空", "旬空", "截路", "空亡",
    "孤辰", "寡宿", "蜚廉", "破碎", "天刑", "阴煞", "天哭", "天虚", "天使",
    "天伤", "年解", "大耗"
};

static const char *CHANGSHENG_NAMES[ZIWEI_GROUP_STAR_COUNT] = {
    "长生", "沐浴", "冠带", "临官", "帝旺", "衰",
    "病", "死", "墓", "绝", "胎", "养"
};

static const char *BOSHI_NAMES[ZIWEI_GROUP_STAR_COUNT] = {
    "博士", "力士", "青龙", "小耗", "将军", "奏书",
    "飞廉", "喜神", "病符", "大耗", "伏兵", "官府"
};

static const char *SUIQIAN_NAMES[ZIWEI_GROUP_STAR_COUNT] = {
    "岁建", "晦气", "丧门", "贯索", "官符", "小耗",
    "大耗", "龙德", "白虎", "天德", "吊客", "病符"
};

static const char *JIANGQIAN_NAMES[ZIWEI_GROUP_STAR_COUNT] = {
    "将星", "攀鞍", "岁驿", "息神", "华盖", "劫煞",
    "灾煞", "天煞", "指背", "咸池", "月煞", "亡神"
};

static const ziwei_star_ref_t NATAL_TRANSFORM_TABLE[10][ZIWEI_TRANSFORM_COUNT] = {
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_LIANZHEN}, {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_POJUN},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_WUQU},     {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYANG}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANJI},   {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANLIANG}, {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_ZIWEI},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYIN}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANTONG}, {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANJI},    {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_WENCHANG},  {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_LIANZHEN}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYIN},   {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANTONG},  {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANJI},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_JUMEN}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TANLANG},  {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYIN},    {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_YOUBI},     {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANJI}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_WUQU},     {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TANLANG},   {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANLIANG}, {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_WENQU}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYANG},  {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_WUQU},      {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYIN},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANTONG}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_JUMEN},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYANG},   {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_WENQU},     {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_WENCHANG}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TIANLIANG},{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_ZIWEI},     {ZIWEI_STAR_REF_MINOR, ZIWEI_MINOR_ZUOFU},     {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_WUQU}},
    {{ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_POJUN},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_JUMEN},     {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TAIYIN},    {ZIWEI_STAR_REF_MAJOR, ZIWEI_MAJOR_TANLANG}}
};

static int wrap_palace(int palace)
{
    while (palace < 0) {
        palace += ZIWEI_PALACE_COUNT;
    }
    return palace % ZIWEI_PALACE_COUNT;
}

static int branch_group_number(int branch)
{
    switch (wrap_palace(branch)) {
    case ZIWEI_BRANCH_ZI:
    case ZIWEI_BRANCH_WU:
    case ZIWEI_BRANCH_CHOU:
    case ZIWEI_BRANCH_WEI:
        return 1;
    case ZIWEI_BRANCH_YIN:
    case ZIWEI_BRANCH_SHEN:
    case ZIWEI_BRANCH_MAO:
    case ZIWEI_BRANCH_YOU:
        return 2;
    default:
        return 3;
    }
}

static int stem_group_number(int stem)
{
    switch (stem % 10) {
    case ZIWEI_STEM_JIA:
    case ZIWEI_STEM_YI:
        return 1;
    case ZIWEI_STEM_BING:
    case ZIWEI_STEM_DING:
        return 2;
    case ZIWEI_STEM_WU:
    case ZIWEI_STEM_JI:
        return 3;
    case ZIWEI_STEM_GENG:
    case ZIWEI_STEM_XIN:
        return 4;
    default:
        return 5;
    }
}

static bool stem_is_yang(int stem)
{
    return (stem % 2) == 0;
}

static bool branch_is_yang(int branch)
{
    return (wrap_palace(branch) % 2) == 0;
}

static bool chart_is_forward_cycle(const ziwei_chart_result_t *chart)
{
    return (chart->input.gender == ZIWEI_GENDER_MALE && branch_is_yang(chart->calendar.year_branch)) ||
           (chart->input.gender == ZIWEI_GENDER_FEMALE && !branch_is_yang(chart->calendar.year_branch));
}

static int calc_yin_palace_stem(int year_stem)
{
    switch (year_stem) {
    case ZIWEI_STEM_JIA:
    case ZIWEI_STEM_JI:
        return ZIWEI_STEM_BING;
    case ZIWEI_STEM_YI:
    case ZIWEI_STEM_GENG:
        return ZIWEI_STEM_WU;
    case ZIWEI_STEM_BING:
    case ZIWEI_STEM_XIN:
        return ZIWEI_STEM_GENG;
    case ZIWEI_STEM_DING:
    case ZIWEI_STEM_REN:
        return ZIWEI_STEM_REN;
    default:
        return ZIWEI_STEM_JIA;
    }
}

static void append_text(char *buffer, size_t buffer_size, const char *fmt, ...)
{
    va_list args;
    size_t used;
    int written;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    used = strlen(buffer);
    if (used >= buffer_size - 1) {
        return;
    }

    va_start(args, fmt);
    written = vsnprintf(buffer + used, buffer_size - used, fmt, args);
    va_end(args);

    if (written < 0) {
        buffer[used] = '\0';
    }
}

static void init_chart_arrays(ziwei_chart_result_t *out)
{
    int i;

    for (i = 0; i < ZIWEI_PALACE_COUNT; ++i) {
        out->palace_role_by_index[i] = -1;
        out->decade_age_start_by_palace[i] = 0;
        out->decade_age_end_by_palace[i] = 0;
    }

    for (i = 0; i < ZIWEI_MAJOR_STAR_COUNT; ++i) {
        out->major_star_palace[i] = -1;
    }

    for (i = 0; i < ZIWEI_MINOR_STAR_COUNT; ++i) {
        out->minor_star_palace[i] = -1;
    }
}

static void assign_palace_roles(ziwei_chart_result_t *out)
{
    int role;

    for (role = 0; role < ZIWEI_PALACE_COUNT; ++role) {
        out->palace_role_by_index[wrap_palace(out->ming_palace + role)] = role;
    }
}

static void calc_bureau(ziwei_chart_result_t *out)
{
    const int yin_stem = calc_yin_palace_stem(out->calendar.year_stem);
    const int distance_from_yin = wrap_palace(out->ming_palace - ZIWEI_BRANCH_YIN);
    const int stem_group = stem_group_number((yin_stem + distance_from_yin) % 10);
    const int branch_group = branch_group_number(out->ming_palace);
    int result = stem_group + branch_group;

    while (result > 5) {
        result -= 5;
    }

    out->ming_palace_stem = (yin_stem + distance_from_yin) % 10;
    out->bureau_result = result;

    switch (result) {
    case 1:
        out->bureau_number = 3;
        out->bureau_name = "木三局";
        break;
    case 2:
        out->bureau_number = 4;
        out->bureau_name = "金四局";
        break;
    case 3:
        out->bureau_number = 2;
        out->bureau_name = "水二局";
        break;
    case 4:
        out->bureau_number = 6;
        out->bureau_name = "火六局";
        break;
    default:
        out->bureau_number = 5;
        out->bureau_name = "土五局";
        break;
    }
}

static int calc_ziwei_palace(const ziwei_chart_result_t *chart)
{
    int n = 0;
    int q;
    int base_palace;

    while (((chart->calendar.lunar_day + n) % chart->bureau_number) != 0) {
        ++n;
    }

    q = (chart->calendar.lunar_day + n) / chart->bureau_number;
    base_palace = wrap_palace(ZIWEI_BRANCH_YIN + q - 1);

    if (n == 0) {
        return base_palace;
    }

    return wrap_palace(base_palace + ((n % 2 == 0) ? n : -n));
}

static void place_major_stars(ziwei_chart_result_t *out)
{
    int ziwei_palace = calc_ziwei_palace(out);
    int tianfu_palace = TIANFU_MAPPING[ziwei_palace];
    int i;

    for (i = 0; i < 6; ++i) {
        out->major_star_palace[i] = wrap_palace(ziwei_palace + ZIWEI_GROUP_OFFSETS[i]);
    }

    for (i = 0; i < 8; ++i) {
        out->major_star_palace[ZIWEI_MAJOR_TIANFU + i] = wrap_palace(tianfu_palace + TIANFU_GROUP_OFFSETS[i]);
    }
}

static void place_minor_stars(ziwei_chart_result_t *out)
{
    const int month = out->calendar.lunar_month;
    const int hour_branch = out->calendar.hour_branch;
    const int year_stem = out->calendar.year_stem;
    const int year_branch = out->calendar.year_branch;
    int huoxing_base;
    int lingxing_base;

    out->minor_star_palace[ZIWEI_MINOR_ZUOFU] = wrap_palace(ZIWEI_BRANCH_CHEN + month - 1);
    out->minor_star_palace[ZIWEI_MINOR_YOUBI] = wrap_palace(ZIWEI_BRANCH_XU - (month - 1));
    out->minor_star_palace[ZIWEI_MINOR_WENQU] = wrap_palace(ZIWEI_BRANCH_CHEN + hour_branch);
    out->minor_star_palace[ZIWEI_MINOR_WENCHANG] = wrap_palace(ZIWEI_BRANCH_XU - hour_branch);

    switch (year_stem) {
    case ZIWEI_STEM_JIA:
    case ZIWEI_STEM_WU:
    case ZIWEI_STEM_GENG:
        out->minor_star_palace[ZIWEI_MINOR_TIANKUI] = ZIWEI_BRANCH_CHOU;
        out->minor_star_palace[ZIWEI_MINOR_TIANYUE] = ZIWEI_BRANCH_WEI;
        break;
    case ZIWEI_STEM_YI:
    case ZIWEI_STEM_JI:
        out->minor_star_palace[ZIWEI_MINOR_TIANKUI] = ZIWEI_BRANCH_ZI;
        out->minor_star_palace[ZIWEI_MINOR_TIANYUE] = ZIWEI_BRANCH_SHEN;
        break;
    case ZIWEI_STEM_BING:
    case ZIWEI_STEM_DING:
        out->minor_star_palace[ZIWEI_MINOR_TIANKUI] = ZIWEI_BRANCH_HAI;
        out->minor_star_palace[ZIWEI_MINOR_TIANYUE] = ZIWEI_BRANCH_YOU;
        break;
    case ZIWEI_STEM_XIN:
        out->minor_star_palace[ZIWEI_MINOR_TIANKUI] = ZIWEI_BRANCH_WU;
        out->minor_star_palace[ZIWEI_MINOR_TIANYUE] = ZIWEI_BRANCH_YIN;
        break;
    default:
        out->minor_star_palace[ZIWEI_MINOR_TIANKUI] = ZIWEI_BRANCH_MAO;
        out->minor_star_palace[ZIWEI_MINOR_TIANYUE] = ZIWEI_BRANCH_SI;
        break;
    }

    switch (year_stem) {
    case ZIWEI_STEM_JIA:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_YIN;
        break;
    case ZIWEI_STEM_YI:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_MAO;
        break;
    case ZIWEI_STEM_BING:
    case ZIWEI_STEM_WU:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_SI;
        break;
    case ZIWEI_STEM_DING:
    case ZIWEI_STEM_JI:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_WU;
        break;
    case ZIWEI_STEM_GENG:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_SHEN;
        break;
    case ZIWEI_STEM_XIN:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_YOU;
        break;
    case ZIWEI_STEM_REN:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_HAI;
        break;
    default:
        out->minor_star_palace[ZIWEI_MINOR_LUCUN] = ZIWEI_BRANCH_ZI;
        break;
    }

    out->minor_star_palace[ZIWEI_MINOR_QINGYANG] = wrap_palace(out->minor_star_palace[ZIWEI_MINOR_LUCUN] + 1);
    out->minor_star_palace[ZIWEI_MINOR_TUOLUO] = wrap_palace(out->minor_star_palace[ZIWEI_MINOR_LUCUN] - 1);

    switch (year_branch) {
    case ZIWEI_BRANCH_YIN:
    case ZIWEI_BRANCH_WU:
    case ZIWEI_BRANCH_XU:
        out->minor_star_palace[ZIWEI_MINOR_TIANMA] = ZIWEI_BRANCH_SHEN;
        break;
    case ZIWEI_BRANCH_SHEN:
    case ZIWEI_BRANCH_ZI:
    case ZIWEI_BRANCH_CHEN:
        out->minor_star_palace[ZIWEI_MINOR_TIANMA] = ZIWEI_BRANCH_YIN;
        break;
    case ZIWEI_BRANCH_SI:
    case ZIWEI_BRANCH_YOU:
    case ZIWEI_BRANCH_CHOU:
        out->minor_star_palace[ZIWEI_MINOR_TIANMA] = ZIWEI_BRANCH_HAI;
        break;
    default:
        out->minor_star_palace[ZIWEI_MINOR_TIANMA] = ZIWEI_BRANCH_SI;
        break;
    }

    out->minor_star_palace[ZIWEI_MINOR_DIJIE] = wrap_palace(ZIWEI_BRANCH_HAI + hour_branch);
    out->minor_star_palace[ZIWEI_MINOR_DIKONG] = wrap_palace(ZIWEI_BRANCH_HAI - hour_branch);

    switch (year_branch) {
    case ZIWEI_BRANCH_SHEN:
    case ZIWEI_BRANCH_ZI:
    case ZIWEI_BRANCH_CHEN:
        huoxing_base = ZIWEI_BRANCH_YIN;
        lingxing_base = ZIWEI_BRANCH_XU;
        break;
    case ZIWEI_BRANCH_YIN:
    case ZIWEI_BRANCH_WU:
    case ZIWEI_BRANCH_XU:
        huoxing_base = ZIWEI_BRANCH_CHOU;
        lingxing_base = ZIWEI_BRANCH_MAO;
        break;
    case ZIWEI_BRANCH_SI:
    case ZIWEI_BRANCH_YOU:
    case ZIWEI_BRANCH_CHOU:
        huoxing_base = ZIWEI_BRANCH_MAO;
        lingxing_base = ZIWEI_BRANCH_XU;
        break;
    default:
        huoxing_base = ZIWEI_BRANCH_YOU;
        lingxing_base = ZIWEI_BRANCH_XU;
        break;
    }

    out->minor_star_palace[ZIWEI_MINOR_HUOXING] = wrap_palace(huoxing_base + hour_branch);
    out->minor_star_palace[ZIWEI_MINOR_LINGXING] = wrap_palace(lingxing_base + hour_branch);
}

static void place_transforms(ziwei_chart_result_t *out)
{
    memcpy(out->natal_transforms,
           NATAL_TRANSFORM_TABLE[out->calendar.year_stem],
           sizeof(out->natal_transforms));
}

static void init_positions(int *positions, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        positions[i] = -1;
    }
}

static void build_extra_star_positions(const ziwei_chart_result_t *chart, int positions[ZIWEI_EXTRA_STAR_COUNT])
{
    static const int tianchu_by_stem[10] = {
        ZIWEI_BRANCH_SI, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_ZI, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_WU,
        ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_HAI
    };
    static const int tianguan_by_stem[10] = {
        ZIWEI_BRANCH_WEI, ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_MAO,
        ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_HAI, ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_XU, ZIWEI_BRANCH_WU
    };
    static const int tianfu_adj_by_stem[10] = {
        ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_ZI, ZIWEI_BRANCH_HAI, ZIWEI_BRANCH_MAO,
        ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_SI
    };
    static const int yuejie_by_month_group[6] = {
        ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_XU, ZIWEI_BRANCH_ZI,
        ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_WU
    };
    static const int yinsha_by_month_mod[6] = {
        ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_ZI, ZIWEI_BRANCH_XU,
        ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_CHEN
    };
    static const int tianyue_by_month[12] = {
        ZIWEI_BRANCH_XU, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_YIN,
        ZIWEI_BRANCH_WEI, ZIWEI_BRANCH_MAO, ZIWEI_BRANCH_HAI, ZIWEI_BRANCH_WEI,
        ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_XU, ZIWEI_BRANCH_YIN
    };
    static const int tianwu_by_month_mod[4] = {
        ZIWEI_BRANCH_SI, ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_HAI
    };
    static const int posui_by_year_mod[3] = {
        ZIWEI_BRANCH_SI, ZIWEI_BRANCH_CHOU, ZIWEI_BRANCH_YOU
    };
    static const int feilian_by_year[12] = {
        ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_XU, ZIWEI_BRANCH_SI,
        ZIWEI_BRANCH_WU, ZIWEI_BRANCH_WEI, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_MAO,
        ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_HAI, ZIWEI_BRANCH_ZI, ZIWEI_BRANCH_CHOU
    };
    static const int nianjie_by_year[12] = {
        ZIWEI_BRANCH_XU, ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_WEI,
        ZIWEI_BRANCH_WU, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_MAO,
        ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_CHOU, ZIWEI_BRANCH_ZI, ZIWEI_BRANCH_HAI
    };
    static const int jielu_by_stem_mod5[5] = {
        ZIWEI_BRANCH_SHEN, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_CHEN, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_ZI
    };
    static const int kongwang_by_stem_mod5[5] = {
        ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_WEI, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_MAO, ZIWEI_BRANCH_CHOU
    };
    const int month = chart->calendar.lunar_month;
    const int day_offset = chart->calendar.lunar_day - 1;
    const int year_stem = chart->calendar.year_stem;
    const int year_branch = chart->calendar.year_branch;
    int xunkong;

    init_positions(positions, ZIWEI_EXTRA_STAR_COUNT);

    positions[ZIWEI_EXTRA_HONGLUAN] = wrap_palace(ZIWEI_BRANCH_MAO - year_branch);
    positions[ZIWEI_EXTRA_TIANXI] = wrap_palace(positions[ZIWEI_EXTRA_HONGLUAN] + 6);
    positions[ZIWEI_EXTRA_TIANYAO] = wrap_palace(ZIWEI_BRANCH_CHOU + month - 1);
    positions[ZIWEI_EXTRA_XIANCHI] = wrap_palace((int[]){ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_MAO, ZIWEI_BRANCH_ZI}[year_branch % 4]);
    positions[ZIWEI_EXTRA_JIESHEN] = yuejie_by_month_group[(month - 1) / 2];
    positions[ZIWEI_EXTRA_SANTAI] = wrap_palace(chart->minor_star_palace[ZIWEI_MINOR_ZUOFU] + day_offset);
    positions[ZIWEI_EXTRA_BAZUO] = wrap_palace(chart->minor_star_palace[ZIWEI_MINOR_YOUBI] - day_offset);
    positions[ZIWEI_EXTRA_ENGGUANG] = wrap_palace(chart->minor_star_palace[ZIWEI_MINOR_WENCHANG] + day_offset - 1);
    positions[ZIWEI_EXTRA_TIANGUI] = wrap_palace(chart->minor_star_palace[ZIWEI_MINOR_WENQU] + day_offset - 1);
    positions[ZIWEI_EXTRA_LONGCHI] = wrap_palace(ZIWEI_BRANCH_CHEN + year_branch);
    positions[ZIWEI_EXTRA_FENGGE] = wrap_palace(ZIWEI_BRANCH_XU - year_branch);
    positions[ZIWEI_EXTRA_TIANCAI] = wrap_palace(chart->ming_palace + year_branch);
    positions[ZIWEI_EXTRA_TIANSHOU] = wrap_palace(chart->shen_palace + year_branch);
    positions[ZIWEI_EXTRA_TAIFU] = wrap_palace(ZIWEI_BRANCH_WU + chart->calendar.hour_branch);
    positions[ZIWEI_EXTRA_FENGGAO] = wrap_palace(ZIWEI_BRANCH_YIN + chart->calendar.hour_branch);
    positions[ZIWEI_EXTRA_TIANWU] = tianwu_by_month_mod[(month - 1) % 4];

    switch (year_branch) {
    case ZIWEI_BRANCH_YIN:
    case ZIWEI_BRANCH_WU:
    case ZIWEI_BRANCH_XU:
        positions[ZIWEI_EXTRA_HUAGAI] = ZIWEI_BRANCH_XU;
        positions[ZIWEI_EXTRA_XIANCHI] = ZIWEI_BRANCH_MAO;
        positions[ZIWEI_EXTRA_GUCHEN] = ZIWEI_BRANCH_HAI;
        positions[ZIWEI_EXTRA_GUASU] = ZIWEI_BRANCH_WEI;
        break;
    case ZIWEI_BRANCH_SHEN:
    case ZIWEI_BRANCH_ZI:
    case ZIWEI_BRANCH_CHEN:
        positions[ZIWEI_EXTRA_HUAGAI] = ZIWEI_BRANCH_CHEN;
        positions[ZIWEI_EXTRA_XIANCHI] = ZIWEI_BRANCH_YOU;
        positions[ZIWEI_EXTRA_GUCHEN] = ZIWEI_BRANCH_SI;
        positions[ZIWEI_EXTRA_GUASU] = ZIWEI_BRANCH_CHOU;
        break;
    case ZIWEI_BRANCH_SI:
    case ZIWEI_BRANCH_YOU:
    case ZIWEI_BRANCH_CHOU:
        positions[ZIWEI_EXTRA_HUAGAI] = ZIWEI_BRANCH_CHOU;
        positions[ZIWEI_EXTRA_XIANCHI] = ZIWEI_BRANCH_WU;
        positions[ZIWEI_EXTRA_GUCHEN] = ZIWEI_BRANCH_YIN;
        positions[ZIWEI_EXTRA_GUASU] = ZIWEI_BRANCH_XU;
        break;
    default:
        positions[ZIWEI_EXTRA_HUAGAI] = ZIWEI_BRANCH_WEI;
        positions[ZIWEI_EXTRA_XIANCHI] = ZIWEI_BRANCH_ZI;
        positions[ZIWEI_EXTRA_GUCHEN] = ZIWEI_BRANCH_SHEN;
        positions[ZIWEI_EXTRA_GUASU] = ZIWEI_BRANCH_CHEN;
        break;
    }

    positions[ZIWEI_EXTRA_TIANGUAN] = tianguan_by_stem[year_stem];
    positions[ZIWEI_EXTRA_TIANFU_ADJ] = tianfu_adj_by_stem[year_stem];
    positions[ZIWEI_EXTRA_TIANCHU] = tianchu_by_stem[year_stem];
    positions[ZIWEI_EXTRA_TIANYUE] = tianyue_by_month[month - 1];
    positions[ZIWEI_EXTRA_TIANDE] = wrap_palace(ZIWEI_BRANCH_YOU + year_branch);
    positions[ZIWEI_EXTRA_YUEDE] = wrap_palace(ZIWEI_BRANCH_SI + year_branch);
    positions[ZIWEI_EXTRA_TIANKONG] = wrap_palace(year_branch + 1);
    xunkong = wrap_palace(year_branch + (ZIWEI_STEM_GUI - year_stem) + 1);
    if ((xunkong % 2) != (year_branch % 2)) {
        xunkong = wrap_palace(xunkong + 1);
    }
    positions[ZIWEI_EXTRA_XUNKONG] = xunkong;
    positions[ZIWEI_EXTRA_JIELU] = jielu_by_stem_mod5[year_stem % 5];
    positions[ZIWEI_EXTRA_KONGWANG] = kongwang_by_stem_mod5[year_stem % 5];
    positions[ZIWEI_EXTRA_FEILIAN] = feilian_by_year[year_branch];
    positions[ZIWEI_EXTRA_POSUI] = posui_by_year_mod[year_branch % 3];
    positions[ZIWEI_EXTRA_TIANXING] = wrap_palace(ZIWEI_BRANCH_YOU + month - 1);
    positions[ZIWEI_EXTRA_YINSHA] = yinsha_by_month_mod[(month - 1) % 6];
    positions[ZIWEI_EXTRA_TIANKU] = wrap_palace(ZIWEI_BRANCH_WU - year_branch);
    positions[ZIWEI_EXTRA_TIANXU] = wrap_palace(ZIWEI_BRANCH_WU + year_branch);
    positions[ZIWEI_EXTRA_TIANSHI] = wrap_palace(chart->ming_palace + ZIWEI_PALACE_ROLE_HEALTH);
    positions[ZIWEI_EXTRA_TIANSHANG] = wrap_palace(chart->ming_palace + ZIWEI_PALACE_ROLE_FRIENDS);
    positions[ZIWEI_EXTRA_NIANJIE] = nianjie_by_year[year_branch];
    positions[ZIWEI_EXTRA_DAHAO_ADJ] = wrap_palace((int[]){ZIWEI_BRANCH_WEI, ZIWEI_BRANCH_WU, ZIWEI_BRANCH_YOU, ZIWEI_BRANCH_SHEN,
                                                          ZIWEI_BRANCH_HAI, ZIWEI_BRANCH_XU, ZIWEI_BRANCH_CHOU, ZIWEI_BRANCH_ZI,
                                                          ZIWEI_BRANCH_MAO, ZIWEI_BRANCH_YIN, ZIWEI_BRANCH_SI, ZIWEI_BRANCH_CHEN}[year_branch]);
}

static void build_changsheng_positions(const ziwei_chart_result_t *chart, int positions[ZIWEI_GROUP_STAR_COUNT])
{
    int start;
    int i;
    bool forward = chart_is_forward_cycle(chart);

    switch (chart->bureau_number) {
    case 2:
        start = ZIWEI_BRANCH_SHEN;
        break;
    case 3:
        start = ZIWEI_BRANCH_HAI;
        break;
    case 4:
        start = ZIWEI_BRANCH_SI;
        break;
    case 6:
        start = ZIWEI_BRANCH_YIN;
        break;
    default:
        start = ZIWEI_BRANCH_SHEN;
        break;
    }

    for (i = 0; i < ZIWEI_GROUP_STAR_COUNT; ++i) {
        positions[i] = wrap_palace(start + (forward ? i : -i));
    }
}

static void build_boshi_positions(const ziwei_chart_result_t *chart, int positions[ZIWEI_GROUP_STAR_COUNT])
{
    int i;
    bool forward = chart_is_forward_cycle(chart);
    const int start = chart->minor_star_palace[ZIWEI_MINOR_LUCUN];

    for (i = 0; i < ZIWEI_GROUP_STAR_COUNT; ++i) {
        positions[i] = wrap_palace(start + (forward ? i : -i));
    }
}

static void build_suiqian_positions(const ziwei_chart_result_t *chart, int positions[ZIWEI_GROUP_STAR_COUNT])
{
    int i;

    for (i = 0; i < ZIWEI_GROUP_STAR_COUNT; ++i) {
        positions[i] = wrap_palace(chart->calendar.year_branch + i);
    }
}

static void build_jiangqian_positions(const ziwei_chart_result_t *chart, int positions[ZIWEI_GROUP_STAR_COUNT])
{
    int start;
    int i;

    switch (chart->calendar.year_branch) {
    case ZIWEI_BRANCH_YIN:
    case ZIWEI_BRANCH_WU:
    case ZIWEI_BRANCH_XU:
        start = ZIWEI_BRANCH_WU;
        break;
    case ZIWEI_BRANCH_SHEN:
    case ZIWEI_BRANCH_ZI:
    case ZIWEI_BRANCH_CHEN:
        start = ZIWEI_BRANCH_ZI;
        break;
    case ZIWEI_BRANCH_SI:
    case ZIWEI_BRANCH_YOU:
    case ZIWEI_BRANCH_CHOU:
        start = ZIWEI_BRANCH_YOU;
        break;
    default:
        start = ZIWEI_BRANCH_MAO;
        break;
    }

    for (i = 0; i < ZIWEI_GROUP_STAR_COUNT; ++i) {
        positions[i] = wrap_palace(start + i);
    }
}

static void append_position_group(char *buffer,
                                  size_t buffer_size,
                                  const char *label,
                                  const char *const *names,
                                  const int *positions,
                                  int count,
                                  int palace_index)
{
    int i;
    bool first = true;

    append_text(buffer, buffer_size, "\n%s：", label);
    for (i = 0; i < count; ++i) {
        if (positions[i] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", names[i]);
            first = false;
        }
    }
    if (first) {
        append_text(buffer, buffer_size, "无");
    }
}

static void calc_decades(ziwei_chart_result_t *out)
{
    int step;

    out->decade_forward = (stem_is_yang(out->calendar.year_stem) && out->input.gender == ZIWEI_GENDER_MALE) ||
                          (!stem_is_yang(out->calendar.year_stem) && out->input.gender == ZIWEI_GENDER_FEMALE);
    out->decade_start_age = out->bureau_number;

    for (step = 0; step < ZIWEI_PALACE_COUNT; ++step) {
        const int palace = wrap_palace(out->ming_palace + (out->decade_forward ? step : -step));
        out->decade_age_start_by_palace[palace] = out->decade_start_age + step * 10;
        out->decade_age_end_by_palace[palace] = out->decade_start_age + step * 10 + 9;
    }
}

static void calc_flow_positions(ziwei_chart_result_t *out)
{
    out->flow_year_palace = out->flow_calendar.year_branch;
    out->flow_month_base_palace = out->flow_year_palace;
    out->flow_month_palace = wrap_palace(out->flow_month_base_palace + (out->flow_calendar.lunar_month - 1));
    out->flow_day_palace = wrap_palace(out->flow_month_palace + (out->flow_calendar.lunar_day - 1));
}

static const char *star_ref_name(const ziwei_star_ref_t *ref)
{
    if (ref->family == ZIWEI_STAR_REF_MAJOR) {
        return ziwei_get_major_star_name(ref->id);
    }
    return ziwei_get_minor_star_name(ref->id);
}

static int star_ref_palace(const ziwei_chart_result_t *chart, const ziwei_star_ref_t *ref)
{
    if (ref->family == ZIWEI_STAR_REF_MAJOR) {
        return chart->major_star_palace[ref->id];
    }
    return chart->minor_star_palace[ref->id];
}

bool ziwei_chart_generate(const ziwei_input_t *input, ziwei_chart_result_t *out)
{
    if (out == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    init_chart_arrays(out);

    if (input == NULL) {
        snprintf(out->error, sizeof(out->error), "缺少输入参数");
        return false;
    }

    out->input = *input;

    if (!ziwei_calendar_convert(input, &out->calendar)) {
        snprintf(out->error, sizeof(out->error), "%s", out->calendar.error);
        return false;
    }

    out->ming_palace = wrap_palace(ZIWEI_BRANCH_YIN + (out->calendar.lunar_month - 1) - out->calendar.hour_branch);
    out->shen_palace = wrap_palace(ZIWEI_BRANCH_YIN + (out->calendar.lunar_month - 1) + out->calendar.hour_branch);

    assign_palace_roles(out);
    calc_bureau(out);
    place_major_stars(out);
    place_minor_stars(out);
    place_transforms(out);
    calc_decades(out);
    out->flow_calendar = out->calendar;
    calc_flow_positions(out);

    out->valid = true;
    out->error[0] = '\0';
    return true;
}

bool ziwei_chart_set_flow_date(ziwei_chart_result_t *chart, int year, int month, int day)
{
    ziwei_input_t flow_input;

    if (chart == NULL || !chart->valid) {
        return false;
    }

    flow_input = chart->input;
    flow_input.year = year;
    flow_input.month = month;
    flow_input.day = day;
    flow_input.hour = 0;
    flow_input.minute = 0;

    if (!ziwei_calendar_convert(&flow_input, &chart->flow_calendar)) {
        return false;
    }

    calc_flow_positions(chart);
    return true;
}

const char *ziwei_get_branch_name(int index)
{
    return BRANCH_NAMES[wrap_palace(index)];
}

const char *ziwei_get_stem_name(int index)
{
    return STEM_NAMES[index % 10];
}

const char *ziwei_get_palace_role_name(int index)
{
    return PALACE_ROLE_NAMES[index % ZIWEI_PALACE_COUNT];
}

const char *ziwei_get_palace_role_short_name(int index)
{
    return PALACE_ROLE_SHORT_NAMES[index % ZIWEI_PALACE_COUNT];
}

const char *ziwei_get_major_star_name(int index)
{
    return MAJOR_STAR_NAMES[index % ZIWEI_MAJOR_STAR_COUNT];
}

const char *ziwei_get_major_star_short_name(int index)
{
    return MAJOR_STAR_SHORT_NAMES[index % ZIWEI_MAJOR_STAR_COUNT];
}

const char *ziwei_get_minor_star_name(int index)
{
    return MINOR_STAR_NAMES[index % ZIWEI_MINOR_STAR_COUNT];
}

const char *ziwei_get_transform_name(int index)
{
    return TRANSFORM_NAMES[index % ZIWEI_TRANSFORM_COUNT];
}

void ziwei_build_chart_summary(const ziwei_chart_result_t *chart, char *buffer, size_t buffer_size)
{
    int i;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    buffer[0] = '\0';

    if (chart == NULL || !chart->valid) {
        append_text(buffer, buffer_size, "尚未生成命盘。");
        return;
    }

    append_text(buffer, buffer_size,
                "公历 %04d-%02d-%02d %02d:%02d\n",
                chart->input.year, chart->input.month, chart->input.day, chart->input.hour, chart->input.minute);
    append_text(buffer, buffer_size,
                "农历 %s%s年%s%d月%d日\n",
                ziwei_get_stem_name(chart->calendar.year_stem),
                ziwei_get_branch_name(chart->calendar.year_branch),
                chart->calendar.lunar_is_leap ? "闰" : "",
                chart->calendar.lunar_month,
                chart->calendar.lunar_day);
    append_text(buffer, buffer_size,
                "命宫 %s，身宫 %s，%s，%s行，%d岁起\n",
                ziwei_get_branch_name(chart->ming_palace),
                ziwei_get_branch_name(chart->shen_palace),
                chart->bureau_name,
                chart->decade_forward ? "顺" : "逆",
                chart->decade_start_age);
    append_text(buffer, buffer_size, "四化 ");
    for (i = 0; i < ZIWEI_TRANSFORM_COUNT; ++i) {
        append_text(buffer, buffer_size,
                    "%s%s%s",
                    i == 0 ? "" : "，",
                    ziwei_get_transform_name(i),
                    star_ref_name(&chart->natal_transforms[i]));
    }
}

void ziwei_build_palace_grid_text(const ziwei_chart_result_t *chart, int palace_index, char *buffer, size_t buffer_size)
{
    int star_id;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    buffer[0] = '\0';

    if (chart == NULL || !chart->valid) {
        snprintf(buffer, buffer_size, "--");
        return;
    }

    append_text(buffer, buffer_size, "%s", ziwei_get_palace_role_short_name(chart->palace_role_by_index[palace_index]));

    for (star_id = 0; star_id < ZIWEI_MAJOR_STAR_COUNT; ++star_id) {
        if (chart->major_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "\n%s", ziwei_get_major_star_short_name(star_id));
            return;
        }
    }

    for (star_id = 0; star_id < ZIWEI_MINOR_STAR_COUNT; ++star_id) {
        if (chart->minor_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "\n%s", ziwei_get_minor_star_name(star_id));
            return;
        }
    }

    append_text(buffer, buffer_size, "\n空宫");
}

void ziwei_build_palace_detail(const ziwei_chart_result_t *chart, int palace_index, char *buffer, size_t buffer_size)
{
    int star_id;
    bool first;
    int extra_positions[ZIWEI_EXTRA_STAR_COUNT];
    int changsheng_positions[ZIWEI_GROUP_STAR_COUNT];
    int boshi_positions[ZIWEI_GROUP_STAR_COUNT];
    int suiqian_positions[ZIWEI_GROUP_STAR_COUNT];
    int jiangqian_positions[ZIWEI_GROUP_STAR_COUNT];

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    buffer[0] = '\0';

    if (chart == NULL || !chart->valid) {
        append_text(buffer, buffer_size, "请先生成命盘。");
        return;
    }

    append_text(buffer, buffer_size,
                "%s%s宫\n",
                ziwei_get_palace_role_name(chart->palace_role_by_index[palace_index]),
                ziwei_get_branch_name(palace_index));

    if (palace_index == chart->ming_palace) {
        append_text(buffer, buffer_size, "标记：命宫\n");
    }
    if (palace_index == chart->shen_palace) {
        append_text(buffer, buffer_size, "标记：身宫\n");
    }

    append_text(buffer, buffer_size,
                "大限：%d-%d岁\n",
                chart->decade_age_start_by_palace[palace_index],
                chart->decade_age_end_by_palace[palace_index]);

    build_extra_star_positions(chart, extra_positions);
    build_changsheng_positions(chart, changsheng_positions);
    build_boshi_positions(chart, boshi_positions);
    build_suiqian_positions(chart, suiqian_positions);
    build_jiangqian_positions(chart, jiangqian_positions);

    append_text(buffer, buffer_size, "同宫星：");
    first = true;
    for (star_id = 0; star_id < ZIWEI_MAJOR_STAR_COUNT; ++star_id) {
        if (chart->major_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", ziwei_get_major_star_name(star_id));
            first = false;
        }
    }
    for (star_id = 0; star_id < ZIWEI_MINOR_STAR_COUNT; ++star_id) {
        if (chart->minor_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", ziwei_get_minor_star_name(star_id));
            first = false;
        }
    }
    for (star_id = 0; star_id < ZIWEI_EXTRA_STAR_COUNT; ++star_id) {
        if (extra_positions[star_id] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", EXTRA_STAR_NAMES[star_id]);
            first = false;
        }
    }
    if (first) {
        append_text(buffer, buffer_size, "无");
    }

    append_text(buffer, buffer_size, "\n主星：");
    first = true;
    for (star_id = 0; star_id < ZIWEI_MAJOR_STAR_COUNT; ++star_id) {
        if (chart->major_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", ziwei_get_major_star_name(star_id));
            first = false;
        }
    }
    if (first) {
        append_text(buffer, buffer_size, "无");
    }

    append_text(buffer, buffer_size, "\n辅星：");
    first = true;
    for (star_id = 0; star_id < ZIWEI_MINOR_STAR_COUNT; ++star_id) {
        if (chart->minor_star_palace[star_id] == palace_index) {
            append_text(buffer, buffer_size, "%s%s", first ? "" : "、", ziwei_get_minor_star_name(star_id));
            first = false;
        }
    }
    if (first) {
        append_text(buffer, buffer_size, "无");
    }

    append_position_group(buffer, buffer_size, "杂曜", EXTRA_STAR_NAMES, extra_positions, ZIWEI_EXTRA_STAR_COUNT, palace_index);
    append_position_group(buffer, buffer_size, "长生十二神", CHANGSHENG_NAMES, changsheng_positions, ZIWEI_GROUP_STAR_COUNT, palace_index);
    append_position_group(buffer, buffer_size, "博士十二神", BOSHI_NAMES, boshi_positions, ZIWEI_GROUP_STAR_COUNT, palace_index);
    append_position_group(buffer, buffer_size, "岁前十二神", SUIQIAN_NAMES, suiqian_positions, ZIWEI_GROUP_STAR_COUNT, palace_index);
    append_position_group(buffer, buffer_size, "将前十二神", JIANGQIAN_NAMES, jiangqian_positions, ZIWEI_GROUP_STAR_COUNT, palace_index);

    append_text(buffer, buffer_size, "\n四化落宫：");
    first = true;
    for (star_id = 0; star_id < ZIWEI_TRANSFORM_COUNT; ++star_id) {
        if (star_ref_palace(chart, &chart->natal_transforms[star_id]) == palace_index) {
            append_text(buffer, buffer_size,
                        "%s%s%s",
                        first ? "" : "、",
                        ziwei_get_transform_name(star_id),
                        star_ref_name(&chart->natal_transforms[star_id]));
            first = false;
        }
    }
    if (first) {
        append_text(buffer, buffer_size, "无");
    }
}
