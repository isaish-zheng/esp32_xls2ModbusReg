/**
 ******************************************************************************
 * @file        cycle_timer.h
 * @author      zyd
 * @version
 * @date
 * @brief       CYCLE TIMER
 *              调用cycle_*()计算自上次调用cycle_*()以来的时间；
 *              首次调用cycle_*()将返回0
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#ifndef __CYCLE_TIMER_H
#define __CYCLE_TIMER_H

#include "lgf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 创建运行周期定时器 */
#define CYCLE_TIMER()                  \
    {._ticks_old = 0U,                 \
     .cycle_us = cycle_timer_cycle_us, \
     .cycle_ms = cycle_timer_cycle_ms, \
     .reset = cycle_timer_reset}

/**
 * @struct cycle_timer_t
 * @brief 运行周期定时器结构体
 */
typedef struct cycle_timer_t {
    /* 历史计时值 */
    int64_t _ticks_old;
    /* 运行定时器函数指针(us) */
    uint32_t (*cycle_us)(struct cycle_timer_t *self);
    /* 运行定时器函数指针(ms) */
    uint32_t (*cycle_ms)(struct cycle_timer_t *self);
    /* 复位定时器函数指针 */
    void (*reset)(struct cycle_timer_t *self);
} cycle_timer_t;

/* 获取运行周期 us */
uint32_t cycle_timer_cycle_us(struct cycle_timer_t *self);

/* 获取运行周期 ms */
uint32_t cycle_timer_cycle_ms(struct cycle_timer_t *self);

/* 复位运行周期定时器 */
void cycle_timer_reset(struct cycle_timer_t *self);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
