#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

/* トレース位置 */
static int t_pos = COURSE_TYPE;

// 制御定数
// @memo この値は適宜調整する
static const int init_power = 70;
//static const float T = LINE_TRACER_PERIOD / (1000 * 1000);
static const float T = 0.002;
static const float Kp = 5.0;
static const float Ki = 0.01;
static const float Kd = 1.5;

static int8_t isEnd = false;

/* ライントレースタスク(10msec周期で関数コールされる) */
void tracer_task(intptr_t unused) {
#ifndef MODE_TEST
    int16_t steering_amount;
    int8_t color_code = COLOR_CODE_MAX;

    if (isEnd) {
        motor_stop();
        ext_tsk();
    }

    /* ステアリング操舵量の計算 */
    steering_amount = steering_amount_calculation();

    /* 走行モータ制御 */
    motor_steer(init_power, steering_amount);

    color_code = get_color(COLOR_CODE_BLUE);
    if (color_code == COLOR_CODE_BLUE) {
        LOG_D_DEBUG("Blue Line found.\n");
        motor_stop();
        isEnd = true;
    }
#else
    test_main(1);
#endif

    /* タスク終了 */
    ext_tsk();
}

/* ステアリング操舵量の計算 */
int16_t steering_amount_calculation(void){

    uint16_t target_brightness; /* 目標輝度値 */
    int16_t  steering_amount;   /* ステアリング操舵量 */

    /* 目標輝度値の計算 */
    target_brightness = (LIGHT_WHITE - LIGHT_BLACK) / 5;

    /* ステアリング操舵量を計算 */
    steering_amount = calculate_turn(target_brightness, t_pos);

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

int16_t calculate_turn(uint16_t target_reflect, int trace_pos) {
    int16_t turn = 0;
    int16_t curb = 0;
    uint16_t sensor_reflect = 0;

    /*
     * @memo: 青線をトレースできるよう、センサの値を 1/3 しておく。
     *        将来的に変更の可能性あり。
     */
    sensor_reflect = ev3_color_sensor_get_reflect(color_sensor) / 3;
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


/*
 * 2つのモータでステアリング動作を行う
 * ev3_motor_steer() の代わりとなる関数
 */
void motor_steer(int power, int16_t turn) {
    int left_power = 0;
    int right_power = 0;

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

