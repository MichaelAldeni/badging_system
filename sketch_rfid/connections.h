#include <WiFi.h>
#include <HTTPClient.h>

void initWiFi();
sendToAws(String id);

RPC_Response processSetLedMode(const RPC_Data &data);
void processSharedAttributes(const Shared_Attribute_Data &data);
void processClientAttributes(const Shared_Attribute_Data &data);
void connectToThingsBoard();
