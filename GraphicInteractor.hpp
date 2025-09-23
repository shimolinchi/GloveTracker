#pragma once
# include <graphics.h>   // EasyX 图形库
# include <conio.h>      // _kbhit, _getch
# include <string>
# include <thread>
# include <chrono>
# include <vector>
# include "SDKClient.hpp"
# include "MotorController.hpp"
#pragma execution_character_set("utf-8")

enum class PanelState {
    MAIN_MENU,
    GLOVE_DATA,
    MOTOR_DATA,
    EXIT
};

class GraphicInteractor {
private:

    bool running = true;  // 控制循环是否继续
    bool paused = false;  // 是否暂停显示
    MotorController* controller;

    ErgonomicsData left_hand_data;// 左手
    ErgonomicsData right_hand_data;// 右手

    std::vector<int> velocity_now;
    std::vector<int> velocity_drive;
    std::vector<int> position_now;
    std::vector<int> position_drive;

    // int PrintHandErgoData(ErgonomicsData& p_ErgoData, bool p_Left, int startX, int startY, int lineHeight);

public:
    GraphicInteractor( MotorController* controller);
    ~GraphicInteractor();
    ClientReturnCode DisplayingDataGlove();
    void DisplayingDataMotor();
    void UpdateData();
    void Init();
    void Run();
};