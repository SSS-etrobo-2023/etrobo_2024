#ifndef __COLOR_H__
#define __COLOR_H__

#define LIGHT_BLACK 2    /* 黒色の光センサ値 */
#define LIGHT_WHITE 40   /* 白色の光センサ値 */

/* カラーコード定義 */
enum color_code{
    COLOR_CODE_RED = 0,
    COLOR_CODE_BLUE,
    COLOR_CODE_GREEN,
    COLOR_CODE_YELLOW,
    COLOR_CODE_BLACK,
    COLOR_CODE_WHITE,
    COLOR_CODE_UNKNOWN
};

/*
ライントレースの目標値となる光の反射値
右コース かつ 左トレース ... 白より
右コース かつ 右トレース ... 黒より (右コース初期値)
左コース かつ 左トレース ... 黒より (左コース初期値)
左コース かつ 右トレース ... 白より
*/
typedef struct {
    unsigned int color;
    unsigned int reflect;
} TARGET_REFLECT_t;

/* 色彩センサ　閾値 */
/* 青 */
#define THRE_B_OF_BLUE 100
#define THRE_G_OF_BLUE 60
#define THRE_R_OF_BLUE 15

/* 赤 */
#define THRE_B_OF_RED 75
#define THRE_G_OF_RED 75
#define THRE_R_OF_RED 90

/* 緑 */
#define THRE_B_OF_GREEN 30
#define THRE_G_OF_GREEN 90
#define THRE_R_OF_GREEN 60

/* 黄 */
#define THRE_B_OF_YELLOW 150
#define THRE_G_OF_YELLOW 150
#define THRE_R_OF_YELLOW 60

/* 黒 */
#define THRE_B_OF_BLACK 25
#define THRE_G_OF_BLACK 25
#define THRE_R_OF_BLACK 25

/* 白 */
#define THRE_B_OF_WHITE 100
#define THRE_G_OF_WHITE 100
#define THRE_R_OF_WHITE 100

#endif
