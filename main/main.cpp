/**
 ******************************************************************************
 * @file     main.c(pp)
 * @author   zyd
 * @version
 * @date
 * @brief
 * @license
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

#include <string.h>

#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <lwip/netif.h>

#include "wifi.h"
#include "eth.h"

#include "mbreg.h"
#include "cycle_timer.h"
#include "retain.h"
#include "mbtcp_server.h"


/* 打印日志的名称 */
static const char *TAG = "Main";

/* 控制任务句柄 */
static TaskHandle_t h_task_control;
/* ModbusTCP服务器任务句柄 */
static TaskHandle_t h_task_mbtcp_server;

/**
 * @brief 控制任务
 * @param[in] pvParameters: 参数(此处为通讯变量访问互斥锁)
 * @retval
 * @note
 */
void control_task(void *pvParameters) {
    /* 输出任务所在CPU */
    BaseType_t coreID = xPortGetCoreID();
    printf("\n");
    ESP_LOGW(TAG, "Task '%s' started on core %d\n", __FUNCTION__, coreID);

    SemaphoreHandle_t mbreg_mutex = *((SemaphoreHandle_t *)pvParameters);
    cycle_timer_t sys_cycle_timer = CYCLE_TIMER();

    while (1) {
        /* 获取通信变量互斥锁 */
        if (xSemaphoreTake(mbreg_mutex, portMAX_DELAY) != pdTRUE) {
            ESP_LOGW(TAG, "Take mutex of 'mbreg' failed");
            continue;
        }

        /* 诊断码 */
        mbreg.sysdiag.code = 12345678;
        /* 行号 */
        mbreg.sysdiag.line = __LINE__;
        /* 文件名 */
        memset(&mbreg.sysdiag.file_name_byte0, 0, 22);
        strncpy((char *)&mbreg.sysdiag.file_name_byte0, __FILE__, 21);
        
        /* 运行周期 */
        mbreg.sysstat.cycle_time_us = sys_cycle_timer.cycle_us(&sys_cycle_timer);
        /* 系统时间 */
        mbreg.sysstat.sys_time_s = (uint32_t)(read_system_time_ms() / SECOND_IN_MILLISECONDS);
        
        /* 释放通信变量互斥锁 */
        xSemaphoreGive(mbreg_mutex);

        /* 延时 */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief 程序入口
 * @param
 * @retval
 * @note
 */
extern "C" void app_main(void) {
    /* 初始化NVS并恢复保持区 */
    retain_nvs_flash_init(&retain);
    retain.recover(&retain);

    /* 设置设备型号 */
    if (mbreg.sysconf.dev_type == 0) {
        mbreg.sysconf.dev_type = 20260101;
    }
    /* 累计启动次数 */
    mbreg.sysconf.start_count++;

    /* 设置1 */
    if (mbreg.otherconf.setting1 == 0) {
        mbreg.otherconf.setting1 = 1;
    }
    /* 设置2 */
    if (mbreg.otherconf.setting2 == 0) {
        mbreg.otherconf.setting2 = 2;
    }

    /* 持久化保持区更改 */
    retain.flush(&retain);
    
    printf("\nchar:%d\n", sizeof(char));
    printf("unsigned char:%d\n", sizeof(unsigned char));
    printf("int:%d\n", sizeof(int));
    printf("short:%d\n", sizeof(short));
    printf("long:%d\n", sizeof(long));
    printf("long long:%d\n", sizeof(long long));
    printf("float:%d\n", sizeof(float));
    printf("double:%d\n", sizeof(double));
    printf("__FILE__ = %s\n", __FILE__);
    printf("__LINE__ = %d\n", __LINE__);
    printf("__DATE__ = %s\n", __DATE__);
    printf("__TIME__ = %s\n", __TIME__);
    printf("__STDC__ = %d\n", __STDC__);
    printf("__FUNCTION__ = %s\n", __FUNCTION__);

    /* 输出编译时间 */
    printf("BUILD_TIMESTAMP: %s\n", MBREG_GEN_TIME);

    /* 输出任务看门狗设置信息 */
#if CONFIG_TASK_WDT_TIMEOUT_S
    printf("CONFIG_TASK_WDT_TIMEOUT_S = %d\n", CONFIG_TASK_WDT_TIMEOUT_S);
#endif
#if CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0
    printf("CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0 = %d\n", CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0);
#endif
    printf("\n");
    
    /* 初始化通信变量互斥锁 */
    mbreg_init_mutex();
    /* 创建控制任务 */
    xTaskCreatePinnedToCore(control_task, "control_task", 4096U,
                            (void*)mbreg_get_mutex(), 5U, &h_task_control, 1U);
    
    /* 初始化TCP/IP协议栈(应用中仅需被调用一次) */
    ESP_ERROR_CHECK(esp_netif_init());
    /* 创建默认事件循环(网络接口驱动程序注册系统事件时会使用默认的事件循环) */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 启动eth */
    // eth_start();
    eth_state_t* eth_state = eth_get_state();

    /* 启动wifi,
       WIFI_MODE_STA: Wi-Fi station mode,
       WIFI_MODE_AP: Wi-Fi soft-AP mode,
       WIFI_MODE_APSTA: Wi-Fi station + soft-AP mode */
    wifi_assign_mode(WIFI_MODE_STA);
    wifi_start();
    wifi_ap_state_t* ap_state = wifi_ap_get_state();
    wifi_sta_state_t* sta_state = wifi_sta_get_state();

    /* 等待Ethernet或者WiFi连接网络 */
    while (!ap_state->has_assigned_ip && !sta_state->has_got_ip && !eth_state->has_got_ip) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /* 创建ModbusTCP服务器任务 */
    xTaskCreatePinnedToCore(mbtcp_server_task, "mbtcp_server_task", 4096U,
                            (void*)mbreg_get_mutex(), 3U, &h_task_mbtcp_server, 0U);
    
    /* 等待Ethernet连接网络 */
    // while (!eth_state->has_got_ip) {
    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }

    /* List current available network interfaces */
    struct netif* netif = NULL;
    ESP_LOGI(TAG, "List of current available network interfaces:");
    NETIF_FOREACH(netif) {
        printf("\t%c%c%d\n", netif->name[0], netif->name[1], netif->num);
    }

    /* Set default network interface */
    if (eth_get_netif() != NULL) {
        esp_netif_set_default_netif(eth_get_netif());
        esp_netif_set_route_prio(eth_get_netif(), 255);
    } else if (wifi_sta_get_netif() != NULL) {
        esp_netif_set_default_netif(wifi_sta_get_netif());
    } else if (wifi_ap_get_netif() != NULL) {
        esp_netif_set_default_netif(wifi_ap_get_netif());
    } else {
        ESP_LOGE(TAG, "Not found available network interface");
        return;
    }

    /* Obtain the default network interface and its name or other information,
       the name consists of characters (2 letters) and numbers (U8 type) */
    char netif_name[6] = { 0 };
    esp_netif_t* default_netif = esp_netif_get_default_netif();
    if (default_netif != NULL) {
        esp_netif_get_netif_impl_name(default_netif, netif_name);
        ESP_LOGI(TAG, "Default network interface is '%s':", netif_name);                       // e.g."en1"
        printf("\tinterface key: %s\n", esp_netif_get_ifkey(default_netif));                   // e.g."ETH_DEF"
        printf("\tinterface type: %s\n", esp_netif_get_desc(default_netif));                   // e.g. "eth"
        printf("\troute priority: %d\n", esp_netif_get_route_prio(default_netif));             // e.g. 50
        printf("\timplementation index: %d\n", esp_netif_get_netif_impl_index(default_netif)); // e.g. 2
    }
}

/****************EOF****************/
