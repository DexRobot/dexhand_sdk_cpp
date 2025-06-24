//
// Created by ryzuo on 25-6-9.
//

#pragma once

#include <map>
#include <memory>
#include <functional>
#include "commondef.h"

#define DX21_DEFAULT_DEVICE_ID 1


namespace DexRobot
{

class ZLG_CANFD_MSG;
class LYS_CANFD_MSG;

template<typename TAdapterMsg>
class DX21TransitMsg;

struct ErrorMessageRx;
struct SysParameterRWRx;

namespace Dex021
{

class DexHand_021;
class DexHand_021S;

enum class ProductType : int
{
    UNSUPPORTED = -1,
    DX021,
    DX021_S,
    DX021_PRO,
};

enum class AdapterType : int
{
    ZLG_200U = 33,
    ZLG_MINI = 42,
    LYS_MINI = 43,
    USB2_485,
};

enum class AdapterChannel : int
{
    CHNX = -1,
    CHN0 = 0,
    CHN1,
    CHN2,
    CHN3,
};

struct DX21Instruction
{
    uint8_t channel;
    uint8_t deviceId;
    uint8_t fingerId;
    uint8_t subDeviceId;
    int16_t controlArg1;
    int16_t controlArg2;
    MotorControlMode mode;
};

class DX21StatusRxData
{
public:
    DX21StatusRxData() = delete;
    virtual ~DX21StatusRxData() = default;

    static DX21StatusRxData * load(ProductType pdType, uint8_t channelId, uint8_t deviceId, const unsigned char * payload);

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

typedef std::function<void (const Dex021::DX21StatusRxData *)> DH21StatusRxCallBack;
typedef std::function<void (const DexRobot::ErrorMessageRx *)> ErrorMessageCallBack;
typedef std::function<void (const DexRobot::SysParameterRWRx *)> ParamRwMessageCallBack;


template<typename TInstance>
class PredefinedGestures final
{
public:
    static_assert((std::is_same_v<Dex021::DexHand_021, TInstance> || std::is_same_v<Dex021::DexHand_021S, TInstance>),
        "Given template argument must be type of Dex021::DexHand_021 or Dex021::DexHand_021S");

    explicit PredefinedGestures(TInstance * instance);
    ~PredefinedGestures() = default;

public:
    DEXHAND_API void HandRelax(uint8_t deviceId);
    DEXHAND_API void Fist(uint8_t deviceId);
    DEXHAND_API void Victory(uint8_t deviceId);
    DEXHAND_API void ThumbUp(uint8_t deviceId);
    DEXHAND_API void Rock(uint8_t deviceId);
    DEXHAND_API void NoProblem(uint8_t deviceId);
    DEXHAND_API void One(uint8_t deviceId);
    DEXHAND_API void Two(uint8_t deviceId);
    DEXHAND_API void Three(uint8_t deviceId);
    DEXHAND_API void Four(uint8_t deviceId);
    DEXHAND_API void Five(uint8_t deviceId);
    DEXHAND_API void Six(uint8_t deviceId);
    DEXHAND_API void Seven(uint8_t deviceId);
    DEXHAND_API void Eight(uint8_t deviceId);
    DEXHAND_API void Nine(uint8_t deviceId);
    DEXHAND_API void Ten(uint8_t deviceId);

private:
    TInstance * dxInstance;
};


class DexHand
{
public:
    using PTR = std::shared_ptr<DexHand>;

public:
    DexHand() = delete;
    virtual ~DexHand() = default;

    /// Factory method to create an interactable instance of specific type of DexHand product, like DexHand_021,
    /// DexHand_021S, etc. The instance of DexHand product requires a specific type of communication adapter, the
    /// supported adapter types are ZLG USBCANFD-200U, USBCANFD-100U-mini, and LYS USBCANFD-Mini.
    /// @param productType The specific type of your DexHand product.
    /// @param adapterType The specific type of your adapter.
    /// @param adpaterIndex The index identified by system USB port of your adapter, start from 0.
    DEXHAND_API static PTR createInstance(ProductType productType, AdapterType adapterType, uint8_t adpaterIndex);

    DEXHAND_API [[nodiscard]] uint8_t numberOfDevices() const;

    DEXHAND_API [[nodiscard]] std::vector<AdapterChannel> channels() const;

    /// Establish connection between your program and the communication adapter your DexHand product is plugged in.
    /// @return true for success, false for failure
    /// @param listen A flag to indicate whether you want your program to listen to status data feedback of DexHand
    /// product in realtime mode or not. To process the response data of your DexHand product, you need to register
    /// callback functions through "set***Callback" methods. Refer to specifications of those methods for details.
    DEXHAND_API virtual bool connect(bool listen) = 0;

    /// Disconnect from the communication adapter.
    /// @return true for success, false for failure
    DEXHAND_API virtual bool disconnect() = 0;

    /// Get the version of firmware of specified hand or finger.
    /// @param deviceId Device ID, AKA hand ID.
    /// @param fingerId Finger ID. For DX021-S, this is ignored.
    /// @return The version number of firmware.
    DEXHAND_API virtual uint32_t getFirmwareVersion(uint8_t deviceId, uint8_t fingerId) = 0;

    /// Set the safe value of electric current of an indicated motor drive in your DexHand product. The DexHand firmware
    /// will drive the motor in particular patterns under different control modes to protect the motor and mechanic units
    /// like gears, strings etc. DexHand product user manual explains the details of different control modes, and how safe
    /// current plays a role in it.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param jointPosition Position ID of a specific joint, if this joint has an independent motor drive. This ID
    /// must be evaluated by enum class JointMotor.
    /// @param maxCurrent Given value of expected safe current.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        uint16_t maxCurrent) = 0;

    /// Set the safe value of pressure(maximum allowed) of an indicated position of a finger, which is reflected by one
    /// or a group of tactile sensors on the finger. The safe pressure value works as threshold to guarantee the motor
    /// drive to STOP during movements, when tactile sensor detects the reflected pressure reaches the safe value.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param jointPosition Position ID of a specific joint, if this joint has an independent tactile sensor. This ID
    /// must be evaluated by enum class JointMotor.
    /// @param maxPressure Given value of expected safe pressure.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        uint8_t maxPressure) = 0;

    /// Set the safe value of working temperature(maximum allowed) of an indicated finger or motor. Continuous long time
    /// tasks usually cause the motor drives and control boeard keep heating and temperature increasing continuously as
    /// well. To prected the DexHand product, we need to limit the workloads of continuous tasks. Once the temperature
    /// reaches the safe value, firmware stops executing any control instruction to protect the motors, until temperature
    /// decreases under the safe value.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param jointPosition Position ID of a specific joint, if this joint has an independent temperature sensor. This ID
    /// must be evaluated by enum class JointMotor. For DexHand021 and 021S, this argument is reserved.
    /// @param maxTemperature Given value of expected safe temperature.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        uint8_t maxTemperature) = 0;

    /// Set realtime status data sampling ON or OFF for a specified finger of DexHand product.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) = 0;

    /// Set realtime status data sampling ON or OFF for all fingers of DexHand device of specified ID.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setRealtimeResponse(uint8_t deviceId, uint16_t sampleRate, bool enable) = 0;

    /// Set realtime status data sampling ON or OFF for all fingers of all DexHand devices on this adapter.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API virtual bool setRealtimeResponse(uint16_t sampleRate, bool enable) = 0;

    /// Send an action control instruction to specified finger and joint of a specified hand, with given values of
    /// movement, and given control mode.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param jointPosition Position ID of a specific joint, if this joint is a driving joint.
    /// @param controlArg1 Target value 1 of initiated action. In different DexHand products, and under different
    /// control mode, it represents differenet semantic meanings. For DexHand021, it represents the target action
    /// value of distal joint of finger. However for 021-S, it represents the target value of MCP of finger, cause
    /// 021-S has only one driving joint of each finger, and it is MCP. And for CASCADED_PID_CONTROL_MODE, it's
    /// the value of target degree*100, but for HALL_POSITION_CONTROL_MODE, it's target value of hall position to
    /// drive the motor.
    /// @param controlArg2 Target value 2 of initiated action. For DexHand021, it represents target action value
    /// of proximal joint(actually MCP). For DexHand_021S, it is the driving velocity of motor, representing the
    /// value of degrees*100 per second. Refer to comments of controlArg1 for other information.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @param delay Number of milliseconds for delaying execution of this instruction, if user does not want it to
    /// be executed immediately.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API virtual bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        int16_t controlArg1, int16_t controlArg2, MotorControlMode mode, int32_t delay) = 0;

    /// Send an action control instruction to specified finger and joint of a specified hand, with given values of
    /// movement, and given control mode. The instruction will be executed immediately once the device accepts it.
    /// @param deviceId The ID number of your DexHand product, AKA hand ID.
    /// @param fingerId The ID number of the indicated finger on your DexHand product.
    /// @param jointPosition Position ID of a specific joint, if this joint is a driving joint.
    /// @param controlArg1 Target value 1 of initiated action. In different DexHand products, and under different
    /// control mode, it represents differenet semantic meanings. For DexHand021, it represents the target action
    /// value of distal joint of finger. However for 021-S, it represents the target value of MCP of finger, cause
    /// 021-S has only one driving joint of each finger, and it is MCP. And for CASCADED_PID_CONTROL_MODE, it's
    /// the value of target degree*100, but for HALL_POSITION_CONTROL_MODE, it's target value of hall position to
    /// drive the motor.
    /// @param controlArg2 Target value 2 of initiated action. For DexHand021, it represents target action value
    /// of proximal joint(actually MCP). For DexHand_021S, it is the driving velocity of motor, representing the
    /// value of degrees*100 per second. Refer to comments of controlArg1 for other information.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API virtual bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        int16_t controlArg1, int16_t controlArg2, MotorControlMode mode) = 0;

    /// Whether this hand instance is available or not. Normally, if the connection between the communication adapter
    /// and your program is workable, then the DexHand device instance is considered as available.
    /// @return true for available, false for not.
    DEXHAND_API [[nodiscard]] bool isAvailable() const;

    /// Get the prooduct type of this hand instance.
    /// @return An enum value represents product type, refer to definitions of ProductType for possible values.
    DEXHAND_API [[nodiscard]] virtual ProductType productType() const = 0;

    /// Set ID for the hand is connecting to the given channel of adapater.
    /// @param channel Target channel of the adapter.
    /// @param handId Given ID will be assigned to the connecting hand
    DEXHAND_API virtual void setHandId(AdapterChannel channel, uint8_t handId);

    /// Get ID of the hand is connecting to the given channel of adapter
    /// @param channel Given channel of the adapter.
    /// @return ID of hand of the corresponding channel.
    DEXHAND_API [[nodiscard]] virtual uint8_t handID(AdapterChannel channel);

    /// Get corresponding channel number on adapter of given hand ID
    /// @param handId ID of the given hand
    /// @return Channel number of the given hand
    DEXHAND_API [[nodiscard]] virtual AdapterChannel channelOfHand(uint8_t handId);

    /// Send instruction to retrieve all Sys IDs of connected DexHand device from specified channel.
    /// For DX021, Sys ID is equivalent of finger ID, thus each device shall response 6 feedbacks for
    /// each finger. For DX021-S, Sys ID is equivalent of hand ID, only 1 feedback will be responsed.
    /// NOTICE: This is a blocking function, which won't return untill it gets all feedbacks from the
    /// DexHand device, nor it reaches default timeout of 30 milliseconds no matter it gets all fingers'
    /// ID or not.
    /// @param channel Target channel of the adapter.
    /// @param sysIdList Output. An reference of vector to carry the responsed Sys IDs from connected
    /// DexHand device. Size of this argument will be 6 for DX021, and only 1 for DX021-S
    DEXHAND_API virtual void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList) = 0;

    /// Set whether listen to the status data feedback of DexHand product in realtime mode or not
    /// @param enable true for listen, false for stop listening.
    DEXHAND_API void listen(bool enable) const;

    /// Determine whether the instance is listening on the realtime feedback of the hands
    /// @return true for listening is on, flase for not.
    DEXHAND_API [[nodiscard]] bool isListening() const;

    /// Get the name of communication adapter, normally the instance names the adapter after its serial number
    /// assigned by provider of the adapter.
    /// @return Name of the adapter in text
    DEXHAND_API [[nodiscard]] const std::string &connAdapterName() const { return this->adapterName; }

    /// Get code of current error of this instance, if there is any. Normally if anything went wrong when creating
    /// or starting the DexHand instance or adapter, error code will be set. This if for diagnostic purpose.
    /// @return The code of current error.
    DEXHAND_API [[nodiscard]] SysErrorCode errorCode() const { return errCode; }

    /// Get detail message of current error.
    /// @return Detail information in text of current error.
    DEXHAND_API [[nodiscard]] const std::string &errorMessage() const { return this->errMsg; }

    /// Register a callback function for processing the status feedback data of DexHand. Status data contains several
    /// fields representing various status of parts of a DexHand, these fields are defined in, and can be retrieved
    /// from class DX21StatusRxData object, normally includes the electic current value of each motor drives, degrees
    /// of each joints, pressure value reflected by tactile sensor, etc. Once you enabled listening on the realtime
    /// feedback, you need this callback function to process these data.
    /// @param callback Callback function to be registered, which is declared as form of DH21StatusRxCallback, AKA
    /// std::function<void (DexRobot::DX21StatusRxData *)>. In your callback function, you don't need to free the
    /// pointer of DX21StatusRxData object.
    DEXHAND_API virtual void setStatusRxCallback(const DH21StatusRxCallBack &callback) const = 0;

    /// Set a callback function for processing the error messages responsed by DexHand.
    /// @param callback Error processing callback function to be registered, which must be declared as form of
    /// ErrorMessageCallBack, AKA std::function<void (DexRobot::ErrorMessageRx *)>. You don't need to free the space
    /// of ErrorMessageRx object in your callback function.
    DEXHAND_API virtual void setErrorRxCallback(const ErrorMessageCallBack &callback) const = 0;

    /// Set a callback function for processing the result of Read/Write specific parameter from/to firmware of DexHand.
    /// @param callback Callback function to be registered for processing firmware parameter R/W results, which must be
    /// declared as form of ParamRwMessageCallBack, AKA std::function<void (DexRobot::SysParameterRWRx *)>. You don't
    /// need to free the space of SysParameterRWRx object in your callback function. For detail of all kinds of firmware
    /// parameters, please refer to the user manual and DexHand protocols specification.
    DEXHAND_API virtual void setParamRWCallback(const ParamRwMessageCallBack &callback) const = 0;

    /// Clear firmware errors on specified finger of device connected on specified adapter, for example motor blocking
    /// error causes protection. For elaboration of possible errors, please refer to our product user manual.
    /// NOTICE: this is a non-blocking call, meaning it returns directly after it sends instruction to device firmware,
    /// and won't wait for device firmware to finished its process. You can try to call this again if your device does
    /// not execute any of your subsequent control instructions after you already called this function.
    /// @param channel Target channel of the adapter, on wich the DexHand device is connected.
    /// @param fingerId ID number of target finger, which encounteres any error you want to clear.
    DEXHAND_API virtual void clearFirmwareError(AdapterChannel channel, uint8_t fingerId) = 0;

    /// Reboot the specified device connected on specified channel.
    /// @param channel Target channel of the adapter, on wich the DexHand device is connected.
    /// @param deviceId Target ID number of device you want to reboot. For DX021, this paramter represents finger ID. For
    /// DX021-S it represents the hand ID.
    DEXHAND_API virtual void rebootDevice(AdapterChannel channel, uint8_t deviceId) = 0;

    /// Reset all finger joints back to initial state position.
    /// @param channel Target channel of the adapter, on wich the DexHand device is connected.
    DEXHAND_API virtual void resetJoints(AdapterChannel channel) = 0;

protected:
    DexHand(AdapterType, uint8_t adpaterIndex);
    [[nodiscard]] std::map<const AdapterChannel, uint8_t> & devices();
    [[nodiscard]] const std::map<const AdapterChannel, uint8_t> & devices() const;

public:
    friend class DexHand_021;
    friend class DexHand_021S;
    friend class PredefinedGestures<DexHand_021>;
    friend class PredefinedGestures<DexHand_021S>;

protected:
    AdapterType adapterType;
    uint8_t adpaterIndex;
    std::string adapterName;

    SysErrorCode errCode;
    std::string errMsg;

    void *hermes;

private:
    std::map<const AdapterChannel, uint8_t> handIds;
};

class DexHand_021 : public DexHand
{
public:
    using PTR = std::shared_ptr<DexHand_021>;

public:
    DexHand_021() = delete;

    /// Constructer of this class to create an instance of DexHand021 with a specific type of adapter, the supported
    /// adapter type includes ZLG USBCANFD-200U, USBCANFD-100U-mini, LYS USBCANFD-Mini.
    /// @param adapterType The specific type of your adapter.
    /// @param adpaterIndex The index identified by system USB port of your adapter, start from 0.
    DEXHAND_API DexHand_021(AdapterType adapterType, uint8_t adpaterIndex);

    ~DexHand_021() override;

    /// Establish connection between your program and the communication adapter with DexHand021 device connected
    /// @return true for success, false for failure
    /// @param listen A flag to indicate whether you want your program to listen to status data feedback of DexHand
    /// product in realtime mode or not. To process the response data of your DexHand product, you need to register
    /// callback functions through "set***Callback" methods. Refer to specifications of those methods for details.
    DEXHAND_API bool connect(bool listen) override;

    /// Disconnect from the communication adapter, while the DexHand021 device will be disconnected as well
    /// @return true for success, false for failure
    DEXHAND_API bool disconnect() override;

    /// Send instruction to retrieve all finger IDs of connected DX021 device from specified channel. Function will
    /// NOT return untill it gets all fingers' ID, nor it reaches default 30 milliseconds timeout, not matter it gets
    /// all fingers' ID or not.
    /// @param channel Target channel of the adapter.
    /// @param sysIdList Output. An reference of vector to carry the responsed finger IDs from connected DX021 device.
    DEXHAND_API void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList) override;

    /// Send instruction to retrieve all finger IDs of connected DX021 device from specified channel.
    /// @param channel Target channel of the adapter.
    /// @param sysIdList Output. An reference of vector to carry the responsed finger IDs from connected DX021 device.
    /// @param timeout number of milliseconds of timeout, this function will return anyway, even it does NOT receive
    /// all feedbacks of ID from each finger.
    DEXHAND_API void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList, uint32_t timeout=30);

    /// Get the version of firmware of specified finger of specified DX021 hand device.
    /// @param deviceId Device ID, AKA hand ID.
    /// @param fingerId Finger ID.
    /// @return The version number of firmware.
    DEXHAND_API uint32_t getFirmwareVersion(uint8_t deviceId, uint8_t fingerId) override;

    /// Clear firmware errors on specified finger on DX021 device of specified CANFD adapter channel. For elaboration
    /// of possible errors, please refer to our product user manual.
    /// NOTICE: this is a non-blocking call, meaning it returns directly after it sends instruction to device firmware,
    /// and won't wait for device firmware to finished its process. You can try to call this again if your device does
    /// not execute any of your subsequent control instructions after you already called this function.
    /// @param channel Channel number on which the connected DexHand device you want to clear the errors.
    /// @param fingerId ID number of target finger, which encounteres any error you want to clear.
    DEXHAND_API void clearFirmwareError(AdapterChannel channel, uint8_t fingerId) override;

    /// Reboot the specified device connected on specified channel.
    /// @param channel Target channel of the adapter, on wich the DexHand device is connected.
    /// @param deviceId Target ID number of device you want to reboot. For DX021, this paramter represents finger ID.
    DEXHAND_API void rebootDevice(AdapterChannel channel, uint8_t deviceId) override;

    /// Reset all finger joints back to initial state position.
    /// @param channel Target channel of the adapter, on wich the DX021 device is connected.
    DEXHAND_API void resetJoints(AdapterChannel channel) override;

    /// Register a callback function for processing the status feedback data of your DexHand021 device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setStatusRxCallback(const DH21StatusRxCallBack &callback) const override;

    /// Register a callback function for processing the error messages responsed by your DexHand021 device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setErrorRxCallback(const ErrorMessageCallBack &callback) const override;

    /// Register a callback function for processing the results of Read/Write firmware parameter from/to your
    /// DexHand021 device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setParamRWCallback(const ParamRwMessageCallBack &callback) const override;

    /// Set the safe value of working electric current for specified motor drive in your DexHand021 device. Refer to
    /// user manual for how "safe current" works in different control modes and workloads, to protect your DexHand021
    /// device.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger of your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint, Acceptable values are evaluated by enum class JointMotor,
    /// 0x01 for distal joint, 0x02 for proximal joint, and 0x03 for both.
    /// @param maxCurrent Given value of expected safe electric current.
    /// @return true for success, false for failure.
    DEXHAND_API bool
    setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition, uint16_t maxCurrent) override;

    /// Set the safe value of pressure(maximum allowed) for specified finger or motor of your DexHand021 device. Corresponding
    /// motor drive immediately brakes once the tactile sensor detects the pressure on finger reaches safe value.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint. A DexHand021 device possesses only one single tactile sensor on
    ///  fingertip of each finger, thus this argument is ignored.
    /// @param maxPressure Given value of expected safe pressure.
    /// @return true for success, false for failure.
    DEXHAND_API bool
    setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition, uint8_t maxPressure) override;

    /// Set the safe value of working temperature(maximum allowed) for specified finger or motor of your DexHand021 device.
    /// Firmware stops accepting any control instruction once the temperature of motor or curcuit board reaches the safe value.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint. A DexHand021 device does not individually sampling temperature for
    /// each motors of a finger(every finger possesses 2 motors for MCP and PIP/DIP), thus this argument is ignored.
    /// @param maxTemperature Given value of expected safe temperature.
    /// @return true for success, false for failure.
    DEXHAND_API bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        uint8_t maxTemperature) override;

    /// Set realtime sampling for status data ON or OFF for a specified finger of DexHand021 device.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) override;

    /// Set realtime status data sampling ON or OFF for all fingers of DexHand device of specified ID.
    /// @param deviceId The ID number of your DX021 device, AKA hand ID.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint8_t deviceId, uint16_t sampleRate, bool enable) override;

    /// Set realtime status data sampling ON or OFF for all fingers of all DexHand devices on this adapter.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint16_t sampleRate, bool enable) override;

    /// Send an action control instruction to specified finger and joint of a specified hand, with given values of
    /// expected motion, via given control mode.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint, acceptable values are evaluated by enum class JointMotor.
    /// e.g. 0x01 for distal joint, 0x02 for proximal joint, and 0x03 for both.
    /// @param distValue Value of motion for distal joint to move. For CASCADED_PID_CONTROL_MODE mode, it's the value of
    /// target degree*100. For HALL_POSITION_CONTROL_MODE, it's incremental value of hall position to drive the motor,
    /// based on its current position.
    /// @param proxValue Value of motion for proximal joint to move.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @param delay Number of milliseconds for delaying execution of this instruction, if user does not want it to
    /// be executed immediately.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        int16_t distValue, int16_t proxValue, MotorControlMode mode, int32_t delay) override;

    /// Send an action control instruction to specified finger and joint of a specified hand, with given values of
    /// expected motion, via given control mode.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint, acceptable values are evaluated by enum class JointMotor.
    /// e.g. 0x01 for distal joint, 0x02 for proximal joint, and 0x03 for both.
    /// @param distValue Value of motion for distal joint to move. For CASCADED_PID_CONTROL_MODE mode, it's the value of
    /// target degree*100. For HALL_POSITION_CONTROL_MODE, it's incremental value of hall position to drive the motor,
    /// based on its current position.
    /// @param proxValue Value of motion for proximal joint to move.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
      , int16_t distValue, int16_t proxValue, DexRobot::MotorControlMode mode) override;

    /// Send an action control instruction to specified finger and joint of a specified hand, with given values of
    /// expected motion, via deteremined CASCADED_PID_CONTROL_MODE.
    /// @param deviceId User assigned ID number of your DexHand021 product, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your DexHand021 device.
    /// @param jointPosition Position ID of a specific joint, acceptable values are evaluated by enum class JointMotor.
    /// e.g. 0x01 for distal joint, 0x02 for proximal joint, and 0x03 for both.
    /// @param distValue Value of motion for distal joint to move.
    /// @param proxValue Value of motion for proximal joint to move.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
      , int16_t distValue, int16_t proxValue);

    /// Get the prooduct type of this hand instance.
    /// @return Always ProductType::DX021
    DEXHAND_API [[nodiscard]] ProductType productType() const override;

private:
};


class DexHand_021S : public DexHand
{
public:
    using PTR = std::shared_ptr<DexHand_021S>;

public:
    DexHand_021S() = delete;

    /// Constructer of this class to create an instance of DexHand_021S with a specific type of adapter, the supported
    /// adapter type includes ZLG USBCANFD-200U, USBCANFD-100U-mini, LYS USBCANFD-Mini.
    /// @param adapterType The specific type of your adapter.
    /// @param adpaterIndex The index identified by system USB port of your adapter, start from 0.
    /// @param listen A flag to indicate whether you want your program to listen to response data of DexHand_021S in
    /// realtime mode or not.
    DEXHAND_API DexHand_021S(AdapterType adapterType, uint8_t adpaterIndex);

    ~DexHand_021S() override;

    /// Establish connection between your program and the communication adapter with DexHand_021S device connected
    /// @return true for success, false for failure
    /// @param listen A flag to indicate whether you want your program to listen to status data feedback of DexHand
    /// product in realtime mode or not. To process the response data of your DexHand product, you need to register
    /// callback functions through "set***Callback" methods. Refer to specifications of those methods for details.
    DEXHAND_API bool connect(bool listen) override;

    /// Disconnect from the communication adapter, while the DexHand_021S device will be disconnected as well
    /// @return true for success, false for failure
    DEXHAND_API bool disconnect() override;

    /// A blocking method to assign given ID number to the hand is connected to the specified channel of adapater.
    ///
    /// NOTICE: Differ from DexHand021, the given hand ID will be written to and memorized by the firmware of 021S
    /// permanently, until you call this method to update it again next time. Thus, if you certainly want to keep
    /// the ID numbers assigned for each device, you don't need to call this method everytime you create instance,
    /// and you can read the Hand ID by calling handID() method if you wish, or you can memorize and manage the ID
    /// numbers for each device in your own code of your application.
    /// @param channel Target channel of the adapter.
    /// @param handId Given ID will be assigned to the connected hand.
    /// @return This function won't return until it successfully writes the given ID to firmware of device, or the
    /// write process timed out. Please call errorCode() after this function returns, to make sure the given device
    /// ID is successfully written.
    DEXHAND_API void setHandId(AdapterChannel channel, uint8_t handId) override;

    /// A blocking method to get the assigned ID number of the hand is connected to specified channel of adapter.
    /// The method won't return until the device firmware sends feedback of its own ID.
    /// @param channel Given channel of the adapter, with a DexHand_021S device connected.
    /// @return ID of hand of the corresponding channel.
    DEXHAND_API [[nodiscard]] uint8_t handID(AdapterChannel channel) override;

    /// Send instruction to retrieve SysID, AKA hand ID of connected DX021-S device from specified channel. This is
    /// an equivalent call of handID(channel).
    /// @param channel Target channel of the adapter.
    /// @param sysIdList Output. An reference of vector to carry the hand ID from connected DX021-S device.
    DEXHAND_API void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList) override;

    /// Send instruction to retrieve SysID, AKA hand ID of connected DX021-S device from specified channel. This is
    /// an equivalent call of handID(channel), excepting users can specify the timeout of their own.
    /// @param channel Target channel of the adapter.
    /// @param sysIdList Output. An reference of vector to carry the hand ID from connected DX021-S device.
    /// @param timeout number of milliseconds of timeout, this function willl return anyway, even it does NOT receive
    /// the hand ID from connected device.
    DEXHAND_API void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList, uint32_t timeout=30);

    /// Get the version of firmware of specified DX021-S hand device.
    /// @param deviceId Device ID, AKA hand ID.
    /// @param fingerId Finger ID. This is ignored for DX021-S poceesses only one control board shared by all fingers.
    /// @return The version number of firmware.
    DEXHAND_API uint32_t getFirmwareVersion(uint8_t deviceId, uint8_t fingerId) override;

    /// Clear firmware errors on specified DX021-S finger of specified CANFD adapter channel. For elaboration
    /// of possible errors, please refer to our product user manual.
    /// NOTICE: this is a non-blocking call, meaning it returns directly after it sends instruction to device firmware,
    /// and won't wait for device firmware to finished its process. You can try to call this again if your device does
    /// not execute any of your subsequent control instructions after you already called this function.
    /// @param channel Channel number on which the connected DexHand device you want to clear the errors.
    /// @param deviceId ID number of target device, which encounteres any error you want to clear.
    DEXHAND_API void clearFirmwareError(AdapterChannel channel, uint8_t deviceId) override;

    /// Reboot the specified device connected on specified channel.
    /// @param channel Target channel of the adapter, on wich the DexHand device is connected.
    /// @param deviceId Target ID number of device you want to reboot. For DX021-S this paramter represents the hand ID.
    /// However, DX021-S pocesses only one single control board, this parameter is ignored here, and the device connected
    /// on specified adapater channel will be reboot anyway, no matter what the hand ID number is.
    DEXHAND_API void rebootDevice(AdapterChannel channel, uint8_t deviceId) override;

    /// Reset all finger joints back to initial state position.
    /// @param channel Target channel of the adapter, on wich the DX021-S device is connected.
    DEXHAND_API void resetJoints(AdapterChannel channel) override;

    /// Register a callback function for processing the status feedback data of your DexHand_021S device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setStatusRxCallback(const DH21StatusRxCallBack &callback) const override;

    /// Register a callback function for processing the error messages responsed by your DexHand_021S device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setErrorRxCallback(const ErrorMessageCallBack &callback) const override;

    /// Register a callback function for processing the results of Read/Write firmware parameter from/to your
    /// DexHand_021S device.
    /// @param callback Callback function to be registered.
    DEXHAND_API void setParamRWCallback(const ParamRwMessageCallBack &callback) const override;

    /// Set the safe value of working electric current for specified motor drive in your DexHand_021S device. Refer to
    /// user manual for how "safe current" works in different control modes and workloads, to protect your DexHand_021S
    /// device.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger of your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one motor drive for each finger.
    /// @param maxCurrent Given value of expected safe electric current.
    /// @return true for success, false for failure.
    DEXHAND_API bool
    setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition, uint16_t maxCurrent) override;

    /// Set the safe value of pressure(maximum allowed) for specified finger or motor of your DexHand021 device. Corresponding
    /// motor drive immediately brakes once the tactile sensor detects the pressure on finger reaches safe value.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one tactile sensor on fingertip for each finger.
    /// @param maxPressure Given value of expected safe pressure.
    /// @return true for success, false for failure.
    DEXHAND_API bool
    setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition, uint8_t maxPressure) override;

    /// Set the safe value of working temperature(maximum allowed) for specified finger or motor of your DexHand_021S device.
    /// Firmware stops accepting any control instruction once the temperature of motor or curcuit board reaches the safe value.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one motor drive for each finger.
    /// @param maxTemperature Given value of expected safe temperature.
    /// @return true for success, false for failure.
    DEXHAND_API bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
        uint8_t maxTemperature) override;

    /// Set realtime sampling for status data ON or OFF for a specified finger of DexHand021 device.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) override;

    /// Set realtime status data sampling ON or OFF for all fingers of DexHand device of specified ID.
    /// @param deviceId The ID number of your DX021-S device, AKA hand ID.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint8_t deviceId, uint16_t sampleRate, bool enable) override;

    /// Set realtime status data sampling ON or OFF for all fingers of all DexHand devices on this adapter.
    /// @param sampleRate Rate(in Hz) of sampling on finger's status data. 1000 in minimum and 3 in maximum
    /// @param enable Switch to represent realtime sampling ON or OFF.
    /// @return true for success, false for failure.
    DEXHAND_API bool setRealtimeResponse(uint16_t sampleRate, bool enable) override;

    /// Send an action control instruction to specified finger and joint of a specified DexHand_021S device, with given
    /// values of expected motion, via given control mode.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one motor drive for each finger.
    /// @param motionValue Motion value for finger to move. Accepted value is target degree*100.
    /// @param velocity Velocity for the steering motor to move. Accepted value is degree*100 per second.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @param delay Number of milliseconds for delaying execution of this instruction, if user does not want it to
    /// be executed immediately.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
      , int16_t motionValue, int16_t velocity, MotorControlMode mode, int32_t delay) override;

    /// Send an action control instruction to specified finger and joint of a specified DexHand_021S device, with given
    /// values of expected motion, via given control mode.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one motor drive for each finger.
    /// @param motionValue Motion value for finger to move. Accepted value is target degree*100.
    /// @param velocity Velocity for the steering motor to move. Accepted value is degree*100 per second.
    /// @param mode Control mode of how the motor is driven. Acceptable values are evaluated by enum MotorControlMode.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
      , int16_t motionValue, int16_t velocity, MotorControlMode mode) override;

    /// Send an action control instruction to specified finger and joint of a specified DexHand_021S device, with given
    /// values of expected motion, via deteremined HALL_POSITION_CONTROL_MODE.
    /// @param deviceId User assigned ID number of your 021S device, AKA hand ID assigned by method setHandId().
    /// @param fingerId The ID number of the specified finger on your 021S device.
    /// @param jointPosition Ignored, 021S device possesses only one motor drive for each finger.
    /// @param motionValue Motion value for finger to move. Accepted value is target degree*100.
    /// @param velocity Velocity for the steering motor to move. Accepted value is degree*100 per second.
    /// @return true if the instruction is sent successfully, false if it is not.
    DEXHAND_API bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
      , int16_t motionValue, int16_t velocity);

    /// Get the prooduct type of this hand instance.
    /// @return Always ProductType::DX021_S
    DEXHAND_API [[nodiscard]] ProductType productType() const override;
};

/*
class DexHand_021Pro final : public DexHand
{
public:
    using PTR = std::shared_ptr<DexHand_021Pro>;

public:
    DexHand_021Pro() = delete;
    DexHand_021Pro(AdapterType adapter, uint8_t adpaterIndex, bool listen=true);
    ~DexHand_021Pro() override;

    bool connect() override;
    bool disconnect() override;

    DEXHAND_API void setStatusRxCallback(const DH21StatusRxCallback &) const override;
    DEXHAND_API void setErrorRxCallback(const ErrorMessageCallBack &) const override;
    DEXHAND_API void setParamRWCallback(const ParamRwMessageCallBack &) const override;

    /// Get the prooduct type of this hand instance.
    /// @return Always ProductType::DX021_PRO
    DEXHAND_API [[nodiscard]] ProductType productType() const override;

private:
    void * hermes;
};
*/

}
}
