#ifndef __EXTERN_H__
#define __EXTERN_H__

/* common.c の extern 宣言 */
extern const char *getFileName(const char *path);
void getDateTime(char **data);
extern void out_log_file(uint8_t level, const char *file, const int32_t line, const char *format, ...);

/* LineTracer.c の extern 宣言*/
extern int16_t steering_amount_calculation(void);
extern int16_t calculate_turn(uint16_t target_reflect, int trace_pos);
extern void motor_steer(int power, int16_t turn);
extern void motor_rotate_spec_count(int left_power, int right_power, int32_t count);
extern void motor_rotate(int power, int degree);
extern void motor_move(int power, int cm);
extern void deb_remove_turn(int turn);
extern void deb_remove_straight();

/* LineTracer_lib.c の extern 宣言 */
extern void change_target_ref_dev_remove(void);
extern void change_target_reflect(int color_code);
extern int change_trace_pos(int trace_pos);
extern void set_motor_power(int left_power, int right_power);
extern void set_motor_power_v2(int left_power, int right_power);
extern void reset_motor_counts(void);
extern void motor_stop(void);

/* color.c の extern 宣言 */
extern int8_t get_color(int8_t code);

/* test.c の extern 宣言 */
extern void test_main(int8_t type);

#endif
