#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <unistd.h>

#include "extern.h"

/* 本番環境 */
//#define PRODUCTION

/* テストモード */
//#define MODE_TEST

/* ファイルにログ出力するかどうか */
//#define OUTLOG_FILE

/* ログファイル名 */
#define LOG_FILE "/tmp/log.txt"

/* RGB値による反射値の計算を行うか */
#define REF_BY_RGB

/* コースタイプの定義 */
#define RIGHT 1
#define LEFT  2
#define COURSE_TYPE RIGHT // 走行するコースタイプ

/* 走行パラメータの最大/最小値 */
#define POWER_MAX  100
#define POWER_MIN  -100
#define TURN_MAX   100
#define TURN_MIN   -100
#define CURB_MAX   100
#define CURB_MIN   -100
#define DEGREE_MAX 360
#define DEGREE_MIN -360

#define LIGHT_BLACK 5    /* 黒色の光センサ値 */
#define LIGHT_WHITE 95   /* 白色の光センサ値 */
#define LIGHT_BLUE  25   /* 青色の光センサ値 */

/* RGB値による反射値 */
#ifndef PRODUCTION
#define LIGHT_BLACK_RGB 5    /* 黒色の光センサ値 */
#define LIGHT_WHITE_RGB 60   /* 白色の光センサ値 */
#define LIGHT_BLUE_RGB  12   /* 青色の光センサ値 */
#else
/* 本番環境 */
#define LIGHT_BLACK_RGB 5    /* 黒色の光センサ値 */
#define LIGHT_WHITE_RGB 60   /* 白色の光センサ値 */
#define LIGHT_BLUE_RGB  15   /* 青色の光センサ値 */
#endif

/* 車輪の半径(cm) */
// TODO:後で測定する
#define DIAMETER 5
#define PI 3

/*
  ログレベル
  1:ERROR, 2:DEBUG, 3:TEST
*/
#define LOG_LEVEL 2

#ifndef OUTLOG_FILE
#define LOG_D_TEST(format, ...)\
    if (LOG_LEVEL >= 3) {\
        printf("[TEST :%10s:L%-5d]: ", getFileName(__FILE__), __LINE__);\
        printf(format, ##__VA_ARGS__);\
    }

#define LOG_D_DEBUG(format, ...)\
    if (LOG_LEVEL >= 2) {\
        printf("[DEBUG:%10s:L%-5d]: ", getFileName(__FILE__), __LINE__);\
        printf(format, ##__VA_ARGS__);\
    }


#define LOG_D_ERROR(format, ...)\
    if (LOG_LEVEL >= 1) {\
        printf("[ERROR:%10s:L%-5d]: ", getFileName(__FILE__), __LINE__);\
        printf(format, ##__VA_ARGS__);\
    }
#else
#define LOG_D_TEST(format, ...)\
    if (LOG_LEVEL >= 3) {\
        out_log_file(3, __FILE__, __LINE__, format, ##__VA_ARGS__);\
    }

#define LOG_D_DEBUG(format, ...)\
    if (LOG_LEVEL >= 2) {\
        out_log_file(2, __FILE__, __LINE__, format, ##__VA_ARGS__);\
    }


#define LOG_D_ERROR(format, ...)\
    if (LOG_LEVEL >= 1) {\
        out_log_file(1, __FILE__, __LINE__, format, ##__VA_ARGS__);\
    }
#endif

enum phase_code {
    PHASE_START = 0,
    LINE_TRACE,
    DOUBLE_LOOP,
    DEBRI_REMOVE,
    SMART_CARRY,
    PHASE_END
};

#endif
