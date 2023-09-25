#include "connections.h";

//Parameters
const char* ssid = "";        //insert your SSID
const char* password = "$$";  //insert your password

//Your Domain name with URL path or IP address with path
String serverName = "https://4nsuna0nia.execute-api.eu-west-2.amazonaws.com/Prod";

bool is_new_tag = false;
bool data_acquisition = false;

const String DEVICE_ID = "123";

// to understand how to obtain an access token
const char* TOKEN = "bqpcw9gvjqtvne59gxx9"; 

// Thingsboard we want to establish a connection too
const char* THINGSBOARD_SERVER = "mqtt.thingsboard.cloud";
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port.
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint32_t MAX_MESSAGE_SIZE = 256U;

// Baud rate for the debugging serial connection.
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

// Initialize underlying client, used to establish a connection
WiFiClient wifiClient;
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(wifiClient, MAX_MESSAGE_SIZE);
// Initializa LCD instance
LiquidCrystal_I2C lcd(0x27,16,2);

// Attribute names for attribute request and attribute updates functionality

constexpr char BLINKING_INTERVAL_ATTR[] = "blinkingInterval";
constexpr char LED_MODE_ATTR[] = "ledMode";
constexpr char LED_STATE_ATTR[] = "ledState";
//constexpr char PHOTORESISTOR[] = "Photoresistor";

// Statuses for subscribing to rpc
bool subscribed = false;

// handle led state and mode changes
volatile bool attributesChanged = false;

// LED modes: 0 - continious state, 1 - blinking
volatile int ledMode = 0;
volatile int getDataAcquisition = 0;

// Current led state
volatile bool ledState = false;

// Settings for interval in blinking mode
constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
volatile uint16_t blinkingInterval = 1000U;

uint32_t previousStateChange;

// For telemetry
constexpr int16_t telemetrySendInterval = 2000U;
uint32_t previousDataSend;

// List of shared attributes for subscribing to their updates
constexpr std::array<const char *, 2U> SHARED_ATTRIBUTES_LIST = {
  LED_STATE_ATTR,
  BLINKING_INTERVAL_ATTR,
  //PHOTORESISTOR
};

// List of client attributes for requesting them (Using to initialize device states)
constexpr std::array<const char *, 1U> CLIENT_ATTRIBUTES_LIST = {
  LED_MODE_ATTR
};


/// @brief Processes function for RPC call "setLedMode"
/// RPC_Data is a JSON variant, that can be queried using operator[]
/// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
/// @param data Data containing the rpc data that was called and its current value
/// @return Response that should be sent to the cloud. Useful for getMethods
RPC_Response processSetLedMode(const RPC_Data &data) {
  Serial.println("Received the set led state RPC method");

  // Process data
  int new_mode = data;

  is_new_tag = !is_new_tag;
  if(is_new_tag)
  {
    lcd.clear();
    lcd.setCursor(6,0);
    lcd.print("Card");
    lcd.setCursor(2,1);
    lcd.print("registration"); 
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ready to bedge!!");
  }
  Serial.print("Mode to change: ");
  Serial.println(new_mode);

  if (new_mode != 0 && new_mode != 1) {
    return RPC_Response("error", "Unknown mode!");
  }

  ledMode = new_mode;

  attributesChanged = true;

  // Returning current mode
  return RPC_Response("newMode", (int)ledMode);
}

RPC_Response processSetDataAcquisition (const RPC_Data &data) {
  
  int new_mode = data;

  data_acquisition = !data_acquisition; 

  if (new_mode != 0 && new_mode != 1) {
    return RPC_Response("error", "Unknown mode!");
  }

  getDataAcquisition = new_mode;

  //attributesChanged = true;

  // Returning current mode
  return RPC_Response("newMode", (int)getDataAcquisition);
}


// Optional, keep subscribed shared attributes empty instead,
// and the callback will be called for every shared attribute changed on the device,
// instead of only the one that were entered instead
const std::array<RPC_Callback, 2U> callbacks = {
  RPC_Callback{ "setLedMode", processSetLedMode },
  RPC_Callback{ "setDataAcquisition", processSetDataAcquisition }
};

// for init camera
//camera_config_t config;

/// @brief Update callback that will be called as soon as one of the provided shared attributes changes value,
/// if none are provided we subscribe to any shared attribute change instead
/// @param data Data containing the shared attributes that were changed and their current value
void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
      const uint16_t new_interval = it->value().as<uint16_t>();
      if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
        blinkingInterval = new_interval;
        Serial.print("Updated blinking interval to: ");
        Serial.println(new_interval);
      }
    } else if(strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
      ledState = it->value().as<bool>();
      digitalWrite(GREEN_LED, ledState ? HIGH : LOW);
      Serial.print("Updated state to: ");
      Serial.println(ledState);
    }
  }
  attributesChanged = true;
}

//change the LED operating mode based on requests sent by external clients.
void processClientAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), LED_MODE_ATTR) == 0) {
      const uint16_t new_mode = it->value().as<uint16_t>();
      ledMode = new_mode;
    }
  }
}

//these objects are used to separately handle requests for shared attributes and client attributes, each with its own associated management function
const Shared_Attribute_Callback attributes_callback(SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend(), &processSharedAttributes);
const Attribute_Request_Callback attribute_shared_request_callback(SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend(), &processSharedAttributes);
const Attribute_Request_Callback attribute_client_request_callback(CLIENT_ATTRIBUTES_LIST.cbegin(), CLIENT_ATTRIBUTES_LIST.cend(), &processClientAttributes);

void Photoresistor(String &_telemetryPayload)
{
  int adcVal = analogRead(PIN_ANALOG_LUX); //read adc
  //Serial.print(adcVal);
  randomSeed(analogRead(0));
  int randomNumber = random(400, 501);
  _telemetryPayload = "{\"luminosity\": " + String(randomNumber) + "}";
  delay(10);
}

void Thermometer(String &_telemetryPayload)
{
  int adcValue = analogRead(PIN_ANALOG_TEMP);                         //read ADC pin
  double voltage = (float)adcValue / 4095.0 * 3.3;                  //calculate voltage
  double Rt = 10 * voltage / (3.3 - voltage);                       //calculate resistance value of thermistor
  double tempK = 1 / (1/(273.15 + 25) + log(Rt / 10)/3950.0);      //calculate temperature (Kelvin)
  double tempC = tempK - 273.15;                                  //calculate temperature (Celsius)
  //Serial.print(String(tempC) + " ");                             
  _telemetryPayload = "{\"temperature\": " + String(tempC) + "}";
  delay(10);

}

//This function can be used to check whether a device with a specified I2C address is connected and responding correctly on the I2C bus
bool i2CAddrTest(uint8_t addr) {
 Wire.begin();
 Wire.beginTransmission(addr);
 if (Wire.endTransmission() == 0) {
 return true;
 }
 return false;
}

//this function is designed to initialize an I2C LCD display, and if the primary device with address 0x27 is not present, it uses an alternative address 0x3F
void initLCD()
{
  Wire.begin(SDA, SCL); // attach the IIC pin
 if (!i2CAddrTest(0x27)) 
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
 
 lcd.init(); // LCD driver initialization
 lcd.backlight(); // Open the backlight
 lcd.setCursor(0,0); // Move the cursor to row 0, column 0
 lcd.print("Ready to bedge!!"); // The print content is displayed on the LCD
}

//This feature manages communication between the device and the ThingsBoard server,
//allowing the device to send telemetry data and receive RPC commands and shared and client attribute updates from the ThingsBoard server.
void connectToThingsBoard(String &_telemetryPayloadT, String &_telemetryPayloadP){
  if (!tb.connected()) {
    subscribed = false;
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
    // Sending a MAC address as an attribute
    tb.sendAttributeString("macAddress", WiFi.macAddress().c_str());
  }

  if (!subscribed) {
    Serial.println("Subscribing for RPC...");
    // Perform a subscription. All consequent data processing will happen in
    // processSetLedState() and processSetLedMode() functions,
    // as denoted by callbacks array.
    if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }

    if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
      Serial.println("Failed to subscribe for shared attribute updates");
      return;
    }

    Serial.println("Subscribe done");
    subscribed = true;

    // Request current states of shared attributes
    if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
      Serial.println("Failed to request for shared attributes");
      return;
    }

    // Request current states of client attributes
    if (!tb.Client_Attributes_Request(attribute_client_request_callback)) {
      Serial.println("Failed to request for client attributes");
      return;
    }
  }

  if (attributesChanged) {
    attributesChanged = false;
    if (ledMode == 0) {
      previousStateChange = millis();
    }
    tb.sendTelemetryInt(LED_MODE_ATTR, ledMode);
    tb.sendTelemetryBool(LED_STATE_ATTR, ledState);
    tb.sendAttributeInt(LED_MODE_ATTR, ledMode);
    tb.sendAttributeBool(LED_STATE_ATTR, ledState);
  }

  if (ledMode == 1 && millis() - previousStateChange > blinkingInterval) {
    previousStateChange = millis();
    ledState = !ledState;
    digitalWrite(GREEN_LED, ledState);
    tb.sendTelemetryBool(LED_STATE_ATTR, ledState);
    tb.sendAttributeBool(LED_STATE_ATTR, ledState);
    if (GREEN_LED == 25) {
      Serial.print("LED state changed to: ");
      Serial.println(ledState);
    }
  }

  // Sending telemetry every telemetrySendInterval time
  if (millis() - previousDataSend > telemetrySendInterval) {
    previousDataSend = millis();
    
    tb.sendAttributeInt("rssi", WiFi.RSSI());
    tb.sendAttributeInt("channel", WiFi.channel());
    tb.sendAttributeString("bssid", WiFi.BSSIDstr().c_str());
    tb.sendAttributeString("localIp", WiFi.localIP().toString().c_str());
    tb.sendAttributeString("ssid", WiFi.SSID().c_str());

    if (data_acquisition){
      tb.sendTelemetryJson(_telemetryPayloadT.c_str());
      tb.sendTelemetryJson(_telemetryPayloadP.c_str());
    }

  }

  tb.loop();
  
}

//This feature is used to connect to a specific Wi-Fi network using a provided network name and password. 
//The feature waits until the Wi-Fi connection is established and then provides information about the successful connection,
//including local IP address assignment.
void initWiFi(){
  WiFi.begin(ssid, password);
  WiFi.SSID();
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

//This feature is used to send HTTP POST requests to an AWS server with specific data and manage the server's response, 
//displaying appropriate messages on the LCD based on the response received.
void sendToAws(String id){

    // Data to send with HTTP POST
    String httpRequestData = "{\"rfid_id\":\"" + id + "\", \"device_id\":\"" + DEVICE_ID + "\"}";

    String endpoint = is_new_tag ? "/register" : "/verify"; 

    WiFiClient client;
    HTTPClient http;
    http.begin(serverName + endpoint);
    // Specify content-type header
    http.addHeader("Content-Type", "application/json");

    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    Serial.println(httpRequestData);

  if(!is_new_tag)
  {
    if(httpResponseCode != 200)
    {
  
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Access");
      lcd.setCursor(5,1);
      lcd.print("denied");
      delay(5000);
      lcd.clear();
      lcd.setCursor(0,0); // Move the cursor to row 0, column 0
      lcd.print("Ready to bedge!!");
    }else{
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Authorized");
      lcd.setCursor(5,1);
      lcd.print("access");   
      delay(5000);
      lcd.clear();
      lcd.setCursor(0,0); // Move the cursor to row 0, column 0
      lcd.print("Ready to bedge!!");
    } 
  }
  else
  {
    if(httpResponseCode == 200 )
    {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Registration");
      lcd.setCursor(3,1);
      lcd.print("completed");   
      delay(5000);
      lcd.clear();
      lcd.setCursor(6,0);
      lcd.print("Card");
      lcd.setCursor(2,1);
      lcd.print("registration");
    }   
    else
    {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Registration");
      lcd.setCursor(5,1);
      lcd.print("failed");   
      delay(5000);
      lcd.clear();
      lcd.setCursor(6,0);
      lcd.print("Card");
      lcd.setCursor(2,1);
      lcd.print("registration");
    }
  }

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
}
