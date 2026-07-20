/**
 ******************************************************************************
 * @file        mbreg.c
 * @author      xls2mbreg.py
 * @version
 * @date        2026/07/20 09:10:18
 * @brief       'mbreg.c' converted by the file 'D:\Project\espProject\esp32_xls2ModbusReg\mbreg.xlsm'
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include "mbreg.h"

/* 打印日志的名称 */
static const char *TAG = "MBREG";

/* 通信变量 */
mbreg_t mbreg = {0};

/* 通信变量访问互斥锁 */
static SemaphoreHandle_t _mutex = NULL;

/**
 * @brief 初始化通信变量互斥锁
 * @param
 * @retval
 * @note
 */
void mbreg_init_mutex(void) {
    _mutex = xSemaphoreCreateMutex();
    if (_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create 'mbreg' mutex");
        esp_system_abort("Create mutex failed");
    }
}

/**
 * @brief 获取通信变量互斥锁
 * @param
 * @retval
 * @note
 */
SemaphoreHandle_t* mbreg_get_mutex(void) {
    return &_mutex;
}

/****************EOF****************/
