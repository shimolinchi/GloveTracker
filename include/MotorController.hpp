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

    void UpdateMotorData();
};