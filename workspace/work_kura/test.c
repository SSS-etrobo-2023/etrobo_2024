#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include "app.h"
#include "color.h"
#include "common.h"
#include "extern.h"

static uint8_t count = 0;
static const uint16_t count_max = 1000 * 1000 / LINE_TRACER_PERIOD; /* 1秒周期 */

char code_str[COLOR_CODE_MAX + 1][10] = {
    "RED", "BLUE", "GREEN", "YELLOW", "BLACK", "WHITE", "UNKNOWN"
};

void test_main(int8_t type) {
    int color_code;

    if (count++ <= count_max) {
        /* 1sおきに実行するように調整 */
        return;
    } else {
        count = 0;
    }

    if (type == 0) {
        /* カラーセンサのテスト */
        color_code = get_color(COLOR_CODE_MAX);
        LOG_D_TEST("code: %s\n", code_str[color_code]);
    } else if (type == 1) {
        /* 反射光のテスト */
        LOG_D_TEST("reflect: %d\n", ev3_color_sensor_get_reflect(color_sensor));
    } else if (type == 2) {
        /* RGB値による反射光を求めるテスト */
        rgb_raw_t rgb;
        uint16_t reflect;

        ev3_color_sensor_get_rgb_raw(color_sensor, &rgb);

        reflect = floor(cbrt((rgb.r * 100 / 255) * (rgb.b * 100 / 255) * (rgb.b * 100 / 255)));
        LOG_D_DEBUG("reflect: %d\n", reflect);
    } else if (type == 3) {
        /* 動作確認 */
        motor_move(75, 30); /* 30cm 前進 */

        sleep(1);

        motor_rotate(75, 90); /* 90度右に回転 */

        sleep(1);
    }

    return;
}
