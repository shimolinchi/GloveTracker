# include "ClientLogging.hpp"
# include "SDKClient.hpp"
# include <thread>
# include <algorithm>
# include <windows.h>
# include <graphics.h>   // EasyX
# include <conio.h>      // kbhit, getch
# include <mutex>
# include <string>
# include "RyHandLib.h"
# include "stdafx.h"
# include "PCANBasic.hpp"
# include "math_utils.hpp"
# include "MotorController.hpp"
# include "GraphicInteractor.hpp"
# include "Recorder.hpp"


/*
    GraphThread:
    初始化并运行窗口进程,用于与用户交互，显示手套与电机控制信息，（校正手指极限角度）。
*/
void GraphThread(GraphicInteractor* interactor)
{
    interactor->Init();
    interactor->Run();
}

/*
    SDKThread:
    初始化并运行 SDK 客户端,与 ManusCore 进行通信。
*/
void SDKThread(SDKClient* client)
{
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
/*
    CtrlThread:
    运行电机控制线程，将手套数据转换成电机控制数据并发送。
*/
void CtrlThread(MotorController* controller)
{
    controller->Run();
}

void RecorderThread(Recorder* recorder) {
    recorder->Run();
}

int main(int argc, char* argv[])
{
    // SDK客户端初始化
    SDKClient sdk_client;
    // pcan通信初始化
    PCANBasic pcan;
    TPCANHandle PcanHandle = PCAN_USBBUS1;
    TPCANBaudrate baudrate = PCAN_BAUD_1M;
    TPCANStatus result = pcan.Initialize(PcanHandle, baudrate);
    // 电机控制器初始化
    MotorController controller(&pcan, PcanHandle, &sdk_client);
    Recorder recorder(&controller);
    // 图形界面初始化
    GraphicInteractor interactor(&controller, &recorder);
    
    // 线程启动
    std::thread sdkclient_thread(SDKThread, &sdk_client);
    std::thread controller_thread(CtrlThread, &controller);
    std::thread interactor_thread(GraphThread, &interactor);
    std::thread recorder_thread(RecorderThread, &recorder);

    sdkclient_thread.join();
    interactor_thread.detach();
    controller_thread.detach();
    return 0;
}