/*********
  https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
  https://github.com/me-no-dev/ESPAsyncWebServer
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
#include <stdio.h>
#include "MPU6050_tockn.h"
#include <Wire.h>

esp_err_t esp_err;

// LED pin
const int left_turn_led_pin = 32;
const int right_turn_led_pin = 33;

// encoder pin
const int encoder_pin = 2;

// motor pwm pin
const int motor_pwm_pin_A = 27;
const int motor_pwm_pin_B = 26;
// servo pwm pin
const int servo_pwm_pin = 13;

// Set your access point network credentials
const char* ssid = "saya";
const char* password = "wasd1234";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// mpu6050
const int mpu_int = 5;
MPU6050 mpu6050(Wire);

// motor parameters
int motor_duty_cycle = 30;
int servo_turn_angle = 45;
float servo_duty_cycle_center = 7.5;
float servo_duty_cycle_differ = 1.5;
float servo_duty_cycle;

// speed
int count = 0;
float car_speed = 0.0;
void IRAM_ATTR count_add() {
  count += 1;
}

// angle
float z_angle = 0.0;

// local time
unsigned long old_millis;

// function statement
void left_light_control(bool state);
void right_light_control(bool state);
void control_all_light(bool);
void motor_control(int motor_speed);
void servo_control(int servo_angle);
// void mpu6050_reinit();

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  if(!WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 0, 0))){
      Serial.println("AP Config Failed");
  }
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // set led pinmode
  pinMode(left_turn_led_pin, OUTPUT);
  pinMode(right_turn_led_pin, OUTPUT);

  // encoder interrupt
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

/*
  Web Serve
*/
  // 灯光控制
   server.on("/left_light_control", HTTP_POST, [](AsyncWebServerRequest * request) {
    // 左转向灯
    boolean state = request->getParam("state")->value().toInt();
    state = !state;
    left_light_control(state);
    request->send(200);
  });
  server.on("/right_light_control", HTTP_POST, [](AsyncWebServerRequest * request) {
    // 右转向灯
    boolean state = request->getParam("state")->value().toInt();
    state = !state;
    right_light_control(state);
    request->send(200);
  });

  // 读取速度
  server.on("/read_speed", HTTP_GET, [](AsyncWebServerRequest * request) {
    // 发送两个车轮的速度
    char speed_str[6];
    sprintf(speed_str, "%f", car_speed);
    String l = speed_str;
    request->send(200, "text/plain", l);
  });
  // 读取角度
  server.on("/read_angle", HTTP_GET, [](AsyncWebServerRequest * request) {
    // 发送两个车轮的速度
    char z_angle_str[6];
    sprintf(z_angle_str, "%f", z_angle);
    String z = z_angle_str;
    request->send(200, "text/plain", z);
  });

  // 准确控制
  server.on("/motor_control", HTTP_POST, [](AsyncWebServerRequest * request) {
    motor_duty_cycle = request->getParam("speed")->value().toInt();
    motor_control(motor_duty_cycle);
    request->send(200);
  });
  server.on("/servo_control", HTTP_POST, [](AsyncWebServerRequest * request) {
    servo_turn_angle = request->getParam("angle")->value().toInt();
    servo_control(servo_turn_angle);
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
  
  pinMode(mpu_int, OUTPUT);
  digitalWrite(mpu_int, HIGH);
  Wire.begin(16, 17);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  control_all_light(true);
  delay(500);
  control_all_light(false);
  delay(500);
  control_all_light(true);
  delay(500);
  control_all_light(false);

  // set initial time
  old_millis = millis();
}

void loop() {
  mpu6050.update();
  z_angle = mpu6050.getAngleZ();

  delay(50);

  if(millis() - old_millis >= 500){
    car_speed = (count / 18.0 / 21 * 6.2 * 3.14 * 1000 / 500)* 0.75;
    count = 0;
    old_millis = millis();
  }
}

// some functions
void left_light_control(bool state) {
  digitalWrite(left_turn_led_pin, state);  
}
void right_light_control(bool state) {
  digitalWrite(right_turn_led_pin, state);  
}

int servo_angle_to_duty_cycle(int angle) {
  return angle/18.0 + 2.5;
}

void motor_control(int motor_speed) {
  if (motor_speed > 0) {
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, motor_speed);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    Serial.print("speed: ");
    Serial.println(motor_speed);
  }
  else if (motor_speed < 0) {
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, -motor_speed);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    Serial.print("speed: ");
    Serial.println(motor_speed);
  }
  else {
    mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 100);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 100);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    Serial.println("stop");
  }
}

void servo_control(int servo_angle) {
  servo_duty_cycle = servo_angle_to_duty_cycle(servo_angle);
  Serial.print("angle: ");
  Serial.println(servo_angle);
  mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, servo_duty_cycle);
}

void control_all_light(bool flag) {
  digitalWrite(left_turn_led_pin, !flag);
  digitalWrite(right_turn_led_pin, !flag);
}
