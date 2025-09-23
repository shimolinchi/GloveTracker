#include "ClientLogging.hpp"
#include "SDKClient.hpp"
#include <thread>
#include <algorithm>

# include "RyHandLib.h"
# include "stdafx.h"
# include "PCANBasic.hpp"
# include "math_utils.hpp"

void PrinterThread(SDKClient* client)
{
    while (true)
    {
        // ��ȡ�����ֵ�����
        ErgonomicsData leftData = client->GetGloveErgoData(true); // true = ����
        ErgonomicsData rightData = client->GetGloveErgoData(false);  // false  = ����

        //// ��ӡ��������
        //std::cout << "[Left Hand] ";
        //for (int i = 0; i < 20; i++)
        //{
        //    std::cout << leftData.data[i] << " ";
        //}
        //std::cout << std::endl;

        //// ��ӡ��������
        //std::cout << "[Right Hand] ";
        //for (int i = 0; i < ERGONOMICS_DATA_SIZE; i++)
        //{
        //    std::cout << rightData.data[i] << " ";
        //}
        //std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void SDKThread(SDKClient* client)
{
    // ��ʼ��
    ClientReturnCode t_Result = client->Initialize();
    if (t_Result != ClientReturnCode::ClientReturnCode_Success)
    {
        client->ShutDown();
        std::cerr << "[SDKThread] Run() failed with code " << static_cast<int>(t_Result) << std::endl;  
    }
    ManusSDK::ClientLog::print("SDK client is initialized.");
    t_Result = client->Run();
    if (t_Result != ClientReturnCode::ClientReturnCode_Success)
    {
        client->ShutDown();
        std::cerr << "[SDKThread] Run() failed with code " << static_cast<int>(t_Result) << std::endl;
    }
}



void CtrlThread(PCANBasic* pcan, TPCANHandle PcanHandle, SDKClient* client)
{
    std::vector<float> pos_norm(16, 0.0f);
    std::vector<int> pos_drive(16, 0);
    std::vector<int> velocity(16, 1000);
    std::vector<int> current(16, 80);
    const float thumb_ip_limit = 65.0f;
    const float dip_limit = 65.0f;
    const float pip_limit = 100.0f;
    const std::vector<float> mcp_stretch_limit = { 8.0f, 90.0f };
    const std::vector<float> thumb_mcp_limit = { 0.0f, 70.0f };
    const std::vector<float> thumb_cmc_stretch_limit = { 45.0f, 0.0f };
    const std::vector<float> thumb_cmc_spread_limit = { 10.0f, 55.0f };
    const std::vector<float> index_mcp_spread_limit = {4.0f, - 32.0f };
    const std::vector<float> middle_mcp_spread_limit = { 6.0f, -26.0f };
    const std::vector<float> ring_mcp_spread_limit = { 7.0f, -22.0f };
    const std::vector<float> pinky_mcp_spread_limit = { 12.0f, -23.0f };
    while (true)

    {
        ErgonomicsData leftData = client->GetGloveErgoData(true); // true = 左手

        //大拇指
		float thumb_ip_norm = LinearMap(leftData.data[3], 0.0f, thumb_ip_limit, 0.0f, 1.0f);
        float thumb_mcp_norm = LinearMap(leftData.data[2], thumb_mcp_limit[0], thumb_mcp_limit[1], 0.0f, 1.0f);
		float thumb_cmc_stretch_norm = LinearMap(leftData.data[1], thumb_cmc_stretch_limit[0], thumb_cmc_stretch_limit[1], 0.0f, 1.0f);
		float thumb_cmc_spread_norm = LinearMap(leftData.data[0], thumb_cmc_spread_limit[0], thumb_cmc_spread_limit[1], 0.0f, 1.0f);

		pos_norm[2] = std::clamp(thumb_ip_norm, 0.0f, 1.0f);
        pos_norm[15] = std::clamp(0.5f * thumb_cmc_stretch_norm + 0.5f * thumb_cmc_spread_norm, 0.0f, 1.0f);
        pos_norm[0] = std::clamp(thumb_mcp_norm, 0.0f, 1.0f);
        pos_norm[1] = std::clamp(thumb_mcp_norm, 0.0f, 1.0f);
        /*pos_norm[0] = std::clamp(thumb_mcp_norm - 0.1f * thumb_cmc_spread_norm - 0.1f * thumb_cmc_stretch_norm, 0.0f, 1.0f);
        pos_norm[1] = std::clamp(thumb_mcp_norm + 0.1f * thumb_cmc_spread_norm + 0.9f * thumb_cmc_stretch_norm, 0.0f, 1.0f);*/
        


        // ​​左手手指的 MCP 侧摆关节​​：向大拇指方向运动​​ 的极限为 ​- 1，​​​​远离大拇指方向（反方向）运动​​ 的极限为 ​​ + 1​
        // 食指
        float index_mcp_spread_norm = LinearMap(leftData.data[4], index_mcp_spread_limit[0], index_mcp_spread_limit[1], -1.0f, 1.0f);
        float index_mcp_strech_norm = LinearMap(leftData.data[5], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
        float index_pip_norm = LinearMap(leftData.data[6], 0.0f, pip_limit, 0.0f, 1.0f);
        float index_dip_norm = LinearMap(leftData.data[7], 0.0f, dip_limit, 0.0f, 1.0f);

		pos_norm[5] = std::clamp(0.5f * index_pip_norm + 0.5f * index_dip_norm, 0.0f, 1.0f);
		pos_norm[4] = (index_mcp_spread_norm < 0) ? std::clamp(index_mcp_strech_norm - 0.65f * index_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(index_mcp_strech_norm, 0.0f, 1.0f);
        pos_norm[3] = (index_mcp_spread_norm > 0) ? std::clamp(index_mcp_strech_norm + 0.5f * index_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(index_mcp_strech_norm, 0.0f, 1.0f);

        // 中指
        float middle_mcp_spread_norm = LinearMap(leftData.data[8], middle_mcp_spread_limit[0], middle_mcp_spread_limit[1], -1.0f, 1.0f);
        float middle_mcp_strech_norm = LinearMap(leftData.data[9], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
        float middle_pip_norm = LinearMap(leftData.data[10], 0.0f, pip_limit, 0.0f, 1.0f);
        float middle_dip_norm = LinearMap(leftData.data[11], 0.0f, dip_limit, 0.0f, 1.0f);

        pos_norm[8] = std::clamp(0.5f * middle_pip_norm + 0.5f * middle_dip_norm, 0.0f, 1.0f);
        pos_norm[7] = (middle_mcp_spread_norm < 0) ? std::clamp(middle_mcp_strech_norm - 0.5f * middle_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(middle_mcp_strech_norm, 0.0f, 1.0f);
        pos_norm[6] = (middle_mcp_spread_norm > 0) ? std::clamp(middle_mcp_strech_norm + 0.5f * middle_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(middle_mcp_strech_norm, 0.0f, 1.0f);
        
		// 无名指
        float ring_mcp_spread_norm = LinearMap(leftData.data[12], ring_mcp_spread_limit[0], ring_mcp_spread_limit[1], -1.0f, 1.0f);
        float ring_mcp_strech_norm = LinearMap(leftData.data[13], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
        float ring_pip_norm = LinearMap(leftData.data[14], 0.0f, pip_limit, 0.0f, 1.0f);
        float ring_dip_norm = LinearMap(leftData.data[15], 0.0f, dip_limit, 0.0f, 1.0f);

        pos_norm[11] = std::clamp(0.5f * ring_pip_norm + 0.5f * ring_dip_norm, 0.0f, 1.0f);
        pos_norm[10] = (ring_mcp_spread_norm < 0) ? std::clamp(ring_mcp_strech_norm - 0.5f * ring_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(ring_mcp_strech_norm, 0.0f, 1.0f);
        pos_norm[9 ] = (ring_mcp_spread_norm > 0) ? std::clamp(ring_mcp_strech_norm + 0.5f * ring_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(ring_mcp_strech_norm, 0.0f, 1.0f);

		// 小拇指
        float pinky_mcp_spread_norm = LinearMap(leftData.data[16], pinky_mcp_spread_limit[0], pinky_mcp_spread_limit[1], -1.0f, 1.0f);
        float pinky_mcp_strech_norm = LinearMap(leftData.data[17], mcp_stretch_limit[0], mcp_stretch_limit[1], 0.0f, 1.0f);
        float pinky_pip_norm = LinearMap(leftData.data[18], 0.0f, pip_limit, 0.0f, 1.0f);
        float pinky_dip_norm = LinearMap(leftData.data[19], 0.0f, dip_limit, 0.0f, 1.0f);

        pos_norm[14] = std::clamp(0.5f * pinky_pip_norm + 0.5f * pinky_dip_norm, 0.0f, 1.0f);
        pos_norm[13] = (pinky_mcp_spread_norm < 0) ? std::clamp(pinky_mcp_strech_norm - 0.5f * pinky_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(pinky_mcp_strech_norm, 0.0f, 1.0f);
        pos_norm[12] = (pinky_mcp_spread_norm > 0) ? std::clamp(pinky_mcp_strech_norm + 0.65f * pinky_mcp_spread_norm, 0.0f, 1.0f) : std::clamp(pinky_mcp_strech_norm, 0.0f, 1.0f);
        
        for (int i = 0; i < 16; ++i) {
            pos_drive[i] = static_cast<int>(pos_norm[i] * 4096.0f);
            //std::cout << std::dec << pos_norm[i] << " ";
        }
		SendPredefinedMessage(*pcan, PcanHandle, velocity, pos_drive, current);

        //std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}
int main(int argc, char* argv[])
{
    ManusSDK::ClientLog::print("Starting SDK client!");

    SDKClient t_SDKClient;
    PCANBasic pcan;

    TPCANHandle PcanHandle = PCAN_USBBUS1;
    TPCANBaudrate baudrate = PCAN_BAUD_1M;
    TPCANStatus result = pcan.Initialize(PcanHandle, baudrate);

    if (result != PCAN_ERROR_OK) {
        std::string errorText;
        pcan.GetErrorText(result, 0, errorText); // ��ȡ��������
        std::cerr << "Failed to initialize the PCAN device: " << errorText << " (Code: 0x" << std::hex << result << ")" << std::endl;
        return 1;
    }

    std::cout << "PCAN device initialized successfully on handle " << std::hex << PcanHandle << "." << std::endl;


    // �����߳�
    std::thread sdkRunner(SDKThread, &t_SDKClient);
    std::thread printer(PrinterThread, &t_SDKClient);
    std::thread controller(CtrlThread, &pcan, PcanHandle, &t_SDKClient);

    // �ȴ� SDK �߳̽���
    sdkRunner.join();

    // ��ӡ�̷߳���������������߳��˳�
    printer.detach();

    ManusSDK::ClientLog::print("SDK client is done, shutting down.");

    ClientReturnCode t_Result = t_SDKClient.ShutDown();
    return static_cast<int>(t_Result);
}