#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>

enum color_code {
    COLOR_CODE_RED = 0,
    COLOR_CODE_BLUE,
    COLOR_CODE_GREEN,
    COLOR_CODE_YELLOW,
    COLOR_CODE_BLACK,
    COLOR_CODE_WHITE,
    COLOR_CODE_MAX
};

/* 各色の閾値 */
typedef struct {
    uint8_t r; /* 赤 */
    uint8_t g; /* 緑 */
    uint8_t b; /* 青 */
} THRE_COLOR_t;

extern const THRE_COLOR_t thre_color[COLOR_CODE_MAX];

#endif
