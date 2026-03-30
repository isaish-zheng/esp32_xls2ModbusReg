/**
 ****************************************************************************************************
 * @file        spi_eth.c
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

#include "spi_eth.h"

/* Name of output log header */
static const char *TAG = "SPI Ethernet";

/* Flag indicating whether GPIO ISR service will be initialized by Ethernet driver */
static bool _gpio_isr_svc_init_by_eth = false;

/**
 * @brief SPI bus initialization (to be used by Ethernet SPI module)
 * @param
 * @retval ESP_OK: success
 * @note
 */
static esp_err_t _spi_bus_init(void) {
    esp_err_t ret = ESP_OK;

#if (SPI_ETH_INT_GPIO >= 0)
    /* Install GPIO ISR handler to be able to service SPI Eth modules interrupts */
    ret = gpio_install_isr_service(0);
    if (ret == ESP_OK) {
        _gpio_isr_svc_init_by_eth = true;
    } else if (ret == ESP_ERR_INVALID_STATE) {
        /* ISR handler has been already installed so no issues */
        ESP_LOGW(TAG, "GPIO ISR handler has been already installed");
        ret = ESP_OK;
    } else {
        ESP_LOGE(TAG, "GPIO ISR handler install failed");
        goto err;
    }
#endif

    /* Init SPI bus */
    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_ETH_MISO_GPIO,
        .mosi_io_num = SPI_ETH_MOSI_GPIO,
        .sclk_io_num = SPI_ETH_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_GOTO_ON_ERROR(spi_bus_initialize(SPI_ETH_HOST, &buscfg, SPI_DMA_CH_AUTO),
                      err, TAG, "SPI host #%d init failed", SPI_ETH_HOST);

err:
    return ret;
}

/**
 * @brief SPI Ethernet module initialization
 * @param[in] spi_eth_module_config specific SPI Ethernet module configuration
 * @param[out] mac_out optionally returns Ethernet MAC object
 * @param[out] phy_out optionally returns Ethernet PHY object
 * @retval esp_eth_handle_t if init succeeded;
 *         NULL if init failed
 * @note
 */
static esp_eth_handle_t _spi_eth_module_init(spi_eth_module_config_t* spi_eth_module_config) {
    esp_eth_handle_t ret = NULL;

    /* Init common MAC and PHY configs to default */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    /* Update PHY config based on board specific configuration */
    phy_config.phy_addr = spi_eth_module_config->phy_addr;
    phy_config.reset_gpio_num = spi_eth_module_config->rst_gpio;

    /* Configure SPI interface for specific SPI module */
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = SPI_ETH_CLOCK_MHZ * 1000 * 1000,
        .queue_size = SPI_ETH_QUEUE_SIZE,
        .spics_io_num = spi_eth_module_config->spi_cs_gpio
    };

    /* Init vendor specific MAC config to default, and create new SPI Ethernet MAC instance
       and new PHY instance based on board configuration */
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI_ETH_HOST, &spi_devcfg);
    w5500_config.int_gpio_num = spi_eth_module_config->int_gpio;
    w5500_config.poll_period_ms = spi_eth_module_config->polling_ms;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    /* Init Ethernet driver to default and install it */
    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_GOTO_ON_FALSE(esp_eth_driver_install(&eth_config, &eth_handle) == ESP_OK,
                      NULL, err, TAG, "SPI Ethernet driver install failed");

    uint8_t mac_addr[ETH_ADDR_LEN] = {0};
    esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
    ESP_LOGI(TAG, "Ethernet HW Addr " MACSTR "", MAC2STR(mac_addr));

    /* The SPI Ethernet module might not have a burned factory MAC address, we can set it manually. */
    if (spi_eth_module_config->mac_addr != NULL) {
        ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, spi_eth_module_config->mac_addr) == ESP_OK,
                          NULL, err, TAG, "SPI Ethernet MAC address config failed");
    }

    /* Start Ethernet Promiscuous Mode */
    bool eth_set_promiscuous = true;
    ESP_GOTO_ON_FALSE(esp_eth_ioctl(eth_handle, ETH_CMD_S_PROMISCUOUS, &eth_set_promiscuous) == ESP_OK,
                      NULL, err, TAG, "Start Ethernet promiscuous failed");

    return eth_handle;

err:
    if (eth_handle != NULL) {
        esp_eth_driver_uninstall(eth_handle);
    }
    if (mac != NULL) {
        mac->del(mac);
    }
    if (phy != NULL) {
        phy->del(phy);
    }
    return ret;
}

/**
 * @brief Initialize Ethernet driver based on Espressif IoT Development Framework Configuration
 * @param[out] eth_handle_out: initialized Ethernet driver handle
 * @retval ESP_OK on: success;
 *         ESP_ERR_INVALID_ARG: passed invalid pointers;
 *         ESP_ERR_NO_MEM: there is no memory to allocate for Ethernet driver handle
 *         ESP_FAIL: any other failure
 * @note
 */
esp_err_t spi_eth_init(esp_eth_handle_t *eth_handle_out) {
    esp_err_t ret = ESP_OK;
    esp_eth_handle_t *eth_handle = NULL;

    /* Check arguments */
    ESP_GOTO_ON_FALSE(eth_handle_out != NULL, ESP_ERR_INVALID_ARG,
                      err, TAG, "Invalid arguments: initialized handle");

    /* Allocate memory for Ethernet handle */
    eth_handle = calloc(1, sizeof(esp_eth_handle_t));
    ESP_GOTO_ON_FALSE(eth_handle != NULL, ESP_ERR_NO_MEM, err, TAG, "No memory");
    
    /* Init SPI bus */
    ESP_GOTO_ON_ERROR(_spi_bus_init(), err, TAG, "SPI bus init failed");

    /* Init specific SPI Ethernet module configuration (CS GPIO, Interrupt GPIO, etc.) */
    spi_eth_module_config_t spi_eth_module_config = { 0 };
    SPI_ETH_MODULE_CONFIG_INIT(spi_eth_module_config);

    /* The SPI Ethernet module might not have a burned factory MAC address, hence use manually configured address.
       In this example, Locally Administered MAC address derived from ESP32x base MAC address is used.
       Note that Locally Administered OUI range should be used only when testing on a LAN under your control! */
    uint8_t base_mac_addr[ETH_ADDR_LEN] = { 0 };
    ESP_GOTO_ON_ERROR(esp_efuse_mac_get_default(base_mac_addr), err, TAG, "Get EFUSE MAC failed");
    uint8_t local_mac_1[ETH_ADDR_LEN] = { 0 };
    /* Just increment last byte to avoid MAC address conflicts,
       e.g. sta mac = base_mac_addr, ap mac = base_mac_addr + 1, so customed mac = base_mac_addr + 2 */
    base_mac_addr[ETH_ADDR_LEN - 1] += 2;
    esp_derive_local_mac(local_mac_1, base_mac_addr);
    spi_eth_module_config.mac_addr = local_mac_1;
    
    /* Install Ethernet driver */
    eth_handle = _spi_eth_module_init(&spi_eth_module_config);
    ESP_GOTO_ON_FALSE(eth_handle, ESP_FAIL, err, TAG, "Install Ethernet driver failed");

    *eth_handle_out = eth_handle;
    return ret;

err:
    if (eth_handle != NULL) {
        free(eth_handle);
    }
    return ret;
}

/**
 * @brief De-initialize Ethernet driver
 * @param[in] eth_handle: Ethernet driver to be de-initialized
 * @retval    ESP_OK: success
 *            ESP_ERR_INVALID_ARG: passed invalid pointers
 * @note Ethernet driver must be stopped prior calling this function.
 */
esp_err_t spi_eth_deinit(esp_eth_handle_t *eth_handle) {
    ESP_RETURN_ON_FALSE(eth_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Ethernet handle cannot be NULL");
    esp_eth_mac_t *mac = NULL;
    esp_eth_phy_t *phy = NULL;
    if (eth_handle != NULL) {
        esp_eth_get_mac_instance(*eth_handle, &mac);
        esp_eth_get_phy_instance(*eth_handle, &phy);
        ESP_RETURN_ON_ERROR(esp_eth_driver_uninstall(*eth_handle), TAG, "Ethernet %p uninstall failed", eth_handle);
    }
    if (mac != NULL) {
        mac->del(mac);
    }
    if (phy != NULL) {
        phy->del(phy);
    }

    spi_bus_free(SPI_ETH_HOST);

#if (SPI_ETH_INT_GPIO >= 0)
    /* We installed the GPIO ISR service so let's uninstall it too.
       BE CAREFUL HERE though since the service might be used by other functionality! */
    if (_gpio_isr_svc_init_by_eth) {
        ESP_LOGW(TAG, "Uninstalling GPIO ISR service!");
        gpio_uninstall_isr_service();
    }
#endif

    return ESP_OK;
}

/****************EOF****************/
