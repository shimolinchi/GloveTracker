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

/*
    RecorderThread:
    运行数据记录线程，将手套数据和电机控制数据记录到文件中。
*/
void RecorderThread(Recorder* recorder) {
    recorder->Run();
}

int main(int argc, char* argv[])
{
    // SDK客户端初始化
    SDKClient* sdk_client = new SDKClient();
    // pcan通信初始化
    PCANBasic* pcan = new PCANBasic();
    TPCANHandle handles[] = { PCAN_USBBUS1, PCAN_USBBUS2 };
    TPCANBaudrate baudrate = PCAN_BAUD_1M;
    const int deviceCount = sizeof(handles) / sizeof(handles[0]);
    bool initialized[deviceCount] = { false };
	int sucscessfulInits = 0;

    for (int i = 0; i < deviceCount; ++i) {
        TPCANHandle handle = handles[i];
        TPCANStatus result = pcan->Initialize(handle, baudrate);

        if (result == PCAN_ERROR_OK) {
            initialized[i] = true;
			sucscessfulInits++;
        }
        else {
            pcan->Uninitialize(handle); // 清理失败的通道
        }
    }
	std::cout << "Successfully initialized " << sucscessfulInits << " PCAN devices." << std::endl;
	std::cout << initialized[0] << initialized[1] << std::endl;
	//Sleep(1500);


    // 电机控制器初始化
    if (!initialized[0]) {
        std::cout << "no pcan devices!" << std::endl;
		Sleep(3000);
        exit(0);
    }

    MotorController* left_controller = new MotorController(pcan, handles[0], sdk_client, true);
    MotorController* right_controller = nullptr;
    if (initialized[1]) {
        right_controller = new MotorController(pcan, handles[1], sdk_client, false);
    }
    // 数据记录器初始化
    Recorder* recorder = new Recorder(left_controller);

    // 图形界面初始化
    GraphicInteractor* interactor = new GraphicInteractor(left_controller,right_controller, recorder);
    
    // 线程启动
    std::thread sdkclient_thread(SDKThread, sdk_client);
    std::thread left_controller_thread(CtrlThread, left_controller);
    std::thread right_controller_thread;
    if (right_controller) right_controller_thread = std::thread(CtrlThread, right_controller);
    std::thread interactor_thread(GraphThread, interactor);
    std::thread recorder_thread(RecorderThread, recorder);

    // 将手套客户端线程与主线程绑定，其他线程各自运行
    sdkclient_thread.join();
    interactor_thread.detach();
    left_controller_thread.detach();
    if (right_controller_thread.joinable()) right_controller_thread.detach();
    recorder_thread.detach();

    if (right_controller) delete right_controller;
    delete left_controller;
    delete interactor;
    delete recorder;
    delete sdk_client;
    delete pcan;
    
    return 0;
}