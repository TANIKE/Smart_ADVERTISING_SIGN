#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
//flag for saving data
bool shouldSaveConfig = false;
////////////////////////////////////
#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
////////////////////////////////////
#include "Display.h"
////////////////////////////////////
//Variables for connecting with FireBase
String wifi_ssid;
String wifi_pass;
char USER_MAIL[30];
char USER_MAIL_PASS[20];
char api_key[40];
char bucket_id[30];

// Photo File Name to save in LittleFS
String FILE_PHOTO_PATH = ""; 
String BUCKET_PHOTO = "";
int32_t count_photos = 0;

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

//Variables for Workflow
bool takeNewPhoto = true;
bool taskCompleted = false;

//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

// Capture Photo and Save it to LittleFS
void capturePhotoSaveLittleFS( void ) {
  // Dispose first pictures because of bad quality
  camera_fb_t* fb = NULL;
  // Skip first 3 frames (increase/decrease number as needed).
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
    
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }  

  // Photo file name, write and rewrite in the same Path
  Serial.printf("Picture file name: %s\n", "/photo.jpg");
  File file = LittleFS.open("/photo.jpg", FILE_WRITE);

  // Insert the data in the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.print("Saved in ");
    Serial.print("/photo.jpg");
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  // Close the file
  file.close();
  esp_camera_fb_return(fb);
}

void initCamera(){
 // OV2640 camera module
  camera_config_t config;
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
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void initFirebase(){
  //Firebase
  String wifi_ssid;
  String wifi_pass;

  // Assign the api key
  configF.api_key = api_key;
  //Assign the user sign in credentials
  auth.user.email = USER_MAIL;
  auth.user.password = USER_MAIL_PASS;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}
void take_sendPicture(){
  /*
  First create paths to save PICTURE in LittleFS and Firebase
  Then take Photo ---> Send it to Firebase
  */
  FILE_PHOTO_PATH = "/photo" + String(count_photos) + ".jpg";
  BUCKET_PHOTO = "/data/photo" + String(count_photos) + ".jpg";
  if (takeNewPhoto) {
    capturePhotoSaveLittleFS();
    takeNewPhoto = false;
  }
  delay(1);
  if (Firebase.ready() && !taskCompleted){
    taskCompleted = true;
    Serial.print("Uploading picture... ");

    if (Firebase.Storage.upload(&fbdo, bucket_id /* Firebase Storage bucket id */, "/photo.jpg" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, BUCKET_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */)){
      //Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
      count_photos++;
    }
    else{
      Serial.println(fbdo.errorReason());
    }
  }
  takeNewPhoto = true;
  taskCompleted = false;
}
///////////////////////////////////

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
bool loadData(){
  if(LittleFS.exists("/config.json")){
    File configfile = LittleFS.open("/config.json", "r");
    if(!configfile){
      Serial.println("Failed to load File");
      return false;
    }
    size_t size = configfile.size();
    if (size > 1024) {
      Serial.println("Data file size is too large");
      return false;
    }
    std::unique_ptr<char[]> buf(new char[size]);
    configfile.readBytes(buf.get(), size);

    StaticJsonDocument<100> json;
    auto deserializeError = deserializeJson(json, buf.get());
    serializeJson(json, Serial);

    Serial.println("\nparsed json");
    strcpy(USER_MAIL, json["USER_MAIL"]);
    strcpy(USER_MAIL_PASS, json["USER_MAIL_PASS"]);
    strcpy(api_key, json["api_key"]);
    strcpy(bucket_id, json["bucket_id"]);
    configfile.close();
    Serial.println("Done Read");
    return true;
  }
  else{
    Serial.println("Fail to mount FS");
  }
}
void saveData(){
  StaticJsonDocument<390> json;
  
  json["USER_MAIL"] = USER_MAIL;
  json["USER_MAIL_PASS"] = USER_MAIL_PASS;
  json["api_key"] = api_key;
  json["bucket_id"] = bucket_id;

  File configfile = LittleFS.open("/config.json", "w");
  serializeJson(json, configfile);
  serializeJson(json, Serial);
  configfile.close();

  Serial.println("DONE SAVE");
}
void initLittleFS(){
  if(!LittleFS.begin()){
    delay(500);
    Serial.println("FAILED while muonting LittleFS");
    display.clear();
    display.drawString(10,10,"Connect LittleFS fail");
    display.display();
    delay(500);
    //ESP.restart();
  }
  else Serial.println("LittleFS mounted successfully");

  if(!loadData()){
    Serial.println("Data loaded");
  }
}