/**
 ****************************************************************************************************
 * @file        mbtcp_server.h
 * @author      zyd
 * @version
 * @date
 * @brief       ModbusTCP 服务端
 * @license
 ****************************************************************************************************
 * @attention
 *              menuconfig中需勾选Enable SO_RCVBUF选项
 ****************************************************************************************************
 */

#ifndef __MBTCP_SERVER_H
#define __MBTCP_SERVER_H

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "wifi.h"
#include "nanomodbus.h"
#include "lgf.h"
#include "retain.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 调试接收消息 */
// #define MBTCP_SERVER_DEBUG_RECV_MESSAGE

/* 服务器端口 */
#define MBTCP_SERVER_PORT 502U
/* 最大客户端连接数量 */
#define MBTCP_SERVER_MAX_CLIENT WIFI_AP_MAX_STA_CONN
/* 等待客户端数据超时时间(>0: 超时时间, 单位:毫秒) */
#define MBTCP_SERVER_WAIT_CLIENT_DATA_TIMEOUT 20000U
#if (MBTCP_SERVER_WAIT_CLIENT_DATA_TIMEOUT <= 0)
    #error Macro definition 'MBTCP_SERVER_WAIT_CLIENT_DATA_TIMEOUT' must be greater than 0!
#endif
/* IO多路复用事件监听超时时间(>0: 超时时间, 单位:毫秒) */
#define MBTCP_SERVER_SELECT_TIMEOUT 1000U
#if (MBTCP_SERVER_SELECT_TIMEOUT <= 0)
    #error Macro definition 'MBTCP_SERVER_SELECT_TIMEOUT' must be greater than 0!
#endif

/* 客户端设置非阻塞 */
#define MBTCP_SERVER_SET_CLIENT_NON_BLOCKING

/* 客户端TCP传输禁用Nagle */
// #define MBTCP_SERVER_SET_CLIENT_TCP_NODELAY

/* 客户端设置保活 */
#define MBTCP_SERVER_SET_CLIENT_KEEPALIVE
#ifdef MBTCP_SERVER_SET_CLIENT_KEEPALIVE
    /* 开启keepalive保活属性 */
    #define MBTCP_KEEPALIVE 1
    /* 在x秒内没有任何数据交互则进行探测 */
    #define MBTCP_KEEPALIVE_IDLE 10
    /* 探测时发探测包的时间间隔为x秒 */
    #define MBTCP_KEEPALIVE_INTERVAL 1
    /* 探测重试的次数,全部超时则认定连接失效 */
    #define MBTCP_KEEPALIVE_COUNT 3
#endif

/* 无效SOCKET标识符 */
#ifndef INVALID_SOCK
    #define INVALID_SOCK (-1)
#endif
   
/**
 * @struct mbtcp_client_t
 * @brief ModbusTCP客户端结构体
 */
typedef struct mbtcp_client_t {
    /* 为客户端提供服务的ModbusTCP服务器对象 */
    nmbs_t nmbs;
    /* 客户端地址 */
    char ip_addr_str[INET_ADDRSTRLEN];
    /* 上次收到数据时的时间戳 */
    int64_t last_timestamp;
    /* 等待客户端数据超时时间 */
    int64_t timeout_ms;
    /* 客户端套接字 */
    int socket;
    /* 客户端端口 */
    uint16_t port;
} mbtcp_client_t;

/* ModbusTCP服务器任务 */
void mbtcp_server_task(void *pvParameters);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
