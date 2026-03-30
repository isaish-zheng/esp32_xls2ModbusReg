项目说明
## 项目信息
**esp32_xls2ModbusReg**
- 运行于`esp32`的`modbus tcp server`完整案例。
- 通讯变量可快捷地创建于`excel`的`sheet`中，寄存器偏移地址自动更新。
- `excel`和`sheet`按其名称转为`c`语言嵌套结构，支持手动或构建项目时自动转换，控制任务代码直接通过结构访问通讯变量。
- 项目还包含一些依赖功能，如无线/有线以太网连接、`select`响应多客户端、`mutex`守护共享资源、`nvs`断电保持区等。

---

## 开发环境
**SoC**
- ESP32-S3-N16R8

**IDE**
- [VS Code](https://code.visualstudio.com/Download)
  1. `ESP-IDF`扩展插件(版本`1.11.1`)
  2. `Tasks Button`扩展插件(发布者`quzhen`)
- [ESP-IDF 5.5.3](https://dl.espressif.cn/dl/esp-idf/)
- [Python 3.11.9](https://www.python.org/ftp/python/3.11.9/python-3.11.9.exe)

---

## Modbus寄存器表格
**表格位于[mbreg.xlsm](mbreg.xlsm)文件，其详细设计见[xls.md](docs/xls.md)**
1. 若要添加一组通讯变量，可以复制已存在的`sheet`到指定位置，然后重命名如`otherconf`，在新的工作表添加变量的`编号`和`名称`并设置`数据类型`等自定义属性。
2. 根据计算公式自动更新变量的`地址类型`、`字节地址`、`位地址`。
3. 在`VS Code`底部单击`xls2mbreg`按钮，将生成与表格文件名称相同的`.c`和`.h`文件以及`struct`。
4. 代码中使用结构体访问通讯变量，如`mbreg.otherconf.setting1`。

| 序号 | 编号 | 名称 | 单位 | 数据类型 | 地址类型 | 字节地址 | 位地址 | 记录 | 保持 | 权限 | 真值表 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | setting1 | 设置1 | | uint16_t | MW | 76 | -1 | TRUE | TRUE | RW | | |
| 2 | setting2 | 设置2 | | int16_t | MW | 78 | -1 | TRUE | TRUE |RW | | |
| 3 | reserved0 | 预留 | | uint16_t | MW| 80 | -1 | TRUE | TRUE | RW | | |

---

## 转换脚本
**脚本位于[scripts/xls2mbreg.py](scripts/xls2mbreg.py)文件，其依赖见[requirements.txt](scripts/requirements.txt)**
- 安装`Tasks Button`扩展插件后，在[.vscode/tasks.json](.vscode/tasks.json)文件中添加如下代码保存，即可在`VS Code`底部显示`xls2mbreg`按钮。
```
{
    "label": "xls2mbreg",
    "type": "shell",
    "icon": {
        "id": "refresh",
        "tooltip": "从xls生成Modbus寄存器的c头文件"
    },
    "command": "python3 ./scripts/xls2mbreg.py ./mbreg.xlsm ./main/pou/mbreg",
    "group": {
        "kind": "none",
        "isDefault": true
    },
    "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared"
    },
    "problemMatcher": []
}
```

- 在[CMakeLists.txt](CMakeLists.txt)文件中添加如下代码保存，即可在构建项目时自动运行转换脚本。建议在表格更新时手动转换后进行构建项目，或者彻底清理后构建。
```
add_custom_target(
    run_script_xls2mbreg ALL
    COMMAND python3 ${PROJECT_DIR}/scripts/xls2mbreg.py ${PROJECT_DIR}/mbreg.xlsm ${PROJECT_DIR}/main/pou/mbreg
    WORKING_DIRECTORY ${PROJECT_DIR}
    COMMENT "Running Python script xls2mbreg.py..."
)

add_dependencies(${PROJECT_NAME}.elf run_script_xls2mbreg)
```

- 上述操作已在本示例完成，生成的文件位于[main/pou/mbreg](main/pou/mbreg)目录下。若路径不存在会自动创建，生成时将自动覆盖旧文件。

---

## 以太网连接
**无线连接，配置文件位于[components/bsp/wifi/wifi.h](components/bsp/wifi/wifi.h)**
- `STA`模式，修改宏
  
  `#define WIFI_STA_SSID "test123"`

  `#define WIFI_STA_PWD "test1234567"`

- `AP`模式，修改宏
  
  `#define WIFI_AP_SSID "XLS2MBREG_AP"`

  `#define WIFI_AP_PWD "xls2mbreg_ap"`

- `APSTA`混合模式，同`AP`模式和`STA`模式

**有线连接，使用`W5500`**
- 引脚配置文件位于[components/bsp/eth/spi_eth.h](components/bsp/eth/spi_eth.h)
- IP地址配置文件位于[components/bsp/eth/eth.h](components/bsp/eth/eth.h)

---

## Modbus TCP Server
**使用库[nanoMODBUS V1.23.0](https://github.com/debevv/nanoMODBUS)，库文件位于[components/nmbs](components/nmbs)目录下**
- 服务器实例位于[main/pou/mbtcp](main/pou/mbtcp)目录下

---

## 断电保持区
**多个连续的通讯变量可以断电保持，配置文件位于[main/pou/retain/retain.h](main/pou/retain/retain.h)**
1. 打开配置文件，找到`RETAIN_BLOBS`配置区域。
2. 严格按照固定格式添加，如`otherconf`内容。
3. 若运行时报错找不到nvs区域，请运行`idf.py erase-flash`后重新下载程序。
```
#define RETAIN_BLOBS                                  \
    {                                                 \
        {.key = "sysconf",                            \
         .value_len = sizeof(mbreg_sysconf_t),        \
         .value_ptr = (uint8_t*)&mbreg.sysconf},      \
        {.key = "otherconf",                          \
        .value_len = sizeof(mbreg_otherconf_t),       \
        .value_ptr = (uint8_t*)&mbreg.otherconf},     \
    }
```

---

## 主程序
**主程序位于[main/main.cpp](main/main.cpp)文件**
- 初始化以及组织各任务运行等。
- 可通过注释代码`eth_start();`和`wifi_start();`选择有线还是无线以太网连接。
- 可通过`wifi_assign_mode();`设置无线模式为`STA`、`AP`或`APSTA`。
- 请阅读源码。

## Modbus Poll测试
**测试文件位于`Mbpoll.mbp`，其内容与`mbreg.xlsm`文件一致**
1. 安装Modbus Poll软件，位于[docs/other/Modbus_Poll_12.1.1.2487_x64.zip](docs/other/Modbus_Poll_12.1.1.2487_x64.zip)。
2. 打开[Mbpoll.mbp](Mbpoll.mbp)文件，输入`Modbus TCP Server`的`IP`以及`PORT(502)`正常连接后即可进行读写测试。

---

结束