#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#include "extern.h"

/* コースタイプの定義 */
#define RIGHT 1
#define LEFT  2
#define COURSE_TYPE RIGHT // 走行するコースタイプ

/* 走行パラメータの最大/最小値 */
#define POWER_MAX  100
#define POWER_MIN  -100
#define TURN_MAX   100
#define TURN_MIN   -100
#define CURB_MAX   80
#define CURB_MIN   -80
#define DEGREE_MAX 360
#define DEGREE_MIN -360

#define LIGHT_BLACK 5    /* 黒色の光センサ値 */
#define LIGHT_WHITE 95   /* 白色の光センサ値 */

/* 車輪の直径(cm) */
// TODO:後で測定する
#define DIAMETER 10
#define PI 3

/*
  ログレベル
  1:ERROR, 2:DEBUG, 3:TEST
*/
#define LOG_LEVEL 3

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

#endif
