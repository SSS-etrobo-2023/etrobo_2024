#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

uint16_t target_ref = 0;

uint16_t blue_count = 0;

/* トレース位置 */
static int t_pos = COURSE_TYPE;

// 制御定数
// @memo この値は適宜調整する
static int init_power = 70;
// ライン変更の際のインターバル
static int black_c = 0;
static int black_interval = 5;
static int blue_c = 0;
static int blue_interval = 5;
// ライントレースの偏り
static bool isLineChange = false;
static int line_change_count = 0;
//static const float T = LINE_TRACER_PERIOD / (1000 * 1000);
static const float T = 0.01;
#ifdef REF_BY_RGB
static const float Kp = 3.5;

#else
static const float Kp = 3.5;
#endif
static const float Ki = 0.01;
static const float Kd = 0.15;

static int8_t isBlue = false;
static int8_t isEnd = false;
static int8_t phase = PHASE_START;
rgb_raw_t testrgb;

/* ライントレースタスク(10msec周期で関数コールされる) */
void tracer_task(intptr_t unused) {
#ifndef MODE_TEST
    int16_t steering_amount;
    int8_t color_code = COLOR_CODE_MAX;

    if (phase == PHASE_END) {
        motor_stop();
        ext_tsk();
    }

    switch (phase) {
        case PHASE_START:
            LOG_D_TEST("phase_start.\n");
            change_target_reflect(COLOR_CODE_BLACK);
            motor_stop();
            phase = LINE_TRACE;
            change_refrect(2);

            ext_tsk();
            break;
        case LINE_TRACE:
            if (linetrace_find_blue() == 1) {
                LOG_D_DEBUG("Blue Line found.\n");
    ev3_color_sensor_get_rgb_raw(color_sensor, &testrgb);
        LOG_D_DEBUG("r: %d\n", testrgb.r);
        LOG_D_DEBUG("g: %d\n", testrgb.g);
        LOG_D_DEBUG("b: %d\n", testrgb.b);
                motor_stop();
                isBlue = true;
                change_refrect(2);
                init_power = 60;
                phase = DOUBLE_LOOP;
                //change_target_reflect(COLOR_CODE_BLUE);

                ext_tsk();
            }
            break;
        case DOUBLE_LOOP:
            if (isBlue) {
                if (linetrace_find_black() == 1) {
                    black_c++;
                    blue_c = 0;
                }
                if (linetrace_find_black() == 1 && black_c >= 2) {
                    LOG_D_DEBUG("Black Line found.\n");
                    isBlue = false;
                    t_pos = t_pos == RIGHT ? LEFT : RIGHT;
                    init_power = 70;
                    blue_count++;
                    isLineChange = true;
                    change_refrect(1);
                    ev3_color_sensor_get_rgb_raw(color_sensor, &testrgb);
                    LOG_D_DEBUG("r: %d\n", testrgb.r);
                    LOG_D_DEBUG("g: %d\n", testrgb.g);
                    LOG_D_DEBUG("b: %d\n", testrgb.b);
                    LOG_D_DEBUG("blue_count: %d\n", blue_count);
                }
            } else {
                if (linetrace_find_blue() == 1) {
                    blue_c++;
                    black_c = 0;
                }
                if (linetrace_find_blue() == 1 && blue_c >= 3) {
                    ev3_color_sensor_get_rgb_raw(color_sensor, &testrgb);
                    LOG_D_DEBUG("r: %d\n", testrgb.r);
                    LOG_D_DEBUG("g: %d\n", testrgb.g);
                    LOG_D_DEBUG("b: %d\n", testrgb.b);
                    LOG_D_DEBUG("Blue Line found.\n");
                    change_refrect(2);
                    init_power = 70;
                    isBlue = true;
                }
            } 
            if (isLineChange) {
                line_change_count++;
            }
            if (isLineChange && line_change_count >= 200) {
                isLineChange = false;
                LOG_D_DEBUG("change");
                line_change_count = 0;
                change_refrect(0);
            }
            if (blue_count > 3) {
                motor_stop();
                phase = DEBRI_REMOVE;
                //change_target_reflect(COLOR_CODE_BLACK);
                init_power = 60;

                ext_tsk();
            }
            break;
        case DEBRI_REMOVE:
            color_code = get_color(COLOR_CODE_BLUE);
            if (color_code == COLOR_CODE_BLUE) {
                LOG_D_DEBUG("Blue Line found.\n");
                motor_stop();
                isBlue = true;
                phase = PHASE_END;

                ext_tsk();
            }
            break;
        case PHASE_END:
            motor_stop();
            ext_tsk();
            break;
    }
	 
    /* ステアリング操舵量の計算 */
    steering_amount = steering_amount_calculation();

    /* 走行モータ制御 */
    motor_steer(init_power, steering_amount);
#else
    test_main(4);
#endif

    /* タスク終了 */
    ext_tsk();
}

/* ステアリング操舵量の計算 */
int16_t steering_amount_calculation(void){

    int16_t  steering_amount;   /* ステアリング操舵量 */

    /* ステアリング操舵量を計算 */
    steering_amount = calculate_turn(target_ref, t_pos);

    return steering_amount;
}

static int16_t PID(uint16_t target_ref, uint16_t obtained_ref) {
    float P = 0, I = 0, D = 0;
    static float dt = 0;
    static float pre = 0;
    static float ie = 0;
    float de = 0;
    int16_t res = 0;

    pre = dt;
    dt = target_ref - obtained_ref;

    // P制御
    P = Kp * dt;

    // I制御
#if 1
    ie = ie + (dt + pre)*T/2;
    I = Ki * ie;
#endif

    // D制御
    de = (dt - pre)/T;
    D = Kd * de;

    res = P + I + D;

    LOG_D_TEST("P: %.2f, I: %.2f, D: %.2f\n", P, I, D);
    return res;
}

#ifdef REF_BY_RGB
/* TODO: RGB値によるライントレース制御に変えてみる */
int16_t calculate_turn(uint16_t target_reflect, int trace_pos) {
    int16_t turn = 0;
    int16_t curb = 0;
    uint16_t sensor_reflect = 0;
    rgb_raw_t rgb;

    ev3_color_sensor_get_rgb_raw(color_sensor, &rgb);

    /* RGB値をもとに、反射値を求める */
    sensor_reflect = floor(cbrt((rgb.r * 100 / 255) * (rgb.b * 100 / 255) * (rgb.b * 100 / 255)));
    LOG_D_TEST("reflect: %d\n", target_reflect - sensor_reflect);

    curb = PID(target_reflect, sensor_reflect);

    /* 曲がる量が大きくなりすぎないよう、制限する */
    if (curb > CURB_MAX) {
        curb = CURB_MAX;
    } else if (curb < CURB_MIN) {
        curb = CURB_MIN;
    }

    /* @memo: 左右で turn を逆にする
              左トレースの場合、回転方向を逆にする */
    if (trace_pos == LEFT) {
        turn = -curb;
    } else {
        turn = curb;
    }

    LOG_D_TEST("turn: %d\n", turn);
    return turn;
}
#else
int16_t calculate_turn(uint16_t target_reflect, int trace_pos) {
    int16_t turn = 0;
    int16_t curb = 0;
    uint16_t sensor_reflect = 0;

    sensor_reflect = ev3_color_sensor_get_reflect(color_sensor);
    LOG_D_TEST("reflect: %d\n", target_reflect - sensor_reflect);

    curb = PID(target_reflect, sensor_reflect);

    /* 曲がる量が大きくなりすぎないよう、制限する */
    if (curb > CURB_MAX) {
        curb = CURB_MAX;
    } else if (curb < CURB_MIN) {
        curb = CURB_MIN;
    }

    /* @memo: 左右で turn を逆にする
              左トレースの場合、回転方向を逆にする */
    if (trace_pos == LEFT) {
        turn = -curb;
    } else {
        turn = curb;
    }

    LOG_D_TEST("turn: %d\n", turn);
    return turn;
}
#endif

/*
 * 2つのモータでステアリング動作を行う
 * ev3_motor_steer() の代わりとなる関数
 */
void motor_steer(int power, int16_t turn) {
    int left_power = 0;
    int right_power = 0;

    /* パラメータチェック */

    /* パラメータチェック */
    if (power > POWER_MAX) {
        LOG_D_ERROR("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power < POWER_MIN) {
        LOG_D_ERROR("invalid power. change %d to %d", power, POWER_MIN);
        power = POWER_MIN;
    }

    if (turn > TURN_MAX) {
        LOG_D_ERROR("invalid turn. change %d to %d", turn, TURN_MAX);
        turn = TURN_MAX;
    } else if (turn < TURN_MIN) {
        LOG_D_ERROR("invalid turn. change %d to %d", turn, TURN_MIN);
        turn = TURN_MIN;
    }

    if (turn > 0) {
        /* 右転回 */
        left_power  = power;
	right_power = (power * (100 - turn)) / 100;
    } else if (turn < 0) {
        /* 左転回 */
        left_power  = (power * (100 + turn)) / 100;
	right_power = power;
    } else {
        /* 直進 */
        left_power = power;
	right_power = power;
    }

    LOG_D_TEST("left_power : %d\n", left_power);
    LOG_D_TEST("right_power: %d\n", right_power);

    set_motor_power(left_power, right_power);

    return;
}

void motor_rotate_spec_count(int left_power, int right_power, int32_t count) {
    int32_t left_count = 0;
    int32_t right_count = 0;

    motor_stop();

    /* @memo: sleep させる？*/

    reset_motor_counts();

    set_motor_power(left_power, right_power);

    while(1) {
        left_count = ev3_motor_get_counts(left_motor);
        right_count = ev3_motor_get_counts(right_motor);

        /* @memo: この判定は実動作で確認して調整すること */
        if ((abs(left_count) >= abs(count)) && (abs(right_count) >= abs(count))) {
            LOG_D_TEST("left_motor: %ld deg, right_motor: %ld deg\n", left_count, right_count);
            break;
        }
    }

    /* 回転終了後は停止状態にする */
    motor_stop();

    return;
}

/*
 * モータを指定した角度だけ転回させる
 * ev3_motor_rotate() の代わりとなる関数
 *
 * degree が正 -> 右に転回
 * degree が負 -> 左に転回
 */
void motor_rotate(int power, int degree) {
    int left_power = 0;
    int right_power = 0;

    /* パラメータチェック */
    if (power > POWER_MAX) {
        LOG_D_ERROR("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power < 0) {
        LOG_D_ERROR("invalid power. change %d to %d", power, 0);
        power = 0;
    }

    if (degree > DEGREE_MAX) {
        LOG_D_ERROR("invalid degree. change %d to %d", degree, DEGREE_MAX);
        degree = DEGREE_MAX;
    } else if (degree < DEGREE_MIN) {
        LOG_D_ERROR("invalid degree. change %d to %d", degree, DEGREE_MIN);
        degree = DEGREE_MIN;
    }

    if (degree > 0) {
        /* 右転回 */
        left_power = power;
        right_power = power * -1;
    } else if (degree < 0) {
        /* 左転回 */
        left_power = power * -1;
        right_power = power;
    } else {
        // 何もしない
        return;
    }

    motor_rotate_spec_count(left_power, right_power, degree);

    return;
}

/* 指定した距離(unit: cm)だけモータを回転させる */
void motor_move(int power, int cm) {
    int degree = 0;

    /* パラメータチェック */
    if (power > POWER_MAX) {
        LOG_D_ERROR("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power == 0) {
        LOG_D_ERROR("invalid power: %d", power);
        return;
    }

    if (cm < 0) {
        power = power * -1;
    } else if (cm == 0) {
        return;
    }

    /* 指定距離動くために必要な角度を求める */
    /* @memo: このパラメータは実動作で確認して調整すること */
    degree = (360 * cm) / (DIAMETER * PI);

    motor_rotate_spec_count(power, power, degree);

    return;
}


int linetrace_find_blue() {
    rgb_raw_t rgb;

    ev3_color_sensor_get_rgb_raw(color_sensor, &rgb);
    
    if (rgb.b > 80 &&
        rgb.b > 1.6 * rgb.r &&
        rgb.g > rgb.r * 1.2 &&
        rgb.b > 1.1 * rgb.g) {
        return 1;
    }
    return 2;
    
}

int linetrace_find_black() {
    rgb_raw_t rgb;

    ev3_color_sensor_get_rgb_raw(color_sensor, &rgb);
    
    if (rgb.r < 90 &&
        rgb.g < 90 &&
        rgb.b < 90 &&
        rgb.b < rgb.r * 1.1) {
        return 1;
    }
    return 2;
}