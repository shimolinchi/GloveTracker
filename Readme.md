# GloveTracker

![项目演示](example.gif)

## 项目简介

本项目通过Manus数据手套采集手部姿态数据，将其映射到 睿研 H1（16）灵巧手。集成了 Manus SDK、PCAN 通信、EasyX 图形界面等模块，实现了手套数据的实时采集、图形交互、电机控制与数据记录。

[Manus官网](https://www.manus-meta.com/products/quantum-metagloves)

## 使用说明

**须在windows系统下使用**

下载项目的release文件中的 GloveTracker.exe 与 ManusSDK.dll 放于同一目录下，连接好灵巧手。启动 manus core 程序,[manus core 下载位置](https://docs.manus-meta.com/latest/Resources/)，推荐3.0.1版本，连接并配置好用户数据后，按要求进行手套校准。

启动GloveTracker.exe程序，若映射不准，按下Calibrate按键后，按照提示进行校准。校准的机制是记录每个手指的侧摆、pip与dip关节弯曲零点。

此外，校准的时候第一步尽量保持四根手指水平伸直，四根手指可以有一点缝隙（这样可以展示向内侧摆），如果完全并拢则无法向中间侧摆。校准第二阶段需要确保四根手指的关节尽可能弯曲到最大值，这样才能保证角度行程采样正确。



## 主要功能

- **手套数据采集**：通过 Manus SDK 客户端与 ManusCore 通信，获取手套传感器数据。
- **电机控制**：将手套数据实时转换为电机控制信号，通过 PCAN 总线发送。
- **图形交互**：基于 EasyX 图形库，显示手套与电机状态，支持用户交互与手指极限角校正。
- **数据记录**：将手套与电机数据同步记录到文件，便于后续分析。

### 图形界面按键说明

+ Motor Data / Glove Data：查看手套关节角度数据、映射电机数据（包含电机驱动位置、速度、电流/回读位置、速度、电流）
+ Calibrate：启动手套校准程序，用户按照要求先将手掌平放桌面，再握拳
+ Recording：电机数据记录，采集电机的驱动数据选项。
    + start recording：开始记录，按照给定频率将数据一直输入到文件中
    + stop recording：停止记录，停止持续记录
    + record once：记录当前姿态的一组数据
    + set frequency：设置持续记录的频率
+ exchange hand：若存在多个手套和灵巧手，切换映射左右手
+ pointing position：查看/修改对指时的点击位置数据，从左到右（列）分别为从食指对大拇指到小拇指对大拇指，改为0为不设置位置，大于0则设置位置。此功能用于应对不同的手标定差异的情况，或者定制一些特殊对指动作。


## 主要模块说明

- `Main.cpp`：主程序入口，负责各模块初始化与多线程启动。
- `SDKClient`：Manus SDK 客户端，负责与手套硬件通信。
- `PCANBasic`：PCAN 通信接口，负责与电机控制器的数据交互。
- `MotorController`：电机控制逻辑，将手套数据转为电机指令。
- `GraphicInteractor`：图形界面与用户交互模块。
- `Recorder`：数据记录模块。

## 线程结构

主程序启动如下线程：

1. **SDKThread**：初始化并运行 SDK 客户端，负责手套数据采集。
2. **CtrlThread**：电机控制线程，将手套数据转为电机控制信号并发送。
3. **GraphThread**：图形界面线程，负责用户交互与状态显示。
4. **RecorderThread**：数据记录线程，负责同步记录手套与电机数据。

## 依赖环境

- Windows 平台（依赖 EasyX、Manus SDK、PCAN 驱动）
- C++17 及以上
- EasyX 图形库
- Manus SDK
- PCANBasic 驱动

## 编译与运行

### MicroSoft Visual Studio 

1. 安装 EasyX、Manus SDK，检查相关 DLL/Lib 文件已放置在项目目录下。
2. 使用 Visual Studio 打开 `SDKClient.vcxproj` 工程文件。
3. 配置好 include/lib 路径，编译生成可执行文件。
4. 运行调试/发布程序程序将自动启动各线程。

### vscode + cl 编译器

1. 下载并配置好 MicroSoft Visual Studio 环境
2. 配置好cl环境变量（[方法见此](https://blog.csdn.net/en_Wency/article/details/124767742)）或在 x64 Native Tools Command Prompt for VS 2022 终端中启动vscode
3. 检查.vscode文件夹中的tasks.json、launch.json文件（仅提供了release版本）
4. 运行程序

## 文件结构简述

- `src/`：主要功能模块实现
- `include/`：头文件
- `ManusSDK/`：Manus SDK 相关文件
- `PlatformSpecific/`：平台相关实现
- `Output/`：编译输出
