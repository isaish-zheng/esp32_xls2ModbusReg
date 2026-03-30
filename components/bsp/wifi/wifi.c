/**
 ****************************************************************************************************
 * @file        wifi.c
 * @author      zyd
 * @version
 * @date
 * @brief       WIFI
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "wifi.h"

/* 打印日志的名称 AP */
static const char *TAG_AP = "WIFI AP";
/* 打印日志的名称 STA */
static const char *TAG_STA = "WIFI STA";

/* AP启动标识 */
static bool _is_ap_start = false;
/* STA启动标识 */
static bool _is_sta_start = false;

/* AP状态 */
static wifi_ap_state_t _ap_state = {0};
/* STA状态 */
static wifi_sta_state_t _sta_state = {0};

/* 事件组的句柄 */
static EventGroupHandle_t _h_event_group;

/* AP网络接口 */
static esp_netif_t *_ap_netif = NULL;
/* STA网络接口 */
static esp_netif_t *_sta_netif = NULL;

/* WIFI模式 */
static wifi_mode_t _wifi_mode = WIFI_MODE_APSTA;

/**
 * @brief 设置中继
 * @param[inout] ap_netif: AP网络接口
 * @param[inout] sta_netif: STA网络接口
 * @retval
 * @note
 */
static void _set_dns_addr(esp_netif_t* ap_netif, esp_netif_t* sta_netif) {
    /* 获取STA的DNS信息 */
    esp_netif_dns_info_t dns;
    esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns);
    /* 停止AP的DHCP服务器 */
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(ap_netif));
    /* 设置DHCP服务器选项标志,向客户端提供DNS服务
     * ap_netif: AP网络接口
     * ESP_NETIF_OP_SET: 设置操作
     * ESP_NETIF_DOMAIN_NAME_SERVER: 设置域名服务器选项
     * dhcps_offer_option: 选项值(启用DNS)
     * sizeof(dhcps_offer_option): 选项值大小
     */
    uint8_t dhcps_offer_option = WIFI_AP_DHCP_OFFER_DNS;
    ESP_ERROR_CHECK(esp_netif_dhcps_option(
        ap_netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER,
        &dhcps_offer_option, sizeof(dhcps_offer_option)));
    /* 将STA获取的DNS服务器地址设置给AP */
    ESP_ERROR_CHECK(esp_netif_set_dns_info(ap_netif, ESP_NETIF_DNS_MAIN, &dns));
    /* 重新启动AP的DHCP服务器 */
    esp_err_t ret = esp_netif_dhcps_start(ap_netif);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    if (ESP_OK == ret) {
        ESP_LOGI(TAG_AP, "Set AP's DNS from STA success");
        /* 设置STA为默认网络接口 */
        ESP_ERROR_CHECK(esp_netif_set_default_netif(sta_netif));
        /* 在AP上开启NAPT网络地址端口转换 */
        if (ESP_OK == esp_netif_napt_enable(ap_netif)) {
            ESP_LOGI(TAG_AP, "Enable NAPT on the netif success: %p", ap_netif);
        } else {
            ESP_LOGW(TAG_AP, "Enable NAPT on the netif failed: %p", ap_netif);
        }
    }
}

/**
 * @brief 无线事件处理
 * @param[in] arg          事件句柄
 * @param[in] event_base   事件组
 * @param[in] event_id     事件ID
 * @param[in] event_data   事件数据
 * @retval
 * @note
 */
static void _event_handler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        /* WIFI 事件 */
        switch (event_id) {
            /* AP */
            case WIFI_EVENT_AP_START: {
                _is_ap_start = true;
                ESP_LOGI(TAG_AP, "AP started");
                break;
            }
            case WIFI_EVENT_AP_STOP: {
                _is_ap_start = false;
                ESP_LOGW(TAG_AP, "AP stopped");
                ESP_ERROR_CHECK(esp_wifi_start());
                break;
            }
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                ESP_LOGI(TAG_AP, "Station join: mac "MACSTR" aid %d",
                         MAC2STR(event->mac), event->aid);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
                _ap_state.has_assigned_ip = false;
                ESP_LOGW(TAG_AP, "Station leave: mac "MACSTR" aid %d, reason %d",
                         MAC2STR(event->mac), event->aid, event->reason);
                break;
            }
            /* STA */
            case WIFI_EVENT_STA_START: {
                _is_sta_start = true;
                ESP_LOGI(TAG_STA, "STA started, attempt connect to AP");
                esp_wifi_connect();
                break;
            }
            case WIFI_EVENT_STA_STOP: {
                _is_sta_start = false;
                ESP_LOGW(TAG_STA, "STA stopped");
                ESP_ERROR_CHECK(esp_wifi_start());
                break;
            }
            case WIFI_EVENT_STA_CONNECTED: {
                wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                _sta_state.has_connected = true;
                ESP_LOGI(TAG_STA, "Connected to the AP: mac "MACSTR" aid %d",
                         MAC2STR(event->mac), event->aid);
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                esp_wifi_connect();
                if (_sta_state.has_connected) {
                    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                    _sta_state.has_connected = false;
                    xEventGroupClearBits(_h_event_group, WIFI_STA_CONNECTED_AP_BIT);
                    ESP_LOGW(TAG_STA, "Disconnected from the AP: mac "MACSTR". Reconnecting...",
                             MAC2STR(event->mac));
                }
                break;
            }
        }
    } else if (event_base == IP_EVENT) {
        /* IP 事件 */
        switch (event_id) {
            /* AP */
            case IP_EVENT_AP_STAIPASSIGNED: {
                ip_event_ap_staipassigned_t* event = (ip_event_ap_staipassigned_t*)event_data;
                _ap_state.has_assigned_ip = true;
                ESP_LOGI(TAG_AP, "Assigned IP address: ip "IPSTR"", IP2STR(&event->ip));
                break;
            }
            /* STA */
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t *event_ip_info = (ip_event_got_ip_t *)event_data;
                sprintf(_sta_state.ip_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.ip));
                sprintf(_sta_state.mask_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.netmask));
                sprintf(_sta_state.gw_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.gw));
                _sta_state.has_got_ip = true;
                xEventGroupSetBits(_h_event_group, WIFI_STA_CONNECTED_AP_BIT);
                ESP_LOGI(TAG_STA, "Got IP address: ip %s, mask %s, gw %s",
                         _sta_state.ip_addr_str,
                         _sta_state.mask_addr_str,
                         _sta_state.gw_addr_str);
                /* 设置中继 */
                wifi_mode_t mode = WIFI_MODE_NULL;
                esp_wifi_get_mode(&mode);
                if (mode == WIFI_MODE_APSTA) {
                    _set_dns_addr(_ap_netif, _sta_netif);
                }
                break;
            }
            case IP_EVENT_STA_LOST_IP: {
                if (_sta_state.has_got_ip) {
                    xEventGroupClearBits(_h_event_group, WIFI_STA_CONNECTED_AP_BIT);
                    bzero(&_sta_state, sizeof(_sta_state));
                    ESP_LOGW(TAG_STA, "Lost IP address");
                }
                break;
            }
        }  
    }
}

/**
 * @brief wifi指定模式
 * @param[in] mode: 模式
 * @retval
 * @note WIFI_MODE_STA\WIFI_MODE_AP\WIFI_MODE_APSTA
 */
void wifi_assign_mode(wifi_mode_t mode) {
    _wifi_mode = mode;
}

/**
 * @brief wifi启动
 * @param
 * @retval
 * @note
 */
void wifi_start(void) {
    ESP_LOGI("WIFI", "Start and initialize Wifi network...");

    /* 创建事件组 */
    _h_event_group = xEventGroupCreate();

    /* 初始化TCP/IP协议栈(应用中仅需被调用一次) */
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
    /* 创建默认事件循环(网络接口驱动程序注册系统事件时会使用默认的事件循环) */
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());

    /* 注册WIFI事件组处理程序 */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &_event_handler,
                                                        NULL,
                                                        NULL));
    /* 注册IP事件组处理程序 */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &_event_handler,
                                                        NULL,
                                                        NULL));
    
    /* 分配WIFI资源 */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* 设置WIFI模式为APSTA */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    /* 注册AP默认接口 */
    _ap_netif = esp_netif_create_default_wifi_ap();
    /* 配置AP参数 */
    wifi_config_t ap_config = {
        .ap =
            {
                .ssid = WIFI_AP_SSID,
                .ssid_len = strlen(WIFI_AP_SSID),
                .password = WIFI_AP_PWD,
                .max_connection = WIFI_AP_MAX_STA_CONN,
                .authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    if (strlen(WIFI_AP_PWD) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    /* 设置AP配置 */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    /* 注册STA默认接口 */
    _sta_netif = esp_netif_create_default_wifi_sta();
    /* 配置STA参数 */
    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PWD,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL
        },
    };
    /* 设置STA配置 */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    /* 关闭 WIFI 节能 */
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    
    /* 重设WIFI模式为指定模式 */
    if (_wifi_mode == WIFI_MODE_AP) {
        esp_netif_destroy(_sta_netif);
        _sta_netif = NULL;
    }
    if (_wifi_mode == WIFI_MODE_STA) {
        esp_netif_destroy(_ap_netif);
        _ap_netif = NULL;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(_wifi_mode));

    /* 启动WIFI */
    ESP_ERROR_CHECK(esp_wifi_start());

    // EventBits_t bits = xEventGroupWaitBits(_h_event_group, WIFI_STA_CONNECTED_AP_BIT,
    //                                        pdFALSE, pdFALSE, portMAX_DELAY);
}

/**
 * @brief AP是否启动
 * @param
 * @retval true:启动; false:未启动
 * @note
 */
bool wifi_ap_is_start(void) {
    return _is_ap_start;
}

/**
 * @brief STA是否启动
 * @param
 * @retval true:启动; false:未启动
 * @note
 */
bool wifi_sta_is_start(void) {
    return _is_sta_start;
}

/**
 * @brief 获取AP状态
 * @param
 * @retval 状态
 * @note
 */
wifi_ap_state_t* wifi_ap_get_state(void) {
    return &_ap_state;
}

/**
 * @brief 获取STA状态
 * @param
 * @retval 状态
 * @note
 */
wifi_sta_state_t* wifi_sta_get_state(void) {
    return &_sta_state;
}

/**
 * @brief 获取AP网络接口
 * @param
 * @retval esp_netif_t
 * @note
 */
 esp_netif_t* wifi_ap_get_netif(void) {
    return _ap_netif;
}

/**
 * @brief 获取STA网络接口
 * @param
 * @retval esp_netif_t
 * @note
 */
esp_netif_t* wifi_sta_get_netif(void) {
    return _sta_netif;
}

/****************EOF****************/
