/*********
  https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
  https://github.com/espressif/arduino-esp32/issues/1313
  https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-timer.c
*********/

#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "time.h"
#include <WiFiUdp.h>
#include "driver/timer.h"

// Replace with your network credentials
const char* ssid = "ZXHcar00";
const char* password = "Zha12345678";
// Set your Static IP address
IPAddress local_IP(192, 168, 4, 5);
// Set your Gateway IP address
IPAddress gateway(192, 168, 4, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean takeNewPhoto = false;
String lastPhoto = "";
String list = "";
hw_timer_t * timer = NULL;
hw_timer_t * timer_1 = NULL;
boolean isRecording = false;

// HTTP GET parameter
const char* PARAM_INPUT_1 = "photo";
const char* PARAM_INPUT_2 = "record_time";
const char* PARAM_INPUT_3 = "record_interval";

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Stores the camera configuration parameters
camera_config_t config;

File root;

void setup() {
  // Turn-off the brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  
  Serial.println("Initializing the camera module...");
  configInitCamera();
  
  Serial.println("Initializing the MicroSD card module... ");
  initMicroSDCard();
  
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (takeNewPhoto) {
      request->send_P(200, "text/plain", "");
    } else {
      camera_fb_t * frame = esp_camera_fb_get();
      request->send_P(200, "image/jpeg", (const uint8_t *)frame->buf, frame->len);
      esp_camera_fb_return(frame);
      frame = NULL;
    }
  });
  
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", list.c_str());
  });
  
  server.on("/view", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/view?photo=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = "/" + request->getParam(PARAM_INPUT_1)->value();
      Serial.print("Trying to open ");
      Serial.println(inputMessage);
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(SD_MMC, inputMessage, "image/jpg", false);
  });
  
  // Send a GET request to <ESP_IP>/delete?photo=<inputMessage>
  server.on("/delete", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/delete?photo=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = "/" + request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    deleteFile(SD_MMC, inputMessage.c_str());
    request->send(200, "text/html", "Done. Your photo named " + inputMessage + " was removed." +
                                     "<br><a href=\"/list\">view/delete other photos</a>.");
  });

  server.on("/record", HTTP_GET, [](AsyncWebServerRequest * request) {
    String inputMessage_2;
    String inputMessage_3;
    // GET input1 value on <ESP_IP>/view?photo=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage_2 = request->getParam(PARAM_INPUT_2)->value();
      if (request->hasParam(PARAM_INPUT_3)) {
        Serial.println("Start record... Can't get ip camera stream now.");
        inputMessage_3 = request->getParam(PARAM_INPUT_3)->value();
        take_save_record(inputMessage_2.toInt(), inputMessage_3.toInt());
      }
      else {
        inputMessage_3 = "Request on /record lacks Param 1!";
        Serial.println(inputMessage_3);
      }
    }
    else {
      inputMessage_2 = "Request on /record lacks Param 2!";
      Serial.println(inputMessage_2);
    }
  });
  
  server.on("/stop_record", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Stop record. You can get ip camera stream now.");
    stop_record();
  });
  
  // Start server
  server.begin();
  
  root = SD_MMC.open("/");
  listDirectory(SD_MMC);
}

void loop() {
  if (takeNewPhoto) {
    takeSavePhoto();
    takeNewPhoto = false;
  }
  delay(1);
}

void configInitCamera(){
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // YUV422,GRAYSCALE,RGB565,JPEG

  config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10; //0-63 lower number means higher quality
  config.fb_count = 2;

//  // Select lower framesize if the camera doesn't support PSRAM
//  if(psramFound()){
//    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
//    config.jpeg_quality = 10; //0-63 lower number means higher quality
//    config.fb_count = 2;
//  } 
//  else {
//    config.frame_size = FRAMESIZE_SVGA;
//    config.jpeg_quality = 12;
//    config.fb_count = 1;
//  }
  
  // Initialize the Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void initMicroSDCard(){
  // Start Micro SD card
  Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
}

void takeSavePhoto(){
  struct tm timeinfo;
  char now[20];
  // Take Picture with Camera
  camera_fb_t  * fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  } 
  // Path where new picture will be saved in SD Card
  getLocalTime(&timeinfo);
  strftime(now, 20, "%Y%m%d_%H%M%S", &timeinfo); // Format Date & Time
  String path = "/photo_" + String(now) +".jpg";
  lastPhoto = path;
  Serial.printf("Picture file name: %s\n", path.c_str());
  // Save picture to microSD card
  fs::FS &fs = SD_MMC; 
  File file = fs.open(path.c_str(),FILE_WRITE);
  if(!file){
    Serial.printf("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf(" Saved: %s\n", path.c_str());
    listDirectory(SD_MMC);
  }
  file.close();
  esp_camera_fb_return(fb); 
}

void IRAM_ATTR onTimer(){
  takeNewPhoto = true;
}

void IRAM_ATTR onTimer1() {
  Serial.println("Recording is complete. You can get ip camera stream now.");
  if (isRecording) {
    isRecording = false;
    //timer_disable_intr(TIMER_GROUP_0, TIMER_0);
    if (timer != NULL) {
      timerAlarmDisable(timer);
      timerDetachInterrupt(timer);
      timerEnd(timer);
      timer = NULL;
    }
    if (timer_1 != NULL) {
      timerAlarmDisable(timer_1);
      timerDetachInterrupt(timer_1);
      timerEnd(timer_1);
      timer_1 = NULL;
    }
  }
}

void take_save_record(long duration, long interval) {
  Serial.print("Duration: ");
  Serial.print(duration);
  Serial.print(" minute, Interval: ");
  Serial.print(interval);
  Serial.println(" s.");
  isRecording = true;
  if (timer == NULL) {
    timer = timerBegin(0, 40, true);
    // Attach onTimer function to our timer
    timerAttachInterrupt(timer, &onTimer, true);
  }
  /* Set alarm to call onTimer function every second 1 tick is 1us
  => 1 second is 1000000us
  Repeat the alarm (third parameter) */
  timerAlarmWrite(timer, interval * 1000000, true);
  /* Start an alarm */
  yield();
  timerAlarmEnable(timer);
  
  timer_1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer_1, &onTimer1, true);
  timerAlarmWrite(timer_1, duration * 60 * 1000000, false);
  yield();
  timerAlarmEnable(timer_1);
}

void stop_record() {
  if (isRecording) {
    isRecording = false;
    //timer_disable_intr(TIMER_GROUP_0, TIMER_0);
    if (timer != NULL) {
      timerAlarmDisable(timer);
      timerDetachInterrupt(timer);
      timerEnd(timer);
      timer = NULL;
    }
    if (timer_1 != NULL) {
      timerAlarmDisable(timer_1);
      timerDetachInterrupt(timer_1);
      timerEnd(timer_1);
      timer_1 = NULL;
    }
  }
}

void listDirectory(fs::FS &fs) {
  File root = fs.open("/");
  list = "";
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(!file.isDirectory()){
      String filename=String(file.name());
      filename.toLowerCase();
      if (filename.indexOf(".jpg")!=-1){
        list = "<tr><td><button onclick=\"window.open('/view?photo="+String(file.name())+"','_blank')\">View</button></td><td><button onclick=\"window.location.href='/delete?photo="+String(file.name())+"'\">Delete</button></td><td>"+String(file.name())+"</td><td></td></tr>"+list;
      }
    }
    lastPhoto = file.name();
    file = root.openNextFile();
  }
  
  if (list=="") {
    list="<tr>No photos Stored</tr>";
  }
  else {
    list="<h1>ESP32-CAM View and Delete Photos</h1><table><th colspan=\"2\">Actions</th><th>Filename</th>"+list+"</table>";
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){  
    Serial.println("File deleted");
    listDirectory(SD_MMC);
  } 
  else {
    Serial.println("Delete failed");
  }
}
