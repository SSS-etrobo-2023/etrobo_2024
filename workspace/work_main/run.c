#include <stdlib.h>
//#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ev3api.h"
#include "common.h"
#include "extern.h"

/* センサー、モーターの接続定義 */
extern const sensor_port_t touch_sensor;
extern const sensor_port_t color_sensor;
extern const sensor_port_t sonar_sensor;
extern const sensor_port_t gyro_sensor;

extern const motor_port_t arm_motor;
extern const motor_port_t right_motor;
extern const motor_port_t left_motor;
extern const motor_port_t center_motor;

//制御定数
// @memo この値は適宜調整する
static const float T  = 0.002;
static const float Kp = 1.5;   // P
static const float Ki = 1.0;   // I
static const float Kd = 0.015; // D

static float PID(unsigned int target_ref, float obtained_ref) {
    float P = 0, I = 0, D = 0;
    static float dt = 0;
    static float pre = 0;
    static float ie = 0;
    float de = 0;
    float res = 0;

    pre = dt;
    dt = target_ref - obtained_ref;

    // P制御
    P = Kp * dt;

    // I制御
    ie = ie + (dt + pre)*T/2;
    I = Ki * ie;

    // D制御
    de = (dt - pre)/T;
    D = Kd * de;

    res = P + I + D;

    LOG_D_TEST("P: %.2f, I: %.2f, D: %.2f\n", P, I, D);
    return res;
}

static void motor_rotate_spec_count(int left_power, int right_power, int32_t count) {
    int32_t left_counts = 0;
    int32_t right_counts = 0;

    motor_stop();
    tslp_tsk(100 * 1000U);

    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    ev3_motor_set_power(left_motor, left_power);
    ev3_motor_set_power(right_motor, right_power);

    while (1) {
        left_counts = ev3_motor_get_counts(left_motor);
        right_counts = ev3_motor_get_counts(right_motor);

        // この判定は実動作次第で調整する
        if ((abs(left_counts) >= abs(count)) && (abs(right_counts) >= abs(count))) {
            LOG_D_TEST("left_motor: %ld deg, right_motor: %ld deg\n", left_counts, right_counts);
            break;
        }
    }

    // 回転終了後は停止状態にする
    motor_stop();
    tslp_tsk(100 * 1000U);

    return;
}

//*****************************************************************************
// 関数名 : culculate_turn
// 引数   : target_reflect - 反射基準値
//          trace_pos      - トレースするエッジ (RIGHT/LEFT)
// 返り値 : 角度
// 概要   : PID制御を行い、角度を返す　
//*****************************************************************************
float culculate_turn(unsigned int target_reflect, int trace_pos) {
    float turn = 0;
    float curb = 0;
    float sensor_reflect = 0;

    sensor_reflect = ev3_color_sensor_get_reflect(color_sensor);
    if(sensor_reflect > LIGHT_WHITE){
        sensor_reflect = LIGHT_WHITE;
    }

    //曲がり角度の決定
    curb = PID(target_reflect, sensor_reflect);

    /* 曲がり角度が大きくなりすぎないように調整する */
    if (curb > 80){
        curb = 80;
    } else if(curb < -80){
        curb = -80;
    }

    if(trace_pos == LEFT) {
        turn = -curb;
    } else {
        turn = curb;
    }

    LOG_D_TEST("turn: %f\n", turn);
    return turn;
}


/**
 * ev3_motor_steer() の代わりとなる関数
 * power : -100 ... 100
 * turn  : -100 ... 100 
 * turn が正 -> 右に転回
 * turn が負 -> 左に転回
 * turn が 0 -> 直進
 */
void motor_steer(int power, float turn) {
    int left_power = 0;
    int right_power = 0;

    /* パラメータチェック */
    if (power > POWER_MAX) {
        LOG_D_DEBUG("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power < POWER_MIN) {
        LOG_D_DEBUG("invalid power. change %d to %d", power, POWER_MIN);
        power = POWER_MIN;
    }

    if (turn > TURN_MAX) {
        LOG_D_DEBUG("invalid turn. change %f to %d", turn, TURN_MAX);
        turn = TURN_MAX;
    } else if (turn < TURN_MIN) {
        LOG_D_DEBUG("invalid turn. change %f to %d", turn, TURN_MIN);
        turn = TURN_MAX;
    }

    if (turn > 0) {
        /* 右転回 */
        left_power = power;
        right_power = (power * (100 - turn)) / 100;
    } else if (turn < 0) {
        /* 左転回 */
        left_power = (power * (100 + turn)) / 100;
        right_power = power;
    } else {
        /* 直進 */
        left_power = power;
        right_power = power;
    }

    ev3_motor_set_power(left_motor, left_power);
    ev3_motor_set_power(right_motor, right_power);

    return;
}

/**
 * ev3_motor_rotate() の代わりとなる関数
 * power  : 0 ... 100
 * degree : -360 ... 360 
 * degree が正 -> 右に転回
 * degree が負 -> 左に転回
 */
void motor_rotate(int power, int degree) {
    int left_power = 0;
    int right_power = 0;

    /* パラメータチェック */
    if (power > POWER_MAX) {
        LOG_D_DEBUG("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power < 0) {
        LOG_D_DEBUG("invalid power. change %d to %d", power, 0);
        power = 0;
    }

    if (degree > DEGREE_MAX) {
        LOG_D_DEBUG("invalid degree. change %d to %d", degree, DEGREE_MAX);
        degree = DEGREE_MAX;
    } else if (degree < DEGREE_MIN) {
        LOG_D_DEBUG("invalid degree. change %d to %d", degree, DEGREE_MIN);
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
        LOG_D_DEBUG("invalid power. change %d to %d", power, POWER_MAX);
        power = POWER_MAX;
    } else if (power < 0) {
        LOG_D_DEBUG("invalid power: %d", power);
        return;
    }

    if (cm < 0) {
        power = power * -1;
    } else if (cm == 0) {
        return;
    }

    // 指定距離動くために必要な角度を求める
    degree = (360 * cm) / (DIAMETER * PI);

    motor_rotate_spec_count(power, power, degree);

    return;
}

//*****************************************************************************
// 関数名 : motor_stop
// 引数 :   
// 返り値 : なし
// 概要 :　車体を停止する
//*****************************************************************************
void motor_stop() {
    ev3_motor_stop(left_motor, true);
    ev3_motor_stop(right_motor, true);
}
