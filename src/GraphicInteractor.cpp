# include "GraphicInteractor.hpp"
#include <sstream>
#include <iomanip>

GraphicInteractor::GraphicInteractor(MotorController* controller): running(true), paused(false) ,controller(controller)
{
    
}

void GraphicInteractor::Init() {
    initgraph(1000, 800);  //宽，高
    setbkcolor(RGB(46, 46, 56));
    cleardevice();
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    BeginBatchDraw();
    state = PanelState::MAIN_MENU;
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


ClientReturnCode GraphicInteractor::DisplayingDataGlove(int start_y)
{
    const int LINE_HEIGHT = 20;
    const int START_X = 50;
    const int START_Y = start_y;

    settextcolor(WHITE);
    settextstyle(16, 0, _T("Consolas"));

    // 格式化浮点数，去掉多余的 0000
    auto fmt = [](double val, int precision = 2) {
        std::wostringstream oss;
        oss << std::fixed << std::setprecision(precision) << val;
        return oss.str();
    };

    auto PrintHandErgoData = [&](ErgonomicsData& p_ErgoData, bool p_Left, int startX, int startY, int lineHeight) {
        settextstyle(18, 0, _T("Consolas"));
        const std::wstring FingerNames[NUM_FINGERS_ON_HAND] = { L"[thumb] ", L"[index] ", L"[middle]", L"[ring]  ", L"[pinky] " };
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

void GraphicInteractor::DisplayingDataMotor(int start_y) {
    
    settextstyle(18, 0, _T("Consolas"));
    int y = start_y;
    int line_space = 20;

    auto DisplayData = [](const std::vector<int>& data, int start_y) {
        const std::wstring FingerNames[NUM_FINGERS_ON_HAND] = { L"[thumb] ", L"[index] ", L"[middle]", L"[ring]  ", L"[pinky] " };
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

    outtextxy(50, y, _T("position now"));   y += line_space;
    y = DisplayData(position_now, y);       y += line_space;
    outtextxy(50, y, _T("position drive")); y += line_space;
    y = DisplayData(position_drive, y);     y += line_space;
    outtextxy(50, y, _T("veloccity now"));  y += line_space;
    y = DisplayData(velocity_now, y);       y += line_space;
    outtextxy(50, y, _T("veloccity drive"));y += line_space;
    y = DisplayData(velocity_drive, y);     y += line_space;
}

// 绘制按钮的函数（带悬停高亮）
void GraphicInteractor::DrawButton(int left, int top, int right, int bottom, LPCTSTR text, bool hover) {
    if (hover) {
        setfillcolor(RGB(57, 127, 155)); // 鼠标悬停时稍微深一点的蓝灰色
    } else {
        setfillcolor(RGB(57, 155, 127)); 
    }
    solidrectangle(left, top, right, bottom);  // 填充矩形

    setlinecolor(RGB(100, 100, 100));   // 边框颜色
    rectangle(left, top, right, bottom);

    settextcolor(WHITE);   // 文字颜色
    settextstyle(26, 0, _T("Consolas"));
    int textX = (left + right) / 2 - textwidth(text) / 2;
    int textY = (top + bottom) / 2 - textheight(text) / 2;
    outtextxy(textX, textY, text);
}

void GraphicInteractor::DrawTitle() {
    settextcolor(WHITE);
    settextstyle(32, 0, _T("Consolas"));

    LPCTSTR text = _T("");
    switch (state) {
        case PanelState::MAIN_MENU:   text = _T("Main Menu");   break;
        case PanelState::GLOVE_DATA:  text = _T("Glove Data");  break;
        case PanelState::MOTOR_DATA:  text = _T("Motor Data");  break;
    }

    int textW = textwidth(text);           // 获取文字宽度
    int winW  = getwidth();                // 窗口宽度
    int x = (winW - textW) / 2;            // 居中计算
    outtextxy(x, 50, text);
}

// 打印任意数据到指定位置
template <typename T>
void GraphicInteractor::PrintSpecificData(const T& data, int start_y, int start_x ) {
    std::wstringstream wss;

    if constexpr (std::is_arithmetic_v<T>) {
        // 数字类型（int, float, double…）
        wss << data;
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        // std::string 转换为 wstring
        wss << std::wstring(data.begin(), data.end());
    }
    else if constexpr (std::is_same_v<T, const char*>) {
        // C 风格字符串
        std::string s(data);
        wss << std::wstring(s.begin(), s.end());
    }
    else if constexpr (std::is_same_v<T, std::wstring>) {
        // 已经是 wstring
        wss << data;
    }
    else if constexpr (std::is_same_v<T, const wchar_t*>) {
        wss << data;
    }
    else if constexpr (std::is_same_v<T, std::vector<int>>) {
        for (size_t i = 0; i < data.size(); i++) {
            wss << data[i];
            if (i < data.size() - 1) wss << L", ";
        }
    }
    else if constexpr (std::is_same_v<T, std::vector<double>>) {
        for (size_t i = 0; i < data.size(); i++) {
            wss << data[i];
            if (i < data.size() - 1) wss << L", ";
        }
    }
    else {
        // 其他没覆盖的类型，尝试用 operator<< 输出
        wss << data;
    }

    std::wstring text = wss.str();
    outtextxy(start_x, start_y, text.c_str());
}

void GraphicInteractor::Run() {
    BeginBatchDraw();  // 开启双缓冲
    int winW  = getwidth();
    int winH  = getheight();
    while (running) {

        UpdateData();
        cleardevice();

        // 绘制标题
        DrawTitle();

        int data_start_y = 100;

        // 主菜单界面
        if (state == PanelState::MAIN_MENU) {
            settextstyle(28, 0, _T("Consolas"));
            settextcolor(WHITE);

            // 窗口宽度
            int btnWidth  = 300;
            int btnHeight = 50;
            int spacing   = 70;
            int startY    = 180;   // 第一个按钮纵向位置中心

            int btnX1 = (winW - btnWidth) / 2;
            int btnX2 = btnX1 + btnWidth;

            // 获取鼠标位置
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(GetHWnd(), &mouse);

            bool hoverGlove = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= startY && mouse.y <= startY + btnHeight);
            bool hoverMotor = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= startY + spacing && mouse.y <= startY + spacing + btnHeight);
            bool hoverExit  = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= startY + spacing*2 && mouse.y <= startY + spacing*2 + btnHeight);

            // 绘制按钮（居中）
            DrawButton(btnX1, startY,                 btnX2, startY + btnHeight,         _T("Check Glove Data"), hoverGlove);
            DrawButton(btnX1, startY + spacing,       btnX2, startY + spacing + btnHeight,_T("Check Motor Data"), hoverMotor);
            DrawButton(btnX1, startY + spacing*2,    btnX2, startY + spacing*2 + btnHeight,_T("Exit Program"), hoverExit);

            // 鼠标点击检测
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    int x = msg.x, y = msg.y;
                    if (hoverGlove) state = PanelState::GLOVE_DATA;
                    else if (hoverMotor) state = PanelState::MOTOR_DATA;
                    else if (hoverExit)  state = PanelState::EXIT;
                }
            }
        }
        // ===== 手套数据界面 =====
        else if (state == PanelState::GLOVE_DATA) {
            DisplayingDataGlove(data_start_y);  
            outtextxy(50, winH - 60, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // ===== 电机数据界面 =====
        else if (state == PanelState::MOTOR_DATA) {
            DisplayingDataMotor(data_start_y);   
            outtextxy(50, winH - 60, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // ===== 退出程序 =====
        else if (state == PanelState::EXIT) {
            exit(0); 
        }

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EndBatchDraw();
}

GraphicInteractor::~GraphicInteractor() {
    closegraph();  // 关闭图形窗口
}