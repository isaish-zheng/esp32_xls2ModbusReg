/**
 ****************************************************************************************************
 * @file        mbtcp_server.c
 * @author      zyd
 * @version
 * @date
 * @brief       ModbusTCP 服务器
 * @license
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "mbtcp_server.h"

/* 打印日志的名称 */
static const char *TAG = "ModbusTCP Server";

/* 客户端数组 */
static mbtcp_client_t _clients[MBTCP_SERVER_MAX_CLIENT] = {0U};

/**
 * @brief 关闭ModbusTCP客户端
 * @param[inout] client  ModbusTCP客户端对象
 * @retval
 * @note
 */
static void _close_client(mbtcp_client_t* client) {
    if (client->socket != INVALID_SOCK) {
        close(client->socket);
        client->socket = INVALID_SOCK;
    }
    memset(client->ip_addr_str, 0, sizeof(client->ip_addr_str));
    client->port = 0U;
    client->last_timestamp = 0;
}

/**
 * @brief ModbusTCP服务器接收数据的回调函数
 * @param[out] buf: 要接收的数据
 * @param[in] count: 要接收的数量
 * @param[in] timeout_ms: 接收超时时间
 * @param[in] arg: 参数，此处为客户端连接SOCKET的标识符
 * @retval 已接收的数量
 * @note 
 */
static int32_t _handle_read(uint8_t *buf, uint16_t count,
                            int32_t timeout_ms, void *arg) {
    if (!arg) {
        ESP_LOGE(TAG, "Function handle_read arg invalid");
        return -1;
    }
    
    int fd = *(int *)arg;
    uint16_t total = 0;
    ssize_t res = 0;
    
#ifdef MBTCP_SERVER_SET_CLIENT_NON_BLOCKING
    int64_t elapsed = read_system_time_ms();
    int64_t now = 0;
    do {
        res = recv(fd, buf + total, count - total, 0);
        if (res > 0) total += res;
        if (total < count) {
            vTaskDelay(pdMS_TO_TICKS(1U));
        } else {
            return total;
        }
        now = read_system_time_ms();
        if (now < elapsed) elapsed = now;
    } while (now - elapsed < timeout_ms);
    return -1;
#else
    while (total < count) {
        res = recv(fd, buf + total, count - total, 0);
        if (res > 0) {
            total += res;
        } else {
            return res;
        }
    }
    return total;
#endif
}

/**
 * @brief ModbusTCP服务器发送数据的回调函数
 * @param[in] buf: 要发送的数据
 * @param[in] count: 要发送的数量
 * @param[in] timeout_ms: 发送超时时间
 * @param[in] arg: 参数，此处为客户端连接SOCKET的标识符
 * @retval 已发送的数量
 * @note 
 */
static int32_t _handle_write(const uint8_t *buf, uint16_t count,
                             int32_t timeout_ms, void *arg) {
    if (!arg) {
        ESP_LOGE(TAG, "Function handle_write arg invalid");
        return -1;
    }
    
    int fd = *(int *)arg;
    uint16_t total = 0;
    ssize_t res = 0;
    
#ifdef MBTCP_SERVER_SET_CLIENT_NON_BLOCKING
    int64_t elapsed = read_system_time_ms();
    int64_t now = 0;
    do {
        res = send(fd, buf + total, count - total, 0);
        if (res > 0) total += res;
        if (total < count) {
            vTaskDelay(pdMS_TO_TICKS(1U));
        } else {
            return total;
        }
        now = read_system_time_ms();
        if (now < elapsed) elapsed = now;
    } while (now - elapsed < timeout_ms);
    return -1;
#else
    while (total < count) {
        res = send(fd, buf + total, count - total, 0);
        if (res > 0) {
            total += res;
        } else {
            return res;
        }
    }
    return total;
#endif
}

/**
 * @brief ModbusTCP服务器处理读多个保持寄存器的回调函数
 * @param[in] address: 保持寄存器首地址
 * @param[in] quantity: 寄存器数量
 * @param[out] registers_out: 存储要读取的寄存器值
 * @param[in] unit_id: 站地址
 * @param[in] arg: 用户数据
 * @retval nmbs_error
 * @note
 */
static nmbs_error _handler_read_holding_registers(uint16_t address,
                                                  uint16_t quantity,
                                                  uint16_t *registers_out,
                                                  uint8_t unit_id, void *arg) {
    UNUSED(unit_id);
    SemaphoreHandle_t mbreg_mutex = *((SemaphoreHandle_t *)arg);

    if ((address + quantity - 1) * 2 + 1 > sizeof(mbreg)) {
        ESP_LOGE(TAG, "Read multiple holding registers out of bounds: address %d, quantity %d",
                 address, quantity);
        return NMBS_ERROR_INVALID_ARGUMENT;
    }

    if (xSemaphoreTake(mbreg_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Take mutex of 'mbreg' failed");
        return NMBS_ERROR_NONE;
    }

    uint8_t *regs_ptr = (uint8_t *)&mbreg;
    memcpy(registers_out, regs_ptr + address * 2, quantity * 2);

    xSemaphoreGive(mbreg_mutex);

    return NMBS_ERROR_NONE;
}

/**
 * @brief ModbusTCP服务器处理写多个保持寄存器的回调函数
 * @param[in] address: 保持寄存器首地址
 * @param[in] quantity: 寄存器数量
 * @param[in] registers: 存储要写入的寄存器值
 * @param[in] unit_id: 站地址
 * @param[in] arg: 用户数据
 * @retval nmbs_error
 * @note
 */
static nmbs_error _handle_write_multiple_registers(uint16_t address, uint16_t quantity,
                                                   const uint16_t *registers,
                                                   uint8_t unit_id, void *arg) {
    UNUSED(unit_id);
    SemaphoreHandle_t mbreg_mutex = *((SemaphoreHandle_t *)arg);

    if ((address + quantity - 1) * 2 + 1 > sizeof(mbreg)) {
        ESP_LOGE(TAG, "Write multiple registers out of bounds: address %d, quantity %d",
                 address, quantity);
        return NMBS_ERROR_INVALID_ARGUMENT;
    }

    if (xSemaphoreTake(mbreg_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Take mutex of 'mbreg' failed");
        return NMBS_ERROR_NONE;
    }

    uint16_t *regs_ptr = (uint16_t *)&mbreg;
    for (int i = 0; i < quantity; i++) {
        regs_ptr[address + i] = registers[i];
    }

    /* 持久化保持区 */
    retain.flush(&retain);

    xSemaphoreGive(mbreg_mutex);
        
    return NMBS_ERROR_NONE;
}

/**
 * @brief ModbusTCP服务器处理写单个保持寄存器的回调函数
 * @param[in] address: 保持寄存器首地址
 * @param[in] value: 要写入的寄存器值
 * @param[in] unit_id: 站地址
 * @param[in] arg: 用户数据
 * @retval nmbs_error
 * @note
 */
static nmbs_error _handle_write_single_register(uint16_t address, uint16_t value,
                                                uint8_t unit_id, void *arg) {
    UNUSED(unit_id);
    SemaphoreHandle_t mbreg_mutex = *((SemaphoreHandle_t *)arg);

    if (address * 2 + 1 > sizeof(mbreg)) {
        ESP_LOGE(TAG, "Write single registers out of bounds: address %d", address);
        return NMBS_ERROR_INVALID_ARGUMENT;
    }

    if (xSemaphoreTake(mbreg_mutex, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Take mutex of 'mbreg' failed");
        return NMBS_ERROR_NONE;
    }

    uint16_t *regs_ptr = (uint16_t *)&mbreg;
    regs_ptr[address] = value;

    /* 持久化保持区 */
    retain.flush(&retain);

    xSemaphoreGive(mbreg_mutex);

    return NMBS_ERROR_NONE;
}

/**
 * @brief ModbusTCP服务器任务
 * @param[in] pvParameters: 参数(此处为通讯变量访问互斥锁)
 * @retval
 * @note
 */
void mbtcp_server_task(void *pvParameters) {
    /* 输出任务所在CPU */
    BaseType_t coreID = xPortGetCoreID();
    printf("\n");
    ESP_LOGW(TAG, "Task '%s' started on core %d\n", __FUNCTION__, coreID);

    int err = 0;
    int server_socket = INVALID_SOCK;

    /* 创建服务器套接字 */
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        ESP_LOGE(TAG, "Unable to create server socket: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Server socket created: socket %d", server_socket);

    /* 设置非阻塞 */
    int flags = fcntl(server_socket, F_GETFL);
    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        ESP_LOGE(TAG, "Unable to set server socket non blocking: errno %d", errno);
        goto CLEAN_UP;
    }

    /* 设置地址重用 */
    int opt = 1;
    if (0 != setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        ESP_LOGW(TAG, "Unable to set server socket option SO_REUSEADDR : errno %d", errno);
    }

    /* 绑定套接字 */
    struct sockaddr_in sockaddr = {0U};
    socklen_t sockaddr_len = sizeof(sockaddr);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(MBTCP_SERVER_PORT);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(server_socket, (struct sockaddr*)&sockaddr, sockaddr_len);
    if (err != 0) {
        ESP_LOGE(TAG, "Server socket unable to bind: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Server socket bound: port %d", MBTCP_SERVER_PORT);
    
    /* 监听 */
    err = listen(server_socket, 1U);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during server socket listen: errno %d", errno);
        goto CLEAN_UP;
    }

    /* 预设待连接的ModbusTCP客户端对象 */
    int slot = 0;
    for (; slot < MBTCP_SERVER_MAX_CLIENT; ++slot) {
        _clients[slot].socket = INVALID_SOCK;
        _clients[slot].timeout_ms = MBTCP_SERVER_WAIT_CLIENT_DATA_TIMEOUT;

        nmbs_platform_conf platform_conf = {0U};
        nmbs_platform_conf_create(&platform_conf);
        platform_conf.transport = NMBS_TRANSPORT_TCP;
        platform_conf.read = _handle_read;
        platform_conf.write = _handle_write;
        platform_conf.arg = NULL; // 等客户端连接后再设置arg(socket fd)

        nmbs_callbacks callbacks = {0U};
        nmbs_callbacks_create(&callbacks);
        callbacks.read_holding_registers = _handler_read_holding_registers;
        callbacks.write_multiple_registers = _handle_write_multiple_registers;
        callbacks.write_single_register = _handle_write_single_register;
        callbacks.arg = pvParameters;

        /* 创建ModbusTCP服务器对象, 设置address_rtu为0因为TCP未用到 */
        nmbs_error nmbs_err = nmbs_server_create(&_clients[slot].nmbs, 0, &platform_conf, &callbacks);
        if (nmbs_err != NMBS_ERROR_NONE) {
            ESP_LOGE(TAG, "Error pre creating modbus server %d: nmbs error %d", slot, nmbs_err);
            goto CLEAN_UP;
        }

        /* 设置超时时间 */
        nmbs_set_byte_timeout(&_clients[slot].nmbs, 1000U); // 发送超时
        nmbs_set_read_timeout(&_clients[slot].nmbs, 1000U); // 接收超时
    }
    
    /* 监听最大FD */
    int max_fd = INVALID_SOCK;
    /* 可读FD集合 */
    fd_set rfds;
    /* 监听超时时间 */
    struct timeval tv;
    /* 当前时间 */
    int64_t now = 0;

    while(1) {
        /* 清空可读FD集合，并添加服务器到FD集合 */
        FD_ZERO(&rfds);
        FD_SET(server_socket, &rfds);

        /* 获取最大FD，并添加客户端到FD集合 */
        max_fd = server_socket;
        for (slot = 0; slot < MBTCP_SERVER_MAX_CLIENT; ++slot) {
            max_fd = MAX(max_fd, _clients[slot].socket);
            if (_clients[slot].socket != INVALID_SOCK) {
                /* 检查等待客户端消息是否超时 */
                now = read_system_time_ms();
                if (now < _clients[slot].last_timestamp) _clients[slot].last_timestamp = now;
                if (now - _clients[slot].last_timestamp > _clients[slot].timeout_ms) {
                    ESP_LOGW(TAG, "Wait message timeout, closed client: socket %d, ip %s, port %d",
                             _clients[slot].socket, _clients[slot].ip_addr_str, _clients[slot].port);
                    _close_client(&_clients[slot]);
                    continue;
                }
                FD_SET(_clients[slot].socket, &rfds);
            }
        }

        /* 等待事件 */
        tv.tv_sec = MBTCP_SERVER_SELECT_TIMEOUT / 1000U;
        tv.tv_usec = (MBTCP_SERVER_SELECT_TIMEOUT % 1000U) * 1000U;
        err = select(max_fd + 1, &rfds, NULL, NULL, &tv);
        if (err < 0) {
            ESP_LOGW(TAG, "Select failed: errno %d", errno);
        } else if (err == 0) {
            // ESP_LOGW(TAG, "Select timeout has been reached and nothing has been received");
        } else {
            /* 处理服务器accept事件 */
            if (FD_ISSET(server_socket, &rfds)) {
                /* 查找空闲客户端 */
                bool is_exist_slot = false;
                for (slot = 0; slot < MBTCP_SERVER_MAX_CLIENT; ++slot) {
                    if (_clients[slot].socket == INVALID_SOCK) {
                        is_exist_slot = true;
                        break;
                    }
                }

                /* 接受客户端连接 */
                memset(&sockaddr, 0, sockaddr_len);
                int new_client_socket = accept(server_socket, (struct sockaddr *)&sockaddr, &sockaddr_len);
                if (new_client_socket < 0) {
                    ESP_LOGW(TAG, "Unable to accept new client: errno %d", errno);
                } else {
                    if (is_exist_slot) {
#ifdef MBTCP_SERVER_SET_CLIENT_KEEPALIVE
                        /* 设置客户端保活 */
                        int keepAlive = MBTCP_KEEPALIVE;
                        int keepIdle = MBTCP_KEEPALIVE_IDLE;
                        int keepInterval = MBTCP_KEEPALIVE_INTERVAL;
                        int keepCount = MBTCP_KEEPALIVE_COUNT;
                        if (0 != setsockopt(new_client_socket, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive))) {
                            ESP_LOGE(TAG, "Unable to set new client socket option SO_KEEPALIVE: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
                        if (0!= setsockopt(new_client_socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle))) {
                            ESP_LOGE(TAG, "Unable to set new client socket option TCP_KEEPIDLE: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
                        if (0!= setsockopt(new_client_socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval))) {
                            ESP_LOGE(TAG, "Unable to set new client socket option TCP_KEEPINTVL: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
                        if (0!= setsockopt(new_client_socket, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(keepCount))) {
                            ESP_LOGE(TAG, "Unable to set new client socket option TCP_KEEPCNT: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
#endif
#ifdef MBTCP_SERVER_SET_CLIENT_NON_BLOCKING
                        /* 设置客户端非阻塞 */
                        flags = fcntl(new_client_socket, F_GETFL);
                        if (fcntl(new_client_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
                            ESP_LOGE(TAG, "Unable to set new client socket non blocking: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
#endif
#ifdef MBTCP_SERVER_SET_CLIENT_TCP_NODELAY
                        /* 设置TCP_NODELAY(禁用Nagle算法) */
                        opt = 1;
                        if (0 != setsockopt(new_client_socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
                            ESP_LOGE(TAG, "Unable to set new client socket option TCP_NODELAY: socket %d, errno %d", new_client_socket, errno);
                            close(new_client_socket);
                            continue;
                        }
#endif
                        /* 设置接收缓冲区大小 */
                        // opt = 10 * 1024;
                        // if (0 != setsockopt(new_client_socket, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt))) {
                        //     ESP_LOGE(TAG, "Unable to set new client socket option SO_RCVBUF: socket %d, errno %d", new_client_socket, errno);
                        //     close(new_client_socket);
                        //     continue;
                        // }
                        
                        /* 设置发送缓冲区大小 */
                        // opt = 10 * 1024;
                        // if (0 != setsockopt(new_client_socket, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt))) {
                        //     ESP_LOGE(TAG, "Unable to set new client socket option SO_SNDBUF: socket %d, errno %d", new_client_socket, errno);
                        //     close(new_client_socket);
                        //     continue;
                        // }

                        /* 设置TCP接收缓存区低水位 */
                        // opt = 7;
                        // if (0!= setsockopt(new_client_socket, SOL_SOCKET, SO_RCVLOWAT, &opt, sizeof(opt))) {
                        //     ESP_LOGE(TAG, "Unable to set new client socket option SO_RCVLOWAT: socket %d, errno %d", new_client_socket, errno);
                        //     close(new_client_socket);
                        //     continue;
                        // }
                        
                        /* 获取客户端地址 */
                        inet_ntoa_r(((struct sockaddr_in *)&sockaddr)->sin_addr, _clients[slot].ip_addr_str,
                                    sizeof(_clients[slot].ip_addr_str) - 1);
                        /* 获取客户端端口 */
                        _clients[slot].port = ntohs(sockaddr.sin_port);

                        /* 设置客户端属性 */
                        _clients[slot].socket = new_client_socket;
                        _clients[slot].last_timestamp = read_system_time_ms();
                        nmbs_set_platform_arg(&_clients[slot].nmbs, (void *)&_clients[slot].socket);
                        ESP_LOGI(TAG, "New client connected: socket %d, ip %s, port %d", _clients[slot].socket,
                                 _clients[slot].ip_addr_str, _clients[slot].port);
                    } else {
                        close(new_client_socket);
                        ESP_LOGW(TAG, "No free socket available, new client socket closed");
                    }
                }
            }

            /* 处理客户端read事件 */
            for (slot = 0; slot < MBTCP_SERVER_MAX_CLIENT; ++slot) {
                if (_clients[slot].socket == INVALID_SOCK) {
                    continue;
                }
                if (FD_ISSET(_clients[slot].socket, &rfds)) {
                    _clients[slot].last_timestamp = read_system_time_ms();
#ifdef MBTCP_SERVER_DEBUG_RECV_MESSAGE
                    char read_buffer[1024] = {0};
                    int read_len = read(_clients[slot].socket, read_buffer, 1024);
                    if (read_len > 0) {
                        printf("Socket %d Message: %s\n", _clients[slot].socket, read_buffer);
                    } else {
                        if (read_len == 0) {
                            ESP_LOGW(TAG, "Connection closed: socket %d, ip %s",
                                     _clients[slot].socket, _clients[slot].ip_addr_str);
                        } else {
                            ESP_LOGW(TAG, "Connection closed abnormally: socket %d, ip %s, errno %d",
                                     _clients[slot].socket, _clients[slot].ip_addr_str, errno);
                            // if (errno == EISCONN) {}
                        }
                        _close_client(&_clients[slot]);
                    }
#else
                    nmbs_error nmbs_err = nmbs_server_poll(&_clients[slot].nmbs);
                    if (nmbs_err != NMBS_ERROR_NONE) {
                        ESP_LOGW(TAG, "Error on server poll, closed client: socket %d, ip %s, port %d, nmbs errno %s",
                                 _clients[slot].socket, _clients[slot].ip_addr_str, _clients[slot].port, nmbs_strerror(nmbs_err));
                        _close_client(&_clients[slot]);
                    }
#endif
                }
            }
        }
    }

CLEAN_UP:
    if (server_socket != INVALID_SOCK) {
        // shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
        server_socket = INVALID_SOCK;
    }
    vTaskDelete(NULL);
}

/****************EOF****************/
