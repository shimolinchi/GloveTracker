#include "stdafx.h"
#include <iostream>
#include <stdexcept>
#include "PCANBasic.hpp"
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// Helper function to load a function from the library
template<typename T>
void PCANBasic::loadFunction(T& funcPtr, const char* funcName) {
#ifdef _WIN32
    funcPtr = (T)GetProcAddress((HMODULE)m_dllHandle, funcName);
#else
    funcPtr = (T)dlsym(m_dllHandle, funcName);
#endif
    if (!funcPtr) {
        throw std::runtime_error("Could not load function: " + std::string(funcName));
    }
}

PCANBasic::PCANBasic() {
#ifdef _WIN32
    m_dllHandle = LoadLibraryA("PCANBasic.dll");
#else
    m_dllHandle = dlopen("libpcanbasic.so", RTLD_LAZY);
#endif

    if (!m_dllHandle) {
        throw std::runtime_error("Could not load the PCAN-Basic library.");
    }

    try {
        loadFunction(p_CAN_Initialize, "CAN_Initialize");
        loadFunction(p_CAN_InitializeFD, "CAN_InitializeFD");
        loadFunction(p_CAN_Uninitialize, "CAN_Uninitialize");
        loadFunction(p_CAN_Reset, "CAN_Reset");
        loadFunction(p_CAN_GetStatus, "CAN_GetStatus");
        loadFunction(p_CAN_Read, "CAN_Read");
        loadFunction(p_CAN_ReadFD, "CAN_ReadFD");
        loadFunction(p_CAN_Write, "CAN_Write");
        loadFunction(p_CAN_WriteFD, "CAN_WriteFD");
        loadFunction(p_CAN_FilterMessages, "CAN_FilterMessages");
        loadFunction(p_CAN_GetValue, "CAN_GetValue");
        loadFunction(p_CAN_SetValue, "CAN_SetValue");
        loadFunction(p_CAN_GetErrorText, "CAN_GetErrorText");
    }
    catch (const std::runtime_error& e) {
#ifdef _WIN32
        FreeLibrary((HMODULE)m_dllHandle);
#else
        dlclose(m_dllHandle);
#endif
        throw; // Re-throw the exception
    }
}

PCANBasic::~PCANBasic() {
    if (m_dllHandle) {
#ifdef _WIN32
        FreeLibrary((HMODULE)m_dllHandle);
#else
        dlclose(m_dllHandle);
#endif
    }
}

// Wrapper implementations
TPCANStatus PCANBasic::Initialize(TPCANHandle Channel, TPCANBaudrate Btr0Btr1, TPCANType HwType, uint32_t IOPort, uint16_t Interrupt) {
    TPCANStatus initialize_result = p_CAN_Initialize(Channel, Btr0Btr1, HwType, IOPort, Interrupt);
    if (initialize_result != PCAN_ERROR_OK) {
        std::string errorText;
        GetErrorText(initialize_result, 0, errorText);
        std::cerr << "Failed to initialize the PCAN device: " << errorText << " (Code: 0x" << std::hex << initialize_result << ")" << std::endl;
        return 1;
    }
    std::cout << "PCAN device initialized successfully" << std::endl;
    return initialize_result;
}

TPCANStatus PCANBasic::InitializeFD(TPCANHandle Channel, TPCANBitrateFD BitrateFD) {
    return p_CAN_InitializeFD(Channel, BitrateFD);
}

TPCANStatus PCANBasic::Uninitialize(TPCANHandle Channel) {
    return p_CAN_Uninitialize(Channel);
}

TPCANStatus PCANBasic::Reset(TPCANHandle Channel) {
    return p_CAN_Reset(Channel);
}

TPCANStatus PCANBasic::GetStatus(TPCANHandle Channel) {
    return p_CAN_GetStatus(Channel);
}

TPCANStatus PCANBasic::Read(TPCANHandle Channel, TPCANMsg& MessageBuffer, TPCANTimestamp& TimestampBuffer) {
    return p_CAN_Read(Channel, &MessageBuffer, &TimestampBuffer);
}

TPCANStatus PCANBasic::ReadFD(TPCANHandle Channel, TPCANMsgFD& MessageBuffer, TPCANTimestampFD& TimestampBuffer) {
    return p_CAN_ReadFD(Channel, &MessageBuffer, &TimestampBuffer);
}

TPCANStatus PCANBasic::Write(TPCANHandle Channel, const TPCANMsg& MessageBuffer) {
    return p_CAN_Write(Channel, &MessageBuffer);
}

TPCANStatus PCANBasic::WriteFD(TPCANHandle Channel, const TPCANMsgFD& MessageBuffer) {
    return p_CAN_WriteFD(Channel, &MessageBuffer);
}

TPCANStatus PCANBasic::FilterMessages(TPCANHandle Channel, uint32_t FromID, uint32_t ToID, TPCANMode Mode) {
    return p_CAN_FilterMessages(Channel, FromID, ToID, Mode);
}

TPCANStatus PCANBasic::GetValue(TPCANHandle Channel, TPCANParameter Parameter, void* Buffer, uint32_t BufferLength) {
    return p_CAN_GetValue(Channel, Parameter, Buffer, BufferLength);
}

TPCANStatus PCANBasic::SetValue(TPCANHandle Channel, TPCANParameter Parameter, const void* Buffer, uint32_t BufferLength) {
    return p_CAN_SetValue(Channel, Parameter, Buffer, BufferLength);
}

TPCANStatus PCANBasic::GetErrorText(TPCANStatus Error, uint16_t Language, std::string& errorText) {
    std::vector<char> buffer(256, 0);
    TPCANStatus result = p_CAN_GetErrorText(Error, Language, buffer.data());
    if (result == PCAN_ERROR_OK) {
        errorText = std::string(buffer.data());
    }
    return result;
}

/**
 * @brief ����һ��CAN��Ϣ
 * @param pcan PCANBasic��������
 * @param PcanHandle PCANͨ�����
 * @param motor_id ���ID
 * @param data Ҫ���͵����ݸ��� (std::vector<uint8_t>)
 */

void SendCANMessage(PCANBasic& pcan, TPCANHandle PcanHandle, uint32_t motor_id, const std::vector<uint8_t>& data) {
    TPCANMsg pcan_msg;
    pcan_msg.ID = motor_id;
    pcan_msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    pcan_msg.LEN = static_cast<uint8_t>(data.size());

    // ȷ�����ݳ��Ȳ�����8�ֽ�
    if (data.size() > 8) {
        std::cerr << "Error: CAN data length cannot exceed 8 bytes." << std::endl;
        return;
    }

    for (size_t i = 0; i < data.size(); ++i) {
        pcan_msg.DATA[i] = data[i];
    }

    TPCANStatus result = pcan.Write(PcanHandle, pcan_msg);
    if (result != PCAN_ERROR_OK) {
        std::string errorText;
        pcan.GetErrorText(result, 0, errorText);
        std::cerr << "Failed to send message: " << errorText << std::endl;
    }
}


/**
 * @brief ����Ԥ�趯��
 * @param pcan PCANBasic��������
 * @param PcanHandle PCANͨ�����
 * @param vel_array �ٶ����� (16��Ԫ��)
 * @param pos_array λ������ (16��Ԫ��)
 * @param current_array �������� (16��Ԫ��)
 */
void SendAction(PCANBasic& pcan, TPCANHandle PcanHandle, const std::vector<int>& vel_array, const std::vector<int>& pos_array, const std::vector<int>& current_array) {
    for (int i = 0; i < 16; ++i) {
        uint32_t motor_id = i + 1; // ���ID��1��16
        int speed = vel_array[i];
        int position = pos_array[i];
        int current = current_array[i];

        // ����CAN��Ϣ����: 0xAA������, ���λ�á��ٶȡ�����
        std::vector<uint8_t> data = {
            0xAA,                                     // ����
            static_cast<uint8_t>(position & 0xFF),    // λ�õ��ֽ�
            static_cast<uint8_t>((position >> 8) & 0xFF), // λ�ø��ֽ�
            static_cast<uint8_t>(speed & 0xFF),       // �ٶȵ��ֽ�
            static_cast<uint8_t>((speed >> 8) & 0xFF),    // �ٶȸ��ֽ�
            static_cast<uint8_t>(current & 0xFF),     // �������ֽ�
            static_cast<uint8_t>((current >> 8) & 0xFF)   // �������ֽ�
        };

        SendCANMessage(pcan, PcanHandle, motor_id, data);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        
    }
}