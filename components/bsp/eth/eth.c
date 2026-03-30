/**
 ****************************************************************************************************
 * @file        eth.c
 * @author      zyd
 * @version
 * @date
 * @brief       Ethernet
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "eth.h"

/* Name of output log header */
static const char *TAG = "Ethernet";

/* Flag indicating whether Ethernet has started */
static bool _is_eth_start = false;
/* Ethernet state */
static eth_state_t _eth_state = {0};
/* Event group handle */
static EventGroupHandle_t _h_event_group;

/* Network interface */
static esp_netif_t *_eth_netif = NULL;
/* Ethernet handle */
static esp_eth_handle_t _eth_handle;
/* Ethernet netif glue handle */
static esp_eth_netif_glue_handle_t _eth_netif_glue;

/**
 * @brief Set DNS server for the given network interface
 * @param[inout] netif: Network interface
 * @param[in] addr: DNS server IP address
 * @param[in] type: DNS type (ESP_NETIF_DNS_MAIN or ESP_NETIF_DNS_BACKUP)
 * @retval ESP_OK: success
 * @note
 */
static esp_err_t _eth_set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type) {
    if (addr && (addr != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}

/**
 * @brief Set static IP address for the given network interface
 * @param[inout] netif: Network interface
 * @retval
 * @note
 */
static void _eth_set_static_ip(esp_netif_t *netif) {
    esp_netif_dhcp_status_t status = 0;
    if (esp_netif_dhcpc_get_status(netif, &status) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get dhcp client status");
    }

    if (status != ESP_NETIF_DHCP_STOPPED) {
        if (esp_netif_dhcpc_stop(netif) == ESP_OK) {
            ESP_LOGI(TAG, "DHCP client already stopped");
        } else {
            ESP_LOGE(TAG, "Failed to stop dhcp client");
            return;
        }
    }
    
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(ETH_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(ETH_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(ETH_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", ETH_STATIC_IP_ADDR, ETH_STATIC_NETMASK_ADDR, ETH_STATIC_GW_ADDR);
    ESP_ERROR_CHECK(_eth_set_dns_server(netif, ipaddr_addr(ETH_MAIN_DNS_SERVER), ESP_NETIF_DNS_MAIN));
    ESP_ERROR_CHECK(_eth_set_dns_server(netif, ipaddr_addr(ETH_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));
}

/**
 * @brief Event handler for Ethernet events and IP events
 * @param[inout] arg: parameter (esp-netif instance)
 * @param[in] event_base: event base
 * @param[in] event_id: event id
 * @param[in] event_data: event data
 * @retval
 * @note
 */
static void _event_handler(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data) {
    if (event_base == ETH_EVENT) {
        /* Ethernet event */
        switch (event_id) {
            case ETHERNET_EVENT_START: {
                _is_eth_start = true;
                ESP_LOGI(TAG, "Ethernet Started");
                break;
            }
            case ETHERNET_EVENT_STOP: {
                _is_eth_start = false;
                ESP_LOGI(TAG, "Ethernet Stopped");
                break;
            }
            case ETHERNET_EVENT_CONNECTED: {
                /* we can get the ethernet driver handle from event data */
                esp_eth_handle_t eth_handle = *(esp_eth_handle_t*)event_data;
                _eth_state.has_connected = true;
                uint8_t mac_addr[ETH_ADDR_LEN] = {0};
                esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
                ESP_LOGI(TAG, "Ethernet Link Up, HW Addr " MACSTR "", MAC2STR(mac_addr));
                _eth_set_static_ip(arg);
                break;
            }
            case ETHERNET_EVENT_DISCONNECTED: {
                if (_eth_state.has_connected) {
                    _eth_state.has_connected = false;
                    xEventGroupClearBits(_h_event_group, ETH_CONNECTED_BIT);
                    ESP_LOGI(TAG, "Ethernet Link Down");
                }
                break;
            }
        }
    } else if (event_base == IP_EVENT) {
        /* IP event */
        switch (event_id) {
            case IP_EVENT_ETH_GOT_IP: {
                ip_event_got_ip_t *event_ip_info = (ip_event_got_ip_t *)event_data;
                sprintf(_eth_state.ip_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.ip));
                sprintf(_eth_state.mask_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.netmask));
                sprintf(_eth_state.gw_addr_str, IPSTR, IP2STR(&event_ip_info->ip_info.gw));
                _eth_state.has_got_ip = true;
                xEventGroupSetBits(_h_event_group, ETH_CONNECTED_BIT);
                ESP_LOGI(TAG, "Got IP address: ip %s, mask %s, gw %s",
                         _eth_state.ip_addr_str,
                         _eth_state.mask_addr_str,
                         _eth_state.gw_addr_str);
                break;
            }
            case IP_EVENT_ETH_LOST_IP: {
                if (_eth_state.has_got_ip) {
                    xEventGroupClearBits(_h_event_group, ETH_CONNECTED_BIT);
                    bzero(&_eth_state, sizeof(_eth_state));
                    ESP_LOGW(TAG, "Lost IP address");
                }
                break;
            }
        }  
    }
}

/**
 * @brief Start Ethernet
 * @param
 * @retval
 * @note
 */
void eth_start(void) {
    ESP_LOGI(TAG, "Start and initialize Ethernet network...");

    /* Create event group */
    _h_event_group = xEventGroupCreate();

    /* Initialize Ethernet driver */
    ESP_ERROR_CHECK(spi_eth_init(&_eth_handle));

    /* Initialize TCP/IP network interface aka the esp-netif (should be
     * called only once in application) */
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
    /* Create default event loop that running in background */
    // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());

    /* Create instance(s) of esp-netif for Ethernet.
       Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used
       and you don't need to modify default esp-netif configuration
       parameters. */
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    _eth_netif = esp_netif_new(&cfg);
    _eth_netif_glue = esp_eth_new_netif_glue(_eth_handle);

    /* Attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(_eth_netif, _eth_netif_glue));

    /* Register Ethernet events handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ETH_EVENT, 
                                                        ESP_EVENT_ANY_ID,
                                                        &_event_handler,
                                                        _eth_netif,
                                                        NULL));
    /* Register IP events handler */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &_event_handler,
                                                        NULL,
                                                        NULL));

    /* Start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(_eth_handle));
}

/**
 * @brief Stop Ethernet
 * @param
 * @retval
 * @note
 */
void eth_stop(void) {
    ESP_LOGI(TAG, "Stop and deinitialize Ethernet network...");
    /* Stop Ethernet driver state machine and destroy netif */
    ESP_ERROR_CHECK(esp_eth_stop(_eth_handle));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(_eth_netif_glue));
    esp_netif_destroy(_eth_netif);

    esp_netif_deinit();
    ESP_ERROR_CHECK(spi_eth_deinit(&_eth_handle));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
                                                 ESP_EVENT_ANY_ID,
                                                 _event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT,
                                                 ESP_EVENT_ANY_ID,
                                                 _event_handler));
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
}

/**
 * @brief Check Ethernet whether has started
 * @param
 * @retval true: started; false: not start
 * @note
 */
bool eth_is_start(void) {
    return _is_eth_start;
}

/**
 * @brief Get Ethernet state
 * @param
 * @retval eth_state_t
 * @note
 */
eth_state_t* eth_get_state(void) {
    return &_eth_state;
}

/**
 * @brief Get Ethernet network interface
 * @param
 * @retval esp_netif_t
 * @note
 */
esp_netif_t* eth_get_netif(void) {
    return _eth_netif;
}

/****************EOF****************/
