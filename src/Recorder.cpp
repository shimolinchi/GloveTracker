# include "Recorder.hpp"
#include <string>
#include <fstream>
#include <codecvt>
#include <locale>


std::wstring s2ws(const std::string& str) {
    // UTF-8 转宽字符
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

Recorder::~Recorder() {
        StopRecording();
        SaveFile();
    }

std::wstring Recorder::ChangeFile(const std::string& f_name) {
    if (recording) {
        return s2ws("Recording, can not change file !");
    }
    if (file.is_open()) {
        file.close();
    }
    file_name = f_name;
    return s2ws("File has been change to:" + file_name);
}

Recorder::Recorder(MotorController* controller, int frequency):controller(controller), recording(false), frequency(frequency)
{
}

std::wstring Recorder::RecordPosOnce() {
    if (!file.is_open()) {
        file.open(file_name, std::ios::out | std::ios::app);
        if (!file) {
            return s2ws("File " + file_name + " has been open");
        }
    }
    if(recording){
        return s2ws("Continuously recording, can't recorde once!");
    }
    for (int i = 0; i < 16; i++) {
        file << controller->position_drive[i];
        if (i < 15) file << ",";
    }
    file << "\n";
    std::ostringstream oss;
    oss << "Record position: ";
    for (int i = 0; i < 16; i++) {
        oss << controller->position_drive[i];
        if (i < 15) oss << " ";  // 用空格分隔
    }
    file.flush();
    file.close();
    return s2ws(oss.str());
}

void Recorder::StartRecording() {
    if (recording) return; // 已在录制中
    recording = true;
}

void Recorder::StopRecording() {
    recording = false;
}

void Recorder::SetFrequency(int frequency){
    this->frequency = frequency;
}

int Recorder::GetFrequency(){return frequency;}

void Recorder::SaveFile() {
    recording = false;
    if (file.is_open()) {
        file.flush();
        file.close();
    }
}

std::string Recorder::GetFileName() const { return file_name; }

void Recorder::SavePointingPosition() {
    
    const std::string& filename = controller->pointing_position_file_name;
    
    // 1. 检查并创建父目录（如果 filename 包含路径）
    // 这解决了当文件路径中目录不存在时 '无法创建文件' 的问题
    std::filesystem::path file_path(filename);
    std::filesystem::path dir_path = file_path.parent_path();
    
    // 如果 dir_path 不为空（即 filename 包含目录），则尝试创建
    if (!dir_path.empty() && !std::filesystem::exists(dir_path)) {
        try {
            if (!std::filesystem::create_directories(dir_path)) {
                // 如果创建目录失败，则输出错误并返回
                std::cerr << "Error: Can not create a directory" << dir_path.string() << std::endl;
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: An exception occurred while creating a directory (" << dir_path.string() << "): " << e.what() << std::endl;
            return;
        }
    }

    bool fileExists = std::filesystem::exists(filename);
    std::ofstream file;

    if (fileExists) {
        // 文件存在：以追加模式打开
        file.open(filename, std::ios::out | std::ios::app);
    } else {
        // 文件不存在：以输出模式打开（创建文件）
        file.open(filename, std::ios::out);
    }

    if (!file.is_open()) {
        // 如果打开失败（无论是追加还是创建），则报告错误
        std::cerr << "Can not read or create file: " << filename << std::endl;
        return;
    }

    // --- 写入数据逻辑 ---

    if (fileExists) {
        // 文件已存在，追加时添加注释
        file << "\n# --- saved position ---\n";
        file << "# pointing_motor_position\n";
    } else {
        // 文件是新创建的，写入头信息
        file << "pointing_motor_position\n";
    }

    // 统一的写入数据的循环
    for (size_t i = 0; i < 4; ++i) {
        if (fileExists) {
            // 如果是追加，将数据行注释掉
            file << "# "; 
        }

        for (size_t j = 0; j < 16; ++j) {
            file << controller->GetPointingPosition(i, j);
            if (j != 15) file << ", ";
        }
        file << "\n";
    }

    if (fileExists) {
        file << "# --- save end ---\n";
        std::cout << "Pointing position saved in: " << filename << std::endl;
    } else {
        std::cout << "File don't exist, create a new file: " << filename << std::endl;
    }

    file.close();
}

void Recorder::Run(){
    while (true) {
        if (recording) {
            if (!file.is_open()) {
                file.open(file_name, std::ios::out | std::ios::app);
            }
            for (int i = 0; i < 16; i++) {
                file << controller->position_drive[i];
                if (i < 15) file << ",";
            }
            file << "\n";  
            file.flush();
            if (file.is_open()) {
                file.close();
            }
        }
        std::this_thread::sleep_for(std::chrono::duration<double>(1.0 / frequency));
    }
}