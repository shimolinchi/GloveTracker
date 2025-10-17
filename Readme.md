# GloveTracker

## 项目简介

GloveTracker 是一个用于手套数据采集、可视化与电机控制的多线程 C++ 应用。该项目集成了 Manus SDK、PCAN 通信、EasyX 图形界面等模块，实现了手套数据的实时采集、图形交互、电机控制与数据记录。

## 主要功能

- **手套数据采集**：通过 Manus SDK 客户端与 ManusCore 通信，获取手套传感器数据。
- **电机控制**：将手套数据实时转换为电机控制信号，通过 PCAN 总线发送。
- **图形交互**：基于 EasyX 图形库，显示手套与电机状态，支持用户交互与手指极限角校正。
- **数据记录**：将手套与电机数据同步记录到文件，便于后续分析。

## 主要模块说明

- `Main.cpp`：主程序入口，负责各模块初始化与多线程启动。
- `SDKClient`：Manus SDK 客户端，负责与手套硬件通信。
- `PCANBasic`：PCAN 通信接口，负责与电机控制器的数据交互。
- `MotorController`：电机控制逻辑，将手套数据转为电机指令。
- `GraphicInteractor`：图形界面与用户交互模块。
- `Recorder`：数据记录模块。

## 线程结构

主程序共启动四个线程：

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

1. 安装 EasyX、Manus SDK，并确保相关 DLL/Lib 文件已放置在项目目录下。
2. 使用 Visual Studio 打开 `SDKClient.vcxproj` 工程文件。
3. 配置好 include/lib 路径，编译生成可执行文件。
4. 运行 `SDKClient_Windows.exe`，程序将自动启动各线程。

## 文件结构简述

- `src/`：主要功能模块实现
- `include/`：头文件
- `ManusSDK/`：Manus SDK 相关文件
- `PlatformSpecific/`：平台相关实现
- `Output/`：编译输出
- `app/1.0/`：发布版及依赖

## 注意事项

- 需连接 Manus 手套与电机控制器，并确保 PCAN 设备驱动正常。
- 日志与数据文件默认输出在 `Output/x64/Debug/` 目录。
