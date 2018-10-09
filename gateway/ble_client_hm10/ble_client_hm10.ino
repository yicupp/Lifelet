/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("0000ffe0-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
//static BLEUUID    charUUID("0000ffe1-0000-1000-8000-00805f9b34fb");

//static BLEUUID serviceUUID        ("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID           ("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
const uint8_t v[]={0x1,0x0};
char buff[50] = {'\0'};
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
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
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
while(1){
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.println("The characteristic value was: ");
    Serial.println(value.c_str());
    delay(200);
}
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    //pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)v,2,true);
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
    Serial.println(advertisedDevice.toString().c_str());
    
    /*
    Serial.print("Payload: ");
    
    //look at payload
    uint8_t *payload = advertisedDevice.getPayload();
    uint8_t length;
    uint8_t ad_type;
    uint8_t sizeConsumed = 0;
    bool finished = false;


    while(!finished) {
        length = *payload;          // Retrieve the length of the record.
        payload++;                  // Skip to type
        sizeConsumed += 1 + length; // increase the size consumed.

        if (length != 0) { // A length of 0 indicates that we have reached the end.
            ad_type = *payload;
            payload++;
            length--;

            for(int x=0;x<length;x++) {
                Serial.print(*payload,HEX);
            }
            
            payload += length;
        } // Length <> 0

        if (sizeConsumed >=31 || length == 0) {
            finished = true;
        }
    }//end
    Serial.println("");
    */
    // We have found a device, let us now see if it contains the service we are looking for.
    //if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
    if (advertisedDevice.toString().c_str()[6] == 'H') {
        
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

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  }
  
  delay(1000); // Delay a second between loops.
} // End of loop
