set(src_dirs
    .
    pou/mbreg
    pou/mbtcp
    pou/retain
)

set(include_dirs
    .
    pou/mbreg
    pou/mbtcp
    pou/retain
)

set(requires
    esp_timer
    nvs_flash

    bsp
    lib
    nmbs
)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
