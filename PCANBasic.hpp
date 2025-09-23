// PCANBasic.hpp

#ifndef PCANBASIC_HPP
#define PCANBASIC_HPP

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>
#include <thread>

// Type definitions from PCANBasic.py
using TPCANHandle = uint16_t;
using TPCANStatus = uint32_t;
using TPCANParameter = uint8_t;
using TPCANDevice = uint8_t;
using TPCANMessageType = uint8_t;
using TPCANType = uint8_t;
using TPCANMode = uint8_t;
using TPCANBaudrate = uint16_t;
using TPCANBitrateFD = const char*;
using TPCANTimestampFD = uint64_t;

// Structure packing is important to ensure memory layout matches the DLL's expectations.
#pragma pack(push, 1)

// Represents a PCAN message
struct TPCANMsg {
    uint32_t ID;
    TPCANMessageType MSGTYPE;
    uint8_t LEN;
    uint8_t DATA[8];
};

// Represents a timestamp of a received PCAN message
struct TPCANTimestamp {
    uint32_t millis;
    uint16_t millis_overflow;
    uint16_t micros;
};

// Represents a PCAN message from a FD capable hardware
struct TPCANMsgFD {
    uint32_t ID;
    TPCANMessageType MSGTYPE;
    uint8_t DLC;
    uint8_t DATA[64];
};

#pragma pack(pop)

// All constant definitions from PCANBasic.py
// (A comprehensive list of all #defines from the Python file)

// PCAN Handles
constexpr TPCANHandle PCAN_NONEBUS = 0x00;
constexpr TPCANHandle PCAN_ISABUS1 = 0x21;
// ... (and all other PCAN_...BUS... handles)
constexpr TPCANHandle PCAN_USBBUS1 = 0x51;
constexpr TPCANHandle PCAN_USBBUS2 = 0x52;
// ... etc.

// PCAN Status & Error Codes
constexpr TPCANStatus PCAN_ERROR_OK = 0x00000;
constexpr TPCANStatus PCAN_ERROR_XMTFULL = 0x00001;
constexpr TPCANStatus PCAN_ERROR_OVERRUN = 0x00002;
constexpr TPCANStatus PCAN_ERROR_BUSLIGHT = 0x00004;
constexpr TPCANStatus PCAN_ERROR_BUSHEAVY = 0x00008;
constexpr TPCANStatus PCAN_ERROR_BUSWARNING = PCAN_ERROR_BUSHEAVY;
constexpr TPCANStatus PCAN_ERROR_BUSPASSIVE = 0x40000;
constexpr TPCANStatus PCAN_ERROR_BUSOFF = 0x00010;
constexpr TPCANStatus PCAN_ERROR_ANYBUSERR = (PCAN_ERROR_BUSWARNING | PCAN_ERROR_BUSLIGHT | PCAN_ERROR_BUSHEAVY | PCAN_ERROR_BUSOFF | PCAN_ERROR_BUSPASSIVE);
constexpr TPCANStatus PCAN_ERROR_QRCVEMPTY = 0x00020;
constexpr TPCANStatus PCAN_ERROR_QOVERRUN = 0x00040;
constexpr TPCANStatus PCAN_ERROR_QXMTFULL = 0x00080;
constexpr TPCANStatus PCAN_ERROR_REGTEST = 0x00100;
constexpr TPCANStatus PCAN_ERROR_NODRIVER = 0x00200;
// ... (and all other error codes)
constexpr TPCANStatus PCAN_ERROR_INITIALIZE = 0x4000000;
constexpr TPCANStatus PCAN_ERROR_ILLOPERATION = 0x8000000;

// PCAN Parameters
constexpr TPCANParameter PCAN_DEVICE_NUMBER = 0x01;
constexpr TPCANParameter PCAN_API_VERSION = 0x05;
constexpr TPCANParameter PCAN_CHANNEL_VERSION = 0x06;
constexpr TPCANParameter PCAN_BUSOFF_AUTORESET = 0x07;
constexpr TPCANParameter PCAN_LISTEN_ONLY = 0x08;
constexpr TPCANParameter PCAN_LOG_LOCATION = 0x09;
constexpr TPCANParameter PCAN_LOG_STATUS = 0x0A;
// ... (and all other parameters)

// PCAN Parameter Values
constexpr int PCAN_PARAMETER_OFF = 0x00;
constexpr int PCAN_PARAMETER_ON = 0x01;
constexpr int PCAN_FILTER_CLOSE = 0x00;
constexpr int PCAN_FILTER_OPEN = 0x01;

// PCAN Message Types
constexpr TPCANMessageType PCAN_MESSAGE_STANDARD = 0x00;
constexpr TPCANMessageType PCAN_MESSAGE_RTR = 0x01;
constexpr TPCANMessageType PCAN_MESSAGE_EXTENDED = 0x02;
constexpr TPCANMessageType PCAN_MESSAGE_FD = 0x04;
// ... (and all other message types)

// Baud Rate Codes
constexpr TPCANBaudrate PCAN_BAUD_1M = 0x0014;
constexpr TPCANBaudrate PCAN_BAUD_500K = 0x001C;
constexpr TPCANBaudrate PCAN_BAUD_250K = 0x011C;
// ... (and all other baud rates)

// FD Bitrate Strings
// Note: In C++, TPCANBitrateFD is const char*.
// Example: "f_clock=80000000,nom_brp=10,..."

// Hardware Types
constexpr TPCANType PCAN_TYPE_ISA = 0x01;
// ... (and all other hardware types)


// Main class that wraps the PCAN-Basic API
class PCANBasic {
public:
    // Constructor: Loads the DLL/shared library and retrieves function pointers
    PCANBasic();

    // Destructor: Unloads the library
    ~PCANBasic();

    // API Functions
    TPCANStatus Initialize(TPCANHandle Channel, TPCANBaudrate Btr0Btr1, TPCANType HwType = 0, uint32_t IOPort = 0, uint16_t Interrupt = 0);
    TPCANStatus InitializeFD(TPCANHandle Channel, TPCANBitrateFD BitrateFD);
    TPCANStatus Uninitialize(TPCANHandle Channel);
    TPCANStatus Reset(TPCANHandle Channel);
    TPCANStatus GetStatus(TPCANHandle Channel);
    TPCANStatus Read(TPCANHandle Channel, TPCANMsg& MessageBuffer, TPCANTimestamp& TimestampBuffer);
    TPCANStatus ReadFD(TPCANHandle Channel, TPCANMsgFD& MessageBuffer, TPCANTimestampFD& TimestampBuffer);
    TPCANStatus Write(TPCANHandle Channel, const TPCANMsg& MessageBuffer);
    TPCANStatus WriteFD(TPCANHandle Channel, const TPCANMsgFD& MessageBuffer);
    TPCANStatus FilterMessages(TPCANHandle Channel, uint32_t FromID, uint32_t ToID, TPCANMode Mode);
    TPCANStatus GetValue(TPCANHandle Channel, TPCANParameter Parameter, void* Buffer, uint32_t BufferLength);
    TPCANStatus SetValue(TPCANHandle Channel, TPCANParameter Parameter, const void* Buffer, uint32_t BufferLength);
    TPCANStatus GetErrorText(TPCANStatus Error, uint16_t Language, std::string& errorText);

private:
    // Pointers to the functions in the DLL
    using CAN_Initialize_t = TPCANStatus(*)(TPCANHandle, TPCANBaudrate, TPCANType, uint32_t, uint16_t);
    using CAN_InitializeFD_t = TPCANStatus(*)(TPCANHandle, TPCANBitrateFD);
    using CAN_Uninitialize_t = TPCANStatus(*)(TPCANHandle);
    using CAN_Reset_t = TPCANStatus(*)(TPCANHandle);
    using CAN_GetStatus_t = TPCANStatus(*)(TPCANHandle);
    using CAN_Read_t = TPCANStatus(*)(TPCANHandle, TPCANMsg*, TPCANTimestamp*);
    using CAN_ReadFD_t = TPCANStatus(*)(TPCANHandle, TPCANMsgFD*, TPCANTimestampFD*);
    using CAN_Write_t = TPCANStatus(*)(TPCANHandle, const TPCANMsg*);
    using CAN_WriteFD_t = TPCANStatus(*)(TPCANHandle, const TPCANMsgFD*);
    using CAN_FilterMessages_t = TPCANStatus(*)(TPCANHandle, uint32_t, uint32_t, TPCANMode);
    using CAN_GetValue_t = TPCANStatus(*)(TPCANHandle, TPCANParameter, void*, uint32_t);
    using CAN_SetValue_t = TPCANStatus(*)(TPCANHandle, TPCANParameter, const void*, uint32_t);
    using CAN_GetErrorText_t = TPCANStatus(*)(TPCANStatus, uint16_t, char*);

    // Function pointers
    CAN_Initialize_t     p_CAN_Initialize;
    CAN_InitializeFD_t   p_CAN_InitializeFD;
    CAN_Uninitialize_t   p_CAN_Uninitialize;
    CAN_Reset_t          p_CAN_Reset;
    CAN_GetStatus_t      p_CAN_GetStatus;
    CAN_Read_t           p_CAN_Read;
    CAN_ReadFD_t         p_CAN_ReadFD;
    CAN_Write_t          p_CAN_Write;
    CAN_WriteFD_t        p_CAN_WriteFD;
    CAN_FilterMessages_t p_CAN_FilterMessages;
    CAN_GetValue_t       p_CAN_GetValue;
    CAN_SetValue_t       p_CAN_SetValue;
    CAN_GetErrorText_t   p_CAN_GetErrorText;

    // Handle to the loaded library
    void* m_dllHandle;

    // Helper to load function pointers
    template<typename T>
    void loadFunction(T& funcPtr, const char* funcName);
};


void SendCANMessage(PCANBasic& pcan, TPCANHandle PcanHandle, uint32_t motor_id, const std::vector<uint8_t>& data);

void SendAction(PCANBasic& pcan, TPCANHandle PcanHandle, const std::vector<int>& vel_array, const std::vector<int>& pos_array, const std::vector<int>& current_array);


#endif // PCANBASIC_HPP