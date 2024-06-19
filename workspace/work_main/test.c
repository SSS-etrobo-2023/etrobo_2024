#include <unistd.h>
#include "common.h"
#include "extern.h"

/* 動作確認用 */
void op_check() {
    // 直進
    motor_steer(30, 0);

    tslp_tsk(3000 * 1000U); /* 3sec ウェイト */

    // 右に90度だけ回転
    motor_rotate(30, 90);

    tslp_tsk(3000 * 1000U); /* 3sec ウェイト */

    // 直進
    motor_steer(30, 0);

    // 左に90度だけ回転
    motor_rotate(30, -90);

    tslp_tsk(3000 * 1000U); /* 3sec ウェイト */

    // 100cm 移動
    motor_move(30, 100);

    //終了
    init_motor();

    return;
}
