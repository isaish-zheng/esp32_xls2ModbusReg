set(src_dirs
    eth
    wifi
)

set(include_dirs
    eth
    wifi
)

set(requires
    driver
    esp_eth
    esp_netif
    esp_timer
    esp_wifi
)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
