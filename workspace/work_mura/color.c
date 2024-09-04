#include <string.h>

#include <app.h>
#include <color.h>
#include <common.h>
#include <extern.h>

const THRE_COLOR_t thre_color[COLOR_CODE_MAX] = {
/*   r  , g  , b */
    {90 , 30 , 40 }, /* 赤 */
    {20 , 40 , 30 }, /* 青 */
    {40 , 50 , 50 }, /* 緑 */
    {100, 90 , 80 }, /* 黄 */
    {25 , 25 , 25 }, /* 黒 */
    {100, 100, 100}  /* 白 */
};

static int8_t judge_color(rgb_raw_t rgb, int8_t code) {
    switch (code) {
        case COLOR_CODE_RED:
            if (rgb.r > thre_color[code].r &&
                rgb.g < thre_color[code].g &&
                rgb.b < thre_color[code].b) {
                return COLOR_CODE_RED;
            }
            break;
        case COLOR_CODE_BLUE:
            if (rgb.b > thre_color[code].b &&
                rgb.b > 3.5 * rgb.r &&
                rgb.b > 1.2 * rgb.g) {
                return COLOR_CODE_BLUE;
            }
            break;
        case COLOR_CODE_GREEN:
            if (rgb.r < thre_color[code].r &&
                rgb.g > thre_color[code].g &&
                rgb.b < thre_color[code].b) {
                return COLOR_CODE_GREEN;
            }
            break;
        case COLOR_CODE_YELLOW:
            if (rgb.r > thre_color[code].r &&
                rgb.g > thre_color[code].g &&
                rgb.b < thre_color[code].b) {
                return COLOR_CODE_YELLOW;
            }
            break;
        case COLOR_CODE_BLACK:
            if (rgb.r < thre_color[code].r &&
                rgb.g < thre_color[code].g &&
                rgb.b < thre_color[code].b) {
                return COLOR_CODE_BLACK;
            }
            break;
        case COLOR_CODE_WHITE:
            if (rgb.r > thre_color[code].r &&
                rgb.g > thre_color[code].g &&
                rgb.b > thre_color[code].b) {
                return COLOR_CODE_WHITE;
            }
            break;
	default:
	    break;
    }

    return COLOR_CODE_MAX;
}

int8_t get_color(int8_t code) {
    int8_t ret = COLOR_CODE_MAX;
    rgb_raw_t rgb;

    ev3_color_sensor_get_rgb_raw(color_sensor, &rgb);
#ifdef MODE_TEST
    LOG_D_TEST("R:%u, G:%u, B:%u\n", rgb.r, rgb.g, rgb.b);
#endif

    if (code != COLOR_CODE_MAX) {
        /* 指定されているカラーコードと一致するかを判定する */
        return judge_color(rgb, code);
    } else {
        /* カラーコード未指定の場合は、すべてのカラーコードを検索して、
           一致したカラーコードを返す */
        for (code = COLOR_CODE_RED; code < COLOR_CODE_MAX; code++) {
            ret = judge_color(rgb, code);
            if (ret != COLOR_CODE_MAX) {
                return ret;
            }
        }
    }

    /* 一致するカラーコードが見つからなかった */
    return COLOR_CODE_MAX;
}
