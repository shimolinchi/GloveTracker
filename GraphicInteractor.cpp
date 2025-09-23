# include "GraphicInteractor.hpp"

GraphicInteractor::GraphicInteractor(MotorController* controller): running(true), paused(false) ,controller(controller)
{
    
}

void GraphicInteractor::Init() {
    initgraph(1000, 600);  //宽，高
    setbkcolor(WHITE);
    cleardevice();
    settextcolor(BLACK);
    settextstyle(32, 0, _T("Consolas"));
}

void GraphicInteractor::UpdateData() {
    left_hand_data = controller->client->GetGloveErgoData(true);
    right_hand_data = controller->client->GetGloveErgoData(false);

    velocity_now = controller->velocity_now;
    velocity_drive = controller->current_drive;
    position_now = controller->position_now;
    position_drive = controller->position_drive;
}

#include <sstream>
#include <iomanip>

ClientReturnCode GraphicInteractor::DisplayingDataGlove()
{
    const int LINE_HEIGHT = 20;
    const int START_X = 50;
    const int START_Y = 50;

    cleardevice();
    settextcolor(BLACK);
    settextstyle(16, 0, _T("Consolas"));

    // 小工具：格式化浮点数，去掉多余的 0000
    auto fmt = [](double val, int precision = 2) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(precision) << val;
        return oss.str();
    };

    // 内嵌 lambda 替代 PrintHandErgoData
    auto PrintHandErgoData = [&](ErgonomicsData& p_ErgoData, bool p_Left, int startX, int startY, int lineHeight) {
        settextstyle(18, 0, _T("Consolas"));
        const std::wstring FingerNames[NUM_FINGERS_ON_HAND] = { L"[thumb]", L"[index]", L"[middle]", L"[ring]", L"[pinky]" };
        const std::wstring FingerJointNames[3] = { L"mcp", L"pip", L"dip" };
        const std::wstring ThumbJointNames[3] = { L"cmc", L"mcp", L"ip " };

        int t_DataOffset = p_Left ? 0 : 20;
        const std::wstring* t_JointNames = ThumbJointNames;

        int y = startY;

        for (unsigned int t_FingerNumber = 0; t_FingerNumber < NUM_FINGERS_ON_HAND; t_FingerNumber++)
        {
            std::wstring line =
                FingerNames[t_FingerNumber] + L" " +
                t_JointNames[0] + L" spread: " + fmt(p_ErgoData.data[t_DataOffset]) + L", " +
                t_JointNames[0] + L" stretch: " + fmt(p_ErgoData.data[t_DataOffset + 1]) + L", " +
                t_JointNames[1] + L" stretch: " + fmt(p_ErgoData.data[t_DataOffset + 2]) + L", " +
                t_JointNames[2] + L" stretch: " + fmt(p_ErgoData.data[t_DataOffset + 3]);

            outtextxy(startX, y, line.c_str());
            y += lineHeight;

            // 除了大拇指外，其他手指用 pip/dip 命名
            t_JointNames = FingerJointNames;
            t_DataOffset += 4;
        }

        return y;
    };

    int y = START_Y;

    // 左手
    {
        std::wstring header = L" -- Left Glove -- 0x" + std::to_wstring(left_hand_data.id) + L" - Angles in degrees";
        outtextxy(START_X, y, header.c_str());
        y += LINE_HEIGHT;
        y = PrintHandErgoData(left_hand_data, true, START_X, y, LINE_HEIGHT);
    }

    // 右手
    {
        y += LINE_HEIGHT;
        std::wstring header = L" -- Right Glove -- 0x" + std::to_wstring(right_hand_data.id) + L" - Angles in degrees";
        outtextxy(START_X, y, header.c_str());
        y += LINE_HEIGHT;
        y = PrintHandErgoData(right_hand_data, false, START_X, y, LINE_HEIGHT);
    }

    return ClientReturnCode::ClientReturnCode_Success;
}

void GraphicInteractor::DisplayingDataMotor() {
    auto DisplayData = [](const std::vector<int>& data, int start_y) {
        const std::wstring FingerNames[NUM_FINGERS_ON_HAND] = { L"[thumb]", L"[index]", L"[middle]", L"[ring]", L"[pinky]" };
        const std::wstring FingerMotorNames[3] = { L"mcp1", L"mcp2", L"dpip" };
        int y = start_y;
        int t_DataOffset = 0;
        for (unsigned int t_FingerNumber = 0; t_FingerNumber < 5; t_FingerNumber++)
        {
            auto fmt = [](int val) {
                std::wostringstream oss;
                oss << val; // 直接输出整数，不带小数
                return oss.str();
            };

            std::wstring line =
                FingerNames[t_FingerNumber] + L" " +
                FingerMotorNames[0] + L" " + fmt(data[t_DataOffset]) + L", " +
                FingerMotorNames[1] + L" " + fmt(data[t_DataOffset + 1]) + L", " +
                FingerMotorNames[2] + L" " + fmt(data[t_DataOffset + 2]);

            outtextxy(50, y, line.c_str());
            y += 20;

            t_DataOffset += 3;
        }
        return y;
    };

    settextstyle(18, 0, _T("Consolas"));
    int y = 50;
    outtextxy(50, y, _T("position now"));   y += 20;
    y = DisplayData(position_now, y);       y += 20;
    outtextxy(50, y, _T("position drive")); y += 20;
    y = DisplayData(position_drive, y);     y += 20;
}

void GraphicInteractor::Run() {
    PanelState state = PanelState::MAIN_MENU;

    while (running) {

        UpdateData();
        // 清屏
        cleardevice();

        // 主菜单界面
        if (state == PanelState::MAIN_MENU) {
            settextstyle(28, 0, _T("Consolas"));
            settextcolor(BLACK);

            outtextxy(300,  70, _T("================="));
            outtextxy(300, 100, _T("=== main menu ==="));
            outtextxy(300, 130, _T("================="));

            // 按钮区域
            rectangle(250, 180, 550, 230);
            outtextxy(300, 190, _T("check glove data"));

            rectangle(250, 250, 550, 300);
            outtextxy(300, 260, _T("check motor data"));

            rectangle(250, 320, 550, 370);
            outtextxy(330, 330, _T("exit progeam"));

            // 鼠标检测
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    int x = msg.x, y = msg.y;
                    if (x >= 250 && x <= 550 && y >= 180 && y <= 230) {
                        state = PanelState::GLOVE_DATA;
                    }
                    else if (x >= 250 && x <= 550 && y >= 250 && y <= 300) {
                        state = PanelState::MOTOR_DATA;
                    }
                    else if (x >= 250 && x <= 550 && y >= 320 && y <= 370) {
                        state = PanelState::EXIT;
                    }
                }
            }
        }
        // 手套数据界面
        else if (state == PanelState::GLOVE_DATA) {
            DisplayingDataGlove();  
            settextstyle(30, 0, _T("Consolas"));
            outtextxy(50, 550, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // 电机数据界面
        else if (state == PanelState::MOTOR_DATA) {
            DisplayingDataMotor();   
            settextstyle(30, 0, _T("Consolas"));
            outtextxy(50, 550, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // 退出程序
        else if (state == PanelState::EXIT) {
            exit(0); 
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

GraphicInteractor::~GraphicInteractor() {
    closegraph();  // 关闭图形窗口
}