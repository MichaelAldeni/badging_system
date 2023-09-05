#include <WiFi.h>
#include <HTTPClient.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
//#include "esp_camera.h"
//#include "camera_pin.h"

#define GREEN_LED 33
#define PIN_ANALOG_LUX 4
#define PIN_ANALOG_TEMP 34
#define PIN_LED 12
#define CHAN 0
#define LIGHT_MIN 372
#define LIGHT_MAX 2048
#define SDA 13 //Define SDA pins
#define SCL 14 //Define SCL pins

void initWiFi();
bool i2CAddrTest(uint8_t addr);
void initLCD();
//void initCamera();
//String base64Encode(const uint8_t* data, size_t len);
//void captureAndSendPhoto();
void Photoresistor(String & _telemetryPayload);
void Thermometer(String &_telemetryPayload);
void sendToAws(String id);

RPC_Response processSetLedMode(const RPC_Data &data);
void processSharedAttributes(const Shared_Attribute_Data &data);
void processClientAttributes(const Shared_Attribute_Data &data);
void connectToThingsBoard(String &_telemetryPayload);



