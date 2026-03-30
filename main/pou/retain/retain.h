/**
 ****************************************************************************************************
 * @file        retain.h
 * @author      zyd
 * @version     
 * @date        
 * @brief       RETAIN
 *              用于处理保持型变量
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#ifndef __RETAIN_H
#define __RETAIN_H

#include <stdio.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "mbreg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 保持区名称最大字符数 */
#define RETAIN_NAME_MAX_SIZE 15U

/* 保持区命名空间名称 */
#define RETAIN_NAMESPACE_NAME "retain_var"

/* 保持区数据块 */
enum { RETAIN_BLOBS_BEGIN = __LINE__ };
#define RETAIN_BLOBS                                  \
    {                                                 \
        {.key = "sysconf",                            \
         .value_len = sizeof(mbreg_sysconf_t),        \
         .value_ptr = (uint8_t*)&mbreg.sysconf},      \
        {.key = "otherconf",                          \
        .value_len = sizeof(mbreg_otherconf_t),       \
        .value_ptr = (uint8_t*)&mbreg.otherconf},     \
    }
enum { RETAIN_BLOBS_END = __LINE__ };

/* 保持区数据块数量 */
#define RETAIN_BLOBS_TOTAL (((int)RETAIN_BLOBS_END - (int)RETAIN_BLOBS_BEGIN - 4U) / 3U)

/* 创建保持区 */
#define RETAIN()                              \
    {.blobs = RETAIN_BLOBS,                   \
     .namespace_name = RETAIN_NAMESPACE_NAME, \
     .flush = retain_flush,                   \
     .recover = retain_recover}

/**
 * @struct retain_blob_t
 * @brief 保持区数据块结构体
 */
typedef struct retain_blob_t {
    /* 键 */
    char key[RETAIN_NAME_MAX_SIZE];
    /* 大小 */
    size_t value_len;
    /* 值 */
    uint8_t *value_ptr;
} retain_blob_t;

/**
 * @struct can_distributer_t
 * @brief 保持区结构体
 */
typedef struct retain_t {
    /* 数据块 */
    retain_blob_t blobs[RETAIN_BLOBS_TOTAL];
    /* 命名空间 */
    char namespace_name[RETAIN_NAME_MAX_SIZE];
    /* 持久化保持区 */
    void (*flush)(struct retain_t *self);
    /* 恢复保持区 */
    void (*recover)(struct retain_t *self);
} retain_t;

/* 保持区 */
extern retain_t retain;

/*初始化NVS分区 */
void retain_nvs_flash_init(retain_t *self);
/* 擦除整个NVS分区上的所有数据 */
void retain_erase(retain_t *self);
/* 将保持区持久化到Flash  */
void retain_flush(retain_t *self);
/* 从Flash恢复保持区 */
void retain_recover(retain_t *self);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
