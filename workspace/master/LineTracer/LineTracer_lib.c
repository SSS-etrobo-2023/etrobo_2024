#include <stdio.h>
#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

extern uint16_t target_ref;

#define REF_AVE_BLACK (LIGHT_WHITE - LIGHT_BLACK)
#define REF_AVE_BLUE  (LIGHT_WHITE - LIGHT_BLUE)

static uint16_t def_reflect[COLOR_CODE_MAX] = {
                  0, /* 赤 */
    REF_AVE_BLUE /2, /* 青 */
                  0, /* 緑 */
                  0, /* 黄 */
    REF_AVE_BLACK/3, /* 黒 */
                  0, /* 白 */
};

void change_target_reflect(int color_code) {
    if (def_reflect[color_code] == 0) {
        LOG_D_ERROR("Invalid color_code(%d).\n", color_code);
        target_ref = def_reflect[COLOR_CODE_BLACK];
        return;
    }

    target_ref = def_reflect[color_code];
    return;
}

int change_trace_pos(int trace_pos) {
    LOG_D_DEBUG("trace_pos change. %s -> %s\n",
                 trace_pos == RIGHT ? "RIGHT" : "LEFT",
                 trace_pos == RIGHT ? "LEFT" : "RIGHT");

    return trace_pos == RIGHT ? LEFT : RIGHT;
}

void set_motor_power(int left_power, int right_power) {
    ev3_motor_set_power(left_motor, left_power);
    ev3_motor_set_power(right_motor, right_power);

    return;
}

void reset_motor_counts(void) {
    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    return;
}

void motor_stop(void) {
    ev3_motor_stop(left_motor, true);
    ev3_motor_stop(right_motor, true);

    return;
}
