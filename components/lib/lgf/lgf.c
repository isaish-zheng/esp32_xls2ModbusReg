/**
 ******************************************************************************
 * @file        lgf.c
 * @author      zyd
 * @version
 * @date
 * @brief       utilities
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "lgf.h"

/**
 * @brief 读取系统启动后的运行时间 us
 * @param  
 * @retval 系统运行时间 ms
 * @note 
 */
int64_t read_system_time_us(void) {
    return esp_timer_get_time();
}

/**
 * @brief 读取系统启动后的运行时间 ms
 * @param  
 * @retval 系统运行时间 ms
 * @note 
 */
int64_t read_system_time_ms(void) {
    return read_system_time_us() / MILLISECOND_IN_MICROSECONDS;
}

/**
 * @brief 滞回继电器
 * @param[in] u: 输入值
 * @param[out] y: 输出值
 * @param[in] rst: 复位阈值
 * @param[in] set: 置位阈值
 * @retval 
 * @note rst < set
 */
void lgf_relay(float u, bool *y, float rst, float set) {
    if (rst - set >= 1E-6F) {
        *y = false;
        return;
    }
    *y = ((u >= set) || ((u > rst) && *y));
}

/**
 * @brief 判断单精度浮点型是否为零
 * @param[in] u: 输入值
 * @retval true or false
 * @note
 */
bool lgf_is_zero_flt(float u) {
    return fabs(u) < EPSILON_FLT;
}

/**
 * @brief 判断双精度浮点型是否为零
 * @param[in] u: 输入值
 * @retval true or false
 * @note
 */
bool lgf_is_zero_dbl(double u) {
    return fabs(u) < EPSILON_DBL;
}

/**
 * @brief 翻转一个数组的元素存储到另一个数组中
 * @param[out] dest_arr 输出数组指针
 * @param[in] src_arr 输入数组指针
 * @param[in] size 元素个数
 * @param[in] elem_size 单个元素字节数
 */
void lgf_flip_array(void *dest_arr, const void *src_arr, size_t size,
                    size_t elem_size) {
    if (dest_arr == NULL || src_arr == NULL || size <= 0 || elem_size <= 0)
        return;

    uint8_t *src = (uint8_t *)src_arr;
    uint8_t *dest = (uint8_t *)dest_arr + (size - 1) * elem_size;

    for (size_t i = 0; i < size; ++i) {
        memcpy(dest - elem_size * i, src + elem_size * i, elem_size);
    }
}

/**
 * @brief 查找数组中的最值及其索引
 * @param[in] arr 输入数组指针
 * @param[in] size 数组长度
 * @param[out] min 最小值输出指针
 * @param[out] min_index 最小值索引输出指针
 * @param[out] max 最大值输出指针
 * @param[out] max_index 最大值索引输出指针
 */
void lgf_get_array_min_max(const float *arr, uint32_t size,
                           float *min, uint32_t *min_index,
                           float *max, uint32_t *max_index) {
    if (size == 0 || arr == NULL) return;

    *min = arr[0];
    *max = arr[0];
    *min_index = 0;
    *max_index = 0;

    for (uint32_t i = 1; i < size; ++i) {
        if (arr[i] < *min) {
            *min = arr[i];
            *min_index = i;
        }
        if (arr[i] > *max) {
            *max = arr[i];
            *max_index = i;
        }
    }
}

/**
 * @brief 输出自调用以来一输入值的最小值、最大值
 * @param[in] val: 输入值
 * @param[in] reset: 复位信号
 * @param[out] min: 指向最小值的指针
 * @param[out] max: 指向最大值的指针
 * @retval
 * @note
 */
void lgf_store_min_max(float val, bool reset, float *min, float *max) {
    if (min == NULL || max == NULL) {
        return;
    }

    if (reset) {
        *min = FLT_MAX;
        *max = FLT_MIN;
        return;
    }

    *min = lgf_min(*min, val);
    *max = lgf_max(*max, val);
}

/**
 * @brief 将输入值x按照给定的两个点(x1, y1)和(x2, y2)进行线性插值，
 *        并且结果会被限制在[y_min, y_max]区间内
 * @param[in] x: 输入值
 * @param[in] x1: 确定直线的第1个点的X坐标
 * @param[in] y1: 确定直线的第1个点的Y坐标
 * @param[in] x2: 确定直线的第2个点的X坐标
 * @param[in] y2: 确定直线的第2个点的Y坐标
 * @param[in] y_min: 限定区间的左端点
 * @param[in] y_max: 限定区间的右端点
 * @retval 插值结果
 * @note 若y_min > y_max，则返回值恒为0.0F，
 *       若x1和x2接近，则返回y1
 */
float lgf_scale_linear(float x, float x1, float y1, float x2, float y2,
                       float y_min, float y_max) {
    const float PRECISION = 1.0E-06;
    float y = 0.0F;

    if (y_min > y_max) {
        return y;
    }

    if (fabs(x1 - x2) <= (PRECISION * fabs(x1))) {
        y = y1;
    } else {
        y = (y2 - y1) / (x2 - x1) * (x - x1) + y1;
    }

    y = lgf_min(y, y_max);
    y = lgf_max(y, y_min);

    return y;
}

/**
 * @brief 计算电机输出功率
 * @param[in] torque: 扭矩 N·m
 * @param[in] rpm: 转速 RPM
 * @retval 功率 W
 * @note 
 */
float lgf_calc_motor_out_power(float torque, float rpm) {
    const float CONVERSION_FACTOR = 9.550F;
    return torque * rpm / CONVERSION_FACTOR;
}

/****************EOF****************/
