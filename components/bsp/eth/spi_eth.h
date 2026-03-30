/**
 ****************************************************************************************************
 * @file        spi_eth.h
 * @author      zyd
 * @version
 * @date
 * @brief       SPI Ethernet
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#ifndef __SPI_ETH_H
#define __SPI_ETH_H

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_eth_driver.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_mac.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set the SPI host used to communicate with the SPI Ethernet Controller. (0~2) */
#define SPI_ETH_HOST             SPI2_HOST
/* Set the GPIO number used by SPI SCLK. */
#define SPI_ETH_SCLK_GPIO        GPIO_NUM_4
/* Set the GPIO number used by SPI MOSI. */
#define SPI_ETH_MOSI_GPIO        GPIO_NUM_5
/* Set the GPIO number used by SPI MISO. */
#define SPI_ETH_MISO_GPIO        GPIO_NUM_6

/* Set the clock speed (MHz) of SPI interface. (5~80)*/
#define SPI_ETH_CLOCK_MHZ (60)
/* Transaction queue size. This sets how many transactions can be 'in the air' */
#define SPI_ETH_QUEUE_SIZE (20)

/* Set the GPIO number used by SPI CS. */
#define SPI_ETH_CS_GPIO          GPIO_NUM_7
/* Set the GPIO number used by the SPI Ethernet module interrupt line.
   Set to -1 to use SPI Ethernet module in polling mode. */
#define SPI_ETH_INT_GPIO         GPIO_NUM_15
/* Set the GPIO number used to reset PHY chip on the first SPI Ethernet module.
   Set to -1 to disable PHY chip hardware reset. */
#define SPI_ETH_PHY_RST_GPIO     GPIO_NUM_16

/* Polling period in msec of SPI Ethernet Module.
   Set 0 if interrupt is enabled. */
#define SPI_ETH_POLLING_MS       (0)
/* Set the first SPI Ethernet module PHY address. (0~31) */
#define SPI_ETH_PHY_ADDR         (1)

/* Macro to initialize SPI Ethernet module configuration structure */
#define SPI_ETH_MODULE_CONFIG_INIT(spi_eth_module_config)             \
    do {                                                              \
        spi_eth_module_config.spi_cs_gpio = SPI_ETH_CS_GPIO;          \
        spi_eth_module_config.int_gpio = SPI_ETH_INT_GPIO;            \
        spi_eth_module_config.rst_gpio = SPI_ETH_PHY_RST_GPIO;        \
        spi_eth_module_config.polling_ms = SPI_ETH_POLLING_MS;        \
        spi_eth_module_config.phy_addr = SPI_ETH_PHY_ADDR;            \
    } while (0)

/**
 * @struct spi_eth_module_config_t
 * @brief SPI Ethernet module configuration structure
 */
typedef struct spi_eth_module_config_t {
    uint8_t spi_cs_gpio;
    int8_t int_gpio;
    int8_t rst_gpio;
    uint32_t polling_ms;
    uint8_t phy_addr;
    uint8_t *mac_addr;
}spi_eth_module_config_t;

/* Initialize Ethernet driver */
esp_err_t spi_eth_init(esp_eth_handle_t *eth_handle_out);
/* De-initialize Ethernet driver */
esp_err_t spi_eth_deinit(esp_eth_handle_t *eth_handle);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif

/****************EOF****************/
