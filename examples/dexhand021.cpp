#include <iostream>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DexHand.h"
#include "StringUtils.h"

#define RL_BUFSIZE 1024

using namespace DexRobot;
using namespace DexRobot::Dex021;

DexHand::PTR gHandInstance = nullptr;

void signalHandler(int signum)
{
    std::cout << "\nGet Message: " << signum << std::endl;

    if (signum == SIGINT) {
        std::cout << "Closing socket..." << std::endl;
        if(gHandInstance != nullptr)
            gHandInstance->disconnect();
        
        exit(0);
    }
    else if (signum == SIGABRT) {
        std::cout << "Closing socket..." << std::endl;
        if(gHandInstance != nullptr)
            gHandInstance->disconnect();
        
        exit(signum);
    }
}

void CallbackFunc(const DexRobot::DX21StatusRxData * statusRx)
{
    auto fingerId = statusRx->deviceId;

    printf("Finger %d: Current1=%d, Degree1=%f, Current2=%d, Degree2=%f\n", fingerId,
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
    signal(SIGINT, signalHandler);
    signal(SIGABRT, signalHandler);

    /// Create an instance of DX021 device, on channel 0 of ZLG USBCANFD-200U adapter
    gHandInstance = DexHand::createInstance(ProductType::DX021, AdapterType::ZLG_200U, 0);
    if(nullptr == gHandInstance)
    {
        std::cout << "Device not available." << std::endl;
        exit(-1);
    }

    /// Register a callback function to process the realtime data of DexHand-021 status feedbacks. In this example,
    /// the callback function simply just print the data on standard output (screen).
    DH21StatusRxCallback callback = std::bind(CallbackFunc, std::placeholders::_1);
    //gHandInstance->setStatusRxCallback(callback);

    if(!gHandInstance->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    uint8_t handId = 0x01;

    /// This is not a "must", by default, the left hand is assigned as ID 1, while right hand is assigned as ID 2
    /// To be certain if you want to change the default ID of hand, then you can call this.
    gHandInstance->setHandId(AdapterChannel::CHN0, handId);

    // Set realtime response for all fingers
    gHandInstance->setRealtimeResponse(handId, 50, true);

    bool exitflg = false;
    do
    {
        printf("DexHand-021 move finger>: ");
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
            if(args.size() != 4)
            {
                free(line);
                std::cout << "[ERROR]: Incomplete execution arguments" << std::endl;
                std::cout << "USAGE: <device_id> <finger_id> <joint_id> <value 1> <value 2>" << std::endl;
                std::cout << "ARGUMENTS:" << std::endl;
                std::cout << "  finger_id:\tFor DX021, it's between [1, 12]" << std::endl;
                std::cout << "  joint_id:\tFor DX021, it stands for the distal or proximal joint of the finger, value from enum JointMotor" << std::endl;
                std::cout << "  value 1:\tFor DX021, it's the target value of distal joint degree*100" << std::endl;
                std::cout << "  value 2:\tFor DX021, it's the target value of proximal joint degree*100" << std::endl;
                continue;
            }

            uint8_t  fingerId = strtoint(args[0].c_str(), nullptr, 10);
            uint8_t  jointId = strtoint(args[1].c_str(), nullptr, 10);
            uint16_t value1   = strtoint(args[2].c_str(), nullptr, 10);
            uint16_t value2   = strtoint(args[3].c_str(), nullptr, 10);

            gHandInstance->moveFinger(handId, fingerId, jointId, value1, value2, MotorControlMode::CASCADED_PID_CONTROL_MODE);
        }

        free(line);
    } while(exitflg != true);

    gHandInstance->disconnect();
    return 0;
}
