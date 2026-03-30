/**
 ****************************************************************************************************
 * @file        wifi.h
 * @author      zyd
 * @version
 * @date
 * @brief       WIFI
 * @license
 ****************************************************************************************************
 * @attention
 *              APSTA模式下开启中继需在menuconfig中勾选如下选项
 *                  Enable IP forwarding
 *                  Enable NAT
 *                  Enable NAT Port Mapping

 ****************************************************************************************************
 */

#ifndef __WIFI_H
#define __WIFI_H

#include <string.h>
#include <netdb.h>

#include "freertos/FreeRTOS.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************
 * AP
 *****************************************/

/* 本地AP名称 */
#define WIFI_AP_SSID "XLS2MBREG_AP"
/* 本地AP名称 */
#define WIFI_AP_PWD "xls2mbreg_ap"
/* 本地AP最大接入数量 */
#define WIFI_AP_MAX_STA_CONN 3U
 /* 本地DHCP服务器选项，为AP提供DNS服务 */
#define WIFI_AP_DHCP_OFFER_DNS 0x02

/**
 * @struct wifi_ap_state_t
 * @brief AP状态结构体
 */
typedef struct wifi_ap_state_t {
    /* 以分配IP状态 */
    bool has_assigned_ip;
} wifi_ap_state_t;

/*****************************************
 * STA
 *****************************************/

/* 目标AP名称 */
#define WIFI_STA_SSID "test123"
/* 目标AP密码 */
#define WIFI_STA_PWD "test1234567"
/* 连接目标AP成功的事件在事件组中的布尔位置 */
#define WIFI_STA_CONNECTED_AP_BIT BIT0

/**
 * @struct wifi_sta_state_t
 * @brief STA状态结构体
 */
typedef struct wifi_sta_state_t {
    /* IP地址 */
    char ip_addr_str[INET_ADDRSTRLEN];
    /* 子网掩码 */
    char mask_addr_str[INET_ADDRSTRLEN];
    /* 网关地址 */
    char gw_addr_str[INET_ADDRSTRLEN];
    /* 连接状态 */
    bool has_connected;
    /* 获取IP状态 */
    bool has_got_ip;
} wifi_sta_state_t;

/*****************************************
 * WIFI
 *****************************************/

/* wifi指定模式 */
void wifi_assign_mode(wifi_mode_t mode);

/* wifi启动 */
void wifi_start(void);

/* AP是否启动 */
bool wifi_ap_is_start(void);
/* 获取AP状态 */
wifi_ap_state_t* wifi_ap_get_state(void);

/* STA是否启动 */
bool wifi_sta_is_start(void);
/* 获取STA状态 */
wifi_sta_state_t* wifi_sta_get_state(void);

/* 获取AP网络接口 */
esp_netif_t* wifi_ap_get_netif(void);

/* 获取STA网络接口 */
esp_netif_t* wifi_sta_get_netif(void);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
