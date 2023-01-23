#include <Arduino.h>
#include <DHTesp.h>

DHTesp dht;

#define BLYNK_TEMPLATE_ID "TMPLLRFBwACC" //templet เอาจากแอป (ไม่เหมือนกัน)
#define BLYNK_DEVICE_NAME "Smoke detector" //ชื่อ(ต้องเปลี่ยนในแอปด้วย)
#define BLYNK_AUTH_TOKEN  "yAy4I3AEZfiw5hh09fPhcQk9ZwnFjnIu" //token เอาจากแอป (ไม่เหมือนกัน)

#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "Kaisit 2.4G"; //ชื่อ wifi ใช้คลื่่น 2.4GHz เท่านั้น

char pass[] = "0818808542"; //รหัส wifi

#define smoke_SensorPin 35 //ขาsensorตรวจควัญ
#define buzzer_pin 32 //ขา buzzer
#define dht_pin 33 // ขา sensor dht22 (วัดอุณหภูมิและความชื้น)
#define led1 25
#define led2 26
#define led3 27

int smoke_value;

BLYNK_WRITE(V3){
  //อ่านค่าจากปุ่มในแอป
  int value = param.asInt();
  digitalWrite(led1, value);
}
BLYNK_WRITE(V4){
  int value = param.asInt();
  digitalWrite(led2, value);
}
BLYNK_WRITE(V5){
  int value = param.asInt();
  digitalWrite(led3, value);
}

void setup(){
  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, HIGH);
  pinMode(smoke_SensorPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  analogReadResolution(10); //ตั้งค่าอ่า analog ไว้ 10 bit (0-1023)

  Serial.begin(115200); //เปิด serial mornitor ไว้ที่ 115200

  Blynk.begin(auth, ssid, pass);

  dht.setup(dht_pin, DHTesp::DHT22);

}

void loop(){
  Blynk.run();
  static unsigned long smoke_delay;

  static bool beep_flag;
  static unsigned long beep_delay;
  static bool beep_toggle;

  //อ่านค่า sensor ทุกๆ 100ms
  if ((millis() - smoke_delay) > 100){
    smoke_value = analogRead(smoke_SensorPin); //อ่านค่าจาก sensor
    if ((smoke_value > 650) && (!beep_flag)){ //ถ้าค่าควัญเกิน 650 ส่งข้อความ 1 ครั้งแล้ว ส่งเสียง
      beep_flag = true;
      digitalWrite(buzzer_pin, LOW);
      beep_toggle = true;
      beep_delay = millis();
      Blynk.logEvent("smoke_detected");
    }

    else {//ถ้าไม่ ให้หยุดร้องปีดๆ
      digitalWrite(buzzer_pin, HIGH);
      beep_flag = false;
    }
    smoke_delay = millis();
    //Serial.println(smoke_value); //ไว้ดูค่าจาก sensor
  }
  
  //ปีดๆโดยไม่ใช้ delay
  if (beep_flag){
    if ((millis() - beep_delay) > 200){
      beep_toggle =! beep_toggle;
      digitalWrite(buzzer_pin, beep_toggle);
      beep_delay = millis();
    }
  }

  //อ่านค่าขึ้นแอปทุกๆ 0.5 วิ
  static unsigned long blynk_sendDelay;
  if ((millis() - blynk_sendDelay) > 500){
    //อ่านค่าต่างๆ
    int temperature = dht.getTemperature();
    int huminity = dht.getHumidity();
    int smoke_percent = map(analogRead(smoke_SensorPin), 150, 900, 0, 100);

    static int last_temperature;
    static int last_huminity;
    static int last_smokePercent;

    //อัฟค่าต่างๆขึ้นแอป เมื่อค่าต่างๆมีการเปลี่ยนแปลง
    if (temperature != last_temperature){
      Blynk.virtualWrite(V0, temperature);
      last_temperature = temperature; //อัพเดทค่าเก่าให้เป็นค่าที่พึ่งอัพ
    }

    if (huminity != last_huminity){
      Blynk.virtualWrite(V1, huminity);
      last_huminity = huminity;
    }

    if (smoke_percent != last_smokePercent){
      Blynk.virtualWrite(V2, smoke_percent);
      last_smokePercent = smoke_percent;
    }

    blynk_sendDelay = millis();
  }
}