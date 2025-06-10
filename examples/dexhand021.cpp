#include <iostream>
#include <cstring>
#include "DexHand.h"
#include "StringUtils.h"

#define RL_BUFSIZE 1024

using namespace DexRobot;
using namespace DexRobot::Dex021;

void CallbackFunc(DexRobot::DX21StatusRxData * statusRx)
{
    auto fingerId = statusRx->deviceId;
    //auto fingerName = DexHandModelHelper::FingerID_2FingerName((FingerID)fingerId);

    printf("Current1=%d, Degree1=%f, Current2=%d, Degree2=%f\n",
            statusRx->MotorCurrent(1),
            statusRx->JointDegree(1),
            statusRx->MotorCurrent(2),
            statusRx->JointDegree(2));
}

char *CmdReadLine(void)
{
    int bufsize = RL_BUFSIZE;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        printf("CmdReadLine: allocation error\n");
        exit(1);
    }

    while (true) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufsize) {
            bufsize += RL_BUFSIZE;
            buffer = (char *)realloc(buffer, bufsize);
            if (!buffer) {
                printf("CmdReadLine: realloc error\n");
                exit(1);
            }
        }
    }
}

int main(int argc, const char ** argv)
{
    auto hand = DexHand::createInstance(ProductType::DX021, AdapterType::ZLG_200U, 0, true);
    if(nullptr == hand)
    {
        std::cout << "Device not available." << std::endl;
        exit(-1);
    }

    DH21StatusRxCallback callback = std::bind(CallbackFunc, std::placeholders::_1);
    hand->setStatusRxCallback(callback);

    if(!hand->connect())
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    uint8_t handId = 0x01;
    hand->setHandId(AdapterChannel::CHN0, handId);
    hand->setRealtimeResponse(handId, 0x01, 50, true);

    //while(true) {
    //}

    bool exitflg = false;
    /*
    */
    do
    {
        printf("DexHand-021>: ");
        char * line = CmdReadLine();

        if(strlen(line) == 0)
        {
            free(line);
            continue;
        }

        std::vector<std::string> args;
        DexRobot::split(args, line, " ");

        if (strcasecmp(line, "exit") == 0
                || strcasecmp(line, "quit") == 0)
        {
            exitflg = true;
        }
        else
        {
            if(args.size() != 5)
            {
                free(line);
                std::cout << "[ERROR]: Incomplete execution arguments" << std::endl;
                std::cout << "USAGE: <device_id> <finger_id> <joint_position> <value 1> <value 2>" << std::endl;
                std::cout << "ARGUMENTS:" << std::endl;
                std::cout << "  device_id:\tFor DX021, AKA hand ID, either be 1(for left hand) or 2(for right hand)" << std::endl;
                std::cout << "  finger_id:\tFor DX021, it's between [1, 12]" << std::endl;
                std::cout << "  joint_position:\tFor DX021, it stands for the distal or proximal joint of the finger, value from enmu JointMotor" << std::endl;
                std::cout << "  value 1:\tFor DX021, it's the target value of distal joint degree*100" << std::endl;
                std::cout << "  value 2:\tFor DX021, it's the target value of proximal joint degree*100" << std::endl;
                continue;
            }

            uint8_t  deviceId = strtoint(args[0].c_str(), nullptr, 10);
            uint8_t  fingerId = strtoint(args[1].c_str(), nullptr, 10);
            uint8_t  jointPos = strtoint(args[2].c_str(), nullptr, 10);
            uint16_t value1   = strtoint(args[3].c_str(), nullptr, 10);
            uint16_t value2   = strtoint(args[4].c_str(), nullptr, 10);

            hand->moveFinger(deviceId, fingerId, jointPos, value1, value2, MotorControlMode::CASCADED_PID_CONTROL_MODE);
        }

        free(line);
    } while(exitflg != true);

    return 0;
}
