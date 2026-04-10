#pragma once

#include <stddef.h>

#include "ziwei_types.h"

bool ziwei_chart_generate(const ziwei_input_t *input, ziwei_chart_result_t *out);
bool ziwei_chart_set_flow_date(ziwei_chart_result_t *chart, int year, int month, int day);

const char *ziwei_get_branch_name(int index);
const char *ziwei_get_stem_name(int index);
const char *ziwei_get_palace_role_name(int index);
const char *ziwei_get_palace_role_short_name(int index);
const char *ziwei_get_major_star_name(int index);
const char *ziwei_get_major_star_short_name(int index);
const char *ziwei_get_minor_star_name(int index);
const char *ziwei_get_transform_name(int index);

void ziwei_build_chart_summary(const ziwei_chart_result_t *chart, char *buffer, size_t buffer_size);
void ziwei_build_palace_grid_text(const ziwei_chart_result_t *chart, int palace_index, char *buffer, size_t buffer_size);
void ziwei_build_palace_detail(const ziwei_chart_result_t *chart, int palace_index, char *buffer, size_t buffer_size);
