//Libraries
#include "connections.h";
#include "rfid.h";


void setup() {
 	//Init Serial USB
 	Serial.begin(115200);

  //init LED
  pinMode(GREEN_LED, OUTPUT);

  initRfid();
  initLCD();
  initWiFi();
}


void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){

    String telemetryPayloadT = "";
    String telemetryPayloadP = "";

    Photoresistor(telemetryPayloadP);
    Thermometer(telemetryPayloadT);
    connectToThingsBoard(telemetryPayloadT, telemetryPayloadP);

    String id = "";

    if (!readRFID(id))
      return;

    id.replace(" ", "");

    sendToAws(id);

  }else {
   Serial.println("WiFi Disconnected");
  }
}
