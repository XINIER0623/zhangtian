#pragma once

#include "ziwei_types.h"

bool ziwei_is_valid_solar_date(int year, int month, int day);
int ziwei_hour_to_branch(int hour);
bool ziwei_calendar_convert(const ziwei_input_t *input, ziwei_calendar_result_t *out);
