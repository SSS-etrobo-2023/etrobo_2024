#include <stdio.h>
#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

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
