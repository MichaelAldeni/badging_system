//Libraries
#include "connections.h";
#include "rfid.h";


void setup() {
 	//Init Serial USB
 	Serial.begin(115200);

  //init LED
  pinMode(GREEN_LED, OUTPUT);

  //init Photoresistor 
  ledcSetup(CHAN, 1000, 12);
  ledcAttachPin(PIN_LED, CHAN);

  initRfid();
  initLCD();
  //initCamera();
  initWiFi();
}


void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){

    String telemetryPayload = "";
    //Photoresistor(telemetryPayload);
    Thermometer(telemetryPayload);
    connectToThingsBoard(telemetryPayload);

    String id = "";

    if (!readRFID(id))
      return;

    id.replace(" ", "");

    sendToAws(id);

  }else {
   Serial.println("WiFi Disconnected");
  }
}
