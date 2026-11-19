# DexHand021Lite SDK 用户接口文档

## 概览

本文档面向使用 DexHand021Lite SDK（C\+\+）进行二次开发的最终用户，覆盖产品的创建、连接、运动控制、状态获取与常用参数设置等全部用户级接口。

DexHand021Lite 通过 485 串口（RTU485 适配器）以 Modbus RTU 协议通讯。CANFD 为后续规划，当前版本暂不支持。跨平台支持Windows和Ubuntu系统。

## 1\. 实例创建与销毁

### 1\.1 `DexHand::createInstance`

```C++
static DexHand::PTR DexHand::createInstance(ProductType productType,
                                             const char * serialPortName,
                                             BaudRate baudRate);
```

**功能**：创建一个指定产品类型的灵巧手实例。返回值非空即创建成功。

|参数|类型|说明|
|---|---|---|
|`productType`|`ProductType`|必须传 `ProductType::DX021_LITE`|
|`serialPortName`|`const char *`|485 适配器连接 PC 后分配的串口设备号。Windows 如 `"COM3"`，Linux 如 `"/dev/ttyUSB0"`|
|`baudRate`|`BaudRate`|串口波特率，见附录 A。默认 `BAUD_921600`|

```C++
auto gHandInstance = std::dynamic_pointer_cast<DexHand_021Lite>(
    DexHand::createInstance(ProductType::DX021_LITE, "COM3", BaudRate::BAUD_921600));
if (nullptr == gHandInstance)
{
    std::cout << "Device not available." << std::endl;
    exit(-1);
}
```

### 1\.2 `DexHand_021Lite` 直接构造

```C++
explicit DexHand_021Lite(const char * adapterName,
                         BaudRate baudRate = BaudRate::BAUD_921600);
```

**功能**：直接以串口名构造 Lite 实例，适合明确只需要 Lite 的场景。

|参数|类型|说明|
|---|---|---|
|`adapterName`|`const char *`|串口设备号，同 1\.1|
|`baudRate`|`BaudRate`|波特率，默认 `BAUD_921600`|

```C++
//设备波特率非921600时，需要填入第二个参数。
const auto hand = std::make_shared<DexHand_021Lite>("COM3");
```

### 1\.3 `DexHand_021Lite` 直接构造（指定适配器类型）

```C++
DexHand_021Lite(AdapterType adapterType, uint8_t adpaterIndex,
                const char * adapterName = "/dev/ttyUSB0",
                BaudRate baudRate = BaudRate::BAUD_921600);
```

**特别说明**：上述构造方式是与其他产品接口形式保持一致，同时为后续做CANFD通讯预留，当前不建议使用。

|参数|类型|说明|
|---|---|---|
|`adapterType`|`AdapterType`|当前仅支持 `AdapterType::USB2_485`；传入其他类型不会报构造错误，但 `errorCode()` 变为 `ADAPTER_NA`，后续所有通讯失败|
|`adpaterIndex`|`uint8_t`|适配器 USB 索引，从 0 开始（当前 RTU485 路径未使用，填 0 即可）|
|`adapterName`|`const char *`|串口设备号|
|`baudRate`|`BaudRate`|波特率，默认 `BAUD_921600`|

实例创建成功后必须发起连接（第 2 节）才能通讯。实例销毁时自动释放内部适配器资源。

---

## 2\. 连接管理

### 2\.1 `connect`

```C++
bool connect(bool listen);
```

**功能**：打开串口，建立与 485 适配器的连接。

|参数|类型|说明|
|---|---|---|
|`listen`|`bool`|**必须传 ** **`true`**：启动接收线程处理设备响应。传 `false` 时连接虽成功，但所有阻塞读接口（状态/参数/版本/在线探测）将永远超时失败|

**返回值**：`true` 打开串口成功；`false` 失败，失败原因通过 `errorCode()` / `errorMessage()` 查询（典型：`ADAPTER_NA`，串口不存在或被占用）。

```C++
if (!gHandInstance->connect(true))
{
    std::cout << gHandInstance->errorMessage() << std::endl;
    exit(-1);
}
```

### 2\.2 `disconnect`

```C++
bool disconnect();
```

**功能**：停止接收线程并关闭串口。调用后设备不再接受本实例的任何指令。无参数，正常情况恒返回 `true`。

### 2\.3 `productType`

```C++
ProductType productType() const;
```

**功能**：返回本实例的产品类型，对 Lite 恒返回 `ProductType::DX021_LITE`。用于在持有基类指针时区分产品。

注意：产品类型参见ProductType中的枚举变量。

### 2\.4 `errorCode` / `errorMessage`

```C++
SysErrorCode errorCode() const;
const std::string & errorMessage() const;
```

**功能**：返回最近一次系统级错误（连接失败、适配器不可用等）。

注意：设备执行层面的失败（如运动指令未被执行）不反映在这里，需通过状态轮询确认。

---

## 3\. 回调注册

三个回调注册接口均为 `const` 成员，可在 `connect` 前后任意时刻调用，重复调用后者覆盖前者。

### 3\.1 `setStatusRxCallback`

```C++
void setStatusRxCallback(const DH21StatusRxCallBack & callback) const;
```

**功能**：注册状态数据处理回调。数据由SDK后台轮询线程按照周期读取整手的六个电机参数分区的相关参数，每完成一轮触发一次回调。

**注意**：

- 必须先调用 `setRealtimeResponse(deviceId, sampleRate, true)` 开启实时轮询；只注册回调不会产生任何数据（见 5\.1）。

- 回调内禁止调用`disconnect()`,也不要执行阻塞通讯或长耗时计算。

**回调签名**：

```C++
typedef std::function<void (const Dex021::DX21StatusRxData *)> DH21StatusRxCallBack;
```

使用示例：

```C++
void StatusCallbackFunc([](const Dex021::DX21StatusRxData * handStatus{
     for (uint8_t fingerId = 0; fingerId < 6; ++fingerId)
      {
          std::cout << "fingerId= " << static_cast<int>(fingerId)
                << "  iq="   << status->MotorCurrent(fingerId)
                << "  rpm="  << status->MotorVelocity(fingerId)
                << "  hall=" << status->MotorHallValue(fingerId)
                << "  err=0x" << std::hex << status->MotorErrorStatus(fingerId)
                << std::dec << std::endl;
      }
});

gHnadInstance->setRealtimeResponse(DEV, 50, true); //开启轮询
```

### 3\.2 `setErrorRxCallback`

```C++
void setErrorRxCallback(const ErrorMessageCallBack & callback) const;
```

**功能**：注册错误消息处理回调。回调参数 `ErrorMessageRx` 包含字段：

|字段|类型|说明|
|---|---|---|
|`channelId`|`uint8_t`|通道号（485 下无意义）|
|`deviceId`|`uint8_t`|报告错误的设备从机地址|
|`overloadError`|`int16_t`|过载错误码；设备返回Modbus异常帧，该字段为异常码（含义见附录C）|
|`motorErrors`|`std::vector<MotorErrorMsg>`|电机级错误明细。Lite下不适用该字段，错误回调只携带Modbus异常帧，电机错误位通过`getHandStatus()`获取（见5\.2）|

设备返回的Modbus的异常帧会触发该回调 ，此时overloadError为异常码，按附录C的R485LiteErrorCode解释。

### 3\.3 `setParamRWCallback`

```C++
void setParamRWCallback(const ParamRwMessageCallBack & callback) const;
```

**功能**：注册参数读写结果回调。设备返回协议 0x03（读保持寄存器）/ 0x06（写单寄存器）响应时触发。回调参数 `SysParameterRWRx` 包含字段：

|字段|类型|说明|
|---|---|---|
|`channelId`|`uint8_t`|通道号（485 下无意义）|
|`deviceId`|`uint8_t`|响应设备的从机地址|
|`operationType`|`uint8_t`|操作类型（如 `HRG_READ`）|
|`parameterId`|`uint8_t`|起始寄存器地址低 8 位|
|`succeed`|`uint8_t`|是否成功|
|`dataLen`|`uint8_t`|结果数据长度|
|`results[10]`|`uint8_t[10]`|结果数据，Modbus 大端（`results[0]` 为高字节）|

**回调使用约束（三类回调通用）**：

1. **状态回调**在 SDK **轮询线程**中执行；**错误/参数回调**在适配器**接收线程**中执行。回调实现必须快速返回、线程安全，禁止在回调内做阻塞通讯、延时或长耗时计算；

2. 状态回调慢会直接拉低下一轮轮询频率；不要在状态回调里调用 `disconnect()`；

3. `getSafeTemperature()、getThumbDegree()` 等**阻塞读接口执行期间，SDK 会临时替换参数回调**，读完后恢复——不要在这些接口阻塞期间依赖参数回调持续触发；`getHandStatus() `只读缓存，不会替换任何回调。

---

## 4\. 运动控制

### 4\.1 `enable`

```C++
bool enable(uint8_t deviceId, bool enableAll);
```

**功能**：一次性使能/失能整手全部 6 个电机。电机只有在使能状态下才执行运动指令。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|
|`enableAll`|`bool`|`true` 使能全部 6 电机；`false` 全部失能。|

**返回值**：`true` 命令已发送；`false` 发送失败（串口未打开等）。

**注意**：`deviceId` 是整机从机地址，与 DX021 系列的手指板 ID 语义不同。首次上电后，需要整帧控制前必须先 `enable(DEV, true)` 才能运动。

### 4\.2 `moveFinger`（完整参数版，推荐）

```C++
bool moveFinger(uint8_t deviceId, uint8_t fingerId, MotorControlMode mode,
                int16_t position, uint16_t speed, uint16_t current,
                uint16_t p1, uint16_t p2, int32_t delay = 10);
```

**功能**：向指定电机发送一帧运动控制指令。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|
|`fingerId`|`uint8_t`|电机索引 0\~5：0=食指、1=中指、2=无名指、3=小指、4=拇指弯曲、5=拇指旋转。|
|`mode`|`MotorControlMode`|控制模式，见下表|
|`position`|`int16_t`|目标位置 AimPos，所有模式均有效|
|`speed`|`uint16_t`|AimSpd，模式 5/6 有效，4 模式忽略，填0x00即可|
|`current`|`uint16_t`|模式5 下为 AimCur；模式6 下为前馈力矩 Toffset，|
|`p1`|`uint16_t`|模式6 下为刚度 Kp；其他模式忽略，填0x00即可|
|`p2`|`uint16_t`|模式6 下为阻尼 Kd；其他模式忽略，填0x00即可|
|`delay`|`int32_t`|发送前延时，毫秒，默认 10|

**返回值**：`true` 命令已发送；`false` 参数非法或发送失败。

**控制模式**：

|模式枚举|模式名称|枚举值|有效参数|
|---|---|---|---|
|`CASCADED_PID_CONTROL_MODE`|位置模式|0x44|position|
|`HALL_POSLIMIT_CONTROL_MODE`|霍尔模式|0x55|position、speed、current|
|`CASCADED_MIT_CONTROL_MODE`|力位混合模式|0x66|position、speed、current、p1、p2（MIT 语义）|

**注意**：目前建议只使用4模式和5模式。

**各参数范围：**

||大拇指弯曲|大拇指侧摆|四指弯曲|
|---|---|---|---|
|HallValue|0\-5900|0\-7200|0\-2000|
|速度|2000\-18000|||
|电流|0\-125|||

```C++
//4模式食指运动位置到1000
gHandInstance->moveFinger(DEV, 0x00, MotorControlMode::CASCADED_PID_CONTROL_MODE,
                          1000, 0, 0, 0, 0);
```

### 4\.3 `moveFinger`（基类重载版）

```C++
bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
                int16_t controlArg1, int16_t controlArg2,
                MotorControlMode mode, int32_t delay);

bool moveFinger(uint8_t deviceId, uint8_t fingerId, uint8_t jointPosition,
                int16_t controlArg1, int16_t controlArg2,
                MotorControlMode mode);   // delay 默认 10ms
```

**功能**：为兼容基类 `DexHand` 通用接口提供的简化重载（**不推荐使用**）。内部映射到 4\.2：`jointPosition` 被忽略，`controlArg1`→position，`controlArg2`→speed，current/p1/p2 固定为 0，因此仅对位置模式（4模式）有意义。参数含义同 4\.2。仅在希望同一份代码控制多种 DexHand 产品时使用，Lite 专用程序请直接使用 4\.2。

### 4\.4 `moveMultipleFingers`

```C++
bool moveMultipleFingers(uint8_t deviceId, const LiteHandControlDesc & data);
```

**功能**：整手6个关节在一次调用中一起下发运动指令，无需逐个手指调用4\.2。

**注意**：控制大拇指进行弯曲和侧摆可使用该方法。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|
|`data`|`LiteHandControlDesc`|整手控制描述，见下|

**`LiteHandControlDesc`** **结构：**

|字段|类型|说明|
|---|---|---|
|`mode`|`MotorControlMode`|控制模式，6 个电机共用，取值同 4\.2|
|`joints[6]`|`ControlLoopArgs`|`joints[i]` 对应电机 i，顺序同 4\.2 的 `fingerId`（0\~3 四指、4 拇指弯曲、5 拇指旋转）|

**`LiteJointTarget`** **结构：**

|字段|类型|说明|
|---|---|---|
|`position`|`uint16_t`|目标位置|
|`speed`|`uint16_t`|目标速度|
|`current`|`uint16_t`|目标电流|
|`p1`|`uint16_t`|P1 / Kp|
|`p2`|`uint16_t`|P2 / Kd|

各字段的实际含义由 `mode` 决定，与 4\.2 的控制模式表一致。

**返回值**：`true` 命令已发送；`false` 发送失败。

**注意**：需先 `enable(DEV, true)`，否则电机不会响应。

```C++
//整帧控制示例代码 四指不动，大拇指运动
LiteHandControlDesc desc{};
desc.mode = MotorControlMode::HALL_POSLIMIT_CONTROL_MODE;
for (uint8_t i = 0; i < 4; ++i)
{
    desc.joints[i].position = 0;
    desc.joints[i].speed    = 10000;
    desc.joints[i].current  = 100;
}
desc.joints[4].position = 2000;
desc.joints[4].speed    = 15000;
desc.joints[4].current  = 100;

desc.joints[5].position = 3000;
desc.joints[5].speed    = 15000;
desc.joints[5].current  = 100;

gHandInstance->moveMultipleFingers(DEV, desc);
```

---

### 4\.5 `homeFingers/homeThumb`\(重要）

```C++
bool homeFingers(uint8_t deviceId);   // 四指回手动复位
bool homeThumb(uint8_t deviceId);     // 大拇指手动复位
```

**功能**：上电以后使四指关节或大拇指关节手动回到零位，设备上电并连接以后，未执行这两条命令时设备拒绝执行运动控制指令（Modbus 异常码 0x0B\)

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|

**返回值**：`true` 命令已发送；`false` 发送失败。上电后的实际姿态不确定（例如握拳时拇指可能在内侧或外侧），因此两个动作分开提供，由调用方按实际情况决定执行顺序。

```C++
gHandInstance->homeThumb(DEV);              // 先回拇指
gHandInstance->homeFingers(DEV);            // 再回四指（建议间隔 500ms 左右）
```

## 5\. 状态获取

### 5\.1 `setRealtimeResponse`

```C++
bool setRealtimeResponse(uint8_t deviceId, uint8_t fingerId, uint16_t sampleRate, bool enable);
bool setRealtimeResponse(uint8_t deviceId, uint16_t sampleRate, bool enable);
bool setRealtimeResponse(uint16_t sampleRate, bool enable);
```

**功能**：该方法用于开启后台轮询线程，每轮读取整手6个电机，读完后更新状态缓存并触发一次`setStatusRxCallback`回调。

|**参数**|**类型**|**说明**|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|
|`fingerId`|`uint8_t`|**Lite 忽略**。整手按周期轮询，传 0\~5 任意值都等价于订阅整手|
|`sampleRate`|`uint16_t`|单位 **Hz**（内部间隔 = `1000 / sample_rate` ms）：`50` = 20 ms、`10` = 100 ms。传得越大间隔越小，但实际刷新率受 485 往返时间限制；`0` 在开启轮询时非法（返回 `False`）|
|`enable`|`bool`|`true` 开始轮询；`false` 停止轮询（已采集的最后一帧仍保留在缓存中）|

三个重载：

- `(deviceId, fingerId, sampleRate, enable)`：`fingerId` 无效，等价于下一个重载；

- `(deviceId, sampleRate, enable)`：订阅/退订指定手；

- `(sampleRate, enable)`：`false` 清空全部订阅；`true` 只修改已订阅手的周期（没有订阅时返回 `false`）。

**返回值**：`sampleRate == 0` 且 `enable == true` 时返回 `false`；停止轮询（`enable == false`）总是成功。

### 5\.2 `getHandStatus`

```C++
DX21StatusRxData::PTR getHandStatus(uint8_t deviceId);
```

**功能**：非阻塞式读取最近一次轮询的整手电机参数的快照：

- 尚未开启轮询或还没完成第一轮时返回`nullptr`;

- 开启轮询后，只要有任一电机应答就更新缓存；

- 停止轮询或设备掉线后，继续返回最后一帧旧值，可用`timestamp`判断新旧。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备的 Modbus 从机地址|

```C++
// 开启轮询
gHandInstance->setRealtimeResponse(DEV, 10, true);

// 读取缓存：微秒级返回，不发 Modbus 请求
const auto status = gHandInstance->getHandStatus(DEV);
if (status != nullptr)
{
    const auto idcur = status->MotorIdCurrent(0);
    const auto iqcur = status->MotorCurrent(0);
    const auto spd   = status->MotorVelocity(0);
    const auto err   = status->MotorErrorStatus(0);
}

// 停止轮询（缓存保留最后一帧）
gHandInstance->setRealtimeResponse(DEV, 10, false);
```

### 5\.3 状态数据访问（`DX21StatusRxData`）

`getHandStatus `返回及状态回调收到的对象为` DX21StatusRxData::PTR`，请只使用公开基类接口访问（参数均为电机索引 0\~5，越界返回 0）：

|接口|返回|含义|
|---|---|---|
|`MotorIdCurrent(motorId)`|`int16_t`|电机电流 Id|
|`MotorCurrent(motorId)`|`int16_t`|电机电流 Iq|
|`MotorVelocity(motorId)`|`int16_t`|电机转速 RPM|
|`MotorHallValue(motorId)`|`int16_t`|Hall 角度基值|
|`MotorErrorStatus(motorId)`|`uint16_t`|电机错误位图（定义见附录 B）|

基类 `DX21StatusRxData` 的其余访问接口（`MotorTemperature`、`JointDegree`、`NormalForce`、`TangentForce`、`Voltage`、`PWMValue`、`ChipsetTemperature` 等）非DexHand021Lite提供，因此在 Lite 中**固定返回 0**

- 板级温度通过 `getTemperature()`（见 5\.5）读取，不在电机快照内；

- 拇指角度（电机 4/5，仅拇指具备角度传感器）通过 `getThumbDegree()`（见 5\.6）单独读取；

- Lite 无力/接近传感器。

### 5\.4 `isAlive`

```C++
bool isAlive(AdapterChannel channel, uint8_t deviceId);
```

**功能**：阻塞式探测指定从机地址的设备是否在线。SDK 发送一次设备信息读取，约 100ms 内收到响应即认为在线。

|参数|类型|说明|
|---|---|---|
|`channel`|`AdapterChannel`|仅为 CANFD 预留，485 下传 `AdapterChannel::CHNX`，参数被忽略|
|`deviceId`|`uint8_t`|待探测的从机地址|

**返回值**：`true` 设备应答；`false` 超时无应答（地址错误、设备未上电、串口异常等）。

```C++
if (!gHandInstance->isAlive(AdapterChannel::CHNX, DEV)) { /* 设备不在线 */ }
```

### 5\.5 `getTemperature`

```C++
bool getTemperature(uint8_t deviceId, uint16_t temperature[2]);
```

**功能**：阻塞式读取设备板级温度。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`temperature`|`uint16_t[2]`|**输出参数**，调用方分配。`[0]` = 传感器 1，`[1]` = 传感器 2，均为原始读数（温度×100，如 2530 表示 25\.30℃）|

**返回值**：`true` 读取成功（输出数组已填充）；`false` 失败（超时/无应答/适配器不可用），此时**输出数组保持原值不动**。

```C++
uint16_t boardTemp[2] = {0};
if (gHandInstance->getTemperature(DEV, boardTemp))
{
    printf("Sensor1=%.2f, Sensor2=%.2f\n", boardTemp[0] / 100.0, boardTemp[1] / 100.0);
}
```

### 5\.6 `getThumbDegree`

```C++
bool getThumbDegree(uint8_t deviceId, int16_t thumbDegree[2]);
```

**功能**：阻塞式读取拇指两个关节的角度。角度传感器仅拇指（电机 4/5）具备，其余手指无角度数据。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`thumbDegree`|`int16_t[2]`|**输出参数**，调用方分配。`[0]` = 拇指弯曲角（电机 4），`[1]` = 拇指旋转角（电机 5），单位为"角度值×100"，负值有效。|

**返回值**：`true` 读取成功（输出数组已填充）；`false` 失败（超时/无应答/适配器不可用），此时**输出数组保持原值不动**。

```C++
int16_t degree[2] = {0};
if (gHandInstance->getThumbDegree(DEV, degree))
{
    printf("Bend=%.2f deg, Rotate=%.2f deg\n", degree[0] / 100.0, degree[1] / 100.0);
}
```

### 5\.7 `getSysStatus`

```C++
uint16_t getSysStatus(uint8_t deviceId);
```

**功能**：阻塞式读取各个关节的在线状态

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：位 N 对应电机 N 的手指板，bit0\~bit5 有效（bit6\~bit7 保留），**1 = 在线、0 = 离线**；读取失败（超时/异常帧/CRC 错误）返回 `0xFFFF`。

```C++
const auto sysStatus = gHandInstance->getSysStatus(DEV);
if (sysStatus != 0xFFFF)   //先用0xFFFF判断是否读取失败
{
    for (int i = 0; i < 6; ++i)
        bool online = (sysStatus & (1u << i)) != 0;
}
```

**注意**：失败返回的 `0xFFFF` 在按 `0x3F` 掩码后会变成 `0x3F`，看起来像"全部在线"，因此必须先与 `0xFFFF` 比较，确认成功后再逐位判断。

### 5\.8 `getMotorStatus`

```C++
uint16_t getMotorStatus(uint8_t deviceId);
```

**功能**：阻塞式读取各电机的运行/待机状态（约 100ms 超时）。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：位 N 对应电机 N，bit0\~bit5 有效（bit6\~bit7 保留），**0 = 运行中、1 = 待机**；读取失败返回 `0xFFFF`，判断方式同 5\.7。

### 5\.9`getEnableStatus`

```C++
uint16_t getEnableStatus(uint8_t deviceId);
```

**功能**：阻塞式读取整手六个电机的使能状态位图。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：位 N 对应电机 N，bit0\~bit5 有效（bit6\~bit7 保留），**0 = 未使能、1 = 已使能**；读取失败返回 `0xFFFF`，判断方式同 5\.7。

---

## 6\. 设备信息查询

### 6\.1 `getFirmwareVersion`

```C++
uint32_t getFirmwareVersion(uint8_t deviceId, uint8_t fingerId);
```

**功能**：阻塞式读取设备应用固件版本（约 100ms 超时）。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`fingerId`|`uint8_t`|`0xFF`（`LITE_MASTER_VERSION`）= 主控板固件版本；`0~5` = 第 N 个从机板固件版本；其他值返回 0|

**返回值**：成功返回 `(major << 16) | (minor << 8) | patch`；失败（超时/无应答）返回 0。

```C++
//固件版本解析：返回值结果为0x10001代表固件版本为V1.0.1
const auto master = gHandInstance->getFirmwareVersion(DEV, LITE_MASTER_VERSION); // 主控板
const auto slave0 = gHandInstance->getFirmwareVersion(DEV, 0x00);                // 从机板 0
printf("master V%u.%u.%u, slave0 V%u.%u.%u\n",
       (master >> 16) & 0x1F, (master >> 8) & 0x1F, master & 0x3F,
       (slave0 >> 16) & 0x1F, (slave0 >> 8) & 0x1F, slave0 & 0x3F);
```

### 6\.2 `getHandType`

```C++
uint8_t getHandType(uint8_t deviceId);
```

**功能**：阻塞式读取手型（左手/右手，约 100ms 超时）。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：

|返回值|含义|
|---|---|
|`0x00`|左手|
|`0x01`|右手|
|`0x06`|未设置|
|`0xFF`|读取失败|

```C++
const auto handType = gHandInstance->getHandType(DEV);
```

### 6\.3 `getSysIds`

```C++
void getSysIds(AdapterChannel channel, std::vector<uint8_t> & sysIdList);
```

**功能**：发现总线上灵巧手的从机地址。以**广播从机地址 0x00** 发起 `SYS_ID` 寄存器读取，设备应答帧中携带其真实从机地址，成功则将地址低 8 位加入输出列表。单设备模型：一条 485 总线只接一只手，列表至多一个元素。

|参数|类型|说明|
|---|---|---|
|`channel`|`AdapterChannel`|传 `AdapterChannel::CHNX`，被忽略|
|`sysIdList`|`std::vector<uint8_t> &`|**输出参数**，调用前无需清空（接口内部清空）|

**返回值**：无。设备无应答时列表为空。

```C++
//未发现设备用法
std::vector<uint8_t> idList;
gHandInstance->getSysIds(AdapterChannel::CHNX, idList);
if (idList.empty()) { /* 总线上未发现设备 */ }

//通过广播获取设备从机地址,设备出厂时默认从机地址为0x01
std::vector<uint8_t> idList;
gHandInstance->getSysIds(AdapterChannel::CHNX, idList);
auto deviceId = idList.empty()? 0x01 : idList.front();
```

### 6\.4 `getSerialNumber`

```C++
std::string getSerialNumber(uint8_t deviceId);
```

**功能**：阻塞式读取设备序列号（约 100ms 超时）。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：成功返回设备序列号字符串；读取失败（超时/无应答）返回**空字符串**。

```C++
const auto sn = gHandInstance->getSerialNumber(DEV);
if (!sn.empty()) { std::cout << "SN=" << sn << std::endl; }
```

---

## 7\. 参数设置

### 7\.1 `setSafeTemperature`

```C++
bool setSafeTemperature(uint8_t deviceId, uint8_t device, uint16_t temperature);
```

**功能**：设置单个电机的保护温度。温度达到该值后固件会对过热设备立刻下使能。

**单位说明**：当前版本 set 接口将参数**按原值****下发**，因此调用时需自行传入"温度×100"（如 60\.00℃ 传 6000）；get 接口已自动换算，直接返回温度值。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`device`|`uint8_t`|电机索引 0\~5，\>=6 返回 `false`|
|`temperature`|`uint16_t`|写入寄存器的原始值（温度×100）|

**返回值**：`true` 命令已发送；`false` 参数非法或发送失败。

### 7\.2 `getSafeTemperature`

```C++
uint16_t getSafeTemperature(uint8_t deviceId, uint8_t device);
```

**功能**：阻塞式读取单个电机的保护温度（约 100ms 超时）。SDK 读取寄存器后自动 ÷100 换算。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`device`|`uint8_t`|电机索引 0\~5|

**返回值**：成功返回换算后的保护温度值；失败（超时/无应答/适配器不可用）返回 `0xFF`。

```C++
gHandInstance->setSafeTemperature(DEV, 0, 6000);   // 写入 60.00℃
const auto t = gHandInstance->getSafeTemperature(DEV, 0);   // 读回约 60
```

### 7\.3 `set/getSafeTemperature`（基类重载版）

以下重载为兼容基类接口保留，内部直接委托 7\.1/7\.2（`jointPosition` 忽略，`fingerId` 即电机索引），但 `maxTemperature` 为 `uint8_t`、返回值截断为 8 位，损失精度，**不推荐使用。**

```C++
bool setSafeTemperature(uint8_t deviceId, uint8_t fingerId,
                        uint8_t jointPosition, uint8_t maxTemperature);
uint8_t getSafeTemperature(uint8_t deviceId, uint8_t fingerId,
                           uint8_t jointPosition);
```

### 7\.4 `setSysId`

```C++
bool setSysId(uint8_t deviceId, uint16_t newSysId);
```

**功能**：修改设备的从机地址。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备当前从机地址|
|`newSysId`|`uint16_t`|新从机地址|

**返回值**：`true` 命令已发送；`false` 发送失败。

**注意**：修改成功后设备立即以新地址应答，后续所有调用的 `deviceId` 必须换为新地址。

### 7\.5 `setBaudRate`

```C++
bool setBaudRate(uint8_t deviceId, uint8_t baudIndex,
                 AdaptertType adatpter = AdapterType::USB2_485);
```

**功能**：切换设备 485 总线波特率。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|
|`baudIndex`|`uint8_t`|波特率索引 0\~5：0=115200、1=230400、2=460800、3=921600、4=1000000、5=2000000。\>5 返回 `false`|
|`adapterType`|`AdapterType`|设置哪条链路的波特率，默认 `AdapterType::USB2_485`；CANFD 类型暂未实现，返回 `false`|

**返回值**：`true` 命令已发送；`false` 参数非法或发送失败。

**特别注意**：设备收到指令后即切换到新波特率，**当前实例随即失效**——必须 `disconnect()` 并销毁实例，再以新波特率重新创建、连接。

```C++
gHandInstance->setBaudRate(DEV, 3);   // 切到 921600
gHandInstance->disconnect();
// 以 BaudRate::BAUD_921600 重建实例
```

---

### 7\.6 用户自定义表

```C++
bool writeUserTable(uint8_t deviceId, const std::map<LiteUserTableField, uint16_t> & table);
bool readUserTableValues(uint8_t deviceId, std::map<LiteUserTableField, uint16_t> & values);
bool writeUserTableValues(uint8_t deviceId, const std::map<LiteUserTableField, uint16_t> & values);
```

**功能**：设备侧维护一张"关键字表"，SDK提供所需的字段，这些字段对应的寄存器地址由固件维护，用户只需要使用字段码进行组表配置，之后通过读取表项进行读写。SDK 用**字段码**（`LiteUserTableField`）表示表项。

|接口|说明|
|---|---|
|`writeUserTable`|建表：将map的key写入表项区，value忽略。选取需要组成自定义表的字段，详情见附录D。|
|`readUserTableValues`|读值：入参map的key作为要读的字段码，返回后按key取值。|
|`writeUserTableValues`|写值：只接受可写字段，map的key/value都要填写。|

**前置条件**：`readUserTableValues` 读取的是设备当前自行设置的关键字表对应值，因此必须先调用 `writeUserTable` 建表，且**两次使用的字段码集合必须一致**。

**字段码**（完整列表见附录 D）：

- 固定字段：`0x0000` \~ `0x0017`

- 电机参数：`liteMotorParamField(motor, param)` = `0x0100 + 0x10 * motor + param`

- 电机输入（只读）：`liteMotorInputField(motor, input)` = `0x0200 + 0x08 * motor + input`

- 其他字段码一律非法，接口返回 `false`

**限制**：

- 保持寄存器表项最多 80 项、输入寄存器表项最多 176 项，超限返回 `false`；

- 可写字段仅：`BAUD_RATE`、`SYS_ID`、电机参数（`MOTOR_TYPE` 除外）；

- 写入只读字段或管理员字段时，整次调用返回 `false`，不会发出任何数据；

- 重复字段码自动去重。

```C++
// 建表：读 6 个电机的 IQ 电流
std::map<LiteUserTableField, uint16_t> table;
for(uint8_t m = 0; m < 6; ++m)
    table[liteMotorInputField(m, LiteMotorInput::IQ_CURRENT)] = 0;

if(gHandInstance->writeUserTable(DEV, table))
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if(gHandInstance->readUserTableValues(DEV, table))
    {
        //打印0x00食指的iq电流
        const auto iq0 = static_cast<int16_t>(
            table[liteMotorInputField(0, LiteMotorInput::IQ_CURRENT)]);
    }
}

// 写值：0x00食指电机的位置环 Kp
std::map<LiteUserTableField, uint16_t> writable;
writable[liteMotorParamField(0, LiteMotorParam::POS_KP)] = 100;
gHandInstance->writeUserTableValues(DEV, writable);
```

**注意**：

1. 写接口为单向发送，`true`代表已发出；执行结果需通过读回确认。

2. 读接口为阻塞式，失败返回`false`,且传入的map内容保持不变。

3. 返回的都是原始 `uint16_t`：Int16 字段需 `static_cast<int16_t>`，Uint32 字段需 `high << 16 | low`，角度/温度类字段需 ÷100。

## 8\. 错误处理与维护

### 8\.1 `clearError`

```C++
bool clearError(uint8_t deviceId);
```

**功能**：清除设备的堵转状态（错误位图见附录 B）。排除物理故障后调用，设备恢复正常执行能力。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：`true` 命令已发送；`false` 发送失败。

### 8\.2 `clearFirmwareError`

```C++
void clearFirmwareError(AdapterChannel channel, uint8_t fingerId);  // channel 版
void clearFirmwareError(uint8_t deviceId, uint8_t fingerId);        // deviceId 版
```

**功能**：与 8\.1 等价的基类兼容接口。`channel` 版内部以默认设备地址发送，`deviceId` 版以指定地址发送；`fingerId` 均被忽略。无返回值。

### 8\.3 `rebootDevice`

```C++
void rebootDevice(AdapterChannel channel, uint8_t deviceId);
void rebootDevice(AdapterChannel channel, uint8_t deviceId, LiteRebootMode mode);
```

**功能**：重启设备。发送后设备重启，重启期间不响应指令，需等待设备重新上线后再通讯。

两参数版本用于兼容基类接口，等价于整链复位，如需复位从机，可使用三参数版本。

|参数|类型|说明|
|---|---|---|
|`channel`|`AdapterChannel`|传 `AdapterChannel::CHNX`，被忽略|
|`deviceId`|`uint8_t`|设备从机地址|
|`mode`|`LiteRebootMode`|`WHOLE_CHAIN`整链复位；`SLAVE`从机复位|

**返回值**：无。命令为单向发送，不等待设备应答。

```C++
gHandInstance->rebootDevice(AdapterChannel::CHNX, DEV);                        // 整链复位
gHandInstance->rebootDevice(AdapterChannel::CHNX, DEV, LiteRebootMode::SLAVE); // 仅复位从机
```

### 8\.4 `resetJoints`

```C++
void resetJoints(AdapterChannel channel);   // 以默认设备地址0x01执行
void resetJoints(uint8_t deviceId);
```

**功能**：将全部 6 个手指关节复位到初始位置。内部对电机进行整帧控制，发送一帧霍尔模式（5模式）运动指令：四指目标位置 0、速度 10000、电流 100；拇指弯曲/侧摆目标位置 0、速度 15000、电流 100。不等待逐帧执行确认。

|参数|类型|说明|
|---|---|---|
|`channel`|`AdapterChannel`|传 `AdapterChannel::CHNX`，内部转默认设备地址|
|`deviceId`|`uint8_t`|设备从机地址|

**注意**：

1. 需先 `enable(DEV, true)`，否则复位指令不会被执行。

2. **使用该方法前请确保大拇指与四指的位置不会重叠，防止各关节手指在复位的时候出现“打架”现象，避免设备损坏。**

### 8\.5 `restore`

```Python
bool restore(uint8_t deviceId);
```

**功能**：恢复设备出厂设置，所有持久化参数恢复为出厂值。

|参数|类型|说明|
|---|---|---|
|`deviceId`|`uint8_t`|设备从机地址|

**返回值**：`true` 命令已发送；`false` 发送失败。

**注意**：恢复出厂后地址/波特率可能改变，当前实例或需按出厂参数重建，调用前请确认设备出厂默认值。

---

## 9\. 完整示例

```C++
#include <iostream>
#include <thread>
#include <chrono>
#include "DexHand.h"
#include "commondef.h"

using namespace DexRobot;

int main()
{
    // 1. 创建实例并连接（必须 listen=true）
    const auto hand = std::dynamic_pointer_cast<DexHand_021Lite>(
        DexHand::createInstance(ProductType::DX021_LITE, "COM3", BaudRate::BAUD_921600));
    if (nullptr == hand || !hand->connect(true))
    {
        std::cout << "Device not available." << std::endl;
        return -1;
    }

    std::vector<uint8_t> sysIds;
    hand->getSysIds(AdapterChannel::CHNX, sysIds);
    uint8_t DEV = sysIds.empty() ? 0x01 : sysIds.front();//通过广播ID获取具体从机地址。

    // 2. 探测在线并读取信息
    if (!hand->isAlive(AdapterChannel::CHNX, DEV))
    {
        std::cout << "Hand not online." << std::endl;
        hand->disconnect();
        return -1;
    }

    const auto master = gHandInstance->getFirmwareVersion(DEV, LITE_MASTER_VERSION); // 主控板
    const auto slave0 = gHandInstance->getFirmwareVersion(DEV, 0x00);                // 从机板
    const auto handType = hand->getHandType(DEV);   // 0x00左 / 0x01右 / 0x06未设置

    // 3. 上电回零（回零前设备拒绝电机控制命令）
    hand->homeThumb(DEV);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    hand->homeFingers(DEV);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 4. 使能并运动
    hand->enable(DEV, true);
    hand->moveFinger(DEV, 0x00, HALL_POSLIMIT_CONTROL_MODE, 1000, 10000, 100, 0, 0);

    // 5. 开启实时轮询并读取缓存
    hand->setRealtimeResponse(DEV, 50, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    hand->setStatusRxCallback([](const DX21StatusRxData * status){

     for (uint8_t fingerId = 0; fingerId < 6; ++fingerId)
     {
         std::cout << "fingerId= " << static_cast<int>(fingerId)
                   << "  iq="   << status->MotorCurrent(fingerId)
                   << "  rpm="  << status->MotorVelocity(fingerId)
                   << "  hall=" << status->MotorHallValue(fingerId)
                   << "  err=0x" << std::hex << status->MotorErrorStatus(fingerId)
                   << std::dec << std::endl;
     }
 });

    hand->setRealtimeResponse(DEV, 50, false);

    // 6. 断开
    hand->enable(DEV, false);
    hand->disconnect();
    return 0;
}
```

---

## 10\. 使用注意事项

1. `connect(true)` 必须传 `true`，否则阻塞读接口和后台轮询都收不到设备响应。

2. `getSafeTemperature()`、`getThumbDegree()` 等阻塞读接口执行期间会临时占用 ParamRW 回调通道，避免与用户回调逻辑相互依赖；`getHandStatus()` 只读缓存，不影响回调。

3. `setBaudRate` 成功后必须以新波特率重建实例；`restore` 后需确认设备出厂默认地址/波特率。

4. 写类接口返回 `true` 仅代表命令发送成功，设备执行结果需通过状态轮询或错误回调确认。

5. `deviceId` 为 Modbus 从机地址（整机地址），与 DX021 的手指板 ID 语义不同，不可混用。

6. 同一串口总线上多设备轮询时，注意请求/响应节奏，避免命令交叠。

7. **基类 ** **`DexHand`** 中以下接口在 Lite 中为占位实现，调用无效： **`setSafeCurrent`  /  `getSafeCurrent` 、 `setSafePressure`  / `getSafePressure`（返回  `false` /0）。**

8. CANFD 传输尚未为 021Lite 提供，后续版本补充；Python实现见Python用户接口文档。

9. **上电后必须先回零**：调用 `homeFingers(DEV)` / `homeThumb(DEV)` 之前，设备会拒绝电机控制命令（Modbus 异常码 0x0B）。

---

## 附录

### 附录 A：`BaudRate` 可选值

Windows 为本地枚举，Linux 为 `LibSerial::BaudRate` 别名，数值一致：

|枚举|波特率|
|---|---|
|`BAUD_115200`|115200|
|`BAUD_230400`|230400|
|`BAUD_460800`|460800|
|`BAUD_921600`|921600（构造默认值）|
|`BAUD_1000000`|1000000|
|`BAUD_2000000`|2000000|

### 附录 B：电机错误位 `MotorErrorCode_021Lite`

|位|值|含义|
|---|---|---|
|bit0|0x0001|温度|
|bit1|0x0002|过流|
|bit2|0x0004|过压|
|bit3|0x0008|堵转|
|bit4|0x0010|缺相|
|bit5|0x0020|角度|
|bit6|0x0040|霍尔偏置错误|
|bit7|0x0080|欠压|
|bit8|0x0100|短路|
|bit9|0x0200|失步|
|bit10|0x0400|编码器错误|
|bit11|0x0800|电流采样错误|

### 附录 C：Modbus 异常码 `R485LiteErrorCode`

|值|含义|
|---|---|
|0x01|功能码错误|
|0x02|非法地址|
|0x03|非法数据|
|0x04|从机故障|
|0x05|处理缓慢|
|0x06|从机忙|
|0x07|无响应|
|0x0B|不允许|

### 附录 D：用户自定义表字段码 `LiteUserTableField`

|**字段码**|**名称**|**含义**|**类型**|**可写**|
|---|---|---|---|---|
|0x00|DEVICE\_TYPE|设备类型|Uint16|否|
|0x01|MASTER\_APP\_VERSION|主控固件版本|Uint16|否|
|0x02|SERIAL\_NUMBER\_HIGH|设备序列号|Uint32 高 16 位|否|
|0x03|SERIAL\_NUMBER\_LOW||Uint32 低 16 位|否|
|0x04|SLAVE0\_APP\_VERSION|从机固件版本（食指）|Uint16|否|
|0x05|SLAVE1\_APP\_VERSION|从机固件版本（中指）|Uint16|否|
|0x06|SLAVE2\_APP\_VERSION|从机固件版本（无名指）|Uint16|否|
|0x07|SLAVE3\_APP\_VERSION|从机固件版本（小拇指）|Uint16|否|
|0x08|SLAVE4\_APP\_VERSION|从机固件版本（大拇指弯曲）|Uint16|否|
|0x09|SLAVE5\_APP\_VERSION|从机固件版本（大拇指侧摆）|Uint16|否|
|0x0A|BAUD\_RATE|波特率|Uint16（0\~5）|是|
|0x0B|SYS\_ID|设备ID|Uint16|是|
|0x0C|APP\_LENGTH\_HIGH|APP长度|Uint32 高 16 位|否|
|0x0D|APP\_LENGTH\_LOW||Uint32 低 16 位|否|
|0x0E|APP\_CHECKSUM\_HIGH|APP校验码|Uint32 高 16 位|否|
|0x0F|APP\_CHECKSUM\_LOW||Uint32 低 16 位|否|
|0x10|ANGLE\_D4\_DEG|大拇指弯曲角度解算值|Int16（角度×100）|否|
|0x11|ANGLE\_D5\_DEG|大拇指侧摆角度解算值|Int16（角度×100）|否|
|0x12|ANGLE\_D4\_RAW|大拇指弯曲角度原始值|Uint16|否|
|0x13|ANGLE\_D5\_RAW|大拇指侧摆角度原始值|Uint16|否|
|0x14|TEMP1|板温1|Uint16（原始值）|否|
|0x15|TEMP2|板温2|Uint16（原始值）|否|
|0x16|SYS\_STATUS|电机在线状态|Uint16 位图|否|
|0x17|MOTOR\_STATUS|电机运行状态|Uint16 位图|否|

**电机参数字段码**

motor 取值 0\~5：0/1/2/3 = 食指/中指/无名指/小指，4 = 拇指弯曲，5 = 拇指旋转。

|**param**|**名称**|**含义**|**可写**|
|---|---|---|---|
|0|POS\_KP|位置环 Kp|是|
|1|POS\_KI|位置环 Ki|是|
|2|POS\_KD|位置环 Kd|是|
|3|POS\_MAXOUT|位置环最大输出|是|
|4|POS\_MINOUT|位置环最小输出|是|
|5|POS\_MAXIOUT|位置环积分限幅|是|
|6|SPD\_KP|速度环 Kp|是|
|7|SPD\_KI|速度环 Ki|是|
|8|SPD\_KD|速度环 Kd|是|
|9|SPD\_MAXOUT|速度环最大输出|是|
|10|SPD\_MINOUT|速度环最小输出|是|
|11|SPD\_MAXIOUT|速度环积分限幅|是|
|12|MOTOR\_TYPE|电机类型|**否**|
|13|SAFE\_TEMP|保护温度 ×100|是|

**电机输入字段码**

|input|名称|含义|类型|备注|
|---|---|---|---|---|
|0|ID\_CURRENT|当前Id电流|Int16|（电流数值\-2000）/9\.1=实际毫安值|
|1|IQ\_CURRENT|当前Iq电流|Int16||
|2|RPM|当前转速|Int16||
|3|HALL|当前Hall位置|Uint16|当前Hall位置|
|4|HALL\_EST|Hall估算角度|Uint16|无效|
|5|ERR\_BIT|错误状态位图|Uint16 位图||

字段码可用 `liteMotorParamField(motor, param)` / `liteMotorInputField(motor, input)` 生成。

---

## 文档修订记录

|**修改时间**|**版本**|**修改内容**|
|---|---|---|
|2026\.09\.20|V1\.0|编写SDK 接口说明|
|2026\.09\.22|V1\.1|针对部分接口进行补充说明|
|2026\.09\.29|V1\.2|更新 getHandStatus 签名，新增 getSysStatus、getMotorStatus、getSerialNumber|
|2026\.10\.10|V1\.3|更新 moveFinger 参数类型与 getHandStatus、getSerialNumber 说明，新增 moveMultipleFingers|
|2026\.10\.10|V1\.4|更新 getTemperature 签名，支持读取两个温度传感器|
|2026\.10\.21|V1\.5|新增initializeThumb、三参数rebootDevice，resetJoints添加注意事项|
|2026\.10\.30|V1\.6|新增实时状态轮询setRealtimeResponsel getHandStatus改为读缓存状态，getThumbDegree更新参数类型为int16\_t，移除initializeThumb, 更新moveFinger中控制移动取值范围和电流取值范围|
|2026\.11\.3|V1\.7|更新4\.2控制模式命名，7\.5setBaudRate添加adapterType参数，8\.4resetJoints更新大拇指角度与电流参数|
|2026\.11\.6|V1\.8|新增homeFingers/homeThumb手指回位与用户自定义表读写接口，完整示例中补充回零步骤|
|2026\.11\.17|V1\.9|优化用户自定义表功能，新增附录D，提供用户可选的自定表所填的字段|



