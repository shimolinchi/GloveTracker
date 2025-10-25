#pragma once
# include <graphics.h>   // EasyX 图形库
# include <conio.h>      // _kbhit, _getch
# include <string>
# include <thread>
# include <chrono>
# include <vector>
# include <sstream>
# include <iomanip>
# include <map>
# include "SDKClient.hpp"
# include "MotorController.hpp"
# include "Recorder.hpp"
# include <math.h>
# include <gdiplus.h>
#pragma execution_character_set("utf-8")

enum class PanelState {
    MAIN_MENU,
    GLOVE_DATA,
    MOTOR_DATA,
    CALIBRATE,
    RECORDING,
    EXCHANGING,
    POINTING_POS,
    EXIT
};


class GraphicInteractor {
private:

    bool running = true;  // 控制循环是否继续
    bool paused = false;  // 是否暂停显示
    MotorController* controller1;
	MotorController* controller2;
    Recorder* recorder;

    ErgonomicsData left_hand_data;// 左手
    ErgonomicsData right_hand_data;// 右手

    std::vector<int> left_velocity_now;
    std::vector<int> left_velocity_drive;
    std::vector<int> left_position_now;
    std::vector<int> left_position_drive;

    std::vector<int> right_velocity_now;
    std::vector<int> right_velocity_drive;
    std::vector<int> right_position_now;
    std::vector<int> right_position_drive;
    PanelState state;

    int button_width ;
    int button_height ;
    int button_spacing ;
    int round_radius;

    int little_button_width;
    int little_button_height;
    int little_button_spacing;
    int little_round_radius;

public:
    GraphicInteractor( MotorController* controller1, MotorController* controller2, Recorder* recorder);
    ~GraphicInteractor();
    ClientReturnCode DisplayingDataGlove(int start_y);
    void DisplayingDataMotor(int start_y);
    void DrawTitle(LPCTSTR text);
    void DrawButton(int left, int top, int right, int bottom, int round_radius, LPCTSTR text, bool hover);
    template <typename T>
    void PrintSpecificData(const T& data, int start_y, int start_x = 50);
    void UpdateData();
    void StartCalibrate();
    void RecordOnce();
    void Recording();
    int  InputNumber(LPCWSTR text);  
	void ExchangeHand();
    void ChangePointingPosition();
    void Init();
    void Run();
};