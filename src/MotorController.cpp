#include "stdafx.h"
#include "MotorController.hpp"
#include <iostream>

// --- 初始化所有静态常量 ---
std::vector<float> MotorController::thumb_ip_limit = { 0.0f, 65.0f };
std::vector<std::vector<float>> MotorController::dip_limit = {{ 0.0f, 65.0f },{ 0.0f, 65.0f },{ 0.0f, 65.0f },{ 0.0f, 65.0f }};
std::vector<std::vector<float>> MotorController::pip_limit = {{ 0.0f, 100.0f },{ 0.0f, 100.0f },{ 0.0f, 100.0f },{ 0.0f, 100.0f }};
std::vector<std::vector<float>> MotorController::mcp_stretch_limit = { {0.0f, 75.0f}, {0.0f, 90.0f}, {0.0f, 75.0f}, {0.0f, 80.0f} };
std::vector<float> MotorController::thumb_mcp_limit = { 0.0f, 70.0f };
std::vector<float> MotorController::thumb_cmc_stretch_limit = { 48.0f, 32.0f };
std::vector<float> MotorController::thumb_cmc_spread_limit = { 18.0f, 58.0f };

MotorController::MotorController(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client, bool hand_side) :
    pcan(pcan),
    PcanHandle(PcanHandle),
    client(client),
	hand_side(hand_side),
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
    spread_coeff_neg({0.4f, 0.4f, 0.4f, 0.4f}),
    spread_coeff_pos({0.4f, 0.4f, 0.4f, 0.5f}),
	tip_distances(4, 100.0f),
    pointing_motor_position({{1900, 1900, 2950, 1200, 1200, 2418, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2000 },
                            { 2170, 2170, 2280 ,0 ,0, 0, 1450, 1450, 2060, 0, 0, 0, 0, 0, 0, 3145 },
                            { 2303, 2369, 2400, 0, 0, 0, 0, 0, 0, 1161, 1161, 2000, 0, 0, 0, 3910 },
                            { 1220, 2160, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1210, 1210, 1660, 4095 }}),

    // 比赛特制动作，食指无名指对大拇指
    // pointing_motor_position({{ 1645, 1911, 2997, 1567, 1159, 2108, 1128, 1598, 2574, 0, 0, 0, 0, 0, 0, 2397  },
    //                         { 2179, 2179, 2264 ,0 ,0, 0, 1482, 1482, 2200, 0, 0, 0, 0, 0, 0, 3145 },
    //                         { 2303, 2369, 2480, 0, 0, 0, 0, 0, 0, 1161, 1161, 2030, 0, 0, 0, 3940 },
    //                         { 1245, 2185, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1299, 1299, 1798, 4095 }})
    second_pointing_motor_position({{ 3600, 3600, 1000, 1, 1, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1500 },
                                    { 3800, 3800, 850 ,0 ,0, 0, 1, 1, 2500, 0, 0, 0, 0, 0, 0, 3145 },
                                    { 3700, 3700, 850, 0, 0, 0, 0, 0, 0, 1, 1, 2200, 0, 0, 0, 3910 },
                                    { 3600, 3900, 850, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2100, 4095 }})
                                   

    {
    }

void MotorController::UpdateTipDistance() {

    if (client->GetCurrentSkeleton()) {
        if (hand_side) {
            skeleton = client->GetCurrentSkeleton()->skeletons[0];
        } else if (client->GetCurrentSkeleton()->skeletons.size() > 1) {
            skeleton = client->GetCurrentSkeleton()->skeletons[1];
        }
        ManusTransform thumb_tf = skeleton.nodes[4].transform;
        ManusTransform index_tf = skeleton.nodes[8].transform;
        ManusTransform middle_tf = skeleton.nodes[12].transform;
        ManusTransform ring_tf = skeleton.nodes[16].transform;
        ManusTransform pinky_tf = skeleton.nodes[20].transform;
        tip_distances[0] = CalculateDistance(thumb_tf.position.x, thumb_tf.position.y, thumb_tf.position.z, index_tf.position.x, index_tf.position.y, index_tf.position.z);
        tip_distances[1] = CalculateDistance(thumb_tf.position.x, thumb_tf.position.y, thumb_tf.position.z, middle_tf.position.x, middle_tf.position.y, middle_tf.position.z);
        tip_distances[2] = CalculateDistance(thumb_tf.position.x, thumb_tf.position.y, thumb_tf.position.z, ring_tf.position.x, ring_tf.position.y, ring_tf.position.z);
        tip_distances[3] = CalculateDistance(thumb_tf.position.x, thumb_tf.position.y, thumb_tf.position.z, pinky_tf.position.x, pinky_tf.position.y, pinky_tf.position.z);
        //std::cout << "Distance " << tip_distances[0] << "," << tip_distances[1] << "," << tip_distances[2] << "," << tip_distances[3] << "," << std::endl;
    }
}

void MotorController::ChangeGloveHandSide() {
    hand_side = !hand_side;
}

/**
 * @brief 处理大拇指的映射逻辑
 */
void MotorController::ProcessThumb() {
    
    float ip_norm      = LinearMap(glove_data.data[3], thumb_ip_limit[0], thumb_ip_limit[1], 0.0f, 1.0f);
    float mcp_norm     = LinearMap(glove_data.data[2], thumb_mcp_limit[0], thumb_mcp_limit[1], 0.0f, 1.0f);
    float cmc_spread_norm = LinearMap(std::max(glove_data.data[0], 0.0f), thumb_cmc_spread_limit[0], thumb_cmc_spread_limit[1], 0.0f, 1.0f);
    float cmc_stretch_norm = LinearMap(glove_data.data[1], thumb_cmc_stretch_limit[0], thumb_cmc_stretch_limit[1], 0.0f, 1.0f);

    position_norm[2]  = std::clamp(ip_norm, 0.0f, 1.0f);
    position_norm[15] = std::clamp(0.3f * cmc_stretch_norm + 0.7f * cmc_spread_norm, 0.0f, 1.0f);

    std::vector<float> mcp_limit_r = {thumb_cmc_stretch_limit[0] - 27.0f * (position_norm[15] * position_norm[15]), thumb_cmc_stretch_limit[1] - 30.0f * (position_norm[15] * position_norm[15])};
    float mcp_conpensate = 0.0f;
    if (0.3f * cmc_stretch_norm + 0.75f * cmc_spread_norm >= 1.0f){
        mcp_conpensate = std::clamp(0.3f * cmc_stretch_norm + 0.7f * cmc_spread_norm - 1.0f, 0.0f, 1.0f);
        mcp_limit_r[0] -= 35.0f * mcp_conpensate;
        mcp_limit_r[1] -= 35.0f * mcp_conpensate;
    }
    float cmc_stretch_norm_r = LinearMap(glove_data.data[1], mcp_limit_r[0], mcp_limit_r[1], 0.0f, 1.0f);
    position_norm[0] = std::clamp((cmc_stretch_norm_r), 0.0f, 1.0f);
    position_norm[1] = std::clamp((cmc_stretch_norm_r + 0.6f * mcp_conpensate), 0.0f, 1.0f);
}

/**
 * @brief 通用处理函数，用于食指、中指、无名指和小指
 */
void MotorController::ProcessFinger(int finger_index) {

    float pip_norm        = LinearMap(glove_data.data[base_data_index[finger_index] + 2], pip_limit[finger_index][0], pip_limit[finger_index][1], 0.0f, 1.0f);
    float dip_norm        = LinearMap(glove_data.data[base_data_index[finger_index] + 3], dip_limit[finger_index][0], dip_limit[finger_index][1], 0.0f, 1.0f);
    position_norm[base_pos_index[finger_index] + 2] = std::clamp(0.5f * pip_norm + 0.5f * dip_norm, 0.0f, 1.0f);
    std::vector<float> mcp_conpensate = {30.0f, 35.0f, 30.0f, 20.0f};


    float mcp_stretch_norm = LinearMap(glove_data.data[base_data_index[finger_index] + 1], mcp_stretch_limit[finger_index][0] + mcp_conpensate[finger_index] * position_norm[base_pos_index[finger_index] + 2], mcp_stretch_limit[finger_index][1], 0.0f, 1.0f);

    float mcp_spread_norm = LinearMap(glove_data.data[base_data_index[finger_index]],  mcp_spread_zero_position[finger_index] + spread_limit[finger_index], mcp_spread_zero_position[finger_index] - spread_limit[finger_index], -1.0f, 1.0f);
    
    // 一定程度上抑制当mcp弯曲较大时（趋近握拳）的mcp侧摆角。
    mcp_spread_norm = copysign(std::max(0.0f, std::abs(mcp_spread_norm) - 0.8f * mcp_stretch_norm), mcp_spread_norm);

    float mcp_inside_motor = (mcp_spread_norm < 0) ? 
          std::clamp(mcp_stretch_norm - spread_coeff_neg[finger_index] * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f): 
          std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

    float mcp_outside_motor = position_norm[base_pos_index[finger_index]] = (mcp_spread_norm > 0) ? 
          std::clamp(mcp_stretch_norm + spread_coeff_pos[finger_index] * mcp_spread_norm * (1-std::clamp(mcp_stretch_norm, 0.0f, 1.0f)), 0.0f, 1.0f): 
          std::clamp(mcp_stretch_norm, 0.0f, 1.0f);

	position_norm[base_pos_index[finger_index] + 1] = hand_side ? mcp_inside_motor : mcp_outside_motor;
	position_norm[base_pos_index[finger_index]] = hand_side ? mcp_outside_motor : mcp_inside_motor;
}

void MotorController::PointingOptimize() {

    std::vector<float> pointing_motor_position_norm(16);
    int closest_tip_index = std::distance(tip_distances.begin(),std::min_element(tip_distances.begin(), tip_distances.end()));
    float closest_tip_distance = tip_distances[closest_tip_index];
    float mcp_stretch_norm = (position_norm[3 + closest_tip_index * 3] + position_norm[4 + closest_tip_index * 3]) / 2.0f;
    mcp_stretch_norm = LinearMap(mcp_stretch_norm, 0.0f, (pointing_motor_position[closest_tip_index][closest_tip_index * 3 + 3] / 4096.0f), 0.0f, 1.0f); // 二次曲线加强效果
    mcp_stretch_norm = std::clamp(mcp_stretch_norm, 0.0f, 1.0f);


    for (size_t i = 0; i < 16; i++) {
        // pointing_motor_position_norm[i] = pointing_motor_position[closest_tip_index][i] / 4096.0f;
        pointing_motor_position_norm[i] = mcp_stretch_norm * pointing_motor_position[closest_tip_index][i] / 4096.0f + 
        (1.0f - mcp_stretch_norm) * second_pointing_motor_position[closest_tip_index][i] / 4096.0f;
	}

    if (tip_distances[closest_tip_index] < 1e-6) {
        for (int i = 0; i < 16; i++) {
            if (pointing_motor_position_norm[i] > 1e-6) { // 只优化和对指相关的电机
                position_norm[i] = pointing_motor_position_norm[i];
            }
        }
    }
    else if (tip_distances[closest_tip_index] < pointing_threadhold) {
        for (int i = 0; i < 16; i++) {
            if (pointing_motor_position_norm[i] > 1e-6) { // 只优化和对指相关的电机
                position_norm[i] += std::clamp(pointing_optimization_strength * (pointing_motor_position_norm[i] - position_norm[i]) / tip_distances[closest_tip_index],
                    pointing_motor_position_norm[i] - position_norm[i] > 0 ? 0.0f : pointing_motor_position_norm[i] - position_norm[i],
                    pointing_motor_position_norm[i] - position_norm[i] > 0 ? pointing_motor_position_norm[i] - position_norm[i] : 0.0f);
            }
        }
	}
}

/**
 * @brief 执行校准过程，计算各手指的零点位置
 */
void MotorController::Calibrate() {

    int sampleCount = 0;
    std::vector<float> sum = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // 前四个用于四指侧摆零点，第四到八个用于pip零点，第九到十二个用于dip零点， 第十三个个用于大拇指侧摆零点
    while(calibrating_process != CalibrateProcess::STEP1) {std::this_thread::sleep_for(std::chrono::milliseconds(10));}
    while (calibrating_process == CalibrateProcess::STEP1) {
        for(int i = 0; i < 4; i++){
            sum[i] += glove_data.data[i * 4 + 4];
            sum[i + 4] += glove_data.data[i * 4 + 7];
            sum[i + 8] += glove_data.data[i * 4 + 6];
        }
        sum[12] += glove_data.data[0];
        sampleCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    for(int i = 0; i < 4; i++){
        mcp_spread_zero_position[i] = sum[i] / sampleCount;
        pip_limit[i][0] = sum[i + 8] / sampleCount + 2.0f;
        dip_limit[i][0] = sum[i + 4] / sampleCount + 2.0f;
    }
    thumb_cmc_spread_limit[0] = sum[12] / sampleCount;
}


/**
 * @brief 主循环函数
 */
void MotorController::Run() {
    
    // 等待手套sdk初始化结束后，加载骨骼模型
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    while (true) {


        if (hand_side) {
            glove_data = client->GetGloveErgoData(true);
        }
        else {
            glove_data = client->GetGloveErgoData(false);
        }

        if (calibrating_process == CalibrateProcess::START) { Calibrate(); }

		// 更新指尖距离数据
        UpdateTipDistance();

        // 单独处理大拇指
        ProcessThumb();

        // 循环处理其余四个手指
        for (int i=0; i<4; i++) { ProcessFinger(i); }

        //进行对指优化
        PointingOptimize();

        // 将归一化位置转换为电机驱动值
        for (int i=0; i<16; ++i) { position_drive[i] = static_cast<int>(position_norm[i] * 4096.0f); }
        
        // 更新电机读取数据
        UpdateMotorData();

        // 发送控制指令
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
