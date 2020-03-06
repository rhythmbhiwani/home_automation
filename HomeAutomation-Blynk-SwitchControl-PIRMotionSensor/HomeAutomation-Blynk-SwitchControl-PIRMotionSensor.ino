#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define BLYNK_PRINT Serial
#include <ESP8266HTTPClient.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

const char* ssid = "WIFI NAME";
const char* password = "WIFI PASSWORD";

#define SWITCH_1 D5
#define SWITCH_2 D6
#define LIGHT D2
#define FAN D3
#define PIR D7

int switch_ON_Flag1_previous_II = 1;
int switch_ON_Flag1_previous_I = 0;

int switch_ON_Flag2_previous_II = 1;
int switch_ON_Flag2_previous_I = 0;

int calibrationTime = 5;
long unsigned int lowIn;
long unsigned int pause = 5000;
boolean lockLow = true;
boolean takeLowTime;

HTTPClient http;

void turnoff(int deviceno)  {
    char offcommand[] = "[\"1\"]";
    if (deviceno==1)
      http.begin("http://188.166.206.43/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/update/D4");
    else
      http.begin("http://188.166.206.43/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/update/D0");
    http.addHeader("Content-Type", "application/json"); 
    int httpResponseCode = http.PUT(offcommand);
  }
  
void turnon(int deviceno)  {
  char offcommand[] = "[\"0\"]";
  if (deviceno==1)
    http.begin("http://188.166.206.43/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/update/D4");
  else
    http.begin("http://188.166.206.43/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/update/D0");
  http.addHeader("Content-Type", "application/json"); 
  int httpResponseCode = http.PUT(offcommand);
  http.end();
}

void Call_WiFiSetup() {
  int interval = 1000;
  int previousMillis = 0;
  Serial.print("Wait for WiFi... ");
  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED)
      break;
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Call_ManualAutomation();
    }
  }
  if (digitalRead(SWITCH_1) == LOW){
    turnon(1);
  }
  else{
    turnoff(1);
  }
  if (digitalRead(SWITCH_2) == LOW){
    turnon(2);
  }
  else{
    turnoff(2);
  }
}

void Call_ManualAutomation() {
  Serial.println("Manual Automation");
    digitalWrite(LIGHT, digitalRead(SWITCH_1));
    delay(1);
    digitalWrite(FAN, digitalRead(SWITCH_2));
    delay(1);
}

void Call_SwitchControl() {
  if (digitalRead(SWITCH_1) == LOW)
  {
    if (switch_ON_Flag1_previous_I == 0 )
    {
      digitalWrite(LIGHT, LOW);
      turnon(1);
      delay(50);
      switch_ON_Flag1_previous_I = 1;
      switch_ON_Flag1_previous_II = 1;
    }
  }
  if (digitalRead(SWITCH_1) == HIGH )
  {
    if (switch_ON_Flag1_previous_II == 1)
    {
      digitalWrite(LIGHT, HIGH);
      turnoff(1);
      delay(50);
      switch_ON_Flag1_previous_II = 0;
      switch_ON_Flag1_previous_I = 0;
    }
  }
  /*********************************************************************/
  if (digitalRead(SWITCH_2) == LOW)
  {
    if (switch_ON_Flag2_previous_I == 0)
    {
      digitalWrite(FAN, LOW);
      turnon(2);
      delay(50);
      switch_ON_Flag2_previous_II = 1;
      switch_ON_Flag2_previous_I = 1;
    }
  }
  if (digitalRead(SWITCH_2) == HIGH)
  {
    if (switch_ON_Flag2_previous_II  == 1)
    {
      digitalWrite(FAN, HIGH);
      turnoff(2);
      delay(50);
      switch_ON_Flag2_previous_II = 0;
      switch_ON_Flag2_previous_I = 0;
    }
  }
}

void checkMotion(){
  if(digitalRead(PIR) == HIGH){
       if(lockLow){  
         lockLow = false;
         delay(50);
         }         
         takeLowTime = true;
       }

     if(digitalRead(PIR) == LOW){       
       if(takeLowTime){
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
        }
       if(!lockLow && millis() - lowIn > pause){  
           lockLow = true;
           Serial.println("No Motion Detected - Turning off all Devices");
           digitalWrite(LIGHT, HIGH);
           digitalWrite(FAN, HIGH);
           if (WiFi.status() == WL_CONNECTED) {
            turnoff(1);
            turnoff(2);
           }                        
           delay(50);
           }
       }
}

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(PIR, INPUT);
  digitalWrite(PIR, LOW);

  Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);

  WiFi.begin(ssid, password);
  Call_WiFiSetup();

  // Setup Blynk
  Blynk.config(auth);
  while (Blynk.connect() == false) {
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Call_WiFiSetup();
  }
  Call_SwitchControl();
  checkMotion();
  Blynk.run();
  delay(1);

}
