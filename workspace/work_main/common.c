#include <stdio.h>
#include "ev3api.h"
#include "../etroboc_common/etroboc_ext.h"
#include "common.h"
#include "color.h"

/* センサー、モーターの接続定義 */
const sensor_port_t touch_sensor = EV3_PORT_1;
const sensor_port_t color_sensor = EV3_PORT_2;
const sensor_port_t sonar_sensor = EV3_PORT_3;
const sensor_port_t gyro_sensor  = EV3_PORT_4;

const motor_port_t arm_motor    = EV3_PORT_A;
const motor_port_t right_motor  = EV3_PORT_B;
const motor_port_t left_motor   = EV3_PORT_C;
const motor_port_t center_motor = EV3_PORT_D;

/* 反射基準値 */
TARGET_REFLECT_t target_reflect_def[3] = {
    {COLOR_CODE_BLACK, (LIGHT_WHITE * 1) / 5},
    {COLOR_CODE_WHITE, (LIGHT_WHITE * 2) / 3},
    {COLOR_CODE_BLUE,  (LIGHT_WHITE * 2) / 5}
};

//*****************************************************************************
// 関数名 : change_target_reflect
// 引数   : color_code - トレースする色
// 返り値 : 反射基準値を含む構造体
// 概要   : 引数で指定した色から、事前に定義した構造体を返す
//*****************************************************************************
TARGET_REFLECT_t change_target_reflect(int color_code) {
    int i = 0;

    for (i = 0; sizeof(target_reflect_def) / sizeof(target_reflect_def[0]); i++) {
        if (target_reflect_def[i].color == color_code) {
            LOG_D_DEBUG("target reflect changd. -> %d\n", target_reflect_def[i].reflect);
            return target_reflect_def[i];
        }
    }

    //デフォルトでは黒に近いラインをトレースするようにする
    return target_reflect_def[0];
}

//*****************************************************************************
// 関数名 : change_trace_pos
// 引数   : now_trace_pos - 現在のトレース方向
// 返り値 : 新しいトレース方向
// 概要   : 現在の逆となる方向を返す
//*****************************************************************************
int change_trace_pos(int now_trace_pos) {
    LOG_D_DEBUG("trace_pos changed. %s -> %s\n",
                now_trace_pos == RIGHT ? "RIGHT" : "LEFT",
                now_trace_pos == RIGHT ? "LEFT" : "RIGHT");
    return now_trace_pos == RIGHT ? LEFT : RIGHT;
}

/* ポート設定初期化 */
void init_port() {
    /* センサー入力ポートの設定 */
    ev3_sensor_config(sonar_sensor, ULTRASONIC_SENSOR);
    ev3_sensor_config(color_sensor, COLOR_SENSOR);
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_color_sensor_get_reflect(color_sensor); /* 反射率モード */
    
    /* モーター出力ポートの設定 */
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);
    ev3_motor_config(center_motor, LARGE_MOTOR);

    return;
}

/* モータ初期化 */
void init_motor() {
    ev3_motor_stop(left_motor, false);
    ev3_motor_stop(right_motor, false);

    ev3_motor_set_power(left_motor, 0);
    ev3_motor_set_power(right_motor, 0);

    ev3_motor_reset_counts(left_motor);
    ev3_motor_reset_counts(right_motor);

    return;
}
