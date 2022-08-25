# automatic-mini-car with esp32-cam
# 智能小车



## 小车运动



### Arduino

#### 下载

##### 方法1：

Arduino 官网：[Software | Arduino](https://www.arduino.cc/en/software)

##### 方法2（推荐）：

> path:\work\SETTING\arduino-1.8.16-windows.exe

版本: arduino-1.8.16-windows

![](./work/pictures/1.png)
![](./work/pictures/2.png)

#### 配置ESP32开发环境

##### 方法1：

①文件>>首选项>>附加开发板管理器网址：https://dl.espressif.com/dl/package_esp32_index.json

②工具>>开发板>>开发板管理器>>esp32（选择版本进行安装）



##### 方法2（推荐）：

> path:\work\SETTING\arduino_conf

两个文件夹 ：AppData； Program Files (x86)

**AppData**

将`AppData\Local\Arduino15`中的文件覆盖到`C:\Users\ZXH\AppData\Local\Arduino15`中

`C:\Users\ZXH`可能无法查看到AppData文件夹。点击右上角查看勾选隐藏的项目即可

**Program Files (x86)**

将`Program Files (x86)\AppData`文件夹覆盖`C:\Program Files (x86)\AppData`文件夹

将`Program Files (x86)\Arduino`文件夹覆盖`C:\Program Files (x86)\Arduino`文件夹



### 运动

> path:\work\MOTION

#### 前进

##### 方法1：PWM控制电机

PWM

- frequency（频率）

 frequency = n

周期 T = $\frac{1}{n}$ 

eg: frequency = 50 Hz

​		T = 0.02s

- resolution（分辨率，占空比精度）

eg: resolution = 8 ($2^8$=256)

占空比取值0~255

- duty_cycle（占空比）

占空比 = $\frac{高电平}{周期}$

所以通过控制**调整占空比**就能控制PWM信号从而能过**控制电机的转速**或者是**控制LED灯的亮度**，亦或是**控制舵机的旋转角度**

![](./work/pictures/3.png)

A，B 接收PWM信号

- channel（16个通道）

A:LOW 	 B:HIGH   A---->>>B

A:HIGH	  B:LOW   A<<<----B

A:HIGH 	 B:HIGH   A---------B

A:LOW  	 B:LOW   A<<<>>>B



```c++
// PWM控制电机
// path:\work\MOTION\PWM_FORWARD\PWM_FORWARD.ino
const int a = 26;
const int b = 27;
// pwm信号设置
// 频率 2000Hz
const int frequency = 2000;
// 分辨率（精确度） 8 2^8=256
const int resolution = 8;
// 16个信号通道 
const int channel_a = 0;
const int channel_b = 1;
// 占空比（7~32）
const int duty_cycle = 128;

void setup(){
  ledcSetup(channel_a,frequency,resolution);
  ledcAttachPin(a,channel_a);
  ledcSetup(channel_b,frequency,resolution);
  ledcAttachPin(b,channel_b);
}

void loop(){
  ledcWrite(channel_a,duty_cycle);
  ledcWrite(channel_b,0);
  delay(10000);
}
```



##### 方法2：MCPWM控制电机

![](./work/pictures/4.png)



```c++
// MCPWM控制电机
// path:\work\MOTION\MCPWM_FORWARD\MCPWM_FORWARD.ino
#include "driver/mcpwm.h"

void setup(){
  // motor
  // 用选定的MCPWM_UNIT_0来初始化gpio口
  mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0A,26);
  mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0B,27);
  // 通过mcpwm_config_t结构体为定时器设置频率和初始值
  mcpwm_config_t motor_pwm_config = {
    .frequency = 1000, // .frequency = 1000 -- motor_pwm_config.frequency = 1000
    .cmpr_a = 0,  // a 的占空比--%
    .cmpr_b = 0,  // b 的占空比--%
    .duty_mode = MCPWM_DUTY_MODE_0,   // 占空比模式（高电平）
    .counter_mode = MCPWM_UP_COUNTER, // 计数器模式（上位计数）
  };
  // 使用以上设置配置PWM0A和PWM0B
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &motor_pwm_config);
}

void loop(){
  // motor
  // 前进
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);  // 设置占空比（mcpwm单元,定时器,操控A,占空比%）
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
}
```



#### 后退

##### 方法1：PWM控制电机

```c++
// PWM控制电机
// path:\work\MOTION\PWM_BACK\PWM_BACK.ino
const int a = 26;
const int b = 27;
// pwm信号设置
// 频率 2000Hz
const int frequency = 2000;
// 分辨率（精确度） 8 2^8=256
const int resolution = 8;
// 16个信号通道 
const int channel_a = 0;
const int channel_b = 1;
// 占空比（7~32）
const int duty_cycle = 128;

void setup(){
  ledcSetup(channel_a,frequency,resolution);
  ledcAttachPin(a,channel_a);
  ledcSetup(channel_b,frequency,resolution);
  ledcAttachPin(b,channel_b);
}

void loop(){
  ledcWrite(channel_b,duty_cycle);
  ledcWrite(channel_a,0);
  delay(10000);
}
```



##### 方法2：MCPWM控制电机

```c++
// MCPWM控制电机
// path:\work\MOTION\MCPWM_BACK\MCPWM_BACK.ino
#include "driver/mcpwm.h"

void setup(){
  // motor
  // 用选定的MCPWM_UNIT_0来初始化gpio口
  mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0A,26);
  mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0B,27);
  // 通过mcpwm_config_t结构体为定时器设置频率和初始值
  mcpwm_config_t motor_pwm_config = {
    .frequency = 1000, // .frequency = 1000 -- motor_pwm_config.frequency = 1000
    .cmpr_a = 0,  // a 的占空比--%
    .cmpr_b = 0,  // a 的占空比--%
    .duty_mode = MCPWM_DUTY_MODE_0,   // 占空比模式（高电平）
    .counter_mode = MCPWM_UP_COUNTER, // 计数器模式（上位计数）
  };
  // 使用以上设置配置PWM0A和PWM0B
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &motor_pwm_config);
}

void loop(){
  // motor
  // 后退
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);  // 设置占空比（mcpwm单元,定时器,操控A,占空比%）
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
}
```



#### 左转/右转/直行

##### PWM/MCPWM控制舵机

![](./work/pictures/5.png)



- PWM

channel = 0

frequency = 50 (T=0.02s=20ms)

resolution = 8

![](./work/pictures/6.png)

0.5ms--->>>0°			1ms--->>>45°		1.5ms--->>>90°

2ms--->>>135°			2.5ms--->>>180°

占空比

0°

$\frac{0.5}{20}$ = 2.5%


256*2.5% = 6.4 $\approx$ 7

90°

$\frac{1.5}{20}$ = 7.5%

256* 7.5% = 19.2 $\approx$ 20

180°

$\frac{2.5}{20}$ = 12.5%

256*12.5% = 32



- MCPWM

0°: $\frac{0.5}{20}$ = 2.5%

90°: $\frac{1.5}{20}$ = 7.5%

180°: $\frac{2.5}{20}$ = 12.5%



##### MCPWM控制电机

```c++
// MCPWM控制电机
// path:\work\MOTION\MCPWM_DIRECTION\MCPWM_DIRECTION.ino
#include "driver/mcpwm.h"

void setup(){
  // servo
  Serial.begin(115200);
  // 用选定的MCPWM_UNIT_1来初始化gpio口
  mcpwm_gpio_init(MCPWM_UNIT_1,MCPWM1A,13);
  // 通过mcpwm_config_t结构体为定时器设置频率和初始值
  mcpwm_config_t servo_pwm_config;
  servo_pwm_config.frequency = 50;
  servo_pwm_config.cmpr_a = 0; // a 的占空比--%
  servo_pwm_config.duty_mode = MCPWM_DUTY_MODE_0;   // 占空比模式（高电平）
  servo_pwm_config.counter_mode = MCPWM_UP_COUNTER; // 计数器模式（上位计数）
  // 使用以上设置配置PWM1A
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &servo_pwm_config);
}

void loop(){
  Serial.println("Setting servo pwm success!");
  // servo
  // mcpwm_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
  // 直行
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, 7.5);  // 设置占空比（mcpwm单元,定时器,操控A,占空比% （2.5%~12.5%））90度
  delay(2000);
  // 左转
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, 2.5); // 0度 2.5%
  delay(2000);
  // 直行
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, 7.5); // 90度 7.5%
  delay(2000);
  // 右转
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, 12.5); // 180度 12.5%
  delay(2000);
}
```



#### 整合

```c++
// path:\work\CAR_FUNCTION\CAR_FUNCTION.ino
```
