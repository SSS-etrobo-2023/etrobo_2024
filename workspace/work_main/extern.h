#ifndef __EXTERN_H__
#define __EXTERN_H__

#include "ev3api.h"
#include "../etroboc_common/etroboc_ext.h"
#include "app.h"
#include "color.h"

#if defined(BUILD_MODULE)
    #include "module_cfg.h"
#else
    #include "kernel_cfg.h"
#endif

// common.c の extern 宣言
extern TARGET_REFLECT_t change_target_reflect(int color_code);
extern int change_trace_pos(int now_trace_pos);
extern void init_port();
extern void init_motor();

// run.c の extern 宣言
extern float culculate_turn(unsigned int target_reflect, int trace_pos);
extern void motor_steer(int power, float turn);
extern void motor_rotate(int power, int degree);
extern void motor_move(int power, int cm);
extern void motor_stop(void);

// color.c の extern 宣言
extern int isfound_red(rgb_raw_t *dbg_rgb);
extern int judge_color(rgb_raw_t *dbg_rgb);
extern int ret_color_code(rgb_raw_t *ret_rgb);

// app.c の extern 宣言
TARGET_REFLECT_t change_target_reflect(int color_code);
int change_trace_pos(int now_trace_pos);

#endif
