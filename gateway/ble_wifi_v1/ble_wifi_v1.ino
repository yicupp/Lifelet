/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001234-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("00006969-0000-1000-8000-00805f9b34fb");

static BLEUUID beaconServUUID("0000abcd-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
static BLEUUID beaconCharUUID("0000dada-0000-1000-8000-00805f9b34fb");

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

#define STOPPED 0
#define STARTED 1000

unsigned long ble_scan_start_t = 0;
int BLEscanState = STOPPED;
#define BLE_SCAN_TIME 30 //scan time in sec
#define BLE_SCAN_TIME_RESTART 1850 //scan restarts every () ms



//wifi
#include <WiFi.h>

const char* ssid     = "yicup";
const char* password = "aaaaaaaa";

const char* host = "47.91.46.124";

char *http_buff[500] = {'\0'};
// \wifi

//BLE packet structure:
//customer_num:key:data[10]

//HTTP keys
#define CODE_HUMIDITY       1
#define  KEY_HUMIDITY       "humidity"
#define CODE_TEMP           2
#define  KEY_TEMP           "temp"
#define CODE_SVM            3
#define  KEY_SVM            "svm"
#define CODE_VEL_MAG        4
#define  KEY_VEL_MAG        "vel_mag"
#define CODE_STEP_COUNT     5
#define  KEY_STEP_COUNT     "step_count"
#define CODE_FALL_DETECTED  6
#define  KEY_FALL_DETECTED  "fall_detected"
#define CODE_GATEWAY_NAME   7
#define  KEY_GATEWAY_NAME   "gateway_name"
#define CODE_DEV_NAME       8
#define  KEY_DEV_NAME       "dev_name"
#define CODE_RSSI           9
#define  KEY_RSSI           "RSSI"

//

//data structure for ble slave
//device mac
//device name
//temp
//humidity
//svm
//fall_detected
//vel_mag
//step_count
//
//data struct for beacon
//device mac
//device name
//rssi
//

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
    for (int i=0;i<length;i++) {
        Serial.print(char(*(pData+i)));
    }
    Serial.println("");
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
    /*void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.print("BLE Advertised Device found: ");
        Serial.print("RSSI: ");
        Serial.print(advertisedDevice.getRSSI());
        Serial.println(advertisedDevice.toString().c_str());
    
        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
            // 
            Serial.println("Found slave device! ^^^"); 
            advertisedDevice.getScan()->stop();
            BLEscanState = STOPPED;
    
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            doConnect = true;
            if (connectToServer(*pServerAddress)) {
                Serial.println("We are now connected to the BLE Server.");
                connected = true;
            } else {
                Serial.println("We have failed to connect to the server; there is nothin more we will do.");
            }
        } // Found our server
        else if(advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(beaconServUUID)) {
            Serial.println("Found beacon! ^^^");
        } // Found a beacon
        else {
            Serial.println("Not our device");
        }// onResult
    }*/
    } // Found our server
  }
}; // MyAdvertisedDeviceCallbacks


void setup() {
    
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");

    Serial.println();
    Serial.println("Starting wifi");
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
    
    pBLEScan->start(BLE_SCAN_TIME);
    BLEscanState = STARTED;
    
    ble_scan_start_t = millis();
    while(1) {
        if(millis()-ble_scan_start_t >= BLE_SCAN_TIME_RESTART || BLEscanState == STOPPED) {
            pBLEScan->start(BLE_SCAN_TIME);
            ble_scan_start_t = millis();
            BLEscanState = STARTED;
        }   
    }
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

void wifi_alive() {
    
}

void http_body() {
    
}
