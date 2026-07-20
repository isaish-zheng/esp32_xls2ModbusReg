/**
 ******************************************************************************
 * @file        mbreg.h
 * @author      xls2mbreg.py
 * @version
 * @date        2026/07/20 09:10:18
 * @brief       'mbreg.h' converted by the file 'D:\Project\espProject\esp32_xls2ModbusReg\mbreg.xlsm'
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#ifndef __MBREG_H
#define __MBREG_H

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 生成时间 */
#define MBREG_GEN_TIME "2026/07/20 09:10:18"

/**
 * @struct mbreg_ldi_t
 * @brief 共16个数据项在表'ldi'中
 */
#pragma pack(push,1)
typedef struct mbreg_ldi_t {
    /* 序号:1 地址:M0.0 名称:数字输入0.0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_0 : 1;

    /* 序号:2 地址:M0.1 名称:数字输入0.1 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_1 : 1;

    /* 序号:3 地址:M0.2 名称:数字输入0.2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_2 : 1;

    /* 序号:4 地址:M0.3 名称:数字输入0.3 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_3 : 1;

    /* 序号:5 地址:M0.4 名称:数字输入0.4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_4 : 1;

    /* 序号:6 地址:M0.5 名称:数字输入0.5 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_5 : 1;

    /* 序号:7 地址:M0.6 名称:数字输入0.6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_6 : 1;

    /* 序号:8 地址:M0.7 名称:数字输入0.7 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i0_7 : 1;

    /* 序号:9 地址:M1.0 名称:数字输入1.0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_0 : 1;

    /* 序号:10 地址:M1.1 名称:数字输入1.1 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_1 : 1;

    /* 序号:11 地址:M1.2 名称:数字输入1.2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_2 : 1;

    /* 序号:12 地址:M1.3 名称:数字输入1.3 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_3 : 1;

    /* 序号:13 地址:M1.4 名称:数字输入1.4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_4 : 1;

    /* 序号:14 地址:M1.5 名称:数字输入1.5 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_5 : 1;

    /* 序号:15 地址:M1.6 名称:数字输入1.6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_6 : 1;

    /* 序号:16 地址:M1.7 名称:数字输入1.7 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool i1_7 : 1;

} mbreg_ldi_t;
#pragma pack(pop)

/**
 * @struct mbreg_lai_t
 * @brief 共4个数据项在表'lai'中
 */
#pragma pack(push,1)
typedef struct mbreg_lai_t {
    /* 序号:1 地址:MW2 名称:模拟输入0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t iw10;

    /* 序号:2 地址:MW4 名称:模拟输入2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t iw12;

    /* 序号:3 地址:MW6 名称:模拟输入4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t iw14;

    /* 序号:4 地址:MW8 名称:模拟输入6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t iw16;

} mbreg_lai_t;
#pragma pack(pop)

/**
 * @struct mbreg_ldo_t
 * @brief 共16个数据项在表'ldo'中
 */
#pragma pack(push,1)
typedef struct mbreg_ldo_t {
    /* 序号:1 地址:M10.0 名称:数字输出0.0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_0 : 1;

    /* 序号:2 地址:M10.1 名称:数字输出0.1 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_1 : 1;

    /* 序号:3 地址:M10.2 名称:数字输出0.2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_2 : 1;

    /* 序号:4 地址:M10.3 名称:数字输出0.3 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_3 : 1;

    /* 序号:5 地址:M10.4 名称:数字输出0.4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_4 : 1;

    /* 序号:6 地址:M10.5 名称:数字输出0.5 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_5 : 1;

    /* 序号:7 地址:M10.6 名称:数字输出0.6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_6 : 1;

    /* 序号:8 地址:M10.7 名称:数字输出0.7 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q0_7 : 1;

    /* 序号:9 地址:M11.0 名称:数字输出1.0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_0 : 1;

    /* 序号:10 地址:M11.1 名称:数字输出1.1 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_1 : 1;

    /* 序号:11 地址:M11.2 名称:数字输出1.2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_2 : 1;

    /* 序号:12 地址:M11.3 名称:数字输出1.3 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_3 : 1;

    /* 序号:13 地址:M11.4 名称:数字输出1.4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_4 : 1;

    /* 序号:14 地址:M11.5 名称:数字输出1.5 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_5 : 1;

    /* 序号:15 地址:M11.6 名称:数字输出1.6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_6 : 1;

    /* 序号:16 地址:M11.7 名称:数字输出1.7 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    bool q1_7 : 1;

} mbreg_ldo_t;
#pragma pack(pop)

/**
 * @struct mbreg_lao_t
 * @brief 共4个数据项在表'lao'中
 */
#pragma pack(push,1)
typedef struct mbreg_lao_t {
    /* 序号:1 地址:MW12 名称:模拟输入0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t qw10;

    /* 序号:2 地址:MW14 名称:模拟输入2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t qw12;

    /* 序号:3 地址:MW16 名称:模拟输入4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t qw14;

    /* 序号:4 地址:MW18 名称:模拟输入6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint16_t qw16;

} mbreg_lao_t;
#pragma pack(pop)

/**
 * @struct mbreg_sysdiag_t
 * @brief 共24个数据项在表'sysdiag'中
 */
#pragma pack(push,1)
typedef struct mbreg_sysdiag_t {
    /* 序号:1 地址:MD20 名称:诊断码 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    int32_t code;

    /* 序号:2 地址:MD24 名称:行号 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint32_t line;

    /* 序号:3 地址:MB28 名称:文件名字节0 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte0;

    /* 序号:4 地址:MB29 名称:文件名字节1 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte1;

    /* 序号:5 地址:MB30 名称:文件名字节2 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte2;

    /* 序号:6 地址:MB31 名称:文件名字节3 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte3;

    /* 序号:7 地址:MB32 名称:文件名字节4 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte4;

    /* 序号:8 地址:MB33 名称:文件名字节5 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte5;

    /* 序号:9 地址:MB34 名称:文件名字节6 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte6;

    /* 序号:10 地址:MB35 名称:文件名字节7 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte7;

    /* 序号:11 地址:MB36 名称:文件名字节8 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte8;

    /* 序号:12 地址:MB37 名称:文件名字节9 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte9;

    /* 序号:13 地址:MB38 名称:文件名字节10 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte10;

    /* 序号:14 地址:MB39 名称:文件名字节11 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte11;

    /* 序号:15 地址:MB40 名称:文件名字节12 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte12;

    /* 序号:16 地址:MB41 名称:文件名字节13 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte13;

    /* 序号:17 地址:MB42 名称:文件名字节14 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte14;

    /* 序号:18 地址:MB43 名称:文件名字节15 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte15;

    /* 序号:19 地址:MB44 名称:文件名字节16 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte16;

    /* 序号:20 地址:MB45 名称:文件名字节17 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte17;

    /* 序号:21 地址:MB46 名称:文件名字节18 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte18;

    /* 序号:22 地址:MB47 名称:文件名字节19 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte19;

    /* 序号:23 地址:MB48 名称:文件名字节20 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte20;

    /* 序号:24 地址:MB49 名称:文件名字节21 单位: 记录:True 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint8_t file_name_byte21;

} mbreg_sysdiag_t;
#pragma pack(pop)

/**
 * @struct mbreg_sysconf_t
 * @brief 共3个数据项在表'sysconf'中
 */
#pragma pack(push,1)
typedef struct mbreg_sysconf_t {
    /* 序号:1 地址:MD50 名称:设备型号 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    int32_t dev_type;

    /* 序号:2 地址:MD54 名称:启动次数 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    uint32_t start_count;

    /* 序号:3 地址:MW58 名称:预留 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    uint16_t reserved0;

} mbreg_sysconf_t;
#pragma pack(pop)

/**
 * @struct mbreg_sysstat_t
 * @brief 共4个数据项在表'sysstat'中
 */
#pragma pack(push,1)
typedef struct mbreg_sysstat_t {
    /* 序号:1 地址:MD60 名称:运行周期 单位:us 记录:False 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint32_t cycle_time_us;

    /* 序号:2 地址:MD64 名称:系统时间 单位:s 记录:False 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint32_t sys_time_s;

    /* 序号:3 地址:MD68 名称:预留 单位: 记录:False 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint32_t reserved1;

    /* 序号:4 地址:MD72 名称:预留 单位: 记录:False 保持:False 权限:R
     * 真值表:
     * 备注:
     */
    uint32_t reserved2;

} mbreg_sysstat_t;
#pragma pack(pop)

/**
 * @struct mbreg_otherconf_t
 * @brief 共3个数据项在表'otherconf'中
 */
#pragma pack(push,1)
typedef struct mbreg_otherconf_t {
    /* 序号:1 地址:MW76 名称:设置1 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    uint16_t setting1;

    /* 序号:2 地址:MW78 名称:设置2 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    int16_t setting2;

    /* 序号:3 地址:MW80 名称:预留 单位: 记录:True 保持:True 权限:RW
     * 真值表:
     * 备注:
     */
    uint16_t reserved0;

} mbreg_otherconf_t;
#pragma pack(pop)

/**
 * @struct mbreg_t
 * @brief 共74个数据项在'mbreg'中
 */
#pragma pack(push,1)
typedef struct mbreg_t {
    /* 共16个数据项在'ldi'中 */
    mbreg_ldi_t ldi;

    /* 共4个数据项在'lai'中 */
    mbreg_lai_t lai;

    /* 共16个数据项在'ldo'中 */
    mbreg_ldo_t ldo;

    /* 共4个数据项在'lao'中 */
    mbreg_lao_t lao;

    /* 共24个数据项在'sysdiag'中 */
    mbreg_sysdiag_t sysdiag;

    /* 共3个数据项在'sysconf'中 */
    mbreg_sysconf_t sysconf;

    /* 共4个数据项在'sysstat'中 */
    mbreg_sysstat_t sysstat;

    /* 共3个数据项在'otherconf'中 */
    mbreg_otherconf_t otherconf;

} mbreg_t;
#pragma pack(pop)

/* 通信变量 */
extern mbreg_t mbreg;

/* 初始化通信变量互斥锁 */
void mbreg_init_mutex(void);
/* 获取通信变量互斥锁 */
SemaphoreHandle_t* mbreg_get_mutex(void);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
