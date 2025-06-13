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

/// global variables for socket connections
int gConnSockFd = -1;
std::string gHostAddr;
uint16_t gHostPort;
bool gSrvrConnected = false;

void InitSocket();
void SendToSocket(const char * frame, size_t frameSize, size_t frameCnt=1);


void signalHandler(int signum)
{
    std::cout << "\nGet Message: " << signum << std::endl;

    if (signum == SIGINT) {
        std::cout << "Closing socket..." << std::endl;
        if(gConnSockFd != -1)
            shutdown(gConnSockFd, SHUT_WR);
        
        exit(0);
    }
    else if (signum == SIGABRT) {
        std::cout << "Closing socket..." << std::endl;
        if(gConnSockFd != -1)
            shutdown(gConnSockFd, SHUT_WR);
        
        exit(signum);
    }
}


void InitSocket()
{
    gConnSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(gConnSockFd == -1)
    {
        fprintf(stderr, "Create socket descriptor failed. Data will not be sent\n");
        return;
    }

    struct sockaddr_in srvr_addr;
    memset((char *)&srvr_addr, 0, sizeof(struct sockaddr_in));

    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = inet_addr(gHostAddr.c_str());
    srvr_addr.sin_port = htons(gHostPort);

    gSrvrConnected = (0 == ::connect(gConnSockFd, (const struct sockaddr *)&srvr_addr, sizeof(srvr_addr)));
    if(gSrvrConnected)
    {
        fprintf(stderr, "Socket connection is not able to be established, data will not be sent\n");
    }
}


void SendToSocket(const char * frame, size_t frameSize)
{
    if(!gSrvrConnected || gConnSockFd == -1)
    {
        std::cout << "Socket not available, skip sending data" << std::endl;
        return;
    }

    size_t buffLen = frameSize;

    ssize_t sentLen = send(gConnSockFd, frame, buffLen, 0);
    if(sentLen < 0)
    {
        shutdown(gConnSockFd, SHUT_WR);
        gConnSockFd = -1;
        gSrvrConnected = false;

        if(sentLen == -1)
        {
            fprintf(stderr, "Connection closed by peer\n");
        }
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

    if(argc == 3)
    {
        gHostAddr = std::string(argv[1]);
        gHostPort = (uint16_t)atoi(argv[2]);
        InitSocket();
    }

    auto hand = DexHand::createInstance(ProductType::DX021, AdapterType::ZLG_200U, 0);
    if(nullptr == hand)
    {
        std::cout << "Device not available." << std::endl;
        exit(-1);
    }

    DH21StatusRxCallback callback = std::bind(CallbackFunc, std::placeholders::_1);
    hand->setStatusRxCallback(callback);

    if(!hand->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    uint8_t handId = 0x01;
    hand->setHandId(AdapterChannel::CHN0, handId);
    hand->setRealtimeResponse(handId, 0x01, 50, true);

    bool exitflg = false;
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
