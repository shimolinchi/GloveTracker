// #include "stdafx.h"
// # include "MotorController.hpp"
// #include <iostream>

// MotorController::MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client) : 
//     position_norm(16, 0.0f),
//     position_now(16, 0),
//     position_drive(16, 0),
//     velocity(16, 1000),
//     current(16, 100),
//     client(client),
//     pcan(pcan),
//     PcanHandle(PcanHandle) {

//     }

// std::optional<TPCANMsg> MotorController::ReceiveCANMessage() {

//         TPCANMsg msg;
//         TPCANTimestamp ts;
//         TPCANStatus result = pcan->Read(PcanHandle, msg, ts);

//         if (result == PCAN_ERROR_OK) {
//             return msg;
//         } else if (result != PCAN_ERROR_QRCVEMPTY) {
//             std::string errorText;
//             pcan->GetErrorText(result, 0, errorText);
//             std::cerr << "PCAN Read error: " << errorText << std::endl;
//         }
//         return std::nullopt;
//     }
   
// std::optional<FingerServoInfo_t> MotorController::ParseFingerFeedback(const uint8_t* data, uint8_t len) {
//         if (len != 8) {
//             return std::nullopt;
//         }

//         // 将8字节数据按小端序合并成一个64位整数
//         uint64_t raw = 0;
//         memcpy(&raw, data, 8);

//         FingerServoInfo_t state;
//         // state.cmd    = (raw >> 0)  & 0xFF;
//         state.ucStatus = (raw >> 8)  & 0xFF;
//         state.ub_P      = (raw >> 16) & 0xFFF;
        
//         // 提取12位原始速度和电流
//         int v_raw = (raw >> 28) & 0xFFF;
//         int i_raw = (raw >> 40) & 0xFFF;
        
//         state.ub_F = (raw >> 52) & 0xFFF;

//         // 对速度和电流进行12位有符号数的手动符号扩展
//         state.ub_V = (v_raw > 2047) ? (v_raw - 4096) : v_raw;
//         state.ub_I = (i_raw > 2047) ? (i_raw - 4096) : i_raw;

//         return state;
//     }

// void MotorController::Run() {

//     while (true)

//     {
//         ErgonomicsData leftData = client->GetGloveErgoData(true); // true = 左手

//         //大拇指
// 		float thumb_ip_norm = LinearMap(leftData.data[3], thumb_ip_limit[0], thumb_ip_limit[1], 0.0f, 1.0f);
//         float thumb_mcp_norm = LinearMap(leftData.data[2], thumb_mcp_limit[0], thumb_mcp_limit[1], 0.0f, 1.0f);
// 		float thumb_cmc_stretch_norm = LinearMap(leftData.data[1], thumb_cmc_stretch_limit[0], thumb_cmc_stretch_limit[1], 0.0f, 1.0f);
// 		float thumb_cmc_spread_norm = LinearMap(leftData.data[0], thumb_cmc_spread_limit[0], thumb_cmc_spread_limit[1], 0.0f, 1.0f);

// 		position_norm[2] = std::clamp(thumb_ip_norm, 0.0f, 1.0f);
//         position_norm[15] = std::clamp(0.5f * thumb_cmc_stretch_norm + 0.5f * thumb_cmc_spread_norm, 0.0f, 1.0f);
//         position_norm[0] = std::clamp(thumb_mcp_norm, 0.0f, 1.0f);
//         position_norm[1] = std::clamp(thumb_mcp_norm, 0.0f, 1.0f);
//         /*position_norm[0] = std::clamp(thumb_mcp_norm - 0.1f * thumb_cmc_spread_norm - 0.1f * thumb_cmc_stretch_norm, 0.0f, 1.0f);
//         position_norm[1] = std::clamp(thumb_mcp_norm + 0.1f * thumb_cmc_spread_norm + 0.9f * thumb_cmc_stretch_norm, 0.0f, 1.0f);*/
        

//         // ​​左手手指的 MCP 侧摆关节​​：向大拇指方向运动​​ 的极限为 ​- 1，​​​​远离大拇指方向（反方向）运动​​ 的极限为 ​​ + 1​
//         // 食指
//         float index_mcp_spread_norm = LinearMap(leftData.data[4], index_mcp_spread_limit[0], index_mcp_spread_limit[1], -1.0f, 1.0f);
//         float index_mcp_strech_norm = LinearMap(leftData.data[5], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
//         float index_pip_norm = LinearMap(leftData.data[6], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
//         float index_dip_norm = LinearMap(leftData.data[7], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

// 		position_norm[5] = std::clamp(0.5f * index_pip_norm + 0.5f * index_dip_norm, 0.0f, 1.0f);
// 		position_norm[4] = (index_mcp_spread_norm < 0) ? std::clamp(index_mcp_strech_norm - 0.65f * index_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(index_mcp_strech_norm, 0.0f, 1.0f);
//         position_norm[3] = (index_mcp_spread_norm > 0) ? std::clamp(index_mcp_strech_norm + 0.5f * index_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(index_mcp_strech_norm, 0.0f, 1.0f);

//         // 中指
//         float middle_mcp_spread_norm = LinearMap(leftData.data[8], middle_mcp_spread_limit[0], middle_mcp_spread_limit[1], -1.0f, 1.0f);
//         float middle_mcp_strech_norm = LinearMap(leftData.data[9], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
//         float middle_pip_norm = LinearMap(leftData.data[10], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
//         float middle_dip_norm = LinearMap(leftData.data[11], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

//         position_norm[8] = std::clamp(0.5f * middle_pip_norm + 0.5f * middle_dip_norm, 0.0f, 1.0f);
//         position_norm[7] = (middle_mcp_spread_norm < 0) ? std::clamp(middle_mcp_strech_norm - 0.5f * middle_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(middle_mcp_strech_norm, 0.0f, 1.0f);
//         position_norm[6] = (middle_mcp_spread_norm > 0) ? std::clamp(middle_mcp_strech_norm + 0.5f * middle_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(middle_mcp_strech_norm, 0.0f, 1.0f);
        
// 		// 无名指
//         float ring_mcp_spread_norm = LinearMap(leftData.data[12], ring_mcp_spread_limit[0], ring_mcp_spread_limit[1], -1.0f, 1.0f);
//         float ring_mcp_strech_norm = LinearMap(leftData.data[13], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
//         float ring_pip_norm = LinearMap(leftData.data[14], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
//         float ring_dip_norm = LinearMap(leftData.data[15], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

//         position_norm[11] = std::clamp(0.5f * ring_pip_norm + 0.5f * ring_dip_norm, 0.0f, 1.0f);
//         position_norm[10] = (ring_mcp_spread_norm < 0) ? std::clamp(ring_mcp_strech_norm - 0.5f * ring_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(ring_mcp_strech_norm, 0.0f, 1.0f);
//         position_norm[9 ] = (ring_mcp_spread_norm > 0) ? std::clamp(ring_mcp_strech_norm + 0.5f * ring_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(ring_mcp_strech_norm, 0.0f, 1.0f);

// 		// 小拇指
//         float pinky_mcp_spread_norm = LinearMap(leftData.data[16], pinky_mcp_spread_limit[0], pinky_mcp_spread_limit[1], -1.0f, 1.0f);
//         float pinky_mcp_strech_norm = LinearMap(leftData.data[17], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
//         float pinky_pip_norm = LinearMap(leftData.data[18], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
//         float pinky_dip_norm = LinearMap(leftData.data[19], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

//         position_norm[14] = std::clamp(0.5f * pinky_pip_norm + 0.5f * pinky_dip_norm, 0.0f, 1.0f);
//         position_norm[13] = (pinky_mcp_spread_norm < 0) ? std::clamp(pinky_mcp_strech_norm - 0.5f * pinky_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(pinky_mcp_strech_norm, 0.0f, 1.0f);
//         position_norm[12] = (pinky_mcp_spread_norm > 0) ? std::clamp(pinky_mcp_strech_norm + 0.65f * pinky_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(pinky_mcp_strech_norm, 0.0f, 1.0f);
        
//         for (int i = 0; i < 16; ++i) {
//             position_drive[i] = static_cast<int>(position_norm[i] * 4096.0f);
//             //std::cout << std::dec << position_norm[i] << " ";
//         }
// 		SendPredefinedMessage(*pcan, PcanHandle, velocity, position_drive, current);

//         //std::cout << std::endl;
//         std::this_thread::sleep_for(std::chrono::milliseconds(1));

//     }
// }












#include "stdafx.h"
#include "MotorController.hpp"
#include <iostream>

// --- 初始化所有静态常量 ---
const std::vector<float> MotorController::thumb_ip_limit = { 0.0f, 65.0f };
const std::vector<float> MotorController::dip_limit = { 0.0f, 65.0f };
const std::vector<float> MotorController::pip_limit = { 0.0f, 100.0f };
const std::vector<std::vector<float>> MotorController::mcp_stretch_limit = { {5.0f, 75.0f}, {5.0f, 90.0f}, {5.0f, 75.0f}, {13.0f, 80.0f} };
const std::vector<float> MotorController::thumb_mcp_limit = { 0.0f, 70.0f };
const std::vector<float> MotorController::thumb_cmc_stretch_limit = { 45.0f, 0.0f };
const std::vector<float> MotorController::thumb_cmc_spread_limit = { 10.0f, 55.0f };
const std::vector<float> MotorController::index_mcp_spread_limit = { -9.0f, -32.0f };
const std::vector<float> MotorController::middle_mcp_spread_limit = { -10.0f, -28.0f };
const std::vector<float> MotorController::ring_mcp_spread_limit = { -3.0f, -26.0f };
const std::vector<float> MotorController::pinky_mcp_spread_limit = { 12.0f, -24.0f };

// --- 初始化手指配置表 ---
const std::vector<MotorController::FingerConfig> MotorController::s_fingerConfigs = {
    // 食指
    // 分别为手套数据的起始索引、灵巧手手指起始索引、mcp侧摆的极限位置、负侧摆角度（远离大拇指方向）的系数、正侧摆角度系数
    {4, 3, MotorController::index_mcp_spread_limit, 0.6f, 0.45f},    
    // 中指
    {8, 6, MotorController::middle_mcp_spread_limit, 0.4f, 0.4f},
    // 无名指
    {12, 9, MotorController::ring_mcp_spread_limit, 0.45f, 0.45f},
    // 小拇指
    {16, 12, MotorController::pinky_mcp_spread_limit, 0.45f, 0.6f}
};

MotorController::MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client) :
    pcan(pcan),
    PcanHandle(PcanHandle),
    client(client),
    position_norm(16, 0.0f),
    position_now(16, 0),
    position_drive(16, 0),
    velocity_drive(16, 1000),
    current_drive(16, 100)
{}

/**
 * @brief 处理大拇指的映射逻辑 (特殊情况)
 */
void MotorController::ProcessThumb(const ErgonomicsData& data) {
    
    float ip_norm      = LinearMap(data.data[3], thumb_ip_limit[0], thumb_ip_limit[1], 0.0f, 1.0f);
    float mcp_norm     = LinearMap(data.data[2], thumb_mcp_limit[0], thumb_mcp_limit[1], 0.0f, 1.0f);
    float cmc_spread_norm = LinearMap(data.data[0], thumb_cmc_spread_limit[0], thumb_cmc_spread_limit[1], 0.0f, 1.0f);
    float cmc_stretch_norm = LinearMap(data.data[1], thumb_cmc_stretch_limit[0], thumb_cmc_stretch_limit[1], 0.0f, 1.0f);

    position_norm[2]  = std::clamp(ip_norm, 0.0f, 1.0f);
    position_norm[15] = std::clamp(0.5f * cmc_stretch_norm + 0.5f * cmc_spread_norm, 0.0f, 1.0f);
    // position_norm[0]  = std::clamp(mcp_norm, 0.0f, 1.0f);
    // position_norm[1]  = std::clamp(mcp_norm, 0.0f, 1.0f);
    float conpensate = LinearMap(cmc_spread_norm, 0.0f, 0.6f, 0.889f, 0.0f);
    
    // position_norm[0]  = std::clamp(0.3f * mcp_norm + 0.7f * (0.707f * (cmc_spread_norm - cmc_stretch_norm)), 0.0f, 1.0f);
    // position_norm[1]  = std::clamp(0.3f * mcp_norm + 0.7f * (0.707f * (cmc_spread_norm - cmc_stretch_norm)), 0.0f, 1.0f);

    // position_norm[0]  = std::clamp(0.3f * mcp_norm + 0.7f * std::clamp((cmc_stretch_norm + conpensate),0.0f, 1.0f), 0.0f, 1.0f);
    // position_norm[1]  = std::clamp(0.3f * mcp_norm + 0.7f * (cmc_stretch_norm + conpensate), 0.0f, 1.0f);

    
    position_norm[0]  = std::clamp(1.6f * cmc_stretch_norm, 0.0f, 1.0f) - 0.8 * cmc_spread_norm * abs(cmc_spread_norm);
    position_norm[1]  = std::clamp(1.6f * cmc_stretch_norm, 0.0f, 1.0f) - 0.8 * cmc_spread_norm * abs(cmc_spread_norm);
}

/**
 * @brief 通用处理函数，用于食指、中指、无名指和小指
 */
void MotorController::ProcessFinger(const ErgonomicsData& data, const FingerConfig& config) {
    float mcp_spread_norm = LinearMap(data.data[config.base_data_index],     config.spread_limit[0], config.spread_limit[1], -1.0f, 1.0f);
    int finger_index = std::max(config.base_data_index / 4 - 1, 0);
    float mcp_stretch_norm = LinearMap(data.data[config.base_data_index + 1], mcp_stretch_limit[finger_index][0], mcp_stretch_limit[finger_index][1], 0.0f, 1.0f);

    // 一定程度上抑制当mcp弯曲较大时（趋近握拳）的mcp侧摆角。
    mcp_spread_norm = copysign(std::max(0.0f, std::abs(mcp_spread_norm) - 0.8f * mcp_stretch_norm), mcp_spread_norm);
    // mcp_spread_norm = copysign(std::max(0.0f, std::abs(mcp_spread_norm) - 0.6f * mcp_stretch_norm), mcp_spread_norm);

    float pip_norm        = LinearMap(data.data[config.base_data_index + 2], pip_limit[0], pip_limit[1], 0.0f, 1.0f);
    float dip_norm        = LinearMap(data.data[config.base_data_index + 3], dip_limit[0], dip_limit[1], 0.0f, 1.0f);

    position_norm[config.base_pos_index + 2] = std::clamp(0.5f * pip_norm + 0.5f * dip_norm, 0.0f, 1.0f);

    // position_norm[config.base_pos_index + 1] = (mcp_spread_norm < 0)
    //     ? std::clamp(mcp_stretch_norm - config.spread_coeff_neg * mcp_spread_norm, 0.0f, 1.0f)
    //     : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

    // position_norm[config.base_pos_index] = (mcp_spread_norm > 0)
    //     ? std::clamp(mcp_stretch_norm + config.spread_coeff_pos * mcp_spread_norm, 0.0f, 1.0f)
    //     : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

    position_norm[config.base_pos_index + 1] = (mcp_spread_norm < 0)
        ? std::clamp(mcp_stretch_norm - config.spread_coeff_neg * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f)
        : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

    position_norm[config.base_pos_index] = (mcp_spread_norm > 0)
        ? std::clamp(mcp_stretch_norm + config.spread_coeff_pos * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f)
        : std::clamp(mcp_stretch_norm, 0.0f, 1.0f);
}


/**
 * @brief 【优雅重构后】的主循环函数
 */
void MotorController::Run() {
    while (true) {
        ErgonomicsData leftData = client->GetGloveErgoData(true); // true = 左手

        // 1. 单独处理大拇指
        ProcessThumb(leftData);

        // 2. 循环处理其余四个手指
        for (const auto& config : s_fingerConfigs) {
            ProcessFinger(leftData, config);
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
        velocity_drive[i] = 200 + float(position_error) / 4096.0 * 800;
    }
    SendAction(*pcan, PcanHandle, velocity_drive, position_drive, current_drive);    
}

// std::optional<TPCANMsg> MotorController::ReceiveCANMessage() {
//     TPCANMsg msg;
//     TPCANTimestamp ts;
//     TPCANStatus result = pcan->Read(PcanHandle, msg, ts);
//     if (result == PCAN_ERROR_OK) return msg;
//     if (result != PCAN_ERROR_QRCVEMPTY) {
//         std::string errorText;
//         pcan->GetErrorText(result, 0, errorText);
//         std::cerr << "PCAN Read error: " << errorText << std::endl;
//     }
//     return std::nullopt;
// }

// std::optional<FingerServoInfo_t> MotorController::ParseFingerFeedback(const uint8_t* data, uint8_t len) {
//     if (len != 8) return std::nullopt;
//     uint64_t raw = 0;
//     memcpy(&raw, data, 8);
//     FingerServoInfo_t state;
//     state.ucStatus = (raw >> 8)  & 0xFF;
//     state.ub_P     = (raw >> 16) & 0xFFF;
//     int v_raw      = (raw >> 28) & 0xFFF;
//     int i_raw      = (raw >> 40) & 0xFFF;
//     state.ub_F     = (raw >> 52) & 0xFFF;
//     state.ub_V     = (v_raw > 2047) ? (v_raw - 4096) : v_raw;
//     state.ub_I     = (i_raw > 2047) ? (i_raw - 4096) : i_raw;
//     return state;
// }


/**
 * @brief [新增] 从CAN总线读取并解析所有传入的电机数据，并更新内部状态.
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
            velocity_now[motor_index] = vel;
            current_now[motor_index] = cur;
        }
    }
}
