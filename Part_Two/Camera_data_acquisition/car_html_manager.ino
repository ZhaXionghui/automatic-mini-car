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
const char* ssid = "ZXHcar01";
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
<!DOCTYPE HTML>
<html>
<head>
  <title>Mini-Car Controller</title>
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
</style>
</head>
<body>
  <nav>
    <h1 align="center">Mini car controller</h1>
  </nav>
  <section class="content">
    <header>
      <h1 align="center">Camera</h1>
    </header>
    <p align="center">
      <img src="http://192.168.4.5/capture" id="photo" width=300em>
      <br>
      <input type="button" id="start_stream" name="Start Stream" value="Start Stream">
      <input type="button" id="stop_stream" name="Stop Stream" value="Stop Stream">
    </p>
    <script>
      var isStreaming = true;
      var isRecording = false;
      document.getElementById('start_stream').onclick = function() {
        isStreaming = true;
      }
      document.getElementById('stop_stream').onclick = function() {
        isStreaming = false;
      }
      function refresh_img() {
        if (isStreaming && (!isRecording)) {
          document.getElementById("photo").src = "http://192.168.4.5/capture"
                                                 + '?_=' + (new Date()).getTime();
        }
      }
      setInterval(refresh_img, 2000);
    </script>
    <br>
    <p align="center">
      Set Record Time (in minute):
      <input class="slider" type="range" min="1" max="10" value="2" step="1" id="record_time">
      <span id="record_time_span"></span>
      <br>
      Set Record Interval (in second):
      <input class="slider" type="range" min="5" max="20" value="5" step="1" id="record_interval">
      <span id="record_interval_span"></span>
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
        document.getElementById('record_time_span').innerHTML = 2;
        document.getElementById('record_interval_span').innerHTML = 5;
        var record_time = document.getElementById('record_time');
        var record_interval = document.getElementById('record_interval');
        var current;
        record_time.oninput = function() {
          current = this.value;
          document.getElementById('record_time_span').innerHTML = current;
        }
        record_interval.oninput = function() {
          current = this.value;
          document.getElementById('record_interval_span').innerHTML = current;
        }
      </script>
    </p>

    <p align="center">
      <input type="button" name="Start Record" value="Start Record" id="start_record">
      <input type="button" name="Stop Record" value="Stop Record" id="stop_record">
      <script>
        // XMLHttpRequest 在不刷新页面的情况下请求特定 URL，获取数据
        var xhttp = new XMLHttpRequest();
        document.getElementById('start_record').onclick = function() {
          xhttp.open("GET", "http://192.168.4.5/record?record_time="
                            + document.getElementById('record_time_span').innerHTML.toString() 
                            + "&record_interval=" 
                            + document.getElementById('record_interval_span').innerHTML.toString() 
          );
          xhttp.send();
          isRecording = true;
          function set_isRecording_false() {
            isRecording = false;
            console.log("Stop record. You can get ip camera stream now.");
          }
          setTimeout(set_isRecording_false, document.getElementById('record_time_span').innerHTML * 60 * 1000);
          console.log("Start record... Can't get ip camera stream now.");
        }
        document.getElementById('stop_record').onclick = function() {
          isRecording = false;
          xhttp.open("GET", "http://192.168.4.5/stop_record");
          xhttp.send();
          console.log("Stop record. You can get ip camera stream now.");
        }
      </script>
    </p>
  </section>
  
  <section class="content">
    <header>
        <h1 align="center">Light</h1>
    </header>
    <p align="center">
      <input type="button" name="Front Light" value="Front Light" id="front_light">
      <input type="button" name="Brake Light" value="Brake Light" id="brake_light">
    </p>
    <script>
      var xhttp = new XMLHttpRequest();
      document.getElementById('front_light').onclick = function() {
        xhttp.open("POST", "/front_light");
        xhttp.send();
        console.log('toggle front light');
      }
      document.getElementById('brake_light').onclick = function() {
        xhttp.open("POST", "/back_light");
        xhttp.send();
        console.log('toggle back light');
      }
    </script>
    <br>
    
    <header>
        <h1 align="center">Move</h1>
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
      Set Speed:
      <input class="slider" type="range" min="30" max="100" value="60" step="10" id="speed">
      <span id="speed_span"></span>
      <br>
      Set Turning Angle:
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
    
    <p align="center">
      <input type="button" name="Forward" value="Forward" id="forward">
      <input type="button" name="Stop" value="Stop" id="stop">
      <input type="button" name="Backward" value="Backward" id="backward">
    </p>
    <p align="center">
      <input type="button" name="Left" value="Left" id="left">
      <input type="button" name="Straight" value="Straight" id="straight">
      <input type="button" name="Right" value="Right" id="right">
    </p>
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

  // set led pinmode
  pinMode(front_led_pin, OUTPUT);
  pinMode(back_led_pin, OUTPUT);
  pinMode(left_turn_led_pin, OUTPUT);
  pinMode(right_turn_led_pin, OUTPUT);
  pinMode(brake_led_pin, OUTPUT);

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
  
  server.on("/front_light", HTTP_POST, [](AsyncWebServerRequest * request) {
    toggle_light(1);
    request->send(200);
  });
  server.on("/back_light", HTTP_POST, [](AsyncWebServerRequest * request) {
    toggle_light(2);
    request->send(200);
  });
  server.on("/get_encoder", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(encoder_speed));
//    request->send_P(200, "text/plain", "123");
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
//    digitalWrite(back_led_pin, LOW);
    move_forward();
    request->send(200);
  });
  server.on("/backward", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(back_led_pin, HIGH);
    move_backward();
    request->send(200);
  });
  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(back_led_pin, LOW);
    motor_stop();
    request->send(200);
  });
  server.on("/left", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(left_turn_led_pin, LOW);
//    digitalWrite(right_turn_led_pin, HIGH);
    turn_left();
    request->send(200);
  });
  server.on("/right", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(left_turn_led_pin, HIGH);
//    digitalWrite(right_turn_led_pin, LOW);
    turn_right();
    request->send(200);
  });
  server.on("/straight", HTTP_POST, [](AsyncWebServerRequest * request) {
//    digitalWrite(left_turn_led_pin, HIGH);
//    digitalWrite(right_turn_led_pin, HIGH);
    straight();
    request->send(200);
  });
  // Start server
  server.begin();

  control_all_light(true);
  delay(500);
  control_all_light(false);
  delay(500);
  control_all_light(true);
  delay(500);
  control_all_light(false);
}

void loop() {
  count = 0;
  delay(encoder_interval_ms);
  encoder_speed = count / 18.0 / 21 * 6.2 * 3.14 * 1000 / encoder_interval_ms;
//  Serial.print("Speed: ");
//  Serial.println(encoder_speed);
}

/*
const int front_led_pin = 21;
const int back_led_pin = 22;
const int left_turn_led_pin = 32;
const int right_turn_led_pin = 33;
const int brake_led_pin = 23;
*/

// some functions
void toggle_light(int color) {
  if (color == 1) {
    bool state = digitalRead(front_led_pin);
    digitalWrite(front_led_pin, !state);
  }
  else if (color == 2) {
    bool state = digitalRead(back_led_pin);
    digitalWrite(back_led_pin, !state);
  }
}
void control_all_light(bool flag) {
  digitalWrite(front_led_pin, !flag);
  digitalWrite(back_led_pin, !flag);
  digitalWrite(left_turn_led_pin, flag);
  digitalWrite(right_turn_led_pin, flag);
  digitalWrite(brake_led_pin, flag);
}

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
