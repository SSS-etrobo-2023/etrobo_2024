#include <stdio.h>
#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

extern uint16_t target_ref;

#define REF_BLACK (LIGHT_WHITE + LIGHT_BLACK)
#define REF_BLUE  (LIGHT_WHITE + LIGHT_BLUE)

#define REF_BLACK_RGB (LIGHT_WHITE_RGB + LIGHT_BLACK_RGB)
#define REF_BLUE_RGB  (LIGHT_WHITE_RGB + LIGHT_BLUE_RGB)

#ifdef REF_BY_RGB
static uint16_t def_reflect[COLOR_CODE_MAX] = {
                  0, /* 赤 */
    REF_BLUE_RGB /3, /* 青 */
                  0, /* 緑 */
                  0, /* 黄 */
    REF_BLACK_RGB/4, /* 黒 */
                  0, /* 白 */
};
#else
static uint16_t def_reflect[COLOR_CODE_MAX] = {
              0, /* 赤 */
    REF_BLUE /2, /* 青 */
              0, /* 緑 */
              0, /* 黄 */
    REF_BLACK/3, /* 黒 */
              0, /* 白 */
};
#endif

void change_target_ref_dev_remove(void) {
    target_ref = REF_BLACK_RGB/3;
    return;
}

#ifdef REF_BY_RGB
void change_target_reflect(int color_code) {
    if (def_reflect[color_code] == 0) {
        LOG_D_ERROR("Invalid color_code(%d).\n", color_code);
        target_ref = def_reflect[COLOR_CODE_BLACK];
        return;
    }

    target_ref = def_reflect[color_code];
    return;
}
#else
void change_target_reflect(int color_code) {
    if (def_reflect[color_code] == 0) {
        LOG_D_ERROR("Invalid color_code(%d).\n", color_code);
        target_ref = def_reflect[COLOR_CODE_BLACK];
        return;
    }

    target_ref = def_reflect[color_code];
    return;
}
#endif

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

void set_motor_power_v2(int left_power, int right_power) {
    ev3_motor_set_power(left_motor, 70);
    ev3_motor_set_power(right_motor, 70);

    usleep(100 * 1000 * 1000);

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
