#include <stdio.h>
#include "app.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

// 制御定数
// @memo この値は適宜調整する
static const int init_power = 70;
static const float T = 0.01; //10msec駆動に合わせて設定
static const float Kp = 5.0;
static const float Ki = 0.01;
static const float Kd = 3.0;

/* 関数プロトタイプ宣言 */
static int16_t steering_amount_calculation(void);
static int16_t calculate_turn(uint16_t, int);
static void motor_steer(int, int16_t);

/* ライントレースタスク(100msec周期で関数コールされる) */
void tracer_task(intptr_t unused) {
    int16_t steering_amount;

    /* ステアリング操舵量の計算 */
    steering_amount = steering_amount_calculation();

    /* 走行モータ制御 */
    motor_steer(init_power, steering_amount);

    /* タスク終了 */
    ext_tsk();
}

/* ステアリング操舵量の計算 */
static int16_t steering_amount_calculation(void){

    uint16_t target_brightness; /* 目標輝度値 */
    int16_t  steering_amount;   /* ステアリング操舵量 */

    /* 目標輝度値の計算 */
    target_brightness = (LIGHT_WHITE - LIGHT_BLACK) / 5;

    /* ステアリング操舵量を計算 */
    steering_amount = calculate_turn(target_brightness, 0);

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

static int16_t calculate_turn(uint16_t target_reflect, int trace_pos) {
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

    if (curb > CURB_MAX) {
        curb = CURB_MAX;
    } else if (curb < CURB_MIN) {
        curb = CURB_MIN;
    }

    /*TODO: 左右で turn を逆にする
            現在は左側をトレースする */
    turn = -curb;

    LOG_D_TEST("turn: %d\n", turn);
    return turn;
}

static void motor_steer(int power, int16_t turn) {
    int left_power = 0;
    int right_power = 0;

    /* パラメータチェック */
    if (power > POWER_MAX) {
        power = POWER_MAX;
    } else if (power < POWER_MIN) {
        power = POWER_MIN;
    }

    if (turn > TURN_MAX) {
        turn = TURN_MAX;
    } else if (turn < TURN_MIN) {
        turn = TURN_MIN;
    }

#if 1
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
#else
    //left_power = power;
    right_power = (power * (100 - turn)) / 100;
    if (right_power > POWER_MAX) {
        right_power = POWER_MAX;
    }

    if (right_power < 30) {
        right_power = 30;
    }
#endif
    LOG_D_TEST("left_power : %d\n", left_power);
    LOG_D_TEST("right_power: %d\n", right_power);

    //ev3_motor_set_power(left_motor, left_power);
    ev3_motor_set_power(right_motor, right_power);

    return;
}
