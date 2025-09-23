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



void GraphThread(GraphicInteractor* interactor)
{
    interactor->Init();
    interactor->Run();
}

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

void CtrlThread(MotorController* controller)
{
    controller->Run();
}
int main(int argc, char* argv[])
{
    ManusSDK::ClientLog::print("Starting SDK client!");

    SDKClient sdk_client;
    PCANBasic pcan;

    TPCANHandle PcanHandle = PCAN_USBBUS1;
    TPCANBaudrate baudrate = PCAN_BAUD_1M;
    TPCANStatus result = pcan.Initialize(PcanHandle, baudrate);
    MotorController controller(&pcan, PcanHandle, &sdk_client);
    GraphicInteractor interactor(&controller);
    

    if (result != PCAN_ERROR_OK) {
        std::string errorText;
        pcan.GetErrorText(result, 0, errorText);
        std::cerr << "Failed to initialize the PCAN device: " << errorText << " (Code: 0x" << std::hex << result << ")" << std::endl;
        return 1;
    }

    std::cout << "PCAN device initialized successfully on handle " << std::hex << PcanHandle << "." << std::endl;


    std::thread sdkRunner(SDKThread, &sdk_client);
    std::thread controller_thread(CtrlThread, &controller);
    std::thread interactor_thread(GraphThread, &interactor);

    sdkRunner.join();

    interactor_thread.detach();

    ManusSDK::ClientLog::print("SDK client is done, shutting down.");

    ClientReturnCode t_Result = sdk_client.ShutDown();
    return static_cast<int>(t_Result);
}