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

Recorder::Recorder(MotorController* controller, int frequency):controller(controller), recording(false), frequency(frequency){
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