#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <String.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

const char *ssid = "liv";
const char *password = "12345679";
//const char* password = "1M@chthailai1";
char path[] = "/";
char host[] = "192.168.43.165";
//char host[] = "doangreenhouse.herokuapp.com";
const int port = 5001;

#define ss D8
#define rst D4
#define dio0 D0

byte gatewayID = 0xBB;
byte localAddress_1 = 0xAA;
byte localAddress_2 = 0xCC;
byte degree[8] = {
    0B01110, 0B01010, 0B01110, 0B00000, 0B00000, 0B00000, 0B00000};
WebSocketClient webSocketClient;
WiFiClient client;

int count = 0;
String nhdo, doam, as, damkk;
String ctrl_quat = "", ctrl_phunsuong = "", ctrl_bom = "";
String ctrl_quat1 = "", ctrl_phunsuong1 = "", ctrl_bom1 = "";
int sw = 1;
int val = 0; //read button
int temp = 0, temp1 = 0, temp2 = 0;
boolean dieukhien = 0;
void setup()
{
  lcd.init();
  lcd.backlight();
  // lcd.createChar(1,degree);
  // lcd.setCursor(10,0);
  //   lcd.print("Arduino LCM IIC 2004");
  // connecting wifi
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);

  // Connect to the websocket server
  if (client.connect(host, 5001))
  {
    Serial.println("Connected Wifi");
  }
  else
  {
    Serial.println("Connection failed.");
    while (1)
    {
      //delay(10);// Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client))
  {
    Serial.println("Handshake successful");
  }
  else
  {
    Serial.println("Handshake failed.");
    while (1)
    {
      // Hang on failure
    }
  }
  ////////// Lora ////////////////
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
  lcd.setCursor(0, 0);
  lcd.print("TB");
  lcd.setCursor(0, 1);
  lcd.print("mode");
  //LoRa.onReceive(onReceive);
  //LoRa.receive();
  Serial.println("LoRa init succeeded.");
  Serial.println("LoRa Gateway started");
  delay(200);
  //connectWs();  Serial.println("WebSocket connected"); // No delete comment
  // Read button
  pinMode(A0, INPUT);
}

void loop()
{
  String data, data1;
  String jsonstr = "", jsonstr1 = "", jsonstr2 = "", jsonctrolstr = "";

  val = analogRead(A0); //read button

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    int recipient = LoRa.read();
    Serial.print("recipient: ");
    Serial.println(recipient);
    byte sender = LoRa.read();
    Serial.print("sender: ");
    Serial.println(sender);
    String incoming = "";
    while (LoRa.available())
    {
      incoming += (char)LoRa.read();
    }
    if (recipient != gatewayID)
    {
    }
    if (sender == localAddress_1)
    {
      StaticJsonBuffer<300> jsonBuffer;
      JsonObject &root = jsonBuffer.parseObject(incoming);
      // Parse raw data (String) into the JSON
      if (!root.success())
      {
        Serial.println("parseObject() failed");
        //return;
      }
      String nhdot = root["Nhietdo"];
      nhdo = nhdot;
      String doamt = root["Doam"];
      doam = doamt;
      String ast = root["as"];
      as = ast;
    }
    else if (sender == localAddress_2)
    {
      damkk = incoming;
    }
  }

  /*---------------- End reci packetSize-------------------------*/
  /*---------------- connected server ---------------------------*/
  if (client.connected())
  {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["Nhietdo"] = nhdo;
    json["Doam"] = doam;
    json["as"] = as;
    json["damkk"] = damkk;
    json.printTo(jsonstr);
    Serial.println(jsonstr);
    webSocketClient.sendData(jsonstr);
    //-------------------------------------------------
    webSocketClient.getData(data);
    if (data.length() > 0)
    {
      Serial.println(data);
      StaticJsonBuffer<500> jsonBuffer;
      JsonObject &jsonctrl = jsonBuffer.parseObject(data);
      String id = jsonctrl["id"];
      if (id == "2")
      {
        String maybom = jsonctrl["maybom"];
        ctrl_bom = maybom;
        String quat = jsonctrl["quat"];
        ctrl_quat = quat;
        String phunsuong = jsonctrl["phunsuong"];
        ctrl_phunsuong = phunsuong;
        String chedo = jsonctrl["chedo"];
        Serial.println("reci data");
      }

      // Send signal control to 2 node
      StaticJsonBuffer<300> jsonBuff;
      JsonObject &json1 = jsonBuff.createObject();
      json1["maybom"] = ctrl_bom;
      if (ctrl_bom == "on")
        temp2 = 1;
      else if (ctrl_bom == "off")
        temp2 = 2;
      json1["quat"] = ctrl_quat;
      if (ctrl_quat == "on")
        temp1 = 1;
      else if (ctrl_quat == "off")
        temp1 = 2;
      json1["phunsuong"] = ctrl_phunsuong;
      if (ctrl_phunsuong == "on")
        temp = 1;
      else if (ctrl_phunsuong == "off")
        temp = 2;
      json1.printTo(Serial);
      json1.prettyPrintTo(Serial);
      json1.printTo(jsonstr1);
      //Sending..........
      for (int i; i < 10; i++)
      {
        LoRa_sendMessage(jsonstr1, localAddress_1);
        delay(10);
        LoRa_sendMessage(jsonstr1, localAddress_2);
        delay(10);
      }
    }
  }
  else
  {
    ESP.restart();
  }
  /*---------------- End connect server ---------------------------*/

  ///////////////////////////////////////////////////
  if ((val < 650) && (val > 500))
  {
    while ((val < 650) && (val > 500))
    {
      val = analogRead(A0);
    }
    Serial.println("1");
    delay(10);
    temp++;
    if (temp == 3)
      temp = 1;
    if (temp == 1)
    {
      ctrl_phunsuong = "on";
    }
    else if (temp == 2)
    {
      ctrl_phunsuong = "off";
    }
  }
  else if ((val < 900) && (val > 700))
  {
    while ((val < 900) && (val > 800))
    {
      val = analogRead(A0);
    }
    Serial.println("2");
    delay(10);
    temp1++;
    if (temp1 == 3)
      temp1 = 1;
    if (temp1 == 1)
    {
      ctrl_quat = "on";
      lcd.setCursor(3, 0);
      lcd.print("ON");
    }
    else if (temp1 == 2)
    {
      ctrl_quat = "off";
      lcd.setCursor(3, 0);
      lcd.print("OFF");
    }
  }
  else if ((val < 400) && (val > 300))
  {
    while ((val < 400) && (val > 300))
    {
      val = analogRead(A0);
    }
    Serial.println("3");
    delay(10);
    temp2++;
    if (temp2 == 3)
      temp2 = 1;
    if (temp2 == 1)
    {
      ctrl_bom = "on";
    }
    else if (temp2 == 2)
    {
      ctrl_bom = "off";
    }
  }
  else if ((val < 200) && (val > 90))
  {
    while ((val < 200) && (val > 90))
    {
      val = analogRead(A0);
    }
    Serial.println("4");
    delay(10);
    dieukhien = !dieukhien;
  }

  StaticJsonBuffer<300> jsonBuff;
  JsonObject &json1 = jsonBuff.createObject();
  json1["maybom"] = ctrl_bom;
  json1["quat"] = ctrl_quat;
  json1["phunsuong"] = ctrl_phunsuong;
  json1.printTo(Serial);
  json1.prettyPrintTo(Serial);
  json1.printTo(jsonstr1);
  ///////////////////////////////////////////////

  ////////////////////////////////////////////////
  if (dieukhien == 1)
  {
    lcd.setCursor(5, 1);
    lcd.print("send");
    Serial.println("send true");
    if (client.connected())
    {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject &jsonctrol = jsonBuffer.createObject();
      jsonctrol["ctrl_quat"] = ctrl_quat;
      jsonctrol["ctrl_bom"] = ctrl_bom;
      jsonctrol["ctrl_phunsuong"] = ctrl_phunsuong;
      jsonctrol["id"] = "3";
      jsonctrol.printTo(jsonctrolstr);
      //Serial.println(jsonctrolstr);
      webSocketClient.sendData(jsonctrolstr);
      webSocketClient.getData(data1);
      for (int i; i < 10; i++)
    {
      LoRa_sendMessage(jsonstr1, localAddress_1);
      delay(20);
      LoRa_sendMessage(jsonstr1, localAddress_2);
      delay(20);
    }
    }
    dieukhien = 0;
  }
  else if (dieukhien == 0)
  {
    lcd.setCursor(5, 1);
    lcd.print("done");
  }
  delay(100);
}

//////////////////////////////////// End Loop //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
/*-----------------------------LoRa Send Message---------------------------*/
void LoRa_sendMessage(String outgoing, byte localAddress1)
{
  //Serial.print("Sending packet: ");  Serial.println(msgCount);
  Serial.print("Lora Message: ");
  Serial.println(outgoing);
  LoRa.beginPacket();        // start packet
  LoRa.write(localAddress1); //CE      // add destination address - gateway
  LoRa.write(gatewayID);     //        // add sender address - device
  LoRa.print(outgoing);      // add payload
  LoRa.endPacket();          // finish packet and send it
}
