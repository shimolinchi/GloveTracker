# pragma once
# include <iostream>
# include <vector>
# include <cstdint>
# include <chrono>
# include <thread>
# include <stdexcept>
# include <iomanip>
# include <optional>
# include <algorithm>
# include <numeric>
# include <fstream>
# include <filesystem>
# include "SDKClient.hpp"
# include "PCANBasic.hpp"
# include "RyHandLib.h"
# include "math_utils.hpp"


enum class CalibrateProcess {
    START,
    STEP1,
    SWITCH,
    STEP2,
    STEP3,
    STEP4,
    FINISH
};

class MotorController {
public:
    SDKClient* client;
    std::vector<float> position_norm;
    std::vector<int> position_now;       // [反馈] 当前电机位置
    std::vector<int> position_drive;     // [指令] 目标驱动位置
    std::vector<int> velocity_drive;     // [指令] 电机速度
    std::vector<int> current_drive;      // [指令] 电机电流
    std::vector<int> velocity_now;       // [反馈] 当前电机速度
    std::vector<int> current_now;        // [反馈] 当前电机电流
    CalibrateProcess calibrating_process;
    ErgonomicsData glove_data;
    ClientSkeleton skeleton;
    const std::string pointing_position_file_name;
    bool hand_side;

    MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client, bool hand_side);
    // void SavePointingPosition();
    void ChangeGloveHandSide();
    void SetPointingPosition(int finger_index, int motor_index, int position);
    int  GetPointingPosition(int finger_index, int motor_index);
    void Run();

private:
    // --- 成员变量 ---
    PCANBasic* pcan;
    TPCANHandle PcanHandle;

    // --- 静态常量定义 (极限值) ---
    static std::vector<float> thumb_ip_limit;
    static std::vector<std::vector<float>> dip_limit;
    static std::vector<std::vector<float>> pip_limit;
    static std::vector<std::vector<float>> mcp_stretch_limit;
    static std::vector<float> thumb_mcp_limit;
    static std::vector<float> thumb_cmc_stretch_limit;
    static std::vector<float> thumb_cmc_spread_limit;

    std::vector<float> mcp_spread_zero_position;
    std::vector<int> base_data_index = {};
    std::vector<int> base_pos_index = {};
    std::vector<float> spread_limit = {};
    std::vector<float> spread_coeff_neg = {};
    std::vector<float> spread_coeff_pos = {};

	float pointing_threadhold = 0.08f; // 对指时的阈值,对应指尖的距离
	float pointing_optimization_strength = 0.012f; // 对指时的优化强度
    std::vector<float> tip_distances; // 四根手指指尖分别与大拇指指尖的距离,单位（米）
    
    std::vector<std::vector<int>> pointing_motor_position ; // 对指时电机位置数据，为0表示和对指无关的电机
    std::vector<std::vector<int>> second_pointing_motor_position ; // 对指时电机位置数据，为0表示和对指无关的电机

    std::vector<std::vector<float>> pointing_motor_position_norm;  // 对指时电机位置数据,为上变量归一化结果

    // --- 新增的配置结构体和静态配置表 ---
    struct FingerConfig {
        int base_data_index; // ErgonomicsData中的起始索引
        int base_pos_index;  // position_norm中的起始索引
        float spread_limit; // 侧摆极限值的引用
        float spread_coeff_neg; // 侧摆为负时的系数，neg为negetive
        float spread_coeff_pos; // 侧摆为正时的系数，pos为positive，不是position
    };


    // --- 私有辅助函数 ---
    void Calibrate();
    void ProcessThumb();
    void ProcessFinger(int finger_index);  //0~3 分别为食指~小拇指
    void MotorControl();
    void PointingOptimize();

    void UpdateMotorData();
    void UpdateTipDistance();
};