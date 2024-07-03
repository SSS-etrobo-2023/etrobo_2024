#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "app.h"
#include "color.h"
#include "common.h"
#include "extern.h"

static uint8_t count = 0;
static const uint16_t count_max = 1000 * 1000 / LINE_TRACER_PERIOD; /* 1秒周期 */

#if 0
char code_str[COLOR_CODE_MAX + 1][10] = {
    {"RED\0", "BULE\0", "GREEN\0", "YELLOW\0", "WHITE\0", "BLACK\0", "UNKNOWN\0"}
};
#endif

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
        //LOG_D_TEST("code: %s\n", code_str[color_code]);
    } else if (type == 1) {
        /* 反射光のテスト */
        LOG_D_TEST("reflect: %d\n", ev3_color_sensor_get_reflect(color_sensor));
    } else if (type == 2) {
        /* 動作確認 */
        motor_move(75, 30); /* 30cm 前進 */

        sleep(1);

        motor_rotate(75, 90); /* 90度右に回転 */

        sleep(1);
    }

    return;
}
