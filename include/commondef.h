#pragma once

#include <cstdint>
#include <vector>

#ifdef WIN32
#include <windows.h>
#include <typedef.h>
#else
#endif

#define MAX_ERROR_MSG_LEN 255


////////////////////////////////
///      Numerical types
////////////////////////////////

using Int8  = signed char;
using Int16 = signed short int;
using Int32 = signed int;
using Int64 = signed long long int;

#ifndef __cpp_char8_t
using char8_t = unsigned char;
#endif

/// This is needed for more strict aliasing. https://godbolt.org/z/xpJBSb https://stackoverflow.com/a/57453713
#if !defined(PVS_STUDIO) /// But PVS-Studio does not treat it correctly.
using UInt8 = char8_t;
#else
using UInt8 = uint8_t;
#endif

using UInt16 = unsigned short int;
using UInt32 = unsigned int;
using UInt64 = unsigned long long int;

using Float32 = float;
using Float64 = double;
using Float128 = long double;

#ifdef WIN32
#define EXPORT_API_DX21 __declspec(dllexport)
#define IMPORT_API_DX21 __declspec(dllimport)

#ifdef BUILD_DLL
#define DEXHAND_API EXPORT_API_DX21
#else
#define DEXHAND_API IMPORT_API_DX21
#endif

#else // Linux
#define DEXHAND_API
#endif

#ifdef WIN32
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;

// Sokcet related
#define SHUT_WR SD_SEND

#else // Linux
typedef unsigned char byte;

#define sprintf_s sprintf

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// Common system calls
#define Sleep(a) sleep(a)

#endif


namespace DexRobot
{

enum class LOG_LEVEL
{
    DX_CLOSE = 0,
    DX_FATAL,
    DX_ERROR,
    DX_WARN,
    DX_INFO,
    DX_DEBUG,
};


enum class SysErrorCode
{
    NONE = 0,
    ADAPTER_NA,
    DEV_TYPE_MISMATCH,
    CONN_LOST,
    CONN_TIMEOUT,
    BOOT_FAILURE,
};

enum MotorControlMode
{
    ZERO_TORQUE_MODE = 0x00,
    HALL_POSITION_CONTROL_MODE = 0x33,
    HALL_POSLIMIT_CONTROL_MODE = 0x55,
    CASCADED_PID_CONTROL_MODE  = 0x44,
    CASCADED_MIT_CONTROL_MODE  = 0x66,
    CASCADED_IMP_CONTROL_MODE  = 0x77,
};

enum ErrorType
{
    None_Error     = 0x00,
    MotorsError    = 0xEE,
    MotorLimitHall = 0xEF,
    MotorOverHeat  = 0xFF,
};

struct MotorErrorMsg
{
    uint8_t  motorId;
    uint8_t  errorType;
    uint16_t errorCode;
    char errorMsg[256];

    MotorErrorMsg() = delete;
    explicit MotorErrorMsg(const uint8_t motorId);
};

struct ErrorMessageRx
{
    uint8_t channelId;
    uint8_t deviceId;
    int16_t overloadError;
    std::vector<MotorErrorMsg> motorErrors;

    ErrorMessageRx() = delete;
    ErrorMessageRx(const uint8_t chnId, const uint8_t fgrId, const int16_t overloadErrCode);
};

enum class MotorErrorCode : int32_t
{
    NONE_ERROR = 0x0000,
    OVERLOADED = 0x0001,
    HALL_ERROR = 0x0002,
    MOT_BLOCK  = 0x0004,
    ANG_ERROR  = 0x0008,
};

enum class MotorErrorCode_21S : int32_t
{
    NONE_ERROR = 0x0000,
    CRT_ERROR  = 0x0001,
    VTG_ERROR  = 0x0002,
    OVER_HEAT  = 0x0004,
    MOT_BLOCK  = 0x0008,
    MOT_ERROR  = 0x0010,
};

struct SysParameterRWRx
{
    uint8_t channelId;
    uint8_t deviceId;
    uint8_t operationType;
    uint8_t parameterId;
    uint8_t succeed;
    uint8_t dataLen;
    uint8_t results[10]; // 11 bytes may not be fully used, also for padding purpose
};

enum HandID
{
    LEFT = 0,
    RIGHT,
    None = 0xFF,
};

enum FingerID
{
    NONE_ID      = 0xFF,
    ENTIRE_HAND  = 0x00,
    LEFT_THUMB   = 0x01,
    LEFT_PALM    = 0x02,
    LEFT_INDEX   = 0x03,
    LEFT_MIDDLE  = 0x04,
    LEFT_RING    = 0x05,
    LEFT_PINKY   = 0x06,

    RIGHT_THUMB  = 0x07,
    RIGHT_PALM   = 0x08,
    RIGHT_INDEX  = 0x09,
    RIGHT_MIDDLE = 0x0A,
    RIGHT_RING   = 0x0B,
    RIGHT_PINKY  = 0x0C,

    FGR_UNDEFINED = 0x0D,
};

enum class JointMotor : uint8_t
{
    DIST = 0x01,
    PROX = 0x02,
    ALL = 0x03
};

enum class JointID : uint8_t
{
    NONE = 0xFF,
    TH_DIP = 0,
    TH_MCP,
    TH_ROT,
    FF_SPR,
    FF_DIP,
    FF_MCP,
    MF_DIP,
    MF_MCP,
    RF_DIP,
    RF_MCP,
    LF_DIP,
    LF_MCP,
};

typedef union ControlEnableMap_t
{
    uint16_t mask;

    struct
    {
        uint8_t MddleProx : 1;
        uint8_t MddleDist : 1;
        uint8_t IndexProx : 1;
        uint8_t IndexDist : 1;
        uint8_t FngerSwng : 1;
        uint8_t ThumbSwng : 1;
        uint8_t ThumbProx : 1;
        uint8_t ThumbDist : 1;
        uint8_t Reserved  : 4;
        uint8_t LttleProx : 1;
        uint8_t LttleDist : 1;
        uint8_t RingFProx : 1;
        uint8_t RingFDist : 1;
    } enables;
} ControlEnableMap;

typedef union ControlParams_t
{
    uint8_t mask;

    struct
    {
        uint8_t HandType : 1;  // 0 for left hand, 1 for right hand
        uint8_t Feedback : 1;  // 0 for disable feedback, 1 for enable
        uint8_t ClearErr : 1;  // 0 for do NOT clear error automatically, 1 for clear
        uint8_t Reserved : 5;
    } params;
} ControlParameter;

typedef struct FingerControlData_t
{
    uint16_t DistPos;//remote end
    uint16_t ProxPos;
    uint16_t DistSpd;
    uint16_t ProxSpd;
    uint8_t  DistCur;
    uint8_t  ProxCur;
    uint16_t Reserve;
} FingerControlData;

typedef struct HandControlDesc_t
{
    MotorControlMode  mode;
    ControlEnableMap  enableMap;
    ControlParameter  controlParam;
    FingerControlData thumb;
    FingerControlData swing;
    FingerControlData index;
    FingerControlData middle;
    FingerControlData ring;
    FingerControlData little;
} HandControlDesc;

//sxl add.
enum class FingerID_Pro
{
    //left hand(0x0x)
    NONE_ID = 0xFF,
    ENTIRE_HAND = 0x00,
    LEFT_THUMB__REMOTE_NEAR = 0x01,
    LEFT_THUMB__ROTATE_BEND = 0x02,
    LEFT_INDEX__OPENCLOSE_BEND = 0x03,
    LEFT_MIDDLE__OPENCLOSE_BEND = 0x04,
    LEFT_RING__OPENCLOSE_BEND = 0x05,
    LEFT_PINKY__OPENCLOSE_BEND = 0x06,
    LEFT_REMOT__INDEX_MIDDLE = 0x07,
    LEFT_REMOT__RING_PINKY = 0x08,
    LEFT_WRIST_EC = 0x09,
    LEFT_OPISTHENAR_EC = 0x0a,

    //right hand(0x1x)
    RIGHT_THUMB__REMOTE_NEAR = 0x11,
    RIGHT_THUMB__ROTATE_BEND = 0x12,
    RIGHT_INDEX__OPENCLOSE_BEND = 0x13,
    RIGHT_MIDDLE__OPENCLOSE_BEND = 0x14,
    RIGHT_RING__OPENCLOSE_BEND = 0x15,
    RIGHT_PINKY__OPENCLOSE_BEND = 0x16,
    RIGHT_REMOT__INDEX_MIDDLE = 0x17,
    RIGHT_REMOT__RING_PINKY = 0x18,
    RIGHT_WRIST_EC = 0x19,
    RIGHT_OPISTHENAR_EC = 0x1a,
};

enum class ControlMode_Pro
{
    JOINT_MODE = 0x04,
    TORQUE_MODE = 0x06,
    IMPEDANCE_MODE = 0x07
};

enum MotorEnable_Pro
{
    ENABLE_MOTOR1 = 0x01,
    ENABLE_MOTOR2 = 0x02,
    ENABLE_ALL = 0x03
};

struct FramewareVer
{
    uint8_t patch;  // 6 bits for patch            (high)
    uint8_t minor;  // 5 bits for minor version
    uint8_t major;  // 5 bits for major version    (low)
};

typedef union ControlEnableMap_Pro_t
{
    uint16_t mask;

    struct
    {
        uint8_t PinkyRotate2 : 1;
        uint8_t PinkyRotate1 : 1;
        uint8_t RINGRotate2 : 1;
        uint8_t RINGRotate1 : 1;
        uint8_t THUMBRotate2__MiddleRotate2 : 1;
        uint8_t THUMBRotate1__MiddleRotate1 : 1;
        uint8_t ThumbMid__IndexRotate2 : 1;
        uint8_t ThumbDist__IndexRotate1 : 1;
        uint8_t Reserved : 4;
        uint8_t LttleRemo : 1;
        uint8_t RingDist : 1;
        uint8_t MiddleRemo : 1;
        uint8_t IndexDist : 1;
    } enables;
} ControlEnableMap_Pro;

typedef union ControlParams_Pro_t
{
    uint8_t mask;

    struct
    {
        uint8_t HandType : 1;  // 0 for left hand, 1 for right hand
        uint8_t Feedback : 1;  // 0 for disable feedback, 1 for enable
        uint8_t ClearErr : 1;  // 0 for do NOT clear error automatically, 1 for clear
        uint8_t HandCtrl : 1;  // 0 for thumb,1 for four fingers
        uint8_t Reserved : 4;
    } params;
} ControlParameter_Pro;

typedef struct HandControlDesc_Pro_t
{
    MotorControlMode  mode;
    ControlEnableMap_Pro  enableMap;
    ControlParameter_Pro  controlParam;
    FingerControlData thumbMid__indexDist;    //Thumb far middle end data/index finger near end data
    FingerControlData thumbDist__middleDis;   //Thumb proximal data/Middle finger proximal data
    FingerControlData ringDist;
    FingerControlData littleDist;
    FingerControlData indexRemo_middleRemo;
    FingerControlData ringRemo_littleRemo;
} HandControlDesc_Pro;
//end.

}
