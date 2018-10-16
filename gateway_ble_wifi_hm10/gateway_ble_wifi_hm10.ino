/**
 * A BLE client example that is rich in capabilities.
 */

#include "BLEDevice.h"
//#include "BLEScan.h"
#include <HardwareSerial.h>

HardwareSerial Bslave(1);
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
#define BLE_SCAN_TIME_RESTART 3500 //scan restarts every () ms

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

#define BLE_BEACON_TIMEOUT  30000//time in ms before advertiser is discarded

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
    unsigned int  dev_id;
    unsigned long time_t;
    char         isSlave;
    char        isBeacon;
}devD;

struct advData {
    char             mac[LEN_MAC];   
    char        dev_name[LEN_DEV_NAME];
    char            rssi[LEN_RSSI];
    unsigned int  dev_id;
    unsigned long time_t;
}advD;

#define LEN_ADV_BUFF    10
//struct devData   devBuf[LEN_DEV_BUFF];
struct advData   advBuf[LEN_ADV_BUFF];
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
int con_count = 0; //number of opssible connections
int adv_count = 0; //number of detected advertisements
//

//Watchdog 
#include "esp_system.h"

const int wdtTimeout = 5000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;

void IRAM_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

//

//Serial comms
#define CMD_BUFF_SIZE   50
#define USR_BUFF_SIZE   100
#define RENEW           
#define RENEW_DELAY     2000

//timeout values
#define TOS     500000
#define TOF     500

char cmdBuff[CMD_BUFF_SIZE] = {'\0'};
char usrBuff[USR_BUFF_SIZE] = {'\0'};

void BslaveSend( char *cmd ) {
    char buf[50];
    sprintf( buf, "AT+%s", cmd);
    Bslave.write( buf );
#ifdef DEBUG
    Serial.print( "AT_set ");
    Serial.println( buf );
#endif
}

int BslaveCmd( char *cmd, int to_s, int to_f ) {
    char msg_buff[50] = {0};
    char msg_size = 50;
    delay(50);
    
#ifdef DEBUG
        Serial.print( "AT_cmd ");
        Serial.print( cmd );
        Serial.println( " sent");
#endif
     
    BslaveSend( cmd );
    BslaveGet( to_s, to_f, msg_buff, msg_size );

#ifdef DEBUG
        Serial.print( "AT_RET ");
        Serial.println( msg_buff );
#endif
    return 0;
}

//get msg from thing with a timeout from slave
int BslaveGet( int to_start, int to_finish, char *buff, int buff_size ) {
    int count = 0;
    int charc = 0;

    while( count < to_start && Bslave.available() == 0) {
        count++;
    }
    if(count == to_start) {
        Serial.println("Msg Receive timed out");
        return 1;
    }
    
    while( count < to_finish && charc < buff_size - 1 ) {
        if( Bslave.available() ) {
             //mySerial.read() ;
             buff[charc] = Bslave.read();
             count = 0;
             charc++;
        }
        count++;
    }
    buff[charc] = '\0';
    return 0;
}

//

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
            Serial.print("Found our device!  address: "); 
            devNum++;
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            //doConnect = true;
        } // Found our server
        else if(advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(beaconServUUID)) {
            Serial.println("Found beacon! ^^^");
            adv_count++;
            BLEstoreAdv(advertisedDevice);
        } // Found a beacon
        else {
            Serial.println("Not our device");
        }// onResult
        timerWrite(timer, 0); //reset timer (feed watchdog)
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
    Serial.begin(9600);
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
   
    Serial.print("Alive at ");
    Serial.println(millis());
    con_count = 0;
    adv_count = 0;
    
    //delay(1000); // Delay a second between loops.
    if(connected) {
        timerWrite(timer, 0); //reset timer (feed watchdog)
        Serial.println("Disconnecting client");
        pClient->disconnect();
        getData = false;
        free(pClient);
        free(pServerAddress);
        delay(100);
        con_count = 0;
        Serial.println("Disconnected");
        connected = false;
        Serial.print("Time ");
        Serial.println(millis()/1000);
        timerWrite(timer, 0); //reset timer (feed watchdog)
        push_to_cloud();
        timerWrite(timer, 0); //reset timer (feed watchdog)
        connected = false;
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

//Structure for reference
/*
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
    char         isSlave;
    char        isBeacon;
    unsigned int  dev_id;
    unsigned long time_t;
}devD;
 */

/*
 * #define CODE_HUMIDITY       1
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
*/

void BLEstoreData(unsigned char *dat, int len) {
    //data might be the wearable name
    int i=0;
    int index = 0;
    int id = 0;
    float scale = 1;
    //device info
    if(dat[0]=='I') {
        id = dat[1];
        int entry_size = dat[3];
        
        Serial.print("ID: ");
        Serial.print(char(id));
        Serial.print(" Device name: ");
        for(int x=5;x<len;x++) Serial.write(dat[x]);
        Serial.println();
    }
    else { //find the scaler value

    }
}

void BLEstoreAdv(BLEAdvertisedDevice advDev) {
    int i = 0;
    int id = String(advDev.getName().c_str()[8]).toInt();
    Serial.print("Storing advertisement data of id");
    Serial.println(id);
    int adv_entry = adv_count-1;

    //find the index
    while(i<adv_entry) {
        if(id == devBuf[i].dev_id) break;
        i++;
    }

    Serial.print("Storing advertisement in index ");
    Serial.println(i);
    devDataClean(adv_entry);
    devData *buffToAdd = &devBuf[i];

    buffToAdd->dev_id=id;
    buffToAdd->time_t=millis();
    buffToAdd->isBeacon = 1;
    sprintf(buffToAdd->rssi,"%d",advDev.getRSSI());
    sprintf(buffToAdd->dev_name,"%s",advDev.getName().c_str());
}

//cleans the ith entry of the data table
void devDataClean(int i) {
    devData* bufToClean = &advBuf[i];
    bufToClean->mac[0]='\0';
    bufToClean->humidity[0]='\0';
    bufToClean->temp[0]='\0';
    bufToClean->svm[0]='\0';
    bufToClean->vel_mag[0]='\0';
    bufToClean->step_count[0]='\0';
    bufToClean->fall_detected[0]='\0';
    bufToClean->dev_name[0]='\0';
    bufToClean->rssi[0]='\0';
    bufToClean->isBeacon = 0;
    bufToClean->isSlave = 0;
    bufToClean->dev_id=0;
    bufToClean->time_t=0;
}
