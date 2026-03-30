/**
 ******************************************************************************
 * @file        cycle_timer.c
 * @author      zyd
 * @version
 * @date
 * @brief       CYCLE TIMER
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "cycle_timer.h"

/**
 * @brief 获取运行周期 us
 * @param[in] self: 定时器结构体指针
 * @retval
 * @note
 */
uint32_t cycle_timer_cycle_us(struct cycle_timer_t *self) {
    if (!self) return ZERO_INT;
    
    if (!self->_ticks_old) {
        self->_ticks_old = read_system_time_us();
        return ZERO_INT;
    }
    
    int64_t now = read_system_time_us();
    uint32_t cycle = (uint32_t)(now - self->_ticks_old);
    self->_ticks_old = now;
    return cycle;
}

/**
 * @brief 获取运行周期 ms
 * @param[in] self: 定时器结构体指针
 * @retval
 * @note
 */
uint32_t cycle_timer_cycle_ms(struct cycle_timer_t *self) {
    return (!self) ? ZERO_INT
                   : (cycle_timer_cycle_us(self) / MILLISECOND_IN_MICROSECONDS);
}

/**
 * @brief 复位运行周期定时器
 * @param[in] self: 定时器结构体指针
 * @retval 
 * @note 
 */
void cycle_timer_reset(struct cycle_timer_t *self) {
    if (!self) return;
    self->_ticks_old = ZERO_INT;
}

/****************EOF****************/
