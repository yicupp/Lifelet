/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001234-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("00006969-0000-1000-8000-00805f9b34fb");

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

//wifi
#include <WiFi.h>

const char* ssid     = "yicup";
const char* password = "aaaaaaaa";

const char* host = "47.91.46.124";

char *http_buff[250] = {'\0'};
// \wifi


static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);

    // Read the value of the characteristic.
    std::string value = pBLERemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
    for (int i=0;i<length;i++) {
        Serial.print(char(*(pData+i)));
    }
    Serial.println("");

    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // This will send the request to the server

    client.print(   String("PUT /tablestore HTTP/1.1") + "\r\n" +
                    "Host: 47.91.46.124:5000" + "\r\n" +
                    "Content-Type: application/json" + "\r\n" +
                    "Content-Length: 108" + "\r\n" +    
                    "\r\n" +
                    "{" + "\r\n" +
                    "\"acceleration_x\": \"1234\"," + "\r\n" +
                    "\"acceleration_y\": \"54\"," + "\r\n" +
                    "\"acceleration_z\": \"343\"" + "\r\n" +
                    "\"fall_detection\": \"1\"" + "\r\n" +
                    "}" "\r\n" + "\r\n");

    unsigned long timeout = millis();
    Serial.println("Sent packet");
    
    // Read all the lines of the reply from server and print them to Serial
    /*while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }*/
}

bool connectToServer(BLEAddress pAddress) {
    Serial.print("Forming a connection to ");
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    // Connect to the remove BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());

    pRemoteCharacteristic->registerForNotify(notifyCallback);
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.print("RSSI: ");
    Serial.print(advertisedDevice.getRSSI());
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

      // 
      Serial.print("Found our device!  address: "); 
      advertisedDevice.getScan()->stop();

      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Sending alive packet to server");
    wifi_alive();
  
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // This will send the request to the server
    /*client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");*/
/*
    client.print(   String("PUT /tablestore HTTP/1.1") + "\r\n" +
                    "Host: 47.91.46.124:80" + "\r\n" +
                    "Content-Type: application/json" + "\r\n" +
                    "Content-Length: 100" + "\r\n" +    
                    "\r\n" +
                    "{" + "\r\n" +
                    "\"acceleration_x\": \"1234\"," + "\r\n" +
                    "\"acceleration_y\": \"54\"," + "\r\n" +
                    "\"acceleration_z\": \"343\"" + "\r\n" +
                    "}" "\r\n" + "\r\n");

    unsigned long timeout = millis();
    Serial.println("Sent packet");

    Serial.println("Return");
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    Serial.println("end return");*/
    
  delay(1000); // Delay a second between loops.
} // End of loop

http_body() {
    
}
