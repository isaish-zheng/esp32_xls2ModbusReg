/**
 ******************************************************************************
 * @file        lgf.h
 * @author      zyd
 * @version
 * @date
 * @brief       Library of General Functions
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#ifndef __L_G_F_H
#define __L_G_F_H

#include <stdbool.h>
#include <stddef.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "esp_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 小整数池 */
#define ZERO_INT 0
#define ONE_INT 1
#define TWO_INT 2
#define THREE_INT 3
#define ZERO_UINT 0U
#define ONE_UINT 1U
#define TWO_UINT 2U
#define THREE_UINT 3U
/* 浮点型值 */
#define ZERO_FLT 0.0F
#define ONE_FLT 1.0F
#define TWO_FLT 2.0F
#define THREE_FLT 3.0F
#define EPSILON_FLT 1E-6F
#define EPSILON_DBL 1E-9
/* 位 */
#define BIT_POS(x) (x##U)

/* 毫秒每秒 */
#define SECOND_IN_MILLISECONDS 1000.0F
/* 微秒每毫秒 */
#define MILLISECOND_IN_MICROSECONDS 1000.0F
/* 微秒每秒 */
#define SECOND_IN_MICROSECONDS 1000000.0F

/* 重力加速度 */
#define ACCELERATION_OF_GRAVITY 9.8F
/* 千克每吨 */
#define KILOGRAM_IN_TON 1000.0F

/* 未使用参数 */
#ifndef UNUSED
    #define UNUSED(x) (void)(x)
#endif

/* 获取指定位置的位(0基) */
#define lgf_bit_get(val, bit) (((val) >> (bit)) & 1U)
/* 设置指定位置的位(0基) */
#define lgf_bit_set(val, bit) ((val) |= (1U << (bit)))
/* 复位指定位置的位(0基) */
#define lgf_bit_reset(val, bit) ((val) &= ~(1U << (bit)))
/* 翻转指定位置的位(0基) */
#define lgf_bit_toggle(val, bit) ((val) ^= (1U << (bit)))

/* 计算两数中的较小值 */
#define lgf_min(a, b) ((a) < (b) ? (a) : (b))
/* 计算两数中的较大值 */
#define lgf_max(a, b) ((a) > (b) ? (a) : (b))

/* 当flag为true时，返回输入值的相反数，否则返回原值 */
#define lgf_negative_if(val, flag) (flag ? -val : val)

/* 读取系统启动后的运行时间 us */
int64_t read_system_time_us(void);
/* 读取系统启动后的运行时间 ms */
int64_t read_system_time_ms(void);

/* 滞回继电器 */
void lgf_relay(float u, bool *y, float rst, float set);

/* 判断单精度浮点型是否为零  */
bool lgf_is_zero_flt(float u);
/* 判断双精度浮点型是否为零  */
bool lgf_is_zero_dbl(double u);

/* 翻转一个数组的元素存储到另一个数组中 */
void lgf_flip_array(void *dest_arr, const void *src_arr, size_t size,
                    size_t elem_size);

/* 查找数组中的最值及其索引 */
void lgf_get_array_min_max(const float *arr, uint32_t size,
                           float *min, uint32_t *min_index,
                           float *max, uint32_t *max_index);

/* 输出自调用以来读取的输入的最小值、最大值 */
void lgf_store_min_max(float val, bool reset, float *min, float *max);

/* 将输入值x按照给定的两个点(x1, y1)和(x2,
 * y2)进行线性插值，并且结果会被限制在[y_min, y_max]区间内 */
float lgf_scale_linear(float x, float x1, float y1, float x2, float y2,
                       float y_min, float y_max);

/* 计算电机输出功率 */
float lgf_calc_motor_out_power(float torque, float rpm);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

    /****************EOF****************/
