#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author  : ZYD
# @Time    : 2025/8/21 下午5:55
# @version : V1.0.0
# @function:

##############################
# Module imports
##############################

import os
import sys
import time
import traceback
from dataclasses import dataclass
from pprint import pprint

import openpyxl
import pandas as pd


def get_str_time() -> str:
    """
    获取当前时间，格式化字符串"%Y/%m/%d %H:%M:%S"

    :return: 当前时间
    :rtype: str
    """
    time_now = time.strftime("%Y/%m/%d %H:%M:%S", time.localtime())  # 时间戳
    return str(time_now)


def text_log(txt: str, *args, **kwargs) -> None:
    """
    输出信息

    :param txt: 信息
    :type txt: str
    :param args: 位置参数，第一个参数为文字颜色
                None-默认,'done'-绿色,'warning'-黄色,'error'-红色
    :param kwargs: 关键字参数（未使用）
    """
    color = '[消息]'
    if args:
        if args[0] == 'done':
            color = Color.GREEN+f'[成功]'
        elif args[0] == 'warning':
            color = Color.YELLOW+'[警告]'
        elif args[0] == 'error':
            color = Color.RED+'[错误]'
    print(f"{color} ({get_str_time()}) {txt}{Color.RESET}")


def get_basename(filepath: str) -> tuple[str, str]:
    """
    根据文件路径获取文件名的前缀和后缀

    :param filepath: 文件路径
    :type filepath: str
    :return: (前缀, 后缀)
    :rtype: tuple[str, str]
    """
    basename = os.path.basename(filepath)
    return os.path.splitext(basename)[0], os.path.splitext(basename)[1]


def isfloat(s: str) -> bool:
    """
    判断字符串是否可转为float类型

    :param s: 文件路径
    :type s: str
    :return: True/False
    :rtype: bool
    """
    try:
        float(s)
        return True
    except ValueError:
        return False


def isint(s: str) -> bool:
    """
    判断字符串是否可转为int类型

    :param s: 文件路径
    :type s: str
    :return: True/False
    :rtype: bool
    """
    try:
        int(s)
        return True
    except ValueError:
        return False


@dataclass(slots=True)
class Color(object):
    """
    定义颜色和样式的ANSI escape码

    示例：打印带颜色和样式的文本
    print(f"{Color.RED}这是红色文本{Color.RESET}")
    print(f"{Color.GREEN}{Color.BOLD}这是加粗的绿色文本{Color.RESET}")
    print(f"{Color.BG_YELLOW}{Color.BLACK}这是黄色背景黑色文本{Color.RESET}")
    print(f"{Color.CYAN}{Color.UNDERLINE}这是下划线的青色文本{Color.RESET}")

    示例：打印不同状态的信息
    print(f"{Color.GREEN}[成功]{Color.RESET} 操作已完成")
    print(f"{Color.YELLOW}[警告]{Color.RESET} 内存空间不足")
    print(f"{Color.RED}[错误]{Color.RESET} 无法连接到服务器")
    """
    # 文本颜色
    BLACK = '\033[30m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'

    # 背景颜色
    BG_BLACK = '\033[40m'
    BG_RED = '\033[41m'
    BG_GREEN = '\033[42m'
    BG_YELLOW = '\033[43m'
    BG_BLUE = '\033[44m'
    BG_MAGENTA = '\033[45m'
    BG_CYAN = '\033[46m'
    BG_WHITE = '\033[47m'

    # 文本样式
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    RESET = '\033[0m'  # 重置所有样式


class GenCommVarException(Exception):
    """
    GenCommVar异常类

    :param message: 要显示的异常消息
    :type message: str
    """

    def __init__(self, message: str):
        """
        构造函数
        """
        self.message = message

    def __str__(self):
        return f"{self.message}"


@dataclass(slots=True)
class DataObject(object):
    """
    数据对象类

    Attributes:
        id (int): 序号
        code (str): 编号
        name (str): 名称
        unit (str): 单位
        data_type (str): 数据类型
        addr_type (str): 地址类型
        byte_pos (int): 字节地址
        bit_pos (int): 位地址
        is_record (bool): 记录
        is_retain (bool): 保持
        access (str): 权限
        value_table (str): 真值表
        comment (str): 备注
    """
    id: int | None = None
    code: str | None = None
    name: str | None = None
    unit: str | None = None
    data_type: str | None = None
    addr_type: str | None = None
    byte_pos: int | None = None
    bit_pos: int | None = None
    is_record: bool | None = None
    is_retain: bool | None = None
    access: str | None = None
    value_table: str | None = None
    comment: str | None = None


@dataclass(slots=True)
class DataObjectField(object):
    """
    数据对象的字段

    """
    id = '序号'
    code = '编号'
    name = '名称'
    unit = '单位'
    data_type = '数据类型'
    addr_type = '地址类型'
    byte_pos = '字节地址'
    bit_pos = '位地址'
    is_record = '记录'
    is_retain = '保持'
    access = '权限'
    value_table = '真值表'
    comment = '备注'


class GenCommVar(object):
    """
    将一个指定模板的Excel文件各工作表中行内容转为数据对象，输出.h文件

    :param read_filename: 待解析文件(eg'./mbreg.xlsm')
    :type read_filename: str
    :param write_filepath: 待写入文件路径(eg'./main/comm')
    :type write_filepath: str
    """

    def __init__(self, read_filename: str, write_filepath: str) -> None:
        """
        构造函数

        """
        text_log(f"初始化信息...")

        # 输入文件信息
        self.SRC_FILEPATH = os.path.abspath(read_filename) # 如"./mbreg.xlsm"

        # 输出文件信息
        self.DEST_PREFIX = get_basename(self.SRC_FILEPATH)[0].lower() # 如"mbreg"
        self.DEST_BASENAME_H = self.DEST_PREFIX + ".h" # 如"mbreg.h"
        self.DEST_BASENAME_C = self.DEST_PREFIX + ".c" # 如"mbreg.c"
        self.DEST_FILEPATH_H = f"{os.path.join(os.path.abspath(write_filepath), self.DEST_BASENAME_H)}" # 如"./mbreg.h"
        self.DEST_FILEPATH_C = f"{os.path.join(os.path.abspath(write_filepath), self.DEST_BASENAME_C)}" # 如"./mbreg.c"

        # 当前脚本文件信息
        self.CURR_BASENAME = "".join(get_basename(os.path.abspath(__file__)))

        # 生成时间
        self.GEN_TIME = get_str_time()

        # .h文件头部
        self.FILE_HEADER_H = \
            (f"/**\n"
             f" ******************************************************************************\n"
             f" * @file        {self.DEST_BASENAME_H}\n"
             f" * @author      {self.CURR_BASENAME}\n"
             f" * @version\n"
             f" * @date        {self.GEN_TIME}\n"
             f" * @brief       '{self.DEST_BASENAME_H}' converted by the file '{self.SRC_FILEPATH}'\n"
             f" * @license\n"
             f" ******************************************************************************\n"
             f" * @attention\n"
             f" *\n"
             f" ******************************************************************************\n"
             f" */\n"
             f"\n"
             f"#ifndef __{self.DEST_BASENAME_H.replace('.', '_').upper()}\n"
             f"#define __{self.DEST_BASENAME_H.replace('.', '_').upper()}\n"
             f"\n"
             f"#include <stdbool.h>\n"
             f"#include <stdint.h>\n"
             f"#include \"freertos/FreeRTOS.h\"\n"
             f"#include \"esp_log.h\"\n"
             f"\n"
             f"#ifdef __cplusplus\n"
             f"extern \"C\" {{\n"
             f"#endif\n"
             f"\n"
             f"/* 生成时间 */\n"
             f"#define {self.DEST_PREFIX.upper()}_GEN_TIME \"{self.GEN_TIME}\"\n"
             f"\n")
             
        # .h文件尾部
        self.FILE_TAIL_H = \
            (f"/* 通信变量 */\n"
             f"extern {self.DEST_PREFIX}_t {self.DEST_PREFIX};\n\n"
             f"/* 初始化通信变量互斥锁 */\n"
             f"void {self.DEST_PREFIX}_init_mutex(void);\n"
             f"/* 获取通信变量互斥锁 */\n"
             f"SemaphoreHandle_t* {self.DEST_PREFIX}_get_mutex(void);\n"
             f"\n"
             f"#ifdef __cplusplus\n"
             f"}}    // extern \"C\"\n"
             f"#endif\n"
             f"\n"
             f"#endif\n"
             f"\n"
             f"/****************EOF****************/\n")

        # .c文件头部
        self.FILE_HEADER_C = \
            (f"/**\n"
             f" ******************************************************************************\n"
             f" * @file        {self.DEST_BASENAME_C}\n"
             f" * @author      {self.CURR_BASENAME}\n"
             f" * @version\n"
             f" * @date        {self.GEN_TIME}\n"
             f" * @brief       '{self.DEST_BASENAME_C}' converted by the file '{self.SRC_FILEPATH}'\n"
             f" * @license\n"
             f" ******************************************************************************\n"
             f" * @attention\n"
             f" *\n"
             f" ******************************************************************************\n"
             f" */\n"
             f"\n"
             f"#include \"{self.DEST_PREFIX}.h\"\n"
             f"\n"
             f"/* 打印日志的名称 */\n"
             f"static const char *TAG = \"{self.DEST_PREFIX.upper()}\";\n\n"
             f"/* 通信变量 */\n"
             f"{self.DEST_PREFIX}_t {self.DEST_PREFIX} = {{0}};\n\n"
             f"/* 通信变量访问互斥锁 */\n"
             f"static SemaphoreHandle_t _mutex = NULL;\n\n"
             f"/**\n"
             f" * @brief 初始化通信变量互斥锁\n"
             f" * @param\n"
             f" * @retval\n"
             f" * @note\n"
             f" */\n"
             f"void {self.DEST_PREFIX}_init_mutex(void) {{\n"
             f"    _mutex = xSemaphoreCreateMutex();\n"
             f"    if (_mutex == NULL) {{\n"
             f"        ESP_LOGE(TAG, \"Failed to create '{self.DEST_PREFIX}' mutex\");\n"
             f"        esp_system_abort(\"Create mutex failed\");\n"
             f"    }}\n"
             f"}}\n\n"
             f"/**\n"
             f" * @brief 获取通信变量互斥锁\n"
             f" * @param\n"
             f" * @retval\n"
             f" * @note\n"
             f" */\n"
             f"SemaphoreHandle_t* {self.DEST_PREFIX}_get_mutex(void) {{\n"
             f"    return &_mutex;\n"
             f"}}\n")
             
        # .c文件尾部
        self.FILE_TAIL_C = \
            (f"\n"
             f"/****************EOF****************/\n")

        text_log(f"初始化信息完成", 'done')

    def parse_file(self) -> dict[str, list[DataObject]] | None:
        """
        将一个指定模板的Excel文件各工作表中行内容转为数据对象

        :return: 字典，如{bms:[do...],tms:[do...]...}
        :rtype: dict[str, list[DataObject]]
        """
        text_log(f"待解析文件{self.SRC_FILEPATH}")

        filepath = self.SRC_FILEPATH
        if not filepath:
            msg = f"待解析文件路径为空"
            raise GenCommVarException(msg)

        # 生成excel中间文件
        excel = openpyxl.load_workbook(filepath, data_only=True)
        prefix, suffix = get_basename(filepath)
        new_basename = '~$' + prefix + '.xlsx'
        filepath = filepath.replace(prefix + suffix, new_basename)
        # print(prefix, suffix, filepath)
        excel.save(filepath)
        excel.close()

        # 加载excel中间文件，解析各表单
        do_dict: dict[str, list[DataObject]] = {}
        excel = pd.ExcelFile(filepath)
        for sheet_name in excel.sheet_names:
            df = excel.parse(sheet_name, dtype=str)
            df.fillna('', inplace=True)
            # print(df.to_string())

            # 根据df表内容生成数据对象列表
            do_list: list[DataObject] = []
            for idx in range(0, df.shape[0]):
                row = df.loc[idx]  # 行数据

                byte_pos = str(row[DataObjectField.byte_pos]).strip()
                byte_pos = int(byte_pos) if isint(byte_pos) else None

                bit_pos = str(row[DataObjectField.bit_pos]).strip()
                bit_pos = int(bit_pos) if isint(bit_pos) else None

                do = DataObject(
                    id=int(str(row[DataObjectField.id]).strip()),
                    code=str(row[DataObjectField.code]).strip(),
                    name=str(row[DataObjectField.name]).strip(),
                    unit=str(row[DataObjectField.unit]).strip(),
                    data_type=str(row[DataObjectField.data_type]).strip(),
                    addr_type=str(row[DataObjectField.addr_type]).strip(),
                    byte_pos=byte_pos,
                    bit_pos=bit_pos,
                    is_record=True if(str(row[DataObjectField.is_record]).strip().upper()=='TRUE') else False,
                    is_retain=True if(str(row[DataObjectField.is_retain]).strip().upper()=='TRUE') else False,
                    access=str(row[DataObjectField.access]).strip(),
                    value_table=str(row[DataObjectField.value_table]).strip(),
                    comment=str(row[DataObjectField.comment]).strip()
                )

                do_list.append(do)  # 添加信号对象到列表
            do_dict[sheet_name] = do_list
        excel.close()
        os.remove(filepath)
        # pprint(do_dict)
        text_log(f"解析文件完成", 'done')
        return do_dict

    def gen_file_h(self, do_dict: dict[str, list[DataObject]]) -> None:
        """
        生成.h文件

        :param do_dict: 数据对象
        :type do_dict: dict[str, list[DataObject]]
        """
        text_log(f"待生成文件{self.DEST_FILEPATH_H}")

        filepath = self.DEST_FILEPATH_H
        if not filepath:
            msg = f"待生成文件路径为空"
            raise GenCommVarException(msg)
        if not do_dict:
            msg = f"待转化数据对象为空"
            raise GenCommVarException(msg)

        # 生成各工作表的结构体内容
        total = 0 # 数据项总数
        collect_struct_name = f"{self.DEST_PREFIX}_t"  # 总结构体名称
        collect_struct_content = ""  # 总结构体内容
        file_body = ""  # 文件内容
        for group, do_list in do_dict.items():
            struct_name = f"{self.DEST_PREFIX}_{group}_t" # 结构体名称

            struct_content = "" # 结构体内容
            for do in do_list:
                addr = f"{do.addr_type}{do.byte_pos}"
                decl = f"{do.data_type} {do.code};"
                if do.bit_pos >= 0:
                    addr += f".{do.bit_pos}"
                    decl = f"{do.data_type} {do.code} : 1;"

                value_table = "\n"
                for line in do.value_table.splitlines():
                    value_table += f"     *    {line}\n"

                comment = "\n"
                for line in do.comment.splitlines():
                    comment += f"     *    {line}\n"

                struct_content += (f"    /* {DataObjectField.id}:{do.id} "
                                   f"地址:{addr} "
                                   f"{DataObjectField.name}:{do.name} "
                                   f"{DataObjectField.unit}:{do.unit} "
                                   f"{DataObjectField.is_record}:{do.is_record} "
                                   f"{DataObjectField.is_retain}:{do.is_retain} "
                                   f"{DataObjectField.access}:{do.access}\n"
                                   f"     * {DataObjectField.value_table}:{value_table}"
                                   f"     * {DataObjectField.comment}:{comment}"
                                   f"     */\n"
                                   f"    {decl}\n\n")

            file_body += (f"/**\n"
                             f" * @struct {struct_name}\n"
                             f" * @brief 共{len(do_list)}个数据项在表'{group}'中\n"
                             f" */\n"
                             f"#pragma pack(push,1)\n"
                             f"typedef struct {struct_name} {{\n"
                             f"{struct_content}"
                             f"}} {struct_name};\n"
                             f"#pragma pack(pop)\n\n")

            collect_struct_content += (f"    /* 共{len(do_list)}个数据项在'{group}'中 */\n"
                                       f"    {struct_name} {group};\n\n")
            total += len(do_list)

        # 汇总
        file_body += (f"/**\n"
                         f" * @struct {collect_struct_name}\n"
                         f" * @brief 共{total}个数据项在'{self.DEST_PREFIX}'中\n"
                         f" */\n"
                         f"#pragma pack(push,1)\n"
                         f"typedef struct {collect_struct_name} {{\n"
                         f"{collect_struct_content}"
                         f"}} {collect_struct_name};\n"
                         f"#pragma pack(pop)\n\n")


        # 确保输出目录存在
        dest_dir = os.path.dirname(self.DEST_FILEPATH_H)
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir, exist_ok=True)
            text_log(f"目标路径不存在, 已创建: {dest_dir}", 'warning')
        # 写入文件
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(self.FILE_HEADER_H + file_body + self.FILE_TAIL_H)
        msg = f"文件生成完成: '{self.DEST_FILEPATH_H}' 从 '{self.SRC_FILEPATH}'"
        text_log(msg, 'done')

    def gen_file_c(self) -> None:
        """
        生成.c文件

        """
        text_log(f"待生成文件{self.DEST_FILEPATH_C}")

        filepath = self.DEST_FILEPATH_C
        if not filepath:
            msg = f"待生成文件路径为空"
            raise GenCommVarException(msg)

        # 写入文件
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(self.FILE_HEADER_C + self.FILE_TAIL_C)
        msg = f"文件生成完成: '{self.DEST_FILEPATH_C}' 从 '{self.SRC_FILEPATH}'"
        text_log(msg, 'done')

if __name__ == '__main__':
    try:
        args = sys.argv
        text_log(f"正在运行: {os.path.abspath(args[0])}", 'warning')

        read_filename = ''
        write_filepath = ''
        if len(args) >= 3:
            read_filename = os.path.abspath(args[1])
            write_filepath = os.path.abspath(args[2])
        else:
            msg =f"请参照如下方式运行: python *.py read_filename(eg'./mbreg.xlsm') write_filepath(eg'./main/comm')"
            raise GenCommVarException(msg)

        gen_mbreg = GenCommVar(read_filename=read_filename, write_filepath=write_filepath)
        gen_mbreg.gen_file_h(gen_mbreg.parse_file())
        gen_mbreg.gen_file_c()
        sys.exit(0)
    except Exception as e:
        text_log(f"{e}", 'error')
        text_log(f"{traceback.format_exc()}")
        sys.exit(1)
    finally:
        pass
