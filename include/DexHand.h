//
// Created by ryzuo on 25-6-9.
//

#pragma once

#include <map>
#include <memory>
#include <functional>

#include "commondef.h"

namespace DexRobot
{

class DX21StatusRxData;
class ErrorMessageRx;
class SysParameterRWRx;

namespace Dex021
{

    enum class ProductType
    {
        DX021,
        DX021_S,
        DX021_PRO,
    };

    enum class AdapterType
    {
        ZLG_200U,
        ZLG_MINI,
        LYS_MINI,
        USB2_485,
    };

    enum class AdapterChannel
    {
        CHNX = -1,
        CHN0 = 0,
        CHN1,
        CHN2,
        CHN3,
    };

    struct DXInstruction
    {
        uint8_t channel;
        uint8_t deviceId;
        uint8_t fingerId;
        uint8_t subDeviceId;
        int16_t controlArg1;
        int16_t controlArg2;
        DexRobot::MotorControlMode mode;
    };

    typedef std::function<void (DexRobot::DX21StatusRxData *)> DH21StatusRxCallback;
    typedef std::function<void (DexRobot::ErrorMessageRx *)>   ErrorMessageCallBack;
    typedef std::function<void (DexRobot::SysParameterRWRx *)> ParamRwMessageCallBack;

    class DexHand
    {
    public:
        using PTR = std::shared_ptr<DexHand>;

    public:
        DexHand() = delete;
        DexHand(AdapterType, uint8_t adpaterIndex, bool listen=true);
        virtual ~DexHand() = default;

        DEXHAND_API static PTR createInstance(ProductType, AdapterType, uint8_t adpaterIndex, bool listen=true);

        virtual bool connect() = 0;
        virtual bool disconnect() = 0;

        virtual bool setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint16_t maxCurrent) = 0;
        virtual bool setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t maxPressure) = 0;
        virtual bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t maxTemperature) = 0;
        virtual bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) = 0;

        virtual bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
            int16_t controlArg1, int16_t controlArg2, DexRobot::MotorControlMode mode) = 0;

        DEXHAND_API [[nodiscard]] bool isAvailable() const;

        [[nodiscard]] virtual ProductType productType() const = 0;

        DEXHAND_API virtual void setHandId(AdapterChannel, uint8_t handId);
        DEXHAND_API [[nodiscard]] virtual uint8_t handID(AdapterChannel channel);
        DEXHAND_API [[nodiscard]] virtual AdapterChannel channelOfHand(uint8_t handId);

        DEXHAND_API void listen(const bool enable) { this->rtListen = enable; }
        DEXHAND_API [[nodiscard]] bool isListening() const { return this->rtListen; }

        DEXHAND_API [[nodiscard]] const std::string & connAdapterName() const { return this->adapterName; }

        DEXHAND_API [[nodiscard]] DexRobot::SysErrorCode errorCode() const { return errCode; }
        DEXHAND_API [[nodiscard]] const std::string & errorMessage() const { return this->errMsg; }

        virtual void setStatusRxCallback(const DH21StatusRxCallback &) const = 0;
        virtual void setErrorRxCallback(const ErrorMessageCallBack &) const = 0;
        virtual void setParamRWCallback(const ParamRwMessageCallBack &) const = 0;

    protected:
        AdapterType adapterType;
        uint8_t adpaterIndex;
        std::string adapterName;

        DexRobot::SysErrorCode errCode;
        std::string errMsg;

        void * hermes;

    private:
        bool rtListen;  // true for listening response in realtime mpde
        std::map<const AdapterChannel, uint8_t> handIds;
    };

    class DexHand_021 final : public DexHand
    {
    public:
        using PTR = std::shared_ptr<DexHand_021>;

    public:
        DexHand_021() = delete;
        DexHand_021(AdapterType adapterType, uint8_t adpaterIndex, bool listen=true);
        ~DexHand_021() override;

        bool connect() override;
        bool disconnect() override;

        bool setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint16_t maxCurrent) override;
        bool setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t maxPressure) override;
        bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t maxTemperature) override;
        bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) override;

        bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
            , int16_t controlArg1, int16_t controlArg2, DexRobot::MotorControlMode mode) override;

        bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition
            , int16_t controlArg1, int16_t controlArg2);

        DEXHAND_API void setStatusRxCallback(const DH21StatusRxCallback &) const override;
        DEXHAND_API void setErrorRxCallback(const ErrorMessageCallBack &) const override;
        DEXHAND_API void setParamRWCallback(const ParamRwMessageCallBack &) const override;

        [[nodiscard]] ProductType productType() const override;

    private:
    };

    class DexHand_021S final : public DexHand
    {
    public:
        using PTR = std::shared_ptr<DexHand_021S>;

    public:
        DexHand_021S() = delete;
        DexHand_021S(AdapterType adapter, uint8_t adpaterIndex, bool listen=true);
        ~DexHand_021S() override;

        bool connect() override;
        bool disconnect() override;

        DEXHAND_API void setStatusRxCallback(const DH21StatusRxCallback &) const override;
        DEXHAND_API void setErrorRxCallback(const ErrorMessageCallBack &) const override;
        DEXHAND_API void setParamRWCallback(const ParamRwMessageCallBack &) const override;

        bool setSafeCurrent(uint8_t deviceId, uint8_t fingerId, uint16_t maxCurrent) override;
        bool setSafePressure(uint8_t deviceId, uint8_t fingerId, uint8_t maxPressure) override;
        bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId, uint8_t maxTemperature) override;
        bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable) override;

        bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointId, int16_t controlArg1, int16_t controlArg2, DexRobot::MotorControlMode mode) override;
        bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointId, int16_t controlArg1, int16_t controlArg2);

        DEXHAND_API [[nodiscard]] ProductType productType() const override;

    private:
        void * hermes;
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

        DEXHAND_API [[nodiscard]] ProductType productType() const override;

    private:
        void * hermes;
    };
    */
}

}
