//Libraries
#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//Constants
#define SS_PIN 5
#define RST_PIN 27

#define RED_LED 33
#define GREEN_LED 25


//Parameters
const char* ssid = "MORDOR";
const char* password = "Sauron99";
//Your Domain name with URL path or IP address with path
String serverName = "https://eapc1h7s93.execute-api.eu-west-2.amazonaws.com/Stage/verify";


//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);


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

void initWiFi(){
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void initRfid(){
  Serial.println(F("Initialize System"));
 	//init rfid D8,D5,D6,D7
 	SPI.begin();
 	rfid.PCD_Init();
 	Serial.print(F("Reader :"));
 	rfid.PCD_DumpVersionToSerial();
}

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
    // Data to send with HTTP POST
    String httpRequestData = "{\"rfid_id\":\"" + id + "\"}";
    //String httpRequestData = "{\"rfid_id\": \"lkj\"}";

    WiFiClient client;
    HTTPClient http;
    http.begin(serverName);
    // Specify content-type header
    http.addHeader("Content-Type", "application/json");

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    Serial.println(httpRequestData);


    if(httpResponseCode != 200){
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    }else{
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);    
    } 
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  }else {
   Serial.println("WiFi Disconnected");
  }
}







