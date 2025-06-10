#pragma once

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

enum class SysErrorCode
{
    NONE = 0,
    ADAPTER_NA,
    CONN_LOST,
};

enum MotorControlMode
{
    ZERO_TORQUE_MODE = 0x00,
    CURRENT_CONTROL_MODE = 0x11,
    SPEED_CONTROL_MODE = 0x22,
    HALL_POSITION_CONTROL_MODE = 0x33,
    CASCADED_PID_CONTROL_MODE = 0x44,
    HALL_POSLIMIT_CONTROL_MODE = 0x55,
    CASCADED_MIT_CONTROL_MODE = 0x66,
    CASCADED_IMP_CONTROL_MODE = 0x77,
};

enum ErrorType
{
    None_Error     = 0x00,
    MotorsError    = 0xEE,
    MotorLimitHall = 0xEF,
    MotorOverHeat  = 0xFF,
};

enum MotorErrorCode
{
    NONE_ERROR   = 0x0000,
    CRT_OVERLOAD = 0x0001,
    HALL_ERROR   = 0x0002,
    ROLL_BLOCKED = 0x0004,
    ADC_ERROR    = 0x0008,
};

enum HandID
{
    None = -1,
    LEFT = 0,
    RIGHT
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

enum JointID
{
    UNKNOWN = -1,
    ThumbProximal = 0,
    ThumbDistal,
    PalmRoll,
    PalmSplit,
    IndexProximal,
    IndexDistal,
    MiddleProximal,
    MiddleDistal,
    RingProximal,
    RingDistal,
    PinkyProximal,
    PinkyDistal,
};

class DX21StatusRxData
{
public:
    DX21StatusRxData() = delete;
    virtual ~DX21StatusRxData() = default;

    DEXHAND_API virtual void update(const unsigned char *) = 0;
    [[nodiscard]] DEXHAND_API virtual const char * data() const = 0;
    [[nodiscard]] DEXHAND_API virtual int16_t hallValue(uint8_t motorId) const = 0;

    [[nodiscard]] DEXHAND_API virtual int16_t MotorCurrent(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int16_t MotorVelocity(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int16_t MotorHallValue(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int16_t MotorTemperature(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int32_t ApprochingValue(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual   float JointDegree(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual   float NormalForce(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual   float TangentForce(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int32_t NormalForceDelta(uint8_t subDeviceId) const = 0;
    [[nodiscard]] DEXHAND_API virtual int32_t TangentForceDelta(uint8_t subDeviceId) const = 0;

protected:
    DX21StatusRxData(uint8_t channel, uint8_t deviceId, int64_t timestamp);

public:
    uint8_t deviceId;
    uint8_t channelId;
    int64_t timestamp;
};

}
