/**
 ****************************************************************************************************
 * @file        eth.h
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

#ifndef __ETH_H
#define __ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"

#include "spi_eth.h"

/* Ethernet static IP address */
#define ETH_STATIC_IP_ADDR        "192.168.1.101"
/* Ethernet static netmask address */
#define ETH_STATIC_NETMASK_ADDR   "255.255.255.0"
/* Ethernet static gateway address */
#define ETH_STATIC_GW_ADDR        "192.168.1.1"
/* Ethernet main dns server */
#define ETH_MAIN_DNS_SERVER       "8.8.8.8"
/* Ethernet backup dns server */
#define ETH_BACKUP_DNS_SERVER     "8.8.4.4"

/* The boolean position of the event of Ethernet connected in the event group */
#define ETH_CONNECTED_BIT BIT0

/**
 * @struct eth_state_t
 * @brief Ethernet state structure
 */
typedef struct eth_state_t {
    /* IP address */
    char ip_addr_str[INET_ADDRSTRLEN];
    /* Netmask address */
    char mask_addr_str[INET_ADDRSTRLEN];
    /* Gateway address */
    char gw_addr_str[INET_ADDRSTRLEN];
    /* Flag indicating whether has connected */
    bool has_connected;
    /* Flag indicating whether has got IP address */
    bool has_got_ip;
} eth_state_t;

/* Start Ethernet */
void eth_start(void);
/* Stop Ethernet */
void eth_stop(void);

/* Check Ethernet whether has started */
bool eth_is_start(void);
/* Get Ethernet state */
eth_state_t* eth_get_state(void);

/* Get Ethernet network interface */
esp_netif_t* eth_get_netif(void);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
