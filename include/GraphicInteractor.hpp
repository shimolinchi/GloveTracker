#pragma once
# include <graphics.h>   // EasyX 图形库
# include <conio.h>      // _kbhit, _getch
# include <string>
# include <thread>
# include <chrono>
# include <vector>
# include "SDKClient.hpp"
# include "MotorController.hpp"
# include "Recorder.hpp"
# include <math.h>
#pragma execution_character_set("utf-8")

enum class PanelState {
    MAIN_MENU,
    GLOVE_DATA,
    MOTOR_DATA,
    CALIBRATE,
    RECORDING,
    EXIT
};


class GraphicInteractor {
private:

    bool running = true;  // 控制循环是否继续
    bool paused = false;  // 是否暂停显示
    MotorController* controller;
    Recorder* recorder;

    ErgonomicsData left_hand_data;// 左手
    ErgonomicsData right_hand_data;// 右手

    std::vector<int> velocity_now;
    std::vector<int> velocity_drive;
    std::vector<int> position_now;
    std::vector<int> position_drive;
    PanelState state;

public:
    GraphicInteractor( MotorController* controller, Recorder* recorder);
    ~GraphicInteractor();
    ClientReturnCode DisplayingDataGlove(int start_y);
    void DisplayingDataMotor(int start_y);
    void DrawTitle();
    void DrawButton(int left, int top, int right, int bottom, LPCTSTR text, bool hover);
    template <typename T>
    void PrintSpecificData(const T& data, int start_y, int start_x = 50);
    void UpdateData();
    void StartCalibrate();
    void RecordOnce();
    void Recording();
    int InputNumber();
    void Init();
    void Run();
};