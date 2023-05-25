#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid
#include <ArduinoJson.h>
#include <ThingsBoard.h>


//Constants
#define SS_PIN 5
#define RST_PIN 27

void initRfid();
bool readRFID(String & id );
bool check();
String getId(byte *buffer, byte bufferSize);
