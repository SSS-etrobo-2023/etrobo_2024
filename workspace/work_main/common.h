#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include "app.h"

#define DOUBLE_LOOP
#undef DOUBLE_LOOP

/* 1:ERROR, 2:DEBUG, 3:TEST */
#define LOG_LEVEL 3

/* コースタイプの定義 */
#define RIGHT 1
#define LEFT  2
#define COURSE_TYPE RIGHT // 走行するコースタイプ

/* 走行パラメータの最大/最小値 */
#define POWER_MAX 100
#define POWER_MIN -100
#define TURN_MAX  100
#define TURN_MIN  -100
#define DEGREE_MAX 360
#define DEGREE_MIN -360

/* 車輪の直径(cm) */
// @memo 後で測定する
#define DIAMETER 10
#define PI 3

//#ifndef MAKE_BT_DISABLE
#if 0
#define LOG_D_TEST(format, ...)\
    if(LOG_LEVEL >= 3){\
	    fprintf(bt, "%s:L%-5d\n[TEST ]: ", __FILE__, __LINE__);\
	    fprintf(bt, format, ##__VA_ARGS__);\
    }

#define LOG_D_DEBUG(format, ...)\
    if(LOG_LEVEL >= 2){\
	    fprintf(bt, "%s:L%-5d\n[DEBUG]: ", __FILE__, __LINE__);\
	    fprintf(bt, format, ##__VA_ARGS__);\
    }

#define LOG_D_ERROR(format, ...)\
    if(LOG_LEVEL >= 1){\
	    fprintf(bt, "%s:L%-5d\n[ERROR]: ", __FILE__, __LINE__);\
	    fprintf(bt, format, ##__VA_ARGS__);\
    }
#else
#define LOG_D_TEST(format, ...)
#define LOG_D_DEBUG(format, ...)
#define LOG_D_ERROR(format, ...)
#endif

extern FILE *bt;

#endif
