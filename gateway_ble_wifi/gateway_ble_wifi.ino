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

#define BLE_SCAN_TIME 2 //scan time in sec
#define BLE_SCAN_TIME_RESTART 3000 //scan restarts every () ms

/*int index_count = 0;
int temp[5]
void storeData() {
    index_count++
    index_count = index_count %5;
    temp[index_count] = temperature;
    acc[index_count] = acceleration;
}*/

//wifi
#include <WiFi.h>

//const char* ssid     = "yicup";
//const char* password = "aaaaaaaa";

const char* ssid     = "Terrortown";
const char* password = "aaaaaaaa";

const char* host = "47.91.46.124";

char *http_buff[500] = {'\0'};
// \wifi

//BLE packet structure:

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

// Data structures

#define LEN_DEV_BUFF     10 //allocated enough space for 10 devices

#define LEN_MAC             20
#define LEN_TEMP            50
#define LEN_HUMIDITY        50
#define LEN_SVM             50
#define LEN_VEL_MAG         50
#define LEN_STEP_COUNT      50
#define LEN_FALL_DETECTED   50
#define LEN_DEV_NAME        50
#define LEN_RSSI            50

struct devData {
    char             mac[LEN_MAC];
    char        humidity[LEN_HUMIDITY];
    char            temp[LEN_TEMP];
    char             svm[LEN_SVM];
    char         vel_mag[LEN_VEL_MAG];
    char      step_count[LEN_STEP_COUNT];
    char   fall_detected[LEN_FALL_DETECTED];
    char        dev_name[LEN_DEV_NAME];
    char            rssi[LEN_RSSI];
    unsigned long   time_t;
}devD;

struct devData   devBuf[LEN_DEV_BUFF];
int    devNum = 0;

unsigned long getStart = 0;
bool getData = false;

//data structure for ble slave
//device mac
//device name
//temp
//humidity
//svm
//fall_detected
//vel_mag
//step_count
//time
//
//data struct for beacon
//device mac
//device name
//rssi
//time
//

// More BLE
BLEClient*  pClient;
char bleBuff[50];
#define BLE_TIMEOUT 400
//

//Watchdog 
#include "esp_system.h"

const int wdtTimeout = 3000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

//
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    int i;
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.println(pBLERemoteCharacteristic->getHandle());
    // Read the value of the characteristic.
    for (i=0;i<length;i++) {
        Serial.print(char(*(pData+i)));
        bleBuff[i]=char(*(pData+i));
    }
    bleBuff[length]='\0';
    Serial.println("");
    timerWrite(timer, 0); //reset timer (feed watchdog)
    getStart = millis();//reset timeout timer
}

bool connectToServer(BLEAddress pAddress) {
    Serial.print("Forming a connection to ");
    Serial.println(pAddress.toString().c_str());
    
    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    timerWrite(timer, 0); //reset timer (feed watchdog)
    // Connect to the remove BLE Server.
    if(pClient->connect(pAddress)) {
        Serial.println(" - Connected to server");
        getData = true;
    } else {
        Serial.println("Failed to connect to server");
        return false;
    }
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
    timerWrite(timer, 0); //reset timer (feed watchdog)
    // Read the value of the characteristic.
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());

    pRemoteCharacteristic->registerForNotify(notifyCallback);
    pRemoteCharacteristic->writeValue('s', 1);
    
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        timerWrite(timer, 0); //reset timer (feed watchdog)
        Serial.print("BLE Advertised Device found: ");
        Serial.print("RSSI: ");
        Serial.print(advertisedDevice.getRSSI());
        Serial.println(advertisedDevice.toString().c_str());
    
        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
    
          // 
            Serial.print("Found our device!  address: "); 
            //advertisedDevice.getScan()->stop();
            //free(pServerAddress);
            devNum++;
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            //doConnect = true;
        } // Found our server
        else if(advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(beaconServUUID)) {
            Serial.println("Found beacon! ^^^");
        } // Found a beacon
        else {
            Serial.println("Not our device");
        }// onResult
        timerWrite(timer, 0); //reset timer (feed watchdog)
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");

    BLEDevice::init("");
    connected = false;
    
    //watchdog
    timer = timerBegin(0, 80, true);                  //timer 0, div 80
    timerAttachInterrupt(timer, &resetModule, true);  //attach callback
    timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
    timerAlarmEnable(timer);                          //enable interrupt
    timerWrite(timer, 0); //reset timer (feed watchdog)
} // End of setup.


// This is the Arduino main loop function.
int con_count = 0;
void loop() {
    timerWrite(timer, 0); //reset timer (feed watchdog)
    if(!connected) {
        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true);
        timerWrite(timer, 0); //reset timer (feed watchdog)
        pBLEScan->start(BLE_SCAN_TIME);
        timerWrite(timer, 0); //reset timer (feed watchdog)
        if(devNum > 0) {
            doConnect = true;
            devNum = 0;
        }
    }
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
      getStart = millis();
      delay(150); //wait for message to be received
      while(getData == true && millis()-getStart<BLE_TIMEOUT) {
        
      }
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    timerWrite(timer, 0); //reset timer (feed watchdog)
    if (connected) {
        String newValue = "Time since boot: " + String(millis()/1000);
        Serial.println("Setting new characteristic value to \"" + newValue + "\"");
        timerWrite(timer, 0); //reset timer (feed watchdog)
        // Set the characteristic's value to be the array of bytes that is actually a string.
        //pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
        con_count++;
        //Serial.println("Written");
    }

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 30 seconds.
   
    Serial.print("Alive at ");
    Serial.println(millis());
    
    //delay(1000); // Delay a second between loops.
    if(connected) {
        timerWrite(timer, 0); //reset timer (feed watchdog)
        Serial.println("Disconnecting client");
        pClient->disconnect();
        getData = false;
        free(pClient);
        free(pServerAddress);
        delay(50);
        con_count = 0;
        Serial.println("Disconnected");
        connected = false;
        Serial.print("Time ");
        Serial.println(millis()/1000);
        timerWrite(timer, 0); //reset timer (feed watchdog)
        push_to_cloud();
        timerWrite(timer, 0); //reset timer (feed watchdog)
    }
    
} // End of loop


void push_to_cloud() {
    
    Serial.println("Starting wifi");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    unsigned long t = millis();
    timerWrite(timer, 0); //reset timer (feed watchdog)
    while (WiFi.status() != WL_CONNECTED) {
        if(millis()-t >= 500) {
            Serial.print(".");
            t = millis();
        }
    }
    timerWrite(timer, 0); //reset timer (feed watchdog)
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("connecting to ");
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    String url = "/input/";


    Serial.print("Requesting URL: ");
    Serial.println(url);
    Serial.println(bleBuff);
    // This will send the request to the server
    client.print(   String("PUT /tablestore HTTP/1.1") + "\r\n" +
                    "Host: 47.91.46.124:5000" + "\r\n" +
                    "Content-Type: application/json" + "\r\n" +
                    "Content-Length: 108" + "\r\n" +    
                    "\r\n" +
                    "{" + "\r\n" +
                    "\"acceleration_x\": \""/*1234*/ +bleBuff+ "\"," + "\r\n" +
                    "\"acceleration_y\": \"54\"," + "\r\n" +
                    "\"acceleration_z\": \"343\"" + "\r\n" +
                    "\"fall_detection\": \"1\"" + "\r\n" +
                    "}" "\r\n" + "\r\n");
    unsigned long timeout = millis();/*
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }*/

    Serial.println();
    Serial.println("closing connection");
    
    WiFi.disconnect();
}
