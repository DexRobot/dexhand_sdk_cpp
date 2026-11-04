#include <iostream>
#include <chrono>
#include <cstdlib>
#include "DexHand.h"
#include "StringUtils.h"
#include "commondef.h"

using namespace DexRobot;
using namespace DexRobot::Dex021;

static bool parseBaudRate(const char* s, BaudRate& baud)
{
    char* end = nullptr;
    const unsigned long n = std::strtoul(s, &end, 10);
    if (end == s || *end != '\0') return false;

    switch (n)
    {
    case 115200:  baud = BaudRate::BAUD_115200;  return true;
    case 230400:  baud = BaudRate::BAUD_230400;  return true;
    case 460800:  baud = BaudRate::BAUD_460800;  return true;
    case 921600:  baud = BaudRate::BAUD_921600;  return true;
    case 1000000: baud = BaudRate::BAUD_1000000; return true;
    case 2000000: baud = BaudRate::BAUD_2000000; return true;
    default:      return false;
    }
}

void DX021Lite_RTU485_IFTests(const char* serialPort, BaudRate baud)
{
    const auto hand = std::dynamic_pointer_cast<DexHand_021Lite>(
        DexHand::DexHand::createInstance(ProductType::DX021_LITE, serialPort, baud));

    if (hand == nullptr || !hand->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    std::vector<uint8_t> sysIds;
    hand->getSysIds(AdapterChannel::CHNX, sysIds);

    uint8_t deviceId = sysIds.empty() ? 0x01 : sysIds.front();

    if (hand->isAlive(AdapterChannel::CHNX, deviceId))
        std::cout << "Device " << std::to_string(deviceId) << " is alive." << std::endl;
    else
        std::cout << "Device " << std::to_string(deviceId) << " not responding." << std::endl;

    for (int i = 0; i <= 5; i++)
    {
        const auto v = hand->getFirmwareVersion(deviceId, i);
        std::cout << "the fingers " << i << " 's firmwareVersion = 0x" << std::hex << std::uppercase << v << std::dec << std::endl;
    }

    const auto version = hand->getFirmwareVersion(deviceId, LITE_MASTER_VERSION);
    std::cout << "master firmwareVersion = 0x" << std::hex << std::uppercase << version << std::dec << std::endl;

    const auto handType = hand->getHandType(deviceId);
    std::cout << "handType        = " << std::hex << std::uppercase
        << static_cast<int>(handType) << std::dec
        << (handType == 0x00 ? " LEFT" :
            handType == 0x01 ? " RIGHT" :
            handType == 0x06 ? " not set" : " read failed")
        << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "Serial Number is: " << hand->getSerialNumber(deviceId) << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    hand->setStatusRxCallback([](const DX21StatusRxData* status) {

        for (uint8_t fingerId = 0; fingerId < 6; ++fingerId)
        {
            std::cout << "fingerId= " << static_cast<int>(fingerId)
                << "  iq=" << status->MotorCurrent(fingerId)
                << "  rpm=" << status->MotorVelocity(fingerId)
                << "  hall=" << status->MotorHallValue(fingerId)
                << "  err=0x" << std::hex << status->MotorErrorStatus(fingerId)
                << std::dec << std::endl;
        }
        });

    hand->setRealtimeResponse(deviceId, 10, true);

    hand->enable(deviceId, true);

    //Full-Frame Control
    LiteHandControlDesc desc1{};
    desc1.mode = HALL_POSLIMIT_CONTROL_MODE;
    for (uint8_t i = 0; i < 4; ++i)
    {
        desc1.joints[i].position = 1000;
        desc1.joints[i].speed = 10000;
        desc1.joints[i].current = 100;
    }
    desc1.joints[4].position = 0;
    desc1.joints[4].speed = 10000;
    desc1.joints[4].current = 100;

    desc1.joints[5].position = 0;
    desc1.joints[5].speed = 10000;
    desc1.joints[5].current = 100;


    LiteHandControlDesc desc2{};
    desc2.mode = HALL_POSLIMIT_CONTROL_MODE;
    for (uint8_t i = 0; i < 4; ++i)
    {
        desc2.joints[i].position = 0;
        desc2.joints[i].speed = 15000;
        desc2.joints[i].current = 100;
    }
    desc2.joints[4].position = 3000;
    desc2.joints[4].speed = 15000;
    desc2.joints[4].current = 100;

    desc2.joints[5].position = 2000;
    desc2.joints[5].speed = 15000;
    desc2.joints[5].current = 100;

    for (int i = 0; i < 5; i++)
    {
        hand->moveMultipleFingers(deviceId, desc1);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        hand->moveMultipleFingers(deviceId, desc2);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    hand->resetJoints(deviceId);

    //Single-Frame Control
    for(int i = 0; i< 5; i++)
    {
        hand->moveFinger(deviceId, 0x04, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x05, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);

        hand->moveFinger(deviceId, 0x00, CASCADED_PID_CONTROL_MODE, 1000, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x01, CASCADED_PID_CONTROL_MODE, 1000, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x02, CASCADED_PID_CONTROL_MODE, 1000, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x03, CASCADED_PID_CONTROL_MODE, 1000, 0, 0, 0, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        hand->moveFinger(deviceId, 0x00, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x01, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x02, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x03, CASCADED_PID_CONTROL_MODE, 0, 0, 0, 0, 0);

        hand->moveFinger(deviceId, 0x04, CASCADED_PID_CONTROL_MODE, 3000, 0, 0, 0, 0);
        hand->moveFinger(deviceId, 0x05, CASCADED_PID_CONTROL_MODE, 2000, 0, 0, 0, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    hand->resetJoints(deviceId);

    hand->setRealtimeResponse(deviceId, 10, false);
    hand->disconnect();
}


int main(int argc, const char* argv[])
{
    if (argc != 2 && argc != 3)
    {
        std::cout << "USAGE: dexhand021Lite <serial-port> [baud]" << std::endl;
        std::cout << "\te.g. dexhand021Lite COM12 921600        (Windows)" << std::endl;
        std::cout << "\t     dexhand021Lite /dev/ttyACM0 921600 (Linux)" << std::endl;
        std::cout << "\tbaud: 115200|230400|460800|921600|1000000|2000000 (default 921600)" << std::endl;
        return 1;
    }

    BaudRate baud = BaudRate::BAUD_115200;
    if (argc == 3 && !parseBaudRate(argv[2], baud))
    {
        std::cout << "Invalid baud rate: " << argv[2] << std::endl;
        return 1;
    }

    DX021Lite_RTU485_IFTests(argv[1], baud);
    return 0;
}