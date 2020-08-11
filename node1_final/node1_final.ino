#include<SPI.h>
#include<LoRa.h>
#include "DHT.h" 
#include <Wire.h>
#include <BH1750.h>
#include<ArduinoJson.h>

//Lora
#define ss 10
#define rst 9
#define dio0 8

//pin control
int dk_quat = 2;
int dk_suong = 3;
char nhdo[10],doamg[10],asg[10];

byte localAddress = 0xAA;  //
byte gatewayID = 0xBB;

byte degree[8] = {
  0B01110,    0B01010,  0B01110,    0B00000,  0B00000,  0B00000,  0B00000
};
 // sensor pin anh sang
BH1750 lightMeter;
// Nhiet do, Do am
const int DHTPIN = 5;       
const int DHTTYPE = DHT11;  
DHT dht(DHTPIN, DHTTYPE);
String ctrl_quat="",ctrl_phunsuong="";
void setup() {
  Serial.begin(115200);
  delay(10);
  dht.begin();  //dht11
  lightMeter.begin(); //anh sang
  pinMode(dk_quat, OUTPUT);
  digitalWrite(dk_quat, LOW);
  pinMode(dk_suong, OUTPUT);
  digitalWrite(dk_suong, LOW);

  LoRa.setPins(ss, rst, dio0); //lora
  if (!LoRa.begin(433E6)) {                  
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1);
  }
  Serial.println("LoRa connected");
  //connected lora
  
}

void loop() {
  
  float h = dht.readHumidity();    
  float t = dht.readTemperature();  
  float lux = lightMeter.readLightLevel();
  
  char data[100];
  char doam[50];
  char temp[10];
  char do_am[10];
  char as[10];
  dtostrf(t, 4, 2, temp);
  dtostrf(h, 4, 2, do_am);
  dtostrf(lux, 4, 2, as);
  sprintf(data,"{\"id\":%d,\"Nhietdo\":\"%s\",\"Doam\":\"%s\",\"as\":\"%s\"}",1,temp,do_am,as);
  LoRa_sendMessage(data);
  delay(50);
  //Serial.print("SEnd okay"); Serial.println("");
  
  //Serial.println("================================="); 

  for (int i =0; i <12; i++) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int recipient = LoRa.read();    Serial.print("recipient: "); Serial.println(recipient);
      byte sender    = LoRa.read();   Serial.print("sender: ");    Serial.println(sender);
      // received a packet
      String incoming = "";
      // read packet
      while (LoRa.available()) {
        incoming += (char)LoRa.read();
      }
      if ( recipient != localAddress ) {
            return;                         
      }
      Serial.println("node 1 reci: "+incoming); 
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(incoming);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      String quat =root["quat"]; ctrl_quat=quat;
      String phunsuong=root["phunsuong"]; ctrl_phunsuong=phunsuong;
	 // if((quat!=ctrl_quat)||(phunsuong!=ctrl_phunsuong)){ctrl_quat=quat;ctrl_phunsuong=phunsuong;return;} 
   }
   delay(50); 
 }
 
  if(ctrl_quat=="on"){
    digitalWrite(dk_quat, HIGH);
  }
  else if(ctrl_quat=="off"){
    digitalWrite(dk_quat, LOW);
  }
  if(ctrl_phunsuong=="on"){
    digitalWrite(dk_suong, HIGH);
  }
  else if(ctrl_phunsuong=="off"){
    digitalWrite(dk_suong, LOW);
  }
}
  
/*-----------------------------LoRa Send Message---------------------------*/
void LoRa_sendMessage(String outgoing) {
  Serial.print("Node 1 send: ");  Serial.println(outgoing); 
  LoRa.beginPacket();                   // start packet
  LoRa.write(gatewayID);      //BB      // add destination address - gateway
  LoRa.write(localAddress);   //AA      // add sender address - device 2
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it

  //Serial.print("sender: ");
}
