//Libraries
#include "connections.h";
#include "rfid.h";

#define RED_LED 33


void setup() {
 	//Init Serial USB
 	Serial.begin(115200);

  //init LED
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  initRfid();

  initWiFi();
}


void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){

    String id = "";

    if (!readRFID(id))
      return;

    id.replace(" ", "");

    sendToAws(id);

  }else {
   Serial.println("WiFi Disconnected");
  }
}
