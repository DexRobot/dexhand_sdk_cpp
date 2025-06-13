#include <iostream>
#include <cstring>
#include "DexHand.h"
#include "StringUtils.h"

#define RL_BUFSIZE 1024

using namespace DexRobot;
using namespace DexRobot::Dex021;

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
    auto hand = DexHand::createInstance(ProductType::DX021_S, AdapterType::ZLG_200U, 0);
    if(!hand->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }

    uint8_t handId = 0x02;
    hand->setHandId(AdapterChannel::CHN0, handId);

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

            //if(deviceId != handId)
            //{
            //    std::cout << "Device " << deviceId << " is not connected" << std::endl;
            //    continue;
            //}

            hand->moveFinger(deviceId, fingerId, 0x0, degree, velocity, MotorControlMode::HALL_POSITION_CONTROL_MODE);
        }

        free(line);
    } while(exitflg != true);

    return 0;
}
