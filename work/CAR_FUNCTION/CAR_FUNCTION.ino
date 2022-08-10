/*********
  https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
*********/

#include "WiFi.h"
#include "esp_timer.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include "driver/mcpwm.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <FS.h>

esp_err_t esp_err;

// LED pin
const int front_led_pin = 32;
const int back_led_pin = 33;
const int left_turn_led_pin = 21;
const int right_turn_led_pin = 22;
const int brake_led_pin = 23;

// encoder pin

const int encoder_pin = 2;
int count = 0;
float encoder_speed = 0.0;
int encoder_interval_ms = 500;

// motor pwm pin
const int motor_pwm_pin_A = 27;
const int motor_pwm_pin_B = 26;
// servo pwm pin
const int servo_pwm_pin = 13;

// Set your access point network credentials
// const char* ssid = "ESP32-Access-Point";
const char* ssid = "ZXHcar00";
const char* password = "Zha12345678";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// motor parameters
int motor_duty_cycle = 30;
int servo_turn_angle = 45;
float servo_duty_cycle_center = 7.5;
float servo_duty_cycle_differ = 5;

void toggle_light(int color);
void control_all_light(bool);
void move_forward();
void move_backward();
void motor_stop();
void turn_left();
void turn_right();
void straight();

void IRAM_ATTR count_add() {
  count += 1;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Car Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        { font-family: sans-serif; background: #eee; padding: 1rem; }
        body { max-width: 1200px; margin: 0 auto; background: white; }
        nav {
            background: rgb(50, 70, 99);
            display: flex;
            align-items: center;
            padding: 0 0.5rem;
            min-height: 4em;
        }
        nav h1 {
            flex: auto; margin: 0;
            color: #ffffff;
            font: 1em lucida-grande;
            font-size: 32px;
            font-weight: 1000;
            margin-left: 0.3em;
        }
        .content { padding: 0 1rem 1rem; }
        .content > header {
            /* border-bottom: 2px solid rgba(115, 133, 159, 0.5); */
            display: flex; align-items: flex-end;
            /* background-color: #9fb2bb; */
        }
        .content > header h1 {
            font: 1em lucida-grande;
            font-size: 24px;
            font-weight: 1000;
            color: #ff0000;
            flex: auto;
            margin: 1rem 0 0.3rem 0;
            margin-left: 0.3em;
        }
        .content p {
            margin: 5px;
            font-family: 'Courier New', Courier, monospace;
            font-size: 16px;
            font-weight: bold;
            line-height: 30px;
        }
        .content input[type=button] {
            align-self: start; min-width: 8em; min-height: 2em;
            font: 1em lucida-grande;
            font-size: 16px;
            font-weight: 1000;
            border: 0px;
            border-radius: 0.4em;
            background: rgba(115, 133, 159, 0.25);
        }
        .content input[type=button]:active {
            background: rgba(115, 133, 159, 0.507);
        }
        .keyboard{
            display: none;
        }
    </style>
</head>
<body>
<nav>
    <h1 align="center">ZXH car controller</h1>
</nav>

<section class="content">
    <header>
        <h1 align="center">MOTION</h1>
    </header>
    <p align="center">
        Real-Time Speed From Encoder: <span id="encoder_span">0.0</span>
    </p>
    <script>
        var xhttp_recorder = new XMLHttpRequest();
        xhttp_recorder.onreadystatechange = function() {
            if (xhttp_recorder.status === 200) {
                document.getElementById('encoder_span').innerHTML = this.responseText;
            }
        }
        function refresh_speed() {
            xhttp_recorder.open("GET", "/get_encoder");
            xhttp_recorder.send();
        }
        setInterval(refresh_speed, 200);
    </script>

    <p align="center">
        设置速度:
        <input class="slider" type="range" min="30" max="100" value="60" step="10" id="speed">
        <span id="speed_span"></span>
        <br>
        设置旋转角度:
        <input class="slider" type="range" min="15" max="45" value="15" step="30" id="angle">
        <span id="angle_span"></span>
        <style>
            input[type=range] {
                /*滑动条背景*/
                -webkit-appearance: none;
                background-color: rgba(115, 133, 159, 0.5);
                height: 8px;
                width: 100px;
            }
            input[type=range]::-webkit-slider-thumb {
                /*滑动条操作按钮样式*/
                -webkit-appearance: none;
                border-radius: 5px;
                background: rgb(255, 0, 0);
                width: 15px;
                height: 15px;
            }
        </style>
        <script>
            var xhttp = new XMLHttpRequest();
            document.getElementById('speed_span').innerHTML = 60;
            document.getElementById('angle_span').innerHTML = 15;
            var motor_speed = document.getElementById('speed');
            var servo_angle = document.getElementById('angle');
            var current;
            motor_speed.oninput = function() {
                current = this.value;
                document.getElementById('speed_span').innerHTML = current;
            }
            servo_angle.oninput = function() {
                current = this.value;
                document.getElementById('angle_span').innerHTML = current;
            }
            motor_speed.onchange = function() {
                current = this.value;
                xhttp.open("POST", "/change_speed?speed=" + current.toString());
                xhttp.send();
                console.log('change speed');
            }
            servo_angle.onchange = function() {
                current = this.value;
                xhttp.open("POST", "/change_turn_angle?angle=" + current.toString());
                xhttp.send();
                console.log('change turn angle');
            }
        </script>
    </p>
    <br>
    <div align="center">运动模式</div>
    <p align="center">
        <input type="button" name="Forward" value="Forward" id="forward" class = "forward">
        <input type="button" name="Stop" value="Stop" id="stop" class = "stop">
        <input type="button" name="Backward" value="Backward" id="backward" class = "backward">
    </p>
    <div align="center">运动方向</div>
    <p align="center">
        <input type="button" name="左转" value="Left" id="left" class = "left">
        <input type="button" name="直行" value="Straight" id="straight" class = "straight">
        <input type="button" name="右转" value="Right" id="right" class = "right">
    </p>
    <input type="text" onkeypress="keydown()" class = "keyboard">
    <script>
        // XMLHttpRequest 在不刷新页面的情况下请求特定 URL，获取数据
        var xhttp = new XMLHttpRequest();
        // button elements
        var forward_button = document.getElementById('forward');
        var backward_button = document.getElementById('backward');
        var stop_button = document.getElementById('stop');
        var left_button = document.getElementById('left');
        var right_button = document.getElementById('right');
        var straight_button = document.getElementById('straight');

        document.addEventListener("keydown",keydown);
        //键盘监听，注意：在非ie浏览器和非ie内核的浏览器
        //参数1：表示事件，keydown:键盘向下按；参数2：表示要触发的事件
        function keydown(event){
            //表示键盘监听所触发的事件，同时传递参数event
            switch(event.keyCode){
                case 38:
                    // alert("上键");
                    xhttp.open("POST", "/forward");
                    xhttp.send();
                    console.log('move forward');
                    break;
                case 40:
                    // alert("下键");
                    xhttp.open("POST", "/backward");
                    xhttp.send();
                    console.log('move backward');
                    break;
                case 83:
                    // alert("s键");
                    xhttp.open("POST", "/stop");
                    xhttp.send();
                    console.log('stop');
                    break;
                case 65:
                    // alert("a键");
                    xhttp.open("POST", "/left");
                    xhttp.send();
                    console.log('left');
                    break;
                case 68:
                    // alert("d键");
                    xhttp.open("POST", "/right");
                    xhttp.send();
                    console.log('right');
                    break;
                case 87:
                    // alert("w键");
                    xhttp.open("POST", "/straight");
                    xhttp.send();
                    console.log('straight');
                    break;
            }
        }

        forward_button.onclick = function() {
            xhttp.open("POST", "/forward");
            xhttp.send();
            console.log('move forward');
        }
        backward_button.onclick = function() {
            xhttp.open("POST", "/backward");
            xhttp.send();
            console.log('move backward');
        }
        stop_button.onclick = function() {
            xhttp.open("POST", "/stop");
            xhttp.send();
            console.log('stop');
        }
        left_button.onclick = function() {
            xhttp.open("POST", "/left");
            xhttp.send();
            console.log('left');
        }
        right_button.onclick = function() {
            xhttp.open("POST", "/right");
            xhttp.send();
            console.log('right');
        }
        straight_button.onclick = function() {
            xhttp.open("POST", "/straight");
            xhttp.send();
            console.log('straight');
        }
    </script>
</section>
</body>
</html>)rawliteral";

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  if(!WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 0, 0))){
      Serial.println("AP Config Failed");
  }
  WiFi.softAP(ssid, password, 1, 0, 10);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // set encoder interrupt
  pinMode(encoder_pin, INPUT);
  attachInterrupt(encoder_pin, count_add, RISING);

  // motor pwm config
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, motor_pwm_pin_A);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, motor_pwm_pin_B);
  mcpwm_config_t motor_pwm_config = {
    .frequency = 1000,
    .cmpr_a = 0,
    .cmpr_b = 0,
    .duty_mode = MCPWM_DUTY_MODE_0,
    .counter_mode = MCPWM_UP_COUNTER,
  };
  esp_err = mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &motor_pwm_config);
  if (esp_err == 0)
    Serial.println("Setting motor pwm success!");
  else {
    Serial.print("Setting motor pwm fail, error code: ");
    Serial.println(esp_err);
  }

  // servo pwm config
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, servo_pwm_pin);
  mcpwm_config_t servo_pwm_config;
  servo_pwm_config.frequency = 50;
  servo_pwm_config.cmpr_a = 0;
  servo_pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  servo_pwm_config.counter_mode = MCPWM_UP_COUNTER;
  esp_err = mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &servo_pwm_config);
  if (esp_err == 0)
    Serial.println("Setting servo pwm success!");
  else {
    Serial.print("Setting servo pwm fail, error code: ");
    Serial.println(esp_err);
  }
  mcpwm_start(MCPWM_UNIT_1, MCPWM_TIMER_1);

  // Route for web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/get_encoder", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(encoder_speed));
  });
  server.on("/change_speed", HTTP_POST, [](AsyncWebServerRequest * request) {
    motor_duty_cycle = request->getParam("speed")->value().toInt();
    request->send(200);
  });
  server.on("/change_turn_angle", HTTP_POST, [](AsyncWebServerRequest * request) {
    servo_turn_angle = request->getParam("angle")->value().toInt();
    if (servo_turn_angle == 45) servo_duty_cycle_differ = 5;
    else servo_duty_cycle_differ = 1.5;
    request->send(200);
  });
  server.on("/forward", HTTP_POST, [](AsyncWebServerRequest * request) {
    move_forward();
    request->send(200);
  });
  server.on("/backward", HTTP_POST, [](AsyncWebServerRequest * request) {
    move_backward();
    request->send(200);
  });
  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(back_led_pin, LOW);
    motor_stop();
    request->send(200);
  });
  server.on("/left", HTTP_POST, [](AsyncWebServerRequest * request) {
    turn_left();
    request->send(200);
  });
  server.on("/right", HTTP_POST, [](AsyncWebServerRequest * request) {
    turn_right();
    request->send(200);
  });
  server.on("/straight", HTTP_POST, [](AsyncWebServerRequest * request) {
    straight();
    request->send(200);
  });
  // Start server
  server.begin();
}

void loop() {
  count = 0;
  delay(encoder_interval_ms);
  encoder_speed = count / 18.0 / 21 * 6.2 * 3.14 * 1000 / encoder_interval_ms;
}

// some functions
void move_forward() {
  Serial.println("--- move forward...");
  mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, motor_duty_cycle);
  mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
}
void move_backward() {
  mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, motor_duty_cycle);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
  mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
  Serial.println("--- move backward...");
}
void motor_stop() {
  Serial.println("--- motor stop...");
  mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
  mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
}
void turn_left() {
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, servo_duty_cycle_center - servo_duty_cycle_differ);
}
void turn_right() {
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, servo_duty_cycle_center + servo_duty_cycle_differ);
}
void straight() {
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, servo_duty_cycle_center);
}
