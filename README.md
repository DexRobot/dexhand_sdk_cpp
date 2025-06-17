# DexHand CPP SDK

## 概览

DexHand CPP SDK为DexRobot灵巧手系列产品的二次开发人员提供了基础C++开发套件，使得开发人员可以通过该SDK对DexRobot的灵巧手系列产品进行基本的运动控制，目前支持的DexRobot灵巧手系列产品型号包括五指灵巧手DexHand-021, 和三指灵巧手DexHand-021S。

## 如何获得并使用DexHand CPP SDK

### 获得DexHand CPP SDK

DexHand CPP SDK可以通过DexRobot的github社区获得，也可以从DexRobot的官方网站下载。

- github地址：<https://github.com/DexRobot/dexhand_sdk_cpp.git>  
  可以通过`git clone <https://github.com/DexRobot/dexhand_sdk_cpp.git>`将远程仓库拉取到本地。
- DexRobot官网下载地址：<https://www.dex-robot.com/downloadCenter/detail>
  也可以通过官网下载页面下载压缩包到本地。

### 目录结构

如果下载的是压缩包，完成下载后，请先将压缩包解压。SDK下有三个子目录：include, lib, 以及examples。
其中include目录下包含了开发人员所需该SDK的所有头文件，lib目录下则包含了该SDK所有的动态库二进制文件。examples目录中有关于如何使用该SDK对DexRobot所有灵巧手系列产品进行基本运动控制的示例代码，包括五指灵巧手DexHand-021和三指灵巧手DexHand-021S。

### 使用流程

目前DexRobot的系列灵巧手产品均支持使用CANFD进行通讯，该SDK提供了使用CANFD-USB适配器与灵巧手设备进行通讯和运动控制的接口

- #### 创建灵巧手设备实例
  
使用DexHand::createInstance()接口创建一个连接在CANFD适配器指定通道上的，指定类型的灵巧手实例，如果该调用的返回值不为空，则实例创建成功，否则实例创建失败：

```CPP
    auto gHandInstance = DexHand::createInstance(ProductType::DX021, AdapterType::ZLG_200U, 0);
    if(nullptr == gHandInstance)
    {
        std::cout << "Device not available." << std::endl;
        exit(-1);
    }
```

实例创建成功后不代表可以直接进行控制，还需要对该灵巧手进行相应的设置，并发起连接后，才可以进行后续的控制。

- #### 设置灵巧手实例

通常在对灵巧手设备进行运动控制的同时，我们还需要了解灵巧手各手指及关节的运动状态，角度，方便进行运动学解算等工作，并对灵巧手内部零部件的工作状态如电机的电流，转速，温度等进行监控，以方便对灵巧手的工作负载进行管理。这要求我们能够获得灵巧手固件反馈的这些状态数据并进行处理。用户可以通过调用setStatusRxCallback()接口，来向该实例注册一个回调函数用于处理灵巧手固件反馈的状态数据：

```CPP
    DH21StatusRxCallback callback = std::bind(CallbackFunc, std::placeholders::_1);
    gHandInstance->setStatusRxCallback(callback);
```

关于状态数据具体包括哪些信息，以及对该回调函数的参数和返回值形式要求等详细信息，需参阅DexRobotd的灵巧手用户手册及SDK使用手册。这些文档均可在官网下载中心获得: <https://www.dex-robot.com/downloadCenter/detail>

- #### 发起连接并设置固件
  
完成回调函数设置后，即可通过该实例向灵巧手设备发起连接:

```CPP
    if(!gHandInstance->connect(true))
    {
        std::cout << "Connection failure." << std::endl;
        exit(-1);
    }
```

对于DexHand-021型灵巧手来说，实例连接成功后，会自动将左手ID设为1, 右手ID设为2，并与CANFD适配器上相应的通道进行绑定，开发者可以通过handID(channel)接口获取指定通道上所连接的手的ID。用户同时也可以使用setHandId()接口来自行决定该映射关系：

```CPP
    uint8_t handId = 0x01;
    gHandInstance->setHandId(AdapterChannel::CHN0, handId);
    uint8_t actualHandId = gHandInstance->handID(AdapterChannel::CHN0); // The returned actualHandId should be the same as handId.
```

开发者可以根据自己的需求，进一步对灵巧手的固件进行相应的设置。例如，在默认状态下，灵巧手设备不会实时的上报自身的状态数据，而是只会在执行每一帧运动指令后进行反馈。如果开发者希望实时的获得灵巧手的状态数据以方便进行运动解算及监控，则需要进行一下设置：

```CPP
    gHandInstance->setRealtimeResponse(handId, 50, true);
```

该调用将两巧手的固件设置为实时上报状态数据的模式，采样率为50Hz。若第三个参数设置为false，则实时采样模式关闭。如开发者不希望实时监控灵巧手的状态，则该调用不是必须的。

- #### 控制灵巧手
  
完成上述所有操作后，开发者便可以开始根据自己的需要对灵巧手的各个手指进行运动控制，控制的接口为moveFinger()。调用方式如下：

```CPP
    gHandInstance->moveFinger(handId, fingerId, jointId, value1, value2, MotorControlMode::CASCADED_PID_CONTROL_MODE);
```

五指灵巧手DexHand-021的手指ID设置为每只手有6个控制单元共12个主动自由度，分别代表5个手指及手掌的分指机构，各自有独立的手指ID。其中左手大拇指ID为1，分指机构ID为2，食指到小拇指依次为3，4，5，6。各手指有两个主动自由度，靠近手掌的近端关节MIP，在该SDK中标识为JointMotor::PROX，取值为2，靠近远端的指腹关节PIP在该SDK中标识为JointMotor::DIST，取值为1。其中对分指机构的DIST为大拇指翻转，而PROX则为其余4指的开合。对于右手，大拇指的ID为7，分指机构ID为8，其余以与左手对称的方式依次标识为9，10，11，12。关于DexHand-021的结构模型及ID分布的详细说明，请参阅用户手册。

该调用中，第二个参数为指定的手指ID，第三个参数代表远近端关节使能，取值为JointMotor::DIST标识只运动远端关节，取值PROX时只运动近端关节，取值ALL时，则所有关节均参与运动。第四个和第五个参数分别代表远端关节和近端关节运动的目标值，该值在不同的控制模式下具有不同的语义。目前对用户开放的控制模式共有4种，代码分别为0x44, 0x55, 0x66, 0x77。示例中的MotorControlMode::CASCADED_PID_CONTROL_MODE。关于不同的控制模式，以及运动的目标取值的含义等相关详细信息，请参阅灵巧手的用户手册和通讯协议手册。

- #### 断开设备

当使用完灵巧手之后，用户需要确保程序实例与灵巧手设备的连接断开：

```CPP
    gHandInstance->disconnect();
```

该调用会自动退出设备的实时状态数据采样模式。此后灵巧手设备将不再接受程序实例发出的任何控制指令。
