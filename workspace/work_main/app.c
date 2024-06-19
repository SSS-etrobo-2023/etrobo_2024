/**
 ******************************************************************************
 ** ファイル名 : app.c
 **
 ** 概要 : 二輪差動型ライントレースロボットのTOPPERS/HRP3用Cサンプルプログラム
 **
 ** 注記 : sample_c4 (sample_c3にBluetooth通信リモートスタート機能を追加)
 ******************************************************************************
 **/

//#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ev3api.h"
#include "app.h"
#include "../etroboc_common/etroboc_ext.h"
#include "common.h"
#include "extern.h"
#include "color.h"
#include "test.h"

#if defined(BUILD_MODULE)
    #include "module_cfg.h"
#else
    #include "kernel_cfg.h"
#endif

#define DEBUG

#if defined(DEBUG)
    #define _debug(x) (x)
#else
    #define _debug(x)
#endif

#if defined(MAKE_BT_DISABLE)
    static const int _bt_enabled = 0;
#else
    static const int _bt_enabled = 1;
#endif

/**
 * シミュレータかどうかの定数を定義します
 */
#if defined(MAKE_SIM)
    static const int _SIM = 1;
#elif defined(MAKE_EV3)
    static const int _SIM = 0;
#else
    static const int _SIM = 0;
#endif

/**
 * Raspike環境か、EV3RT環境下を定義
 */
#if defined(MAKE_RASPIKE)
    static const int _RASPIKE = 1; /* RasPike用ビルド */
#elif defined(MAKE_EV3)
    static const int _RASPIKE = 0; /* EV3用ビルド */
#else
    static const int _RSAPIKE = 0;
#endif

/**
 * 左コース/右コース向けの設定を定義します
 * デフォルトは左コース(ラインの右エッジをトレース)です
 */
#if defined(MAKE_RIGHT)
    static const int _LEFT = 0;
    #define _EDGE -1
#else
    static const int _LEFT = 1;
    #define _EDGE 1
#endif

#define REFLECT_LOG_SIZE 255

static int      bt_cmd = 0;     /* Bluetoothコマンド 1:リモートスタート */

/* sample_c2マクロ */
#define SONAR_ALERT_DISTANCE 30 /* 超音波センサによる障害物検知距離[cm] */
/* sample_c4マクロ */
//#define DEVICE_NAME     "ET0"  /* Bluetooth名 sdcard:\ev3rt\etc\rc.conf.ini LocalNameで設定 */
//#define PASS_KEY        "1234" /* パスキー    sdcard:\ev3rt\etc\rc.conf.ini PinCodeで設定 */
#define CMD_START         '1'    /* リモートスタートコマンド */

/* LCDフォントサイズ */
#define CALIB_FONT (EV3_FONT_SMALL)
#define CALIB_FONT_WIDTH (6/*TODO: magic number*/)
#define CALIB_FONT_HEIGHT (8/*TODO: magic number*/)

/* 関数プロトタイプ宣言 */
static int sonar_alert(void);
static void _syslog(int level, char* text);
static void _log(char* text);

/* センサー、モーターの接続定義 */
extern const sensor_port_t touch_sensor;
extern const sensor_port_t color_sensor;
extern const sensor_port_t sonar_sensor;
extern const sensor_port_t gyro_sensor;

extern const motor_port_t arm_motor;
extern const motor_port_t right_motor;
extern const motor_port_t left_motor;
extern const motor_port_t center_motor;

FILE *bt = NULL;

/* メインタスク */
void main_task(intptr_t unused)
{
    int forward = 30; /* 前後進命令 */
    float turn;         /* 旋回命令 */

    /* LCD画面表示 (未実装) */
    //ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

    _log("HackEV sample_c4");
    if (_LEFT)  _log("Left course:");
    else        _log("Right course:");

    init_port();

    //テールモーター固定
    ev3_motor_stop(center_motor, true);

    if (_bt_enabled) {
        /* Open Bluetooth file */
        // 今後サポート予定。現状は動くか不明
        //bt = ev3_serial_open_file(EV3_SERIAL_BT);
        //assert(bt != NULL);

        /* Bluetooth通信タスクの起動 */
        //act_tsk(BT_TASK);
    }

    ev3_led_set_color(LED_ORANGE); /* 初期化完了通知 */

    _log("Go to the start, ready?");
    if (_SIM)   _log("Hit SPACE bar to start");
    else        _log("Tap Touch Sensor to start");

#if 0
    if (_bt_enabled) {
        fprintf(bt, "Bluetooth Remote Start: Ready.\n");
        fprintf(bt, "1 : start\n");
        //fprintf(bt, "2 : get light power\n");
        //fprintf(bt, "3 : get RGB every second\n");
    }
#endif

    /* スタート待機 */
    while(1) {
#if 1
#if 0
    	if (bt_cmd == 1) {
            break; /* リモートスタート */
        } 

        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1) {
            break; /* タッチセンサが押された */
        }
#endif
#if 0
        // デバッグ用。残しておく。
        if (bt_cmd == 1) {
            break; /* リモートスタート */
        } else if (bt_cmd == 2) {
            /* 光取得用のデバッグ処理 */
            rgb_raw_t dbg_raw;
            ev3_color_sensor_get_rgb_raw(color_sensor, &dbg_raw);
            LOG_D_DEBUG("R:%u, G:%u, B:%u\n", dbg_raw.r, dbg_raw.g, dbg_raw.b);
            LOG_D_DEBUG("Reflect:%d\n", ev3_color_sensor_get_reflect(color_sensor));
            bt_cmd = 0;
        } else if (bt_cmd == 3) {
            /* 1秒おきにRGB値取得/判定 */
            while (bt_cmd == 3) {
                rgb_raw_t dbg_raw;
                char color_msg[COLOR_CODE_UNKNOWN + 1][10] = {
                    "RED", "BULE", "GREEN", "YELLOW", "BLACK", "WHITE", "UNKNOWN"
                };
                int line_color = judge_color(&dbg_raw);

                LOG_D_DEBUG("R:%u, G:%u, B:%u, color:%s\n",
                            dbg_raw.r, dbg_raw.g, dbg_raw.b, color_msg[line_color]);

                tslp_tsk(1000 * 1000U);
            }
            bt_cmd = 0;
        }    

        if (ev3_touch_sensor_is_pressed(touch_sensor) == 1)
        {
            break; /* タッチセンサが押された */
        }
#endif
#else
        if (ev3_button_is_pressed(LEFT_BUTTON) ){
            LOG_D_DEBUG("LEFT  BUTTON is pushed\n");
        }
        if (ev3_button_is_pressed(RIGHT_BUTTON) ){
            LOG_D_DEBUG("RIGHT BUTTON is pushed\n");
        }
        if (ev3_button_is_pressed(UP_BUTTON) ){
            LOG_D_DEBUG("UP    BUTTON is pushed\n");
        }
        if (ev3_button_is_pressed(DOWN_BUTTON) ){
            LOG_D_DEBUG("DOWN  BUTTON is pushed\n");
        }
#endif
        tslp_tsk(10 * 1000U); /* 10msecウェイト */
    }

    init_motor();

    ev3_led_set_color(LED_GREEN); /* スタート通知 */
    tslp_tsk(1000 * 1000U); /* 10msecウェイト */

    /* ライントレース */

    /* コンフィグ 初期設定 */
    rgb_raw_t main_rgb;
    rgb_raw_t dbg_rgb;
    float sensor_reflect = 0;
   
#ifdef DOUBLE_LOOP
    int count = 0;
    int first_counter = 0;
    int blue_count = 0;
    int is_blue = 0;
    int wait_flame = 45;
    int interval = -1;
    int chg_flag = 0;
    float blue_rate = 1.5;
#endif
 
    /*
        最初のライントレースでは、
        右コースはラインの右、左コースはラインの左を黒寄りでトレース
    */
    TARGET_REFLECT_t target_reflect;
    target_reflect = change_target_reflect(COLOR_CODE_BLACK);
    int trace_pos = (COURSE_TYPE == RIGHT) ? RIGHT : LEFT;

#ifndef TYPE_TEST
    while(1) {
        if (isfound_red(&dbg_rgb) == COLOR_CODE_RED){
            //赤色を検出したのでライントレースを終了する
            LOG_D_DEBUG("red discoverd.\n");
            LOG_D_TEST("R:%u, G:%u, B:%u\n", dbg_rgb.r, dbg_rgb.g, dbg_rgb.b);

            break;
        }

#ifdef DOUBLE_LOOP
        ev3_color_sensor_get_rgb_raw(color_sensor, &main_rgb);

        //青かどうかの判定
        if((blue_rate * main_rgb.r < main_rgb.b ) && (blue_rate * main_rgb.g < main_rgb.b) && (is_blue == 0)
        && (main_rgb.r > THRE_R_OF_WHITE * 0.2 && main_rgb.g > THRE_G_OF_WHITE * 0.2 && main_rgb.b > THRE_B_OF_WHITE * 0.2)) {
            is_blue = 1;
            chg_flag = 1;
            LOG_D_DEBUG("isBlue, count: %d, blue_count; %d\n", count, blue_count);
        } else if(!((1.8 * main_rgb.r < main_rgb.b ) && (1.8 * main_rgb.g < main_rgb.b))
         && (main_rgb.r < THRE_R_OF_WHITE * 0.8 && main_rgb.g < THRE_G_OF_WHITE * 0.8 && main_rgb.b < THRE_B_OF_WHITE * 0.8)
         && (is_blue == 1)) {
            //5sec 青を検出しない
            is_blue = 0;
            blue_count++;
            LOG_D_DEBUG("isNotBlue, count: %d, blue_count; %d\n", count, blue_count);
        }
        //LOG_D_DEBUG("interval: %d\n", interval);

        if (blue_count == 1 && chg_flag == 1) {
            forward = 35;
            //反射基準値を切り替える -> 青を検出しやすくする
            pid[0] = 2.0;
            pid[1] = 1.0;
            pid[2] = 0.015;
            blue_rate = 1.8;
            target_reflect = change_target_reflect(COLOR_CODE_BLUE);
            chg_flag = 0;
        }

        if(blue_count == 3 && chg_flag == 1){
            sensor_reflect = ev3_color_sensor_get_reflect(color_sensor);
            //LOG_D_DEBUG("sensor_reflect: %d\ntarget_reflect.color: %d\ntarget_reflect.reflect: %d\n",
            //            sensor_reflect, target_reflect.color, target_reflect.reflect);
            //取得値が基準値よりも小さい = 黒(or青)にいるケースで切り替える想定
            if (true) {
                //トレース方向を切り替える
                trace_pos = change_trace_pos(trace_pos);
                blue_count++;
                chg_flag = 0;
            }
            //LOG_D_DEBUG("右回り中");
        }

        if(blue_count == 5 && count < wait_flame){
            if(chg_flag == 1){
                trace_pos = change_trace_pos(trace_pos);
                chg_flag = 0;
                LOG_D_DEBUG("aaa");
            }
            count++;
        }

        if(blue_count >= 5 && count == wait_flame){
            //反射基準値を切り替える
            LOG_D_DEBUG("bbb");
            trace_pos = change_trace_pos(trace_pos);
            count++;
        }
#endif

        turn = culculate_turn(target_reflect.reflect, trace_pos);
        motor_steer(forward, turn);

        tslp_tsk(2 * 1000U); /* 2msec周期起動 */
    }

    //ライントレース完了
    LOG_D_DEBUG("linetrace completed.\n");
#else
    // 動作確認用
    op_check();
#endif
    init_motor();
    tslp_tsk(100 * 1000U); /* 100msec 停止*/


#if 0
    if (_bt_enabled) {
        ter_tsk(BT_TASK);
        fclose(bt);
    }
#endif

    ext_tsk();
}

//*****************************************************************************
// 関数名 : sonar_alert
// 引数 : 無し
// 返り値 : 1(障害物あり)/0(障害物無し)
// 概要 : 超音波センサによる障害物検知
//*****************************************************************************
static int sonar_alert(void) {
    static unsigned int counter = 0;
    static int alert = 0;

    signed int distance;

    /* 約40msec周期毎に障害物検知  */
    if (++counter == 40/4) {
        /*
         * 超音波センサによる距離測定周期は、超音波の減衰特性に依存します。
         * NXTの場合は、40msec周期程度が経験上の最短測定周期です。
         * EV3の場合は、要確認
         */
        distance = ev3_ultrasonic_sensor_get_distance(sonar_sensor);
        if ((distance <= SONAR_ALERT_DISTANCE) && (distance >= 0))
        {
            alert = 1; /* 障害物を検知 */
        }
        else
        {
            alert = 0; /* 障害物無し */
        }
        counter = 0;
    }

    return alert;
}

//*****************************************************************************
// 関数名 : bt_task
// 引数 : unused
// 返り値 : なし
// 概要 : Bluetooth通信によるリモートスタート。 Tera Termなどのターミナルソフトから、
//       ASCIIコードで1を送信すると、リモートスタートする。
//*****************************************************************************
void bt_task(intptr_t unused) {
    while(1) {
        if (_bt_enabled) {
            uint8_t c = fgetc(bt); /* 受信 */
            switch(c) {
            case '1':
                bt_cmd = 1;
                break;
            case '2':
                bt_cmd = 2;
                break;
            case '3':
                bt_cmd = 3;
                break;
            default:
                break;
            }
            fputc(c, bt); /* エコーバック */
        }
    }
}

//*****************************************************************************
// 関数名 : _syslog
// 引数 :   int   level - SYSLOGレベル
//          char* text  - 出力文字列
// 返り値 : なし
// 概要 : SYSLOGレベルlebelのログメッセージtextを出力します。
//        SYSLOGレベルはRFC3164のレベル名をそのまま（ERRだけはERROR）
//        `LOG_WARNING`の様に定数で指定できます。
//*****************************************************************************
static void _syslog(int level, char* text) {
    static int _log_line = 0;
    if (_SIM) {
        syslog(level, text);
    } else {
        ev3_lcd_draw_string(text, 0, CALIB_FONT_HEIGHT*_log_line++);
    }
}

//*****************************************************************************
// 関数名 : _log
// 引数 :   char* text  - 出力文字列
// 返り値 : なし
// 概要 : SYSLOGレベルNOTICEのログメッセージtextを出力します。
//*****************************************************************************
static void _log(char *text) {
    _syslog(LOG_NOTICE, text);
}
