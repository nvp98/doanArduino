#include<String.h>
#include<SPI.h>
#include<LoRa.h>
#include<ArduinoJson.h>

#define ss 10
#define rst 9
#define dio0 8

#define pin_dam A0
int ctrl_bom=3;
byte localAddress = 0xCC;     // address of this device
byte gatewayID    = 0xBB;      // destination to send to

String maybom="";

void setup() {
  Serial.begin(115200);
  pinMode(ctrl_bom, OUTPUT);
  digitalWrite(ctrl_bom, LOW);
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
 Serial.println("LoRa Sender");
}

void loop() {
  float doamdat=getdoamdat();
  LoRa_sendMessage(String(doamdat));
  //Serial.print("SEnd okay"); Serial.println("");
  delay(50);
  for (int i =0; i <10; i++) {
   int packetSize = LoRa.parsePacket();
    if (packetSize) {
      int recipient = LoRa.read();      Serial.print("recipient: "); Serial.println(recipient);
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
       Serial.println("node 2 reci"+incoming);
       StaticJsonBuffer<200> jsonBuffer;
       JsonObject& root = jsonBuffer.parseObject(incoming);
       if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
          }
       String bomt =root["maybom"];maybom=bomt;
    }
    //else Serial.println("node 2 fail");
  delay(50); 
  }
  
  if(maybom=="on"){
       digitalWrite(ctrl_bom, HIGH);
  }
  else if(maybom=="off"){
        digitalWrite(ctrl_bom, LOW);
  }
}

int getdoamdat()
{
  int i = 0;
  int anaValue = 0;
  for (i = 0; i < 10; i++)  //
  {
    anaValue += analogRead(pin_dam); //Đọc giá trị cảm biến độ ẩm đất
    delay(50);   // Đợi đọc giá trị ADC
  }
  anaValue = anaValue / (i);
  anaValue = map(anaValue, 1023, 0, 0, 100); //Ít nước:0%  ==> Nhiều nước 100%
  return anaValue; // dưới 66%, cần phải bơm nước
}
/*-----------------------------LoRa Send Message---------------------------*/
void LoRa_sendMessage(String outgoing) {
  //Serial.print("Lora Message: ");  Serial.println(outgoing); 
  LoRa.beginPacket();                   // start packet
  LoRa.write(gatewayID);      //BB      // add destination address - gateway
  LoRa.write(localAddress);   //AA      // add sender address - device 2
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  //Serial.print("okaypkaoay ");
}
