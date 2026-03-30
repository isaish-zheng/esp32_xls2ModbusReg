/**
 ****************************************************************************************************
 * @file        retain.c
 * @author      zyd
 * @version     
 * @date        
 * @brief       RETAIN
 *              用于处理保持型变量
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "retain.h"

/* 打印日志的名称 */
static const char *TAG = "Retian";

/* 保持区 */
retain_t retain = RETAIN();

/**
 * @brief 初始化NVS分区
 * @param[in] self: 保持区结构体指针
 * @retval 
 * @note 初始化后才能操作NVS
 */
void retain_nvs_flash_init(retain_t *self) {
    printf("\n");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "NVS flash initialized");
    } else if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    printf("\n");
}

/**
 * @brief 擦除整个NVS分区上的所有数据
 * @param[in] self: 保持区结构体指针
 * @retval 
 * @note 明确需要清除所有NVS数据时使用,擦除后自动重新初始化
 */
void retain_erase(retain_t *self) {
    printf("\n");
    esp_err_t ret = nvs_flash_erase();
    if (ESP_OK == ret) {
        ESP_LOGI(TAG, "NVS flash erased\n");
        retain_nvs_flash_init(self);
    } else {
        ESP_ERROR_CHECK(ret);
    }
    printf("\n");
}

/**
 * @brief 将保持区持久化到Flash
 * @param[in] self: 保持区结构体指针
 * @retval 
 * @note 
 */
void retain_flush(retain_t *self) {
    printf("\n");
    /* 打开 */
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(self->namespace_name, NVS_READWRITE, &handle);
    if (ESP_OK == ret) {
        ESP_LOGI(TAG, "NVS [%s] opened for flush", self->namespace_name);
    } else {
        ESP_LOGW(TAG, "NVS [%s] open for flush failed: error [%s]", self->namespace_name, esp_err_to_name(ret));
        return;
    }

    /* 写入 */
    int total = 0;
    for (int i = 0; i < RETAIN_BLOBS_TOTAL; ++i) {
        ret = nvs_set_blob(handle, self->blobs[i].key, self->blobs[i].value_ptr, self->blobs[i].value_len);
        if (ESP_OK == ret) {
            total++;
            ESP_LOGI(TAG, "Blob [%s] set", self->blobs[i].key);
        } else {
            ESP_LOGW(TAG, "Blob [%s] set failed: error [%s]", self->blobs[i].key, esp_err_to_name(ret));
        }
    }

    /* 提交更改到Flash，确保持久化 */
    ret = nvs_commit(handle);
    
    /* 关闭 */
    nvs_close(handle);

    if (ESP_OK == ret) {
        if (total == RETAIN_BLOBS_TOTAL) {
            ESP_LOGI(TAG, "Blobs flushed: %d/%d", total, RETAIN_BLOBS_TOTAL);
        } else {
            ESP_LOGW(TAG, "Blobs flushed: %d/%d", total, RETAIN_BLOBS_TOTAL);
        }
    } else {
        ESP_LOGW(TAG, "Blob flush failed: error [%s]", esp_err_to_name(ret));
    }
    printf("\n");
}

/**
 * @brief 从Flash恢复保持区
 * @param[in] self: 保持区结构体指针
 * @retval 
 * @note 
 */
void retain_recover(retain_t *self) {
    printf("\n");
    /* 打开 */
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(self->namespace_name, NVS_READONLY, &handle);
    if (ESP_OK == ret) {
        ESP_LOGI(TAG, "NVS [%s] opened for recover", self->namespace_name);
    } else {
        ESP_LOGW(TAG, "NVS [%s] open for recover failed: error [%s]", self->namespace_name, esp_err_to_name(ret));
        return;
    }

    /* 读取 */
    int total = 0;
    for (int i = 0; i < RETAIN_BLOBS_TOTAL; i++) {
        size_t value_len = self->blobs[i].value_len;
        ret = nvs_get_blob(handle, self->blobs[i].key, self->blobs[i].value_ptr, &value_len);
        if (ESP_OK == ret) {
            total++;
            ESP_LOGI(TAG, "Blob [%s] got", self->blobs[i].key);
        } else {
            ESP_LOGW(TAG, "Blob [%s] get failed: error [%s]", self->blobs[i].key, esp_err_to_name(ret));
        }

        if (value_len != self->blobs[i].value_len) {
            ESP_LOGW(TAG, "Blob [%s] get failed! error value's length mismatched", self->blobs[i].key);
        }
    }
    
    /* 关闭 */
    nvs_close(handle);

    if (total == RETAIN_BLOBS_TOTAL) {
        ESP_LOGI(TAG, "Blobs recovered: %d/%d", total, RETAIN_BLOBS_TOTAL);
    } else {
        ESP_LOGW(TAG, "Blobs recovered: %d/%d", total, RETAIN_BLOBS_TOTAL);
    }
    printf("\n");
}

/****************EOF****************/
