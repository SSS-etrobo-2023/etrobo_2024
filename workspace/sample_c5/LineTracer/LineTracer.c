#include "app.h"
#include "LineTracer.h"
#include <stdio.h>

// 制御定数
// @memo この値は適宜調整する
static const float T = 0.1; //100msec駆動に合わせて設定
static const float Kp = 2.5;
static const float Ki = 0.2;
static const float Kd = 0.5;

/* 関数プロトタイプ宣言 */
static int16_t steering_amount_calculation(void);
static void motor_drive_control(int16_t);
static int16_t calculate_turn(uint16_t, int);
static void motor_steer(int, int16_t);

/* ライントレースタスク(100msec周期で関数コールされる) */
void tracer_task(intptr_t unused) {

    int16_t steering_amount; /* ステアリング操舵量の計算 */
    
    /* ステアリング操舵量の計算 */
    steering_amount = steering_amount_calculation();

    /* 走行モータ制御 */
    motor_drive_control(steering_amount);

    /* タスク終了 */
    ext_tsk();
}

/* ステアリング操舵量の計算 */
static int16_t steering_amount_calculation(void){

    uint16_t  target_brightness; /* 目標輝度値 */
    float32_t diff_brightness;   /* 目標輝度との差分値 */
    int16_t   steering_amount;   /* ステアリング操舵量 */
    //rgb_raw_t rgb_val;           /* カラーセンサ取得値 */
    int32_t sensor_reflect;

    /* 目標輝度値の計算 */
    target_brightness = (WHITE_BRIGHTNESS - BLACK_BRIGHTNESS) / 2;

    /* カラーセンサ値の取得 */
    sensor_reflect = ev3_color_sensor_get_reflect(color_sensor);
    //ev3_color_sensor_get_rgb_raw(color_sensor, &rgb_val);

    /* 目標輝度値とカラーセンサ値の差分を計算 */
    //diff_brightness = (float32_t)(target_brightness - rgb_val.g);
    diff_brightness = (float32_t)(target_brightness - sensor_reflect);
    printf("target: %d\n", target_brightness);
    printf("sensor: %d\n", sensor_reflect);

    /* ステアリング操舵量を計算 */
    //steering_amount = (int16_t)(diff_brightness * STEERING_COEF);
    steering_amount = calculate_turn(target_brightness, 0);

    return steering_amount;
}

/* 走行モータ制御 */
static void motor_drive_control(int16_t steering_amount){

    int left_motor_power, right_motor_power; /*左右モータ設定パワー*/

    /* 左右モータ駆動パワーの計算(走行エッジを右にする場合はRIGHT_EDGEに書き換えること) */
    left_motor_power  = (int)(BASE_SPEED + (steering_amount * LEFT_EDGE));
    right_motor_power = (int)(BASE_SPEED - (steering_amount * LEFT_EDGE));

    /* 左右モータ駆動パワーの設定 */
#if 1
    motor_steer(60, steering_amount);
    //ev3_motor_set_power(left_motor, left_motor_power);
    //ev3_motor_set_power(right_motor, right_motor_power);
#else
    ev3_motor_set_power(left_motor, 0);
    ev3_motor_set_power(right_motor, 0);
#endif

    return;
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
    ie = ie + (dt + pre)*T/2;
    I = Ki * ie;

    // D制御
    de = (dt - pre)/T;
    D = Kd * de;

    res = P + I + D;

    printf("P: %.2f, I: %.2f, D: %.2f\n", P, I, D);
    return res;
}

static int16_t calculate_turn(uint16_t target_reflect, int trace_pos) {
    int16_t turn = 0;
    int16_t curb = 0;
    uint16_t sensor_reflect = 0;

    sensor_reflect = ev3_color_sensor_get_reflect(color_sensor);
    if (sensor_reflect > WHITE_BRIGHTNESS) {
        sensor_reflect = WHITE_BRIGHTNESS;
    }

    curb = PID(target_reflect, sensor_reflect);

    if (curb > 80) {
        curb = 80;
    } else if (curb < -80) {
        curb = -80;
    }

    //TODO: 左右で turn を逆にする
    turn = -curb;

    printf("turn: %d\n", turn);
    return turn;
}

static void motor_steer(int power, int16_t turn) {
    int left_power = 0;
    int right_power = 0;

    if (power > 100) {
        power = 100;
    } else if (power < -100) {
        power = -100;
    }

    if (turn > 100) {
        turn = 80;
    } else if (turn < -100) {
        turn = -80;
    }

    if (turn > 0) {
        left_power = power;
	right_power = (power * (100 - turn)) / 100;
    } else if (turn < 0) {
        left_power = (power * (100 + turn)) / 100;
	right_power = power;
    } else {
        left_power = power;
	right_power = power;
    }

    if (left_power < 5) {
        left_power = 5;
    } else if (right_power < 5) {
        right_power = 5;
    }

    printf("left_power : %d\n", left_power);
    printf("right_power: %d\n", right_power);

    ev3_motor_set_power(left_motor, left_power);
    ev3_motor_set_power(right_motor, right_power);

    return;
}
