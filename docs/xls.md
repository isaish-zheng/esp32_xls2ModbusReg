xls文件说明

## 启用宏
**在`Excel`中启用宏功能，以便使用`VBA`编写自定义函数和宏。**
1. 打开`Excel`文件。
2. 在菜单栏中，选择`文件`->`选项`。
3. 在弹出的`Excel选项`对话框中，选择`信任中心`->`信任中心设置`->`宏设置`。
4. 选择`启用VBA宏`、`启用VBA宏时启用Excel 4.0 宏`。
5. 点击`确定`保存设置。
6. 将文件另存为`.xlsm`格式后***重新打开文件***。

---

## 定义名称
**菜单栏`公式`->`名称管理器`新建名称：**
- 名称：`SHEETNAMES`，引用位置：`=GET.WORKBOOK(1)`，功能：获取所有`sheet`名称。
- 名称：`LEFTSHEET`，引用位置：`=INDEX(SHEETNAMES,SHEET()-1)`，功能：获取当前工作表左侧`sheet`名称。

---

## 定义函数
**编写函数：**
1. 在菜单栏中，选择`开发工具`->`Visual Basic`。
2. 在弹出的`Visual Basic for Applications`窗口中，右键`插入`->`模块`。
3. 在`模块`中，编写自定义函数的代码。
4. 保存并关闭`Visual Basic for Applications`窗口。
```
' 函数功能：
'   根据数据类型获取地址类型
' 参数说明：
'   dataType：数据类型
Function GetAddrType(dataType As String) As String
    Select Case dataType
        Case "bool"
            GetAddrType = "M"
        Case "uint8_t", "int8_t"
            GetAddrType = "MB"
        Case "uint16_t", "int16_t"
            GetAddrType = "MW"
        Case "uint32_t", "int32_t", "float"
            GetAddrType = "MD"
        Case "uint64_t", "int64_t", "double"
            GetAddrType = "ML"
        Case Else
            GetAddrType = Null
    End Select
End Function
```
```
' 函数功能：
'   根据地址类型获取类型长度
' 参数说明：
'   addrType：地址类型
Function GetTypeSize(addrType As String) As Integer
    Select Case addrType
        Case "M", "MB"
            GetTypeSize = 1
        Case "MW"
            GetTypeSize = 2
        Case "MD"
            GetTypeSize = 4
        Case "ML"
            GetTypeSize = 8
        Case Else
            GetTypeSize = Null
    End Select
End Function
```

## 各列属性
**表中各列对应的属性：**
| A | B | C | D | E | F | G | H | I | J | K | L | M |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 序号 | 编号 | 名称 | 单位 | 数据类型 | 地址类型 | 字节地址 | 位地址 | 记录 | 保持 | 权限 | 真值表 | 备注 |

---



## 数据类型(E列)
**允许的数据类型：**
| bool | uint8_t | uint16_t | uint32_t | uint64_t | int8_t | int16_t | int32_t | int64_t | float | double |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |

**为`数据类型`一列添加`数据验证`，以便在Excel中限制输入的数据类型。**
1. 选择要设置数据验证的单元格。
2. 在菜单栏中，选择`数据`->`数据验证`。
3. 在弹出的`数据验证`对话框中，选择`设置`选项卡。
4. 在`允许`下拉列表中选择`序列`。
5. 选择`忽略空值`、`提供下拉箭头`。
6. 在`来源`框中输入以下数据类型：`bool,uint8_t,uint16_t,uint32_t,uint64_t,int8_t,int16_t,int32_t,int64_t,float,double`
6. 点击`确定`保存设置。

---

## 地址类型(F列)
**允许的地址类型：**
| 类型 | 位长度 |
| :---: | :---: |
| M | 1 |
| MB | 8 |
| MW | 16 |
| MD | 32 |
| ML | 64 |

**计算公式，例如首个变量：**
`=GetAddrType(E2)`

---

## 位地址(H列)
**计算公式，例如首个变量：**
`=IFS(ROW()=2,IF(F2="M",0,-1), F2<>"M",-1, F1<>"M",0, H1=7,0, H1<7,H1+1)`

---

## 字节地址(G列)
**计算公式**
- **首个`sheet`的首个变量`字节地址`手动填充(一般为`0`)，后续`sheet`的`字节地址`通过公式自动计算。**
- **非首个变量`字节地址`计算公式，例如第二个变量：**
`=IFS(ROW()=2,0, H3<=0,G2+GetTypeSize(F2), H3>0,G2)`
- **非首个`sheet`的首个变量`字节地址`计算公式：**
`=INDIRECT(LEFTSHEET&"!"&"G"&COUNTA(INDIRECT(LEFTSHEET&"!"&"G:G")))+GetTypeSize(INDIRECT(LEFTSHEET&"!"&"F"&COUNTA(INDIRECT(LEFTSHEET&"!"&"G:G"))))`

---

**其它属性可手动输入。**

---

结束