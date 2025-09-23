// #pragma once
// #include <iostream>
// #include <vector>
// #include <cstdint>
// #include <chrono>
// #include <thread>
// #include <stdexcept>
// #include <iomanip> // 用于 std::hex, std::setw
// #include <optional> // 用于 std::optional
// #include <cmath>   // 用于 std::abs
// #include <algorithm> // 用于 std::min, std::max
// #include "SDKClient.hpp"
// # include "PCANBasic.hpp"
// # include "RyHandLib.h"
// # include "math_utils.hpp"

// // typedef unsigned long long  u64_t;

// // typedef struct
// // {
// //   u64_t cmd: 8;            // 前面的CMD
// //   u64_t ucStatus: 8;    // 故障状态 ，0 表示无故障，异常详情见 enret_t
// //   u64_t ub_P: 12;       // 当前位置，0-4095 对应 0到满行程
// //   u64_t ub_V: 12;       // 当前速度，-2048~2047 单位 0.001行程/s
// //   u64_t ub_I: 12;       // 当前电流，-2048~2047 单位 0.01A
// //   u64_t ub_F: 12;       // 当前位置，0-4095 对应手指压力传感器Adc原始值
// // } FingerServoInfo_t;

// class MotorController {
// public:
//         MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client) ;
    
// private:
//     std::vector<float> position_norm = std::vector<float>(16, 0.0f);     // 当前电机位置
//     std::vector<int> position_now = std::vector<int>(16, 0);     // 当前电机位置
//     std::vector<int> position_drive = std::vector<int>(16, 0);   // 目标驱动位置
//     std::vector<int> velocity = std::vector<int>(16, 1000);    // 电机速度
//     std::vector<int> current = std::vector<int>(16, 100); // 电机电流
//     const std::vector<float> thumb_ip_limit = { 0.0f, 65.0f };
//     const std::vector<float> dip_limit = { 0.0f, 65.0f };
//     const std::vector<float> pip_limit = { 0.0f, 100.0f };
//     const std::vector<float> mcp_stretch_limit = { 8.0f, 90.0f };
//     const std::vector<float> thumb_mcp_limit = { 0.0f, 70.0f };
//     const std::vector<float> thumb_cmc_stretch_limit = { 45.0f, 0.0f };
//     const std::vector<float> thumb_cmc_spread_limit = { 10.0f, 55.0f };
//     const std::vector<float> index_mcp_spread_limit = {4.0f, - 32.0f };
//     const std::vector<float> middle_mcp_spread_limit = { 6.0f, -26.0f };
//     const std::vector<float> ring_mcp_spread_limit = { 7.0f, -22.0f };
//     const std::vector<float> pinky_mcp_spread_limit = { 12.0f, -23.0f };
//     SDKClient* client;
//     PCANBasic* pcan;
//     TPCANHandle PcanHandle;

//     /**
//      * @brief [私有方法] 非阻塞接收一条CAN消息.
//      */
//     std::optional<TPCANMsg> ReceiveCANMessage() ;
//     /**
//      * @brief [私有方法] 解析手指反馈的8字节数据包.
//      */
//     std::optional<FingerServoInfo_t> ParseFingerFeedback(const uint8_t* data, uint8_t len) ;
// public:
//     void Run(); 
// };













#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <iomanip>
#include <optional>
#include <cmath>
#include <algorithm>
#include "SDKClient.hpp"
#include "PCANBasic.hpp"
#include "RyHandLib.h"
#include "math_utils.hpp"

class MotorController {
public:
    SDKClient* client;
    std::vector<float> position_norm;
    std::vector<int> position_now;      // [反馈] 当前电机位置
    std::vector<int> position_drive;    // [指令] 目标驱动位置
    std::vector<int> velocity_drive;     // [指令] 电机速度
    std::vector<int> current_drive;      // [指令] 电机电流
    std::vector<int> velocity_now; // [反馈] 当前电机速度
    std::vector<int> current_now;  // [反馈] 当前电机电流
    MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client);
    void Run();

private:
    // --- 成员变量 ---
    PCANBasic* pcan;
    TPCANHandle PcanHandle;

    // --- 静态常量定义 (极限值) ---
    static const std::vector<float> thumb_ip_limit;
    static const std::vector<float> dip_limit;
    static const std::vector<float> pip_limit;
    static const std::vector<std::vector<float>> mcp_stretch_limit;
    static const std::vector<float> thumb_mcp_limit;
    static const std::vector<float> thumb_cmc_stretch_limit;
    static const std::vector<float> thumb_cmc_spread_limit;
    static const std::vector<float> index_mcp_spread_limit;
    static const std::vector<float> middle_mcp_spread_limit;
    static const std::vector<float> ring_mcp_spread_limit;
    static const std::vector<float> pinky_mcp_spread_limit;

    // --- 新增的配置结构体和静态配置表 ---
    struct FingerConfig {
        int base_data_index; // ErgonomicsData中的起始索引
        int base_pos_index;  // position_norm中的起始索引
        const std::vector<float>& spread_limit; // 侧摆极限值的引用
        float spread_coeff_neg; // 侧摆为负时的系数，neg为negetive
        float spread_coeff_pos; // 侧摆为正时的系数，pos为positive，不是position
    };
    static const std::vector<FingerConfig> s_fingerConfigs;


    // --- 私有辅助函数 ---
    void ProcessThumb(const ErgonomicsData& data);
    void ProcessFinger(const ErgonomicsData& data, const FingerConfig& config);
    void MotorControl();

    // std::optional<TPCANMsg> ReceiveCANMessage();
    // std::optional<FingerServoInfo_t> ParseFingerFeedback(const uint8_t* data, uint8_t len);

    void UpdateMotorData();
};