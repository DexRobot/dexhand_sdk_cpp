#include <iostream>
#include <cstring>
#include "DexHand.h"
#include "StringUtils.h"

#define RL_BUFSIZE 1024

using namespace DexRobot;
using namespace DexRobot::Dex021;


void CallbackFunc(const DX21StatusRxData * status)
{
    //printf("BoradTemp = %d\n", status->boardTemper);
    printf("Position1 = %d, Speed1 = %d, Current1=%d, MT Temp1=%d\n"
        , status->MotorHallValue(0)
        , status->MotorVelocity(0)
        , status->MotorCurrent(0)
        , status->MotorTemperature(0));

    printf("Position2 = %d, Speed2 = %d, Current2=%d, MT Temp2=%d\n"
        , status->MotorHallValue(1)
        , status->MotorVelocity(1)
        , status->MotorCurrent(1)
        , status->MotorTemperature(1));

    printf("Position3 = %d, Speed3 = %d, Current3=%d, MT Temp3=%d\n"
        , status->MotorHallValue(2)
        , status->MotorVelocity(2)
        , status->MotorCurrent(2)
        , status->MotorTemperature(2));

    printf("Position4 = %d, Speed4 = %d, Current4=%d, MT Temp4=%d\n"
        , status->MotorHallValue(3)
        , status->MotorVelocity(3)
        , status->MotorCurrent(3)
        , status->MotorTemperature(3));

    printf("\n");
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

void show_usage()
{
    std::cout << "USAGE: dexhand021s <-zlg2u | -zlgm | -lys> [-l]" << std::endl;
    std::cout << "\t-zlg2u, use ZLG USB CANFD 200U adapter" << std::endl;
    std::cout << "\t-zlgm, use ZLG USB CANFD mini adapter" << std::endl;
    std::cout << "\t-lys, use LYS USB CANFD mini adapter" << std::endl;
    std::cout << "\t-l, optional, use for whether listening to realtime response or not" << std::endl;
}


int main(int argc, const char ** argv)
{
    bool bListen = false;
    AdapterType atype;

    if(argc != 3 && argc != 2)
    {
        show_usage();
        exit(1);
    }

    if(0 == strncmp(argv[1], "-zlg2u", 6))
    {
        atype = AdapterType::ZLG_200U;
    }
    else if(0 == strncmp(argv[1], "-zlgm", 6))
    {
        atype = AdapterType::ZLG_MINI;
    }
    else if(0 != strncmp(argv[1], "-lys", 4))
    {
        atype = AdapterType::LYS_MINI;
    }
    else
    {
        show_usage();
        exit(1);
    }

    if(argc == 3 && (0 == strncmp(argv[2], "-l", 2)))
        bListen = true;

    auto hand = DexHand::createInstance(ProductType::DX021_S, atype, 0);

    DH21StatusRxCallBack callback = std::bind(CallbackFunc, std::placeholders::_1);
    hand->setStatusRxCallback(callback);

    if(!hand->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    uint8_t handId = 0x02;

    hand->setHandId(AdapterChannel::CHN0, handId);
    hand->setRealtimeResponse(handId, 0x00, 50, bListen);

    /*
    bool exitflg = false;
    do
    {
        printf("DexHand-021S>: ");
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
            hand->setRealtimeResponse(handId, 0x00, 50, false);
        }
        else
        {
            if(args.size() != 4)
            {
                free(line);
                std::cout << "[ERROR]: Incomplete execution arguments" << std::endl;
                std::cout << "USAGE: <device_id> <finger_id> <degree> <velocity>" << std::endl;
                std::cout << "ARGUMENTS:" << std::endl;
                std::cout << "  device_id:\tFor DX021S, AKA hand ID, can be updated by user via API" << std::endl;
                std::cout << "  finger_id:\tFor DX021S, it's between [1, 12]" << std::endl;
                std::cout << "  degree:\tFor DX021S, it's the value of target degree*100 for steering motor" << std::endl;
                std::cout << "  velocity:\tFor DX021S, it's the value of running degree*100 of steering motor per second" << std::endl;
                continue;
            }

            uint8_t  deviceId = strtoint(args[0].c_str(), nullptr, 10);
            uint8_t  fingerId = strtoint(args[1].c_str(), nullptr, 10);
            uint16_t degree   = strtoint(args[2].c_str(), nullptr, 10);
            uint16_t velocity = strtoint(args[3].c_str(), nullptr, 10);

            if(deviceId != handId)
            {
                std::cout << "Device " << deviceId << " is not connected" << std::endl;
                continue;
            }

            hand->moveFinger(deviceId, fingerId, 0x0, degree, velocity, MotorControlMode::HALL_POSITION_CONTROL_MODE);
        }

        free(line);
    } while(exitflg != true);
    */

    return 0;
}
