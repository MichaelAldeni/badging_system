#include "rfid.h"

//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);



void initRfid(){
  Serial.println(F("Initialize System"));
   //init rfid D8,D5,D6,D7
  SPI.begin();
  rfid.PCD_Init();
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
}

bool readRFID(String & id ) { /* function readRFID */
   ////Read RFID card
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }

  if(check()){
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
    }
    
    id = getId(rfid.uid.uidByte, rfid.uid.size);
    
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
     return true;
  }
  return false;
}

bool check(){
  // Look for new 1 cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return false;
    
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
      return false;

  return true;     
}

String getId(byte *buffer, byte bufferSize) {
  String x = "";
  for (byte i = 0; i < bufferSize; i++) {
     x += buffer[i] < 0x10 ? " 0" : " ";
     x += buffer[i];
  }
  return x;
}
