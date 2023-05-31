#include <WiFi.h>
#include <HTTPClient.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>

#define GREEN_LED 25


void initWiFi();
void sendToAws(String id);

RPC_Response processSetLedMode(const RPC_Data &data);
void processSharedAttributes(const Shared_Attribute_Data &data);
void processClientAttributes(const Shared_Attribute_Data &data);
void connectToThingsBoard();
