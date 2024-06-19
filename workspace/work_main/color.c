#include <string.h>

#include "ev3api.h"
#include "common.h"
#include "extern.h"
#include "color.h"

/* センサー、モーターの接続定義 */
extern const sensor_port_t touch_sensor;
extern const sensor_port_t color_sensor;
extern const sensor_port_t sonar_sensor;
extern const sensor_port_t gyro_sensor;

extern const motor_port_t arm_motor;
extern const motor_port_t right_motor;
extern const motor_port_t left_motor;
extern const motor_port_t center_motor;

#if 1 //このあたりのソースは整理したい
//*****************************************************************************
// 関数名 : isfound_red
// 引数   : *dbg_rbg - 取得した値を格納(デバッグ用)
// 返り値 : カラーコード
// 概要   : 
//*****************************************************************************
int isfound_red(rgb_raw_t *dbg_rgb) {
    rgb_raw_t read_rgb;
    ev3_color_sensor_get_rgb_raw(color_sensor, &read_rgb);

    if (NULL != dbg_rgb) {
        memcpy(dbg_rgb, &read_rgb, sizeof(rgb_raw_t));
    }

    if ((read_rgb.r >= THRE_R_OF_RED) && (read_rgb.r > 1.5 * read_rgb.b) && (read_rgb.r > 1.5 * read_rgb.g)) {
        return COLOR_CODE_RED;
    }

    return COLOR_CODE_UNKNOWN;
}

int judge_color(rgb_raw_t *dbg_rgb) {
    int ret = COLOR_CODE_UNKNOWN;
    rgb_raw_t read_rgb;
    ev3_color_sensor_get_rgb_raw(color_sensor, &read_rgb);

    if (read_rgb.r > 100 && read_rgb.g > 100 && read_rgb.b > 100) {
        ret = COLOR_CODE_WHITE;
    } else if (read_rgb.r < 25 && read_rgb.g < 30 && read_rgb.b < 25) {
        ret = COLOR_CODE_BLACK;
    } else if (read_rgb.r > 100 && read_rgb.r > 1.5 * read_rgb.g && read_rgb.r > 1.5 * read_rgb.b) {
        ret = COLOR_CODE_RED;
    } else if (read_rgb.g > 60 && read_rgb.g > 2 * read_rgb.r && read_rgb.g > 1.7 * read_rgb.b) {
        ret = COLOR_CODE_GREEN;
    } else if (read_rgb.b > 100 && read_rgb.b > 3 * read_rgb.r && read_rgb.b > 2 * read_rgb.g) {
        ret = COLOR_CODE_BLUE;
    } else if (read_rgb.r > 100 && read_rgb.g > 100 &&
               read_rgb.r > 2 * read_rgb.b && read_rgb.g > 2 * read_rgb.b) {
        ret = COLOR_CODE_YELLOW;
    }

    if (NULL != dbg_rgb) {
        memcpy(dbg_rgb, &read_rgb, sizeof(rgb_raw_t));
    }

    return ret;
}

//*****************************************************************************
// 関数名 : ret_color_code
// 引数 :   rgb_raw_t *ret_rgb - 取得した値を格納(デバッグ用)
// 返り値 : COLOR_CODEの整数
// 概要 :　
//*****************************************************************************
int ret_color_code(rgb_raw_t *ret_rgb) {
    
    int ret;
    rgb_raw_t read_rgb;
    ev3_color_sensor_get_rgb_raw(color_sensor, &read_rgb);

    if (read_rgb.r>THRE_R_OF_WHITE && read_rgb.g>THRE_G_OF_WHITE && read_rgb.b>THRE_B_OF_WHITE) {
        //白の時
        ret= COLOR_CODE_WHITE;
        //LOG_D_DEBUG("WHITE  r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);

    } else if (read_rgb.r<THRE_R_OF_BLUE && read_rgb.g<THRE_G_OF_BLUE && read_rgb.b>THRE_B_OF_BLUE) {
        //青の時
        ret= COLOR_CODE_BLUE;
        //LOG_D_DEBUG("BLUE   r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);

    } else if (read_rgb.r>THRE_R_OF_YELLOW && read_rgb.g>THRE_G_OF_YELLOW && read_rgb.b<THRE_B_OF_YELLOW) {
        //黄の時
        ret= COLOR_CODE_YELLOW;
        ///LOG_D_DEBUG("YELLOW r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);

    } else if (read_rgb.r>THRE_R_OF_RED && read_rgb.g<THRE_G_OF_RED && read_rgb.b<THRE_B_OF_RED) {
        //赤の時
        ret= COLOR_CODE_RED;
        //LOG_D_DEBUG("RED    r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);

    } else if (read_rgb.r<THRE_R_OF_GREEN && read_rgb.g>THRE_G_OF_GREEN && read_rgb.b<THRE_B_OF_GREEN) {
        //緑の時
        ret= COLOR_CODE_GREEN;
        //LOG_D_DEBUG("GREEN  r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);

    } else if (read_rgb.r < THRE_R_OF_BLACK && read_rgb.g < THRE_G_OF_BLACK && read_rgb.b < THRE_B_OF_BLACK) {
        ret = COLOR_CODE_BLACK;
    } else {
        //不明の時
        ret= COLOR_CODE_UNKNOWN;
        //LOG_D_DEBUG("UNKNOWN r= %u g= %u b=%u \n",read_rgb.r, read_rgb.g, read_rgb.b);
    }

    if (NULL != ret_rgb) {
        memcpy(ret_rgb, &read_rgb, sizeof(rgb_raw_t));
    }

    return ret;

}
#endif
