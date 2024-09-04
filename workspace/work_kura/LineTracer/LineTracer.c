#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "app.h"
#include "color.h"
#include "LineTracer.h"
#include "common.h"
#include "extern.h"

uint16_t target_ref = 0;

/* トレース位置 */
static int t_pos = COURSE_TYPE;

// 制御定数
// @memo この値は適宜調整する
static int init_power = 60;
//static const float T = LINE_TRACER_PERIOD / (1000 * 1000);
static const float T = 0.01;
#ifdef REF_BY_RGB
static const float Kp = 2.5;
#else
static const float Kp = 1.5;
#endif
static const float Ki = 0.1;
static const float Kd = 0.03;

static int8_t isBlue = false;
static int8_t phase = PHASE_START;

/* デブリリムーバブル用フラグ */
static int8_t deb_cnt = 0;
static uint32_t pwr_cnt = 100;
static int8_t isStart = 0;
static int8_t isOnCircle = false;
static int8_t isOnLine = false;

char color_str[COLOR_CODE_MAX + 1][10] = {
    "RED", "BLUE", "GREEN", "YELLOW", "BLACK", "WHITE", "UNKNOWN"
};

/* EBD_DEB の前は STRAIGHT にすること */
#if 0 /* for test */
const int8_t deb_order[] = {
    TURN_LEFT,
    STRAIGHT,
    TURN_RIGHT,
    TURN_RIGHT,
    STRAIGHT,
    TURN_LEFT,
    STRAIGHT,
    STRAIGHT,
    END_DEB
};
#else
const int8_t deb_order[] = {
    //STRAIGHT,
    TURN_LEFT,
    STRAIGHT,
    TURN_RIGHT,
    STRAIGHT,
    END_DEB
};

#endif
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

            ext_tsk();
            break;
        case LINE_TRACE:
            color_code = get_color(COLOR_CODE_BLUE);
            if (color_code == COLOR_CODE_BLUE) {
                LOG_D_DEBUG("Blue Line found.\n");
                motor_stop();
                isBlue = true;
                phase = DOUBLE_LOOP;
                //change_target_reflect(COLOR_CODE_BLUE);

                ext_tsk();
            }
            break;
        case DOUBLE_LOOP:
            if (isBlue) {
                color_code = get_color(COLOR_CODE_BLACK);
                if (color_code == COLOR_CODE_BLACK) {
                    LOG_D_DEBUG("Black Line found.\n");
                    motor_stop();
                    isBlue = false;
                    phase = DEBRI_REMOVE;
                    isOnLine = true;
                    //change_target_reflect(COLOR_CODE_BLACK);
                    init_power = 40;

                    ext_tsk();
                }
            }
            break;
        case DEBRI_REMOVE:
            if (isOnCircle) {
                switch (deb_order[deb_cnt]) {
                    case STRAIGHT:
                        deb_remove_straight();
                        break;
                    case TURN_RIGHT:
                        deb_remove_turn(RIGHT);
                        break;
                    case TURN_LEFT:
                        deb_remove_turn(LEFT);
                        break;
                    case END_DEB:
                        phase = PHASE_END;
                        LOG_D_DEBUG("end run.\n");
                        break;
                }

                deb_cnt++;
                isStart = 1;
                pwr_cnt = 0;
                isOnCircle = false;
                isOnLine = false;
            } else if (isOnLine) {
                if (!isStart) {
                    1+1;
                } else if (pwr_cnt++ < 10) {
                    LOG_D_DEBUG("TEST(1) pwr_cnt: %d!!!\n", pwr_cnt);
                    init_power = 65;
                } else {
                    init_power = 35;
                }

                color_code = get_color(COLOR_CODE_MAX);
                if (color_code != COLOR_CODE_BLACK &&
                    color_code != COLOR_CODE_WHITE &&
                    color_code != COLOR_CODE_MAX) {
                    LOG_D_DEBUG("circle found. color: %s\n", color_str[color_code]);
                    motor_stop();
                    isOnCircle = true;
                    isOnLine = false;

                    ext_tsk();
                }
            } else {
                if (pwr_cnt++ < 30) {
                    init_power = 65;
                } else {
                    init_power = 35;
                }

                color_code = get_color(COLOR_CODE_BLACK);
                if (color_code == COLOR_CODE_BLACK) {
                    LOG_D_DEBUG("Black Line found.\n");
                    isOnCircle = false;
                    isOnLine = true;
                    pwr_cnt = 0;
                }
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
    test_main(0);
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
        if (right_power < (power/10)) {
            right_power = power/10;
        }
    } else if (turn < 0) {
        /* 左転回 */
        left_power  = (power * (100 + turn)) / 100;
	right_power = power;
        if (left_power < (power/10)) {
            left_power = power/10;
        }
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
    int32_t init_l_cnt = 0;
    int32_t init_r_cnt = 0;
    int32_t left_count = 0;
    int32_t right_count = 0;
    //int8_t l_end = 0, r_end = 0;

    init_l_cnt = ev3_motor_get_counts(left_motor);
    init_r_cnt = ev3_motor_get_counts(right_motor);
    //LOG_D_DEBUG("init_left_motor: %ld deg, init_right_motor: %ld deg\n", init_l_cnt, init_r_cnt);
    motor_stop();

    reset_motor_counts();
    /* motor_count が reset されるまで待つ */
    while(1) {
        left_count = ev3_motor_get_counts(left_motor);
        right_count = ev3_motor_get_counts(right_motor);
        if (abs(left_count) < abs(init_l_cnt) && abs(right_count) < abs(init_r_cnt)) {
            LOG_D_DEBUG("START: left_motor: %ld deg, right_motor: %ld deg\n", left_count, right_count);
            init_l_cnt = left_count;
            init_r_cnt = right_count;
            break;
        }
    }

    set_motor_power(left_power, right_power);

    while(1) {
        left_count = ev3_motor_get_counts(left_motor);
        right_count = ev3_motor_get_counts(right_motor);

        /* @memo: この判定は実動作で確認して調整すること */
        if (abs(left_count - init_l_cnt) >= abs(count) &&
            abs(right_count - init_r_cnt) >= abs(count)) {
            LOG_D_DEBUG("END: left_motor: %ld deg, right_motor: %ld deg\n", left_count, right_count);
            LOG_D_DEBUG("left_motor: %ld deg, right_motor: %ld deg\n",
                        abs(left_count - init_l_cnt), abs(right_count - init_r_cnt));
            break;
        }
    }

    /* 回転終了後は停止状態にする */
    //motor_stop();

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
    degree = (180 * cm) / (DIAMETER * PI * 2);

    motor_rotate_spec_count(power, power, degree);

    return;
}

/* サークルを転回する */
void deb_remove_turn(int turn) {
    int8_t color_code = COLOR_CODE_MAX;
    int8_t deg_1st = 45;
    int8_t deg_2nd = 45;

    if (LEFT == turn) {
        deg_1st *= -1;
        deg_2nd *= -1;
    } else {
        //deg_1st = 30;
        //deg_2nd = 25;
    }

    /* 45度回転 */
    motor_rotate(50, deg_1st);

    /* 少し進む(誤検知防止) */
    motor_move(50, 3);

    set_motor_power(55, 50);

    /* 黒線を見つけるまで直進 */
    while (1) {
        color_code = get_color(COLOR_CODE_BLACK);
        if (color_code == COLOR_CODE_BLACK) {
            LOG_D_DEBUG("Black found.\n");
            break;
        }
    }

    /* さらに45度回転 */
    motor_rotate(50, deg_2nd);

    /* 左回転の後は左追っかけ、
       右回転の後は右追っかけになるよう設定 */
    t_pos = change_trace_pos(turn == LEFT ? RIGHT : LEFT);

    set_motor_power(0, 0);

    return;
}

/* サークルを直進する */
void deb_remove_straight(void) {
#ifndef PRODUCTION
    motor_move(65, 10);
#else
    motor_move(65, 9);
#endif
    set_motor_power(0, 0);

    return;
}
