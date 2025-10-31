/*
 * filename: Recorder.hpp
 * author: Rui Wang
 * version: 1.0.0
 * created: 2025
 * 
 * This program is protected by copyright law. 
 * Copyright © 2025 Rui Wang. All rights reserved.
 *
 * Unauthorized reproduction, distribution, or modification of this software, 
 * in whole or in part, without explicit written permission from the author 
 * is strictly prohibited.
 *
 * Contact: shimolinchi@gmail.com
 */
#ifndef RECORDER_HPP
#define RECORDER_HPP

# include <iostream>
# include <fstream>   // 文件流头文件
# include <thread>
# include <atomic>
# include <chrono>
# include "MotorController.hpp"

class Recorder {
private:
    MotorController* controller;
    std::ofstream file;
    std::string file_name = "position_list.csv";
    int frequency;
    // const std::string pointing_position_file_name;
public:
    std::atomic<bool> recording{false};
    Recorder(MotorController* controller, int frequency = 2);
    ~Recorder();
    std::wstring RecordPosOnce();
    void StartRecording();
    std::wstring ChangeFile(const std::string& filename);
    void StopRecording();
    void SetFrequency(int frequency);
    int  GetFrequency();
    void SaveFile();
    void SavePointingPosition();
    void Run();
    std::string GetFileName() const ;
};

#endif