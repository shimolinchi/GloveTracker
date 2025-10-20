# include "GraphicInteractor.hpp"
#include <sstream>
#include <iomanip>

GraphicInteractor::GraphicInteractor(MotorController* controller1, MotorController* controller2, Recorder* recorder): 
    running(true), 
    paused(false) ,
    controller1(controller1), 
    controller2(controller2), 
    recorder(recorder),
	left_position_drive(16, 0),
	left_position_now(16, 0),
	left_velocity_drive(16, 0),
	left_velocity_now(16, 0),
	right_position_drive(16, 0),
	right_position_now(16, 0),
	right_velocity_drive(16, 0),
	right_velocity_now(16, 0),
    button_height(60),
    button_spacing(85),
    button_width(250)
{
}

void GraphicInteractor::Init() {
    initgraph(1000, 850);  //宽，高
    setbkcolor(RGB(30, 30, 30));
    cleardevice();
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    BeginBatchDraw();
    state = PanelState::MAIN_MENU;
    settextstyle(32, 0, _T("Consolas"));
}

void GraphicInteractor::UpdateData() {
    if(controller1 != nullptr) {
        left_hand_data = controller1->client->GetGloveErgoData(true);
        left_velocity_now = controller1->velocity_now;
        left_velocity_drive = controller1->velocity_drive;
        left_position_now = controller1->position_now;
        left_position_drive = controller1->position_drive;
    }


    if (controller2 != nullptr) {
        right_hand_data = controller2->client->GetGloveErgoData(false);

        right_velocity_now = controller2->velocity_now;
        right_velocity_drive = controller2->velocity_drive;
        right_position_now = controller2->position_now;
        right_position_drive = controller2->position_drive;
    }
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
        settextstyle(20, 0, _T("Consolas"));
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
        settextstyle(20, 0, _T("Consolas"));
        std::wstring header = L" -- Left Glove -- 0x" + std::to_wstring(left_hand_data.id) + L" - Angles in degrees";
        outtextxy(START_X, y, header.c_str());
        y += LINE_HEIGHT + 5;
        y = PrintHandErgoData(left_hand_data, true, START_X, y, LINE_HEIGHT);
    }

    // 右手
    {
        y += LINE_HEIGHT + 10;
        std::wstring header = L" -- Right Glove -- 0x" + std::to_wstring(right_hand_data.id) + L" - Angles in degrees";
        outtextxy(START_X, y, header.c_str());
        y += LINE_HEIGHT + 5;
        y = PrintHandErgoData(right_hand_data, false, START_X, y, LINE_HEIGHT);
    }

    return ClientReturnCode::ClientReturnCode_Success;
}

void GraphicInteractor::DisplayingDataMotor(int start_y) {
    
    settextstyle(20, 0, _T("Consolas"));
    int y = start_y;
    int line_space = 20;

    auto DisplayData = [](const std::vector<int>& data, int start_x, int start_y ) {
        const std::wstring FingerNames[NUM_FINGERS_ON_HAND] = { L"[thumb] ", L"[index] ", L"[middle]", L"[ring]  ", L"[pinky] " };
        const std::wstring FingerMotorNames[3] = { L"mcp1", L"mcp2", L"dpip" };
        int y = start_y;
        int t_DataOffset = 0;
        auto fmt = [](int val) {
            std::wostringstream oss;
            oss << val; // 直接输出整数，不带小数
            return oss.str();
        };
        for (unsigned int t_FingerNumber = 0; t_FingerNumber < 5; t_FingerNumber++)
        {

            std::wstring line =
                FingerNames[t_FingerNumber] + L" " +
                FingerMotorNames[0] + L" " + fmt(data[t_DataOffset]) + L", " +
                FingerMotorNames[1] + L" " + fmt(data[t_DataOffset + 1]) + L", " +
                FingerMotorNames[2] + L" " + fmt(data[t_DataOffset + 2]);

            outtextxy(start_x, y, line.c_str());
            y += 20;

            t_DataOffset += 3;
        }
        std::wstring line = L"[wrist] " + fmt(data[15]);
        outtextxy(start_x, y, line.c_str());
        y += 20;
        return y;
    };

	int data_start_y  = y;

    outtextxy(50, y, _T("position now"));   y += line_space;
    y = DisplayData(left_position_now, 50, y);       y += line_space;
    outtextxy(50, y, _T("position drive")); y += line_space;
    y = DisplayData(left_position_drive, 50, y);     y += line_space;
    outtextxy(50, y, _T("veloccity now"));  y += line_space;
    y = DisplayData(left_velocity_now, 50, y);       y += line_space;
    outtextxy(50, y, _T("veloccity drive"));y += line_space;
    y = DisplayData(left_velocity_drive, 50, y);     y += line_space;

	y = data_start_y;
    outtextxy(550, y, _T("position now"));   y += line_space;
    y = DisplayData(right_position_now, 550, y);       y += line_space;
    outtextxy(550, y, _T("position drive")); y += line_space;
    y = DisplayData(right_position_drive, 550, y);     y += line_space;
    outtextxy(550, y, _T("veloccity now"));  y += line_space;
    y = DisplayData(right_velocity_now, 550, y);       y += line_space;
    outtextxy(550, y, _T("veloccity drive"));y += line_space;
    y = DisplayData(right_velocity_drive, 550, y);     y += line_space;
}

void GraphicInteractor::DrawButton(int left, int top, int right, int bottom, LPCTSTR text, bool hover) {
    // 圆角半径，可以根据需要调整
    const int roundRadius = 20;

    if (hover) {
        setfillcolor(RGB(57, 127, 155)); // 鼠标悬停时稍微深一点的蓝灰色
    }
    else {
        setfillcolor(RGB(57, 155, 127));
    }
    fillroundrect(left, top, right, bottom, roundRadius, roundRadius);  // 填充圆角矩形

    // 设置更粗的边框
    setlinestyle(PS_SOLID, 2);  // 2像素宽的实线
    setlinecolor(RGB(100, 100, 100));   // 边框颜色
    roundrect(left, top, right, bottom, roundRadius, roundRadius);      // 绘制圆角边框

    // 恢复默认线宽（1像素），以免影响其他绘图操作
    setlinestyle(PS_SOLID, 1);

    settextcolor(WHITE);   // 文字颜色
    settextstyle(26, 0, _T("Consolas"));
    int textX = (left + right) / 2 - textwidth(text) / 2;
    int textY = (top + bottom) / 2 - textheight(text) / 2;
    outtextxy(textX, textY, text);
}

void GraphicInteractor::DrawTitle(LPCTSTR text) {
    settextcolor(WHITE);
    settextstyle(32, 0, _T("Consolas"));

    //LPCTSTR text = _T("");
    //switch (state) {
    //    case PanelState::MAIN_MENU:   text = _T("Main Menu");   break;
    //    case PanelState::GLOVE_DATA:  text = _T("Glove Data");  break;
    //    case PanelState::MOTOR_DATA:  text = _T("Motor Data");  break;
    //}

    int textW = textwidth(text);           // 获取文字宽度
    int winW  = getwidth();                // 窗口宽度
    int x = (winW - textW) / 2;            // 居中计算
    outtextxy(x, 50, text);
}void GraphicInteractor::StartCalibrate() {
    cleardevice();
    settextcolor(WHITE);
    settextstyle(32, 0, _T("Consolas"));
    LPCTSTR text = _T("");
    int winW = getwidth();
    int winH = getheight();
    controller1->calibrating_process = CalibrateProcess::START;

    // ---------------- 加载带透明通道的 PNG 图片 ----------------
    IMAGE handPostureImg;
    loadimage(&handPostureImg, _T("hand_posture.png")); // 替换为你的文件名

    // 获取背景色
    COLORREF bgColor = getbkcolor();

    // 取出原图像像素数据
    int w = handPostureImg.getwidth();
    int h = handPostureImg.getheight();
    DWORD* pSrc = GetImageBuffer(&handPostureImg);

    // 创建一张新的图片，背景为当前背景色
    IMAGE handPostureImgBG(w, h);
    DWORD* pDst = GetImageBuffer(&handPostureImgBG);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            DWORD c = pSrc[y * w + x];
            BYTE a = (c >> 24) & 0xFF; // Alpha 通道
            if (a < 128) { // 透明像素 → 背景色
                pDst[y * w + x] = bgColor;
            }
            else {
                pDst[y * w + x] = c;
            }
        }
    }

    // ---------- 手动缩放 ----------
    double scale = 0.6;
    int newW = static_cast<int>(w * scale);
    int newH = static_cast<int>(h * scale);
    IMAGE smallImg(newW, newH);

    HDC srcDC = GetImageHDC(&handPostureImgBG);
    HDC dstDC = GetImageHDC(&smallImg);
    SetStretchBltMode(dstDC, HALFTONE);
    StretchBlt(dstDC, 0, 0, newW, newH, srcDC, 0, 0, w, h, SRCCOPY);

    int imgX = (winW - newW) / 2;
    int imgY = 120;

    // ---------- 第一段：3 秒 ----------
    auto start1 = std::chrono::steady_clock::now();
    while (controller1->calibrating_process == CalibrateProcess::START) {
        cleardevice();

        // 检测右键退出
        if (MouseHit()) {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_RBUTTONDOWN) {
                controller1->calibrating_process = CalibrateProcess::FINISH;
                return; // 直接退出函数
            }
        }

        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start1).count();
        int remain = 3 - elapsed;
        if (remain <= 0) {
            controller1->calibrating_process = CalibrateProcess::STEP1;
            break;
        }

        TCHAR buf[256];
        _stprintf_s(buf, _T("Start calibrate in %d seconds, put your hand flat on the table"), remain);
        int textW = textwidth(buf);
        int x = (winW - textW) / 2;
        outtextxy(x, 50, buf);
        putimage(imgX, imgY, &smallImg);

        settextstyle(26, 0, _T("Consolas"));
        outtextxy(50, winH - 50, _T("Right click to return..."));

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // ---------- 第二段：3 秒 ----------
    auto start2 = std::chrono::steady_clock::now();
    while (controller1->calibrating_process == CalibrateProcess::STEP1) {
        cleardevice();

        // 检测右键退出
        if (MouseHit()) {
            MOUSEMSG m = GetMouseMsg();
            if (m.uMsg == WM_RBUTTONDOWN) {
                controller1->calibrating_process = CalibrateProcess::FINISH;
                return;
            }
        }

        auto now = std::chrono::steady_clock::now();
        int elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start2).count();
        int remain = 3 - elapsed;
        if (remain <= 0) {
            controller1->calibrating_process = CalibrateProcess::SWITCH;
            break;
        }

        TCHAR buf[256];
        _stprintf_s(buf, _T("Calibrating! Hold this gesture in %d seconds"), remain);
        int textW = textwidth(buf);
        int x = (winW - textW) / 2;
        outtextxy(x, 50, buf);
        putimage(imgX, imgY, &smallImg);

        settextstyle(26, 0, _T("Consolas"));
        outtextxy(50, winH - 50, _T("Right click to return..."));

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
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

void GraphicInteractor::RecordOnce() {
    std::wstring msg = recorder->RecordPosOnce();  // 获取提示信息
    int winW = getwidth();
    int winH = getheight();

    BeginBatchDraw(); 
    while (true) {
        cleardevice();

        // 显示提示信息
        settextstyle(26, 0, _T("Consolas"));
        settextcolor(WHITE);
        outtextxy(50, winH - 100, msg.c_str());
        outtextxy(50, winH - 50, _T("Right click to return...")); 

        FlushBatchDraw();

        // 检测鼠标右键
        if (MouseHit()) {
            MOUSEMSG msgMouse = GetMouseMsg();
            if (msgMouse.uMsg == WM_RBUTTONDOWN) {
                state = PanelState::MAIN_MENU;
                EndBatchDraw(); 
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int GraphicInteractor::InputNumber() {
    std::wstring inputStr;
    settextstyle(26, 0, L"Consolas");
    settextcolor(WHITE);

    ExMessage msg;
    bool cursorVisible = true;
    bool inputActive = true;  // 输入框激活状态
    auto lastBlink = std::chrono::steady_clock::now();

    int boxX = 50, boxY = 150;
    int boxW = 400, boxH = 40;

    while (true) {
        // 取所有消息（-1 表示不过滤）
        while (peekmessage(&msg, -1, true)) {
            switch (msg.message) {
            case WM_LBUTTONDOWN:
                // 鼠标点击，检查是否点到输入框
                if (msg.x >= boxX && msg.x <= boxX + boxW &&
                    msg.y >= boxY && msg.y <= boxY + boxH) {
                    inputActive = true;
                } else {
                    inputActive = false;
                }
                break;

            case WM_KEYDOWN:  // 功能键
                if (!inputActive) break;

                switch (msg.vkcode) {
                case VK_RETURN:  // Enter 键
                    if (!inputStr.empty()) {
                        try {
                            return std::stoi(inputStr); // 返回输入的数字
                        } catch (...) {
                            inputStr.clear(); // 输入非法时清空
                        }
                    }
                    break;

                case VK_BACK:  // 退格键
                    if (!inputStr.empty()) {
                        inputStr.pop_back();
                    }
                    break;

                case VK_ESCAPE:  // ESC 退出
                    return -1;
                }
                break;

            case WM_CHAR:  // 输入字符
                if (inputActive) {
                    if (msg.ch >= L'0' && msg.ch <= L'9') {
                        inputStr += static_cast<wchar_t>(msg.ch);
                    }
                }
                break;
            }
        }
        // 绘制界面
        cleardevice();

        outtextxy(50, 100, L"Input new frequency (Hz), press Enter to confirm");

        if (inputActive) {
            setlinecolor(YELLOW);
        } else {
            setlinecolor(WHITE);
        }

        // 绘制输入框
        setfillcolor(0x222222);
        fillrectangle(boxX, boxY, boxX + boxW, boxY + boxH);
        rectangle(boxX, boxY, boxX + boxW, boxY + boxH);

        // 显示输入内容
        settextcolor(WHITE);
        if (!inputStr.empty()) {
            outtextxy(boxX + 10, boxY + 8, inputStr.c_str());
        } else if (inputActive) {
            settextcolor(0x888888);
            outtextxy(boxX + 10, boxY + 8, L"Click here and input number...");
            settextcolor(WHITE);
        }

        // 光标闪烁
        if (inputActive) {
            int textW = textwidth(inputStr.c_str());
            int cursorX = boxX + 10 + textW;
            int cursorY1 = boxY + 5;
            int cursorY2 = boxY + boxH - 5;

            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlink).count() > 500) {
                cursorVisible = !cursorVisible;
                lastBlink = now;
            }

            if (cursorVisible) {
                line(cursorX, cursorY1, cursorX, cursorY2);
            }
        }

        if (!inputActive) {
            settextcolor(0xFFFF00);
            outtextxy(boxX, boxY + boxH + 10, L"Click on the input box to activate");
            settextcolor(WHITE);
        }

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return -1;
}



void GraphicInteractor::Recording() {

    settextcolor(WHITE);   
    settextstyle(32, 0, _T("Consolas"));
    int winW  = getwidth();           
    int winH  = getheight();

    int start_y = 180;
    int btnX1 = (winW - button_width) / 2;
    int btnX2 = btnX1 + button_width;
    if (controller1 == nullptr) {
        outtextxy(50, winH - 100, _T("No gloves connected!"));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return;
    }
    while (true) {
        cleardevice();
        DrawTitle(_T("Recording Options"));
        std::string file_info = "Current file: " + recorder->GetFileName();
        std::string freq_info = "Current frequency: " + std::to_string(recorder->GetFrequency()); 
        std::wstring w_file_info(file_info.begin(), file_info.end());
        std::wstring w_freq_info(freq_info.begin(), freq_info.end());
        outtextxy(50, winH - 120, w_file_info.c_str()); 
        outtextxy(50, winH - 180, w_freq_info.c_str()); 

        POINT mouse;
        if (GetCursorPos(&mouse)) {
            HWND hwnd = GetHWnd();
            if (hwnd) {
                ScreenToClient(hwnd, &mouse);
            }
        } else {
            mouse.x = mouse.y = 0;
        }
        int y = start_y;

        // 按钮 hover 检测
        bool hoverStartRec  = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
        DrawButton(btnX1, y,   btnX2, y + button_height, _T("Start Recording"), hoverStartRec); y += button_spacing;
        bool hoverStopRec   = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
        DrawButton(btnX1, y,   btnX2, y + button_height, _T("Stop Recording"),  hoverStopRec); y += button_spacing;
        bool hoverRecOnce   = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y+ button_height);
        DrawButton(btnX1, y,   btnX2, y + button_height, _T("Record Once"),     hoverRecOnce); y += button_spacing;
        bool hoverInputFreq = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
        DrawButton(btnX1, y,   btnX2, y + button_height, _T("Set Frequency"),   hoverInputFreq); y += button_spacing;

        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {   // 左键触发功能
                if (hoverRecOnce)       RecordOnce();
                else if (hoverStartRec) {
                    recorder->StartRecording();
                }
                else if (hoverStopRec)  {
                    recorder->StopRecording();
                }
                else if (hoverInputFreq) {
                    int freq = InputNumber();
                    recorder->SetFrequency(freq);
                    // 更新频率显示
                    freq_info = "Current frequency: " + std::to_string(freq);
                    w_freq_info.assign(freq_info.begin(), freq_info.end());
                }
            }
            else if (msg.uMsg == WM_RBUTTONDOWN) {  
                // 右键退出 Recording 菜单
                state = PanelState::MAIN_MENU;
                return;
            }
        }
        settextstyle(26, 0, _T("Consolas"));
        outtextxy(50, winH - 50, _T("right click back to menu"));
        if (recorder->recording)outtextxy(50, winH - 250, L"Recording! Stop before exit");
        else outtextxy(50, winH - 240, L"Not recording");

        std::string position_info = "" ;
        for (int i = 0; i < 16; i++) {
            position_info += std::to_string(controller1->position_drive[i]);
            if (i != 15) position_info += ",";
        }
        std::wstring w_position_info(position_info.begin(), position_info.end());
        outtextxy(50, winH - 310, L"Current position:");
        outtextxy(50, winH - 280, w_position_info.c_str());

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void GraphicInteractor::ExchangeHand() {
    if (controller1 != nullptr) {
		controller1->ChangeGloveHandSide();
    }
    if (controller2 != nullptr) {
        controller2->ChangeGloveHandSide();
    }
}

void GraphicInteractor::Run() {
    BeginBatchDraw();  // 开启双缓冲
    int winW  = getwidth();
    int winH  = getheight();
    while (controller1 == nullptr) {
        std::cout << "Waiting for gloves..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    while (running) {

        UpdateData();
        cleardevice();
        int data_start_y = 120;
        if (controller1->calibrating_process == CalibrateProcess::START){
            StartCalibrate();
        }

        // 主菜单界面
        if (state == PanelState::MAIN_MENU) {
            settextstyle(28, 0, _T("Consolas"));
            settextcolor(WHITE);
            DrawTitle(_T("Main Manu"));

            // 窗口宽度
            int start_y    = 180;   // 第一个按钮纵向位置中心

            int btnX1 = (winW - button_width) / 2;
            int btnX2 = btnX1 + button_width;

            // 获取鼠标位置
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(GetHWnd(), &mouse);
            int y = start_y;

            bool hoverGlove = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y,                 btnX2, y + button_height,         _T("Glove Data"), hoverGlove);
			y += button_spacing;
            bool hoverMotor = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y,       btnX2, y + button_height,_T("Motor Data"), hoverMotor);
            y += button_spacing;
            bool hoverCalibrate  = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y,    btnX2, y + button_height,_T("Calibrate"), hoverCalibrate);
            y += button_spacing;
            bool hoverRecording  = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y,    btnX2, y + button_height,_T("Recording"), hoverRecording);
            y += button_spacing;
            bool hoverExchanging = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y, btnX2, y + button_height, _T("Exchange Hand"), hoverExchanging);
            y += button_spacing;
            bool hoverExit  = (mouse.x >= btnX1 && mouse.x <= btnX2 && mouse.y >= y && mouse.y <= y + button_height);
            DrawButton(btnX1, y,    btnX2, y + button_height,_T("Exit Program"), hoverExit);
            y += button_spacing;


            // 鼠标点击检测
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_LBUTTONDOWN) {
                    int x = msg.x, y = msg.y;
                    if (hoverGlove) state = PanelState::GLOVE_DATA;
                    else if (hoverMotor) state = PanelState::MOTOR_DATA;
                    else if (hoverCalibrate)  state = PanelState::CALIBRATE;
                    else if (hoverRecording) state = PanelState::RECORDING;
                    else if (hoverExchanging) ExchangeHand();
                    else if (hoverExit)  state = PanelState::EXIT;
                }
            }
        }
        // ===== 手套数据界面 =====
        else if (state == PanelState::GLOVE_DATA) {
            DrawTitle(_T("Glove Data"));
            DisplayingDataGlove(data_start_y);  
            settextstyle(26, 0, _T("Consolas"));
            outtextxy(50, winH - 50, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // ===== 电机数据界面 =====
        else if (state == PanelState::MOTOR_DATA) {
            DrawTitle(_T("Motor Data"));
            DisplayingDataMotor(data_start_y);   
            settextstyle(26, 0, _T("Consolas"));
            outtextxy(50, winH - 50, _T("right click back to menu"));
            if (MouseHit()) {
                MOUSEMSG msg = GetMouseMsg();
                if (msg.uMsg == WM_RBUTTONDOWN) {
                    state = PanelState::MAIN_MENU;
                }
            }
        }
        // ===== 手套校准界面 =====
        else if (state == PanelState::CALIBRATE) {
            StartCalibrate();
            state = PanelState::MAIN_MENU;
        }
        else if (state == PanelState::RECORDING) {
            Recording();
        }
        // ===== 退出程序 =====
        else if (state == PanelState::EXIT) {
            TerminateProcess(GetCurrentProcess(), 0);
        }

        FlushBatchDraw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    EndBatchDraw();
}

GraphicInteractor::~GraphicInteractor() {
    closegraph();  // 关闭图形窗口
}