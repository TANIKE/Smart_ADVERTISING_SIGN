#include "Rotary.h"
#include "Firebase.h"


void setup() {
  Serial.begin(115200);

  display.init();
  display.setContrast(200);
  display.drawString(10,10,"Waiting");
  display.display();

  delay(2000); //Need time to initialize display

  pinMode(CLK_pin, INPUT);
  pinMode(DT_pin, INPUT);

  last_CLK = digitalRead(CLK_pin);
  last_DT = digitalRead(DT_pin);

  display.clear();
  Serial.println("mounting FS...");
  display.drawString(10,10,"mounting");
  display.display();

  initLittleFS();

  //Create custom parameters for Webserver
  WiFiManagerParameter custom_mail("mail", "Mail account", USER_MAIL, 30);
  WiFiManagerParameter custom_mail_pass("pass", "Mail pass", USER_MAIL_PASS, 20);
  WiFiManagerParameter custom_apikey("apikey", "API token", api_key, 40);
  WiFiManagerParameter custom_bucketID("bucket_id", "Bucket ID", bucket_id, 30);
  
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  //set static ip
  wifiManager.setAPCallback(configModeCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mail);
  wifiManager.addParameter(&custom_mail_pass);
  wifiManager.addParameter(&custom_apikey);
  wifiManager.addParameter(&custom_bucketID);

  //reset settings - for testing
  //wifiManager.resetSettings();
  
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect and hit timeout");
    display.clear();
    display.drawString(10,10,"Connect fail");
    display.display();
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  Serial.println("connected...yeey :)");
  display.clear();
  display.drawString(10,10,"connected");
  display.display();

  //read updated parameters
  strcpy(USER_MAIL, custom_mail.getValue());
  strcpy(USER_MAIL_PASS, custom_mail_pass.getValue());
  strcpy(api_key, custom_apikey.getValue());
  strcpy(bucket_id, custom_bucketID.getValue());

  Serial.println("The values in the file are: ");
  Serial.println("\tMail : " + String(USER_MAIL));
  Serial.println("\tMail_pass : " + String(USER_MAIL_PASS));
  Serial.println("\tApi : " + String(api_key));
  Serial.println("\tBucket_id : " + String(bucket_id));
  if(shouldSaveConfig){
    saveData();
  }
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

    // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();
  initFirebase();

  delay(1000);
  display.clear();
  display.drawString(10,10,"DONE SETUP");
  display.display();
}

void loop() {
  button.tick();
  menu_change();
}
