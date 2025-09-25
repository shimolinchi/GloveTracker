#include "stdafx.h"
#include "MotorController.hpp"
#include <iostream>

// --- 初始化所有静态常量 ---
std::vector<float> MotorController::thumb_ip_limit = { 0.0f, 65.0f };
std::vector<float> MotorController::dip_limit = { 0.0f, 65.0f };
std::vector<float> MotorController::pip_limit = { 0.0f, 100.0f };
std::vector<std::vector<float>> MotorController::mcp_stretch_limit = { {0.0f, 75.0f}, {0.0f, 90.0f}, {0.0f, 75.0f}, {0.0f, 80.0f} };
std::vector<float> MotorController::thumb_mcp_limit = { 0.0f, 70.0f };
std::vector<float> MotorController::thumb_cmc_stretch_limit = { 54.0f, 30.0f };
std::vector<float> MotorController::thumb_cmc_spread_limit = { 8.0f, 54.0f };

// --- 初始化手指配置表 ---


MotorController::MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client) :
    pcan(pcan),
    PcanHandle(PcanHandle),
    client(client),
    position_norm(16, 0.0f),
    position_now(16, 0),
    position_drive(16, 0),
    velocity_drive(16, 1000),
    velocity_now(16, 0),
    current_drive(16, 200),
    mcp_spread_zero_position(4, 0.0f),
    calibrating_process(CalibrateProcess::FINISH),
    glove_data(client->GetGloveErgoData(true)),
    base_data_index ({4, 8, 12, 16}),
    base_pos_index  ({3, 6, 9, 12}),
    spread_limit    ({18.0f, 15.0f, 15.0f, 18.0f}),
    spread_coeff_neg({0.6f, 0.45f, 0.45f, 0.45f}),
    spread_coeff_pos({0.45f, 0.45f, 0.45f, 0.6f})
    {
    }

/**
 * @brief 处理大拇指的映射逻辑 (特殊情况)
 */
void MotorController::ProcessThumb() {
    
    float ip_norm      = LinearMap(glove_data.data[3], thumb_ip_limit[0], thumb_ip_limit[1], 0.0f, 1.0f);
    float mcp_norm     = LinearMap(glove_data.data[2], thumb_mcp_limit[0], thumb_mcp_limit[1], 0.0f, 1.0f);
    float cmc_spread_norm = LinearMap(std::max(glove_data.data[0], 0.0f), thumb_cmc_spread_limit[0], thumb_cmc_spread_limit[1], 0.0f, 1.0f);
    float cmc_stretch_norm = LinearMap(glove_data.data[1], thumb_cmc_stretch_limit[0], thumb_cmc_stretch_limit[1], 0.0f, 1.0f);

    position_norm[2]  = std::clamp(ip_norm, 0.0f, 1.0f);
    position_norm[15] = std::clamp(0.3f * cmc_stretch_norm + 0.7f * cmc_spread_norm, 0.0f, 1.0f);
    float conpensate = LinearMap(cmc_spread_norm, 0.0f, 0.6f, 0.889f, 0.0f);

    std::vector<float> mcp_limit_r = {thumb_cmc_stretch_limit[0] - 31.0f * (position_norm[15]), thumb_cmc_stretch_limit[1] - 31.0f * (position_norm[15])};
    float cmc_stretch_norm_r = LinearMap(glove_data.data[1], mcp_limit_r[0], mcp_limit_r[1], 0.0f, 1.0f);

    position_norm[0] = std::clamp((cmc_stretch_norm_r), 0.0f, 1.0f);
    position_norm[1] = std::clamp((cmc_stretch_norm_r), 0.0f, 1.0f);
}

/**
 * @brief 通用处理函数，用于食指、中指、无名指和小指
 */
void MotorController::ProcessFinger(int finger_index) {
    float mcp_spread_norm = LinearMap(glove_data.data[base_data_index[finger_index]],  mcp_spread_zero_position[finger_index] + spread_limit[finger_index], mcp_spread_zero_position[finger_index] - spread_limit[finger_index], -1.0f, 1.0f);
    float mcp_stretch_norm = LinearMap(glove_data.data[base_data_index[finger_index] + 1], mcp_stretch_limit[finger_index][0], mcp_stretch_limit[finger_index][1], 0.0f, 1.0f);

    // 一定程度上抑制当mcp弯曲较大时（趋近握拳）的mcp侧摆角。
    mcp_spread_norm = copysign(std::max(0.0f, std::abs(mcp_spread_norm) - 0.8f * mcp_stretch_norm), mcp_spread_norm);
    // mcp_spread_norm = copysign(std::max(0.0f, std::abs(mcp_spread_norm) - 0.6f * mcp_stretch_norm), mcp_spread_norm);

    float pip_norm        = LinearMap(glove_data.data[base_data_index[finger_index] + 2], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
    float dip_norm        = LinearMap(glove_data.data[base_data_index[finger_index] + 3], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

    position_norm[base_pos_index[finger_index] + 2] = std::clamp(0.5f * pip_norm + 0.5f * dip_norm, 0.0f, 1.0f);

    position_norm[base_pos_index[finger_index] + 1] = (mcp_spread_norm < 0)
        ? std::clamp(mcp_stretch_norm - spread_coeff_neg[finger_index] * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f)
        : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

    position_norm[base_pos_index[finger_index]] = (mcp_spread_norm > 0)
        ? std::clamp(mcp_stretch_norm + spread_coeff_pos[finger_index] * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f)
        : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);
}

void MotorController::Calibrate() {

    int sampleCount = 0;
    std::vector<float> sum = {0.0f, 0.0f, 0.0f, 0.0f};
    while(calibrating_process != CalibrateProcess::STEP1) {std::this_thread::sleep_for(std::chrono::milliseconds(10));}
    while (calibrating_process == CalibrateProcess::STEP1) {
        for(int i = 0; i < 4; i++){
            sum[i] += glove_data.data[i * 4 + 4];
        }
        sampleCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // std::cout << mcp_spread_zero_position[0] << std::endl;
    }

    for(int i = 0; i < 4; i++){
        mcp_spread_zero_position[i] = sum[i] / sampleCount;
    }
    for (int i = 0; i < 200; i++){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << mcp_spread_zero_position[0];
    }

}


/**
 * @brief 主循环函数
 */
void MotorController::Run() {
    
    while (true) {
        ErgonomicsData leftData = client->GetGloveErgoData(true); // true = 左手
        glove_data = client->GetGloveErgoData(true);
        if (calibrating_process == CalibrateProcess::START) {
            Calibrate();
        }

        // 1. 单独处理大拇指
        ProcessThumb();

        // 2. 循环处理其余四个手指
        for (int i = 0; i < 4; i++) {
            ProcessFinger(i);
        }

        // 3. 将归一化位置转换为电机驱动值
        for (int i = 0; i < 16; ++i) {
            position_drive[i] = static_cast<int>(position_norm[i] * 4096.0f);
        }
        
        // 4. 更新电机读取数据
        UpdateMotorData();

        // 5. 发送最终指令
        MotorControl();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// --- 其他成员函数的实现 (与之前版本相同) ---

void MotorController::MotorControl() {
     for (int i = 0; i < 16; ++i) {
         int position_error = abs(position_drive[i] - position_now[i]);
         velocity_drive[i] = std::clamp(200 + int(float(position_error) / 4096.0 * 3000), 200, 2000);
     }
    SendAction(*pcan, PcanHandle, velocity_drive, position_drive, current_drive);    
}


/**
 * @brief 从CAN总线读取并解析所有传入的电机数据，并更新内部状态.
 */
void MotorController::UpdateMotorData() {
    // 循环读取，直到CAN接收缓冲区为空
    while (true) {
        TPCANMsg msg;
        TPCANTimestamp ts;
        TPCANStatus result = pcan->Read(PcanHandle, msg, ts);

        if (result == PCAN_ERROR_QRCVEMPTY) {
            // 缓冲区已空，退出循环
            break;
        }
        
        if (result != PCAN_ERROR_OK) {
            // 发生读取错误 (非空队列)
            std::string errorText;
            pcan->GetErrorText(result, 0, errorText);
            std::cerr << "PCAN Read error: " << errorText << std::endl;
            continue; // 继续尝试读取下一条消息
        }

        // 检查ID是否为电机反馈ID (257-272) 且数据长度是否为8
        if (msg.ID >= 257 && msg.ID <= 272 && msg.LEN == 8) {
            // 解析8字节数据
            uint64_t raw = 0;
            memcpy(&raw, msg.DATA, 8);
            
            // 提取数据
            int pos   = (raw >> 16) & 0xFFF;
            int v_raw = (raw >> 28) & 0xFFF;
            int i_raw = (raw >> 40) & 0xFFF;

            // 符号扩展
            int vel = (v_raw > 2047) ? (v_raw - 4096) : v_raw;
            int cur = (i_raw > 2047) ? (i_raw - 4096) : i_raw;

            // 根据ID计算电机索引并更新对应的状态变量
            int motor_index = msg.ID - 257;
             position_now[motor_index] = pos;
             velocity_now[motor_index] = vel;/*
             current_now[motor_index] = cur;*/
        }
    }
}
