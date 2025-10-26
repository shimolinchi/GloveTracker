# include "stdafx.h"
# include "ClientLogging.hpp"
# include "SDKClient.hpp"
# include "RyHandLib.h"
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
        std::cerr <<"SDK client initialize failed with code " << static_cast<int>(t_Result) << std::endl;  
    }
    ManusSDK::ClientLog::print("SDK client is initialized.");
    t_Result = client->Run();
    if (t_Result != ClientReturnCode::ClientReturnCode_Success)
    {
        client->ShutDown();
        std::cerr << "SDK client run failed with code " << static_cast<int>(t_Result) << std::endl;
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
    bool pcan_initialized[deviceCount] = { false };
	int success_inits = 0;

    for (int i = 0; i < deviceCount; ++i) {
        TPCANHandle handle = handles[i];
        TPCANStatus result = pcan->Initialize(handle, baudrate);

        if (result == PCAN_ERROR_OK) {
            pcan_initialized[i] = true;
			success_inits++;
        }
        else {
            pcan->Uninitialize(handle); // 清理失败的通道
        }
    }

    std::thread sdkclient_thread(SDKThread, sdk_client); Sleep(2000);
    // 将 SDK 线程与主线程分离，允许其在后台独立运行
    sdkclient_thread.detach();
    std::cout << "SDK Client thread detached." << std::endl;

	// 电机控制器初始化
	int loaded_skeletons = 0;
    int left_connected = 0;
    int right_connected = 0;
    MotorController* controller_1 = nullptr; 
    MotorController* controller_2 = nullptr;

	sdk_client->LoadSkeleton(Side::Side_Left); Sleep(500); //等待骨骼加载
    if (pcan_initialized[0] && sdk_client->GetSkeletonCount() > 0) {
		controller_1 = new MotorController(pcan, handles[0], sdk_client, true);
		loaded_skeletons++;
		std::cout << "Left controller initialized." << std::endl;
    }
    sdk_client->LoadSkeleton(Side::Side_Right); Sleep(100);
    if (pcan_initialized[1] && sdk_client->GetSkeletonCount() > loaded_skeletons) {
        controller_2 = new MotorController(pcan, handles[1], sdk_client, false);
		loaded_skeletons++;
		std::cout << "Right controller initialized." << std::endl;
    }
	std::cout << "Loaded " << loaded_skeletons << " hand skeletons." << std::endl;

    // 数据记录器初始化
    Recorder* recorder = new Recorder(controller_1);
	std::cout << "Recorder initialized." << std::endl;

    // 图形界面初始化
    GraphicInteractor* interactor = new GraphicInteractor(controller_1, controller_2, recorder);
	std::cout << "Graphic interactor initialized." << std::endl;

    if (success_inits == 0) std::cout << "No pcan device connected!" << std::endl;
	else std::cout << std::endl << "Successfully initialized " << success_inits << " PCAN devices." << std::endl;
    
    std::thread controller_1_thread;
	if (controller_1) controller_1_thread = std::thread(CtrlThread, controller_1);
    std::thread controller_2_thread;
    if (controller_2) controller_2_thread = std::thread(CtrlThread, controller_2);
    std::thread interactor_thread(GraphThread, interactor);
    std::thread recorder_thread(RecorderThread, recorder);

	if (controller_1_thread.joinable()) controller_1_thread.detach();
    if (controller_2_thread.joinable()) controller_2_thread.detach();
    interactor_thread.detach();
    recorder_thread.detach();

    while (true)Sleep(1000);
    if (controller_2) delete controller_2;
    if (controller_1) delete controller_1;
    delete interactor;
    delete recorder;
    delete sdk_client;
    delete pcan;
    
    return 0;
}