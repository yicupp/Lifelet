#include <HardwareSerial.h>

//timeout values
#define TOS_BAC     100
#define TOF_BAC     50
#define TOS_SLV     100
#define TOF_SLV     50

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
#define  KEY_DEV_NAME       "device_name"
#define CODE_RSSI           9
#define  KEY_RSSI           "rssi_value"

#define RENEW           
#define RENEW_DELAY     2000
#define DEBUG

#define WEARABLE_BASE_NAME "LLWEARABLE"
#define GATEWAY_BASE_NAME  "LLGATE01"
const int BAC_CONST_SIZE = sizeof(KEY_DEV_NAME)+sizeof(WEARABLE_BASE_NAME)+
sizeof(KEY_RSSI)+sizeof(KEY_GATEWAY_NAME)+sizeof(GATEWAY_BASE_NAME)-5;

//#define DEBUG_TASK

HardwareSerial hmSlave(1);
HardwareSerial hmBacon(2);

#define CMD_BUF_LEN 500
static char cmdBuf[CMD_BUF_LEN] = {'\0'};

#define BLE_RES_FILTER "4C000215" //manufacturer id

//wifi
#include <WiFi.h>
WiFiClient client;

const char* ssid     = "Terrortown";
const char* password = "aaaaaaaa";

const char* host = "47.91.46.124";

char *http_buff[500] = {'\0'};
// \wifi

void bacCmd(char * cmd, unsigned long tos, unsigned long tof) {
    int i = 0;
    //send at command
    sprintf(cmdBuf,"AT+%s",cmd);
    hmBacon.write(cmdBuf);

    Serial.print("BAC sent ");
    Serial.write(cmdBuf);
    unsigned long t=millis();
    while(millis()-t<tos && hmBacon.available() <= 0) {
        
    }
    if(millis()-t>=tos) Serial.println("Bacon response timed out");

    t=millis();
    i=0;
    while(millis()-t<tof) {
        while(hmBacon.available()>0) {
            cmdBuf[i]=hmBacon.read();
            i++;
            t=millis();
        }
        cmdBuf[i]='\0'; 
    }
    Serial.print("Bac ret ");
    Serial.println(cmdBuf);
}

void slvCmd(char * cmd, unsigned long tos, unsigned long tof) {
    //send at command
    int i=0;
    sprintf(cmdBuf,"AT+%s",cmd);
    hmSlave.write(cmdBuf);

    Serial.print("SLV sent ");
    Serial.println(cmdBuf);
    unsigned long t=millis();
    while(millis()-t<tos && hmSlave.available() <= 0) {
        
    }
    if(millis()-t>=tos) Serial.println("Slave response timed out");

    t=millis();
    i = 0;
    while(millis()-t<tof) {
        while(hmSlave.available()>0) {
            cmdBuf[i]=hmSlave.read();
            i++;
            t=millis();
        }
        cmdBuf[i]='\0';
    }
    Serial.print("Slv ret ");
    Serial.println(cmdBuf);
}

bool bacOK() {
    hmBacon.write("AT");
    delay(50);
    if(hmBacon.available() > 0) {
        if(hmBacon.read() == 'O' && hmBacon.read() == 'K') return true;
    }
    return false;
}
bool slvOK() {
    hmSlave.write("AT");
    delay(50);
    if(hmSlave.available() > 0) {
        if(hmSlave.read() == 'O' && hmSlave.read() == 'K') return true;
    }
    return false;
}
unsigned long int sysClk;
unsigned long int slvSchedT;
unsigned long int wifiSchedT;
unsigned long int bacSchedT;
unsigned long int bacSchedR;

void setup() {
    hmSlave.begin(9600, SERIAL_8N1, 16, 17);
    hmBacon.begin(9600, SERIAL_8N1, 13, 14);

    
    Serial.begin(9600);
    Serial.println("***********************************************\n");
    Serial.println( "Initialising BLE" ); 

    Serial.println("Checking device status");
    while(bacOK()==false);
    Serial.println("Bacon ok");
    while(slvOK()==false);
    Serial.println("Slave ok");
    
    Serial.println( "Finding bac Device MAC: " );
    bacCmd( "ADDR?", TOS_BAC, TOF_BAC );
    Serial.println( "" );

    Serial.println( "Finding bac Device MAC: " );
    bacCmd( "ADDR?", TOS_BAC, TOF_BAC );
    Serial.println( "" );

    Serial.println( "Finding bac software version: " );
    bacCmd( "VERR?", TOS_BAC, TOF_BAC );
    Serial.println( "" );

    Serial.println( "Finding slv Device MAC: " );
    slvCmd( "ADDR?", TOS_SLV, TOF_SLV );
    Serial.println( "" );

    Serial.println( "Finding slv software version: " );
    slvCmd( "VERR?", TOS_SLV, TOF_SLV );
    Serial.println( "" );
    
    #ifdef RENEW
        Serial.println("Restoring to default settings");
        bacCmd( "RENEW", TOS_BAC, TOF_BAC );
        Serial.println( "" );
        slvCmd( "RENEW", TOS_SLV, TOF_SLV );
        Serial.println( "" );
        delay(RENEW_DELAY);
    #endif

    Serial.println( "DO not connect IMME1" );
    bacCmd("IMME1", TOS_BAC, TOF_BAC);
    Serial.println( "" );
    slvCmd("IMME1", TOS_SLV, TOF_SLV);
    Serial.println( "" );

    Serial.println( "Set notifications NOTI1" );
    bacCmd("NOTI1", TOS_BAC, TOF_BAC);
    Serial.println( "" );
    slvCmd("NOTI1", TOS_SLV, TOF_SLV);
    Serial.println( "" );

    Serial.println( "Master ROLE1" );
    bacCmd("ROLE1", TOS_BAC, TOF_BAC);
    Serial.println( "" );
    slvCmd("ROLE1", TOS_SLV, TOF_SLV);
    Serial.println( "" );

    Serial.println( "Show name SHOW1" );
    bacCmd("SHOW1", TOS_BAC, TOF_BAC);
    Serial.println( "" );
    slvCmd("SHOW1", TOS_SLV, TOF_SLV);
    Serial.println( "" );

    Serial.println( "Discovery time" );
    bacCmd("SCAN1", TOS_BAC, TOF_BAC);
    Serial.println( "" );
    slvCmd("SCAN1", TOS_SLV, TOF_SLV);
    Serial.println( "" );

    sysClk=millis();
    slvSchedT=millis()+slvSchedT;
    wifiSchedT=millis()+wifiSchedT;
    bacSchedT=millis()+bacSchedT;
}

unsigned long tl = 0;
char usrBuf[50]={'\0'};

bool BLEconnected = false;
bool WIFIconnected = false;



#define SLAVE_PERIOD    70
#define WIFI_PERIOD     333
#define BACON_PERIOD    1200
#define BACON_READ_PERIOD 200

#define SLV_BUF_SIZE    500
#define BAC_BUF_SIZE    500
#define WIFI_BUF_SIZE   750

char slvBuf[SLV_BUF_SIZE]={'\0'};
char bacBuf[BAC_BUF_SIZE]={'\0'};
char wifiBuf[WIFI_BUF_SIZE]={'\0'};

#define BEACON_ADV_UUID0    "BBBBBBBB" //type is beacon




int slvRead(char *buf) {
    int i=0;
    while (hmSlave.available() > 0) {
        buf[i]=hmSlave.read();
        i++;  
    }
    buf[i]='\0';
    Serial.println("--Bacon read buffer--");
    Serial.println(buf);
    Serial.println("---------End---------");
    return i;
}

int bacRead(char *buf) {
    int i = 0;
#ifdef DEBUG_TASK
    Serial.println("bacRead::");
#endif
    while(hmBacon.available() > 0) {
        buf[i]=hmBacon.read();
        i++;
    }
    buf[i]='\0';
    Serial.println("--Bacon read buffer--");
    Serial.println(buf);
    Serial.println(i);
    Serial.println("---------End---------");
    return i;
}

void serialCmd() {
    int i=0;
    while (hmSlave.available() > 0) {
        //Serial.write(hmSlave.read());//Serial.println(millis());
        tl = millis();
        i=0;
        while(millis()-tl<2 && hmSlave.available()>0) {
            usrBuf[i]=hmSlave.read();
            i++;
        }
        usrBuf[i]='\0';
        Serial.println(usrBuf);
    }
    while(hmBacon.available() > 0) {
        //Serial.write(hmBacon.read());//Serial.println(millis());
        tl = millis();
        i=0;
        while(millis()-tl<2) {
            usrBuf[i]=hmBacon.read();
            i++;
        }
        usrBuf[i]='\0';
        Serial.println(usrBuf);
    }
    while(Serial.available() > 0) {
        char in = Serial.read();//Serial.println(millis());
        if(in == 'b' || in == 'B') hmBacon.write(in);
        else hmSlave.write(in);
    }
}

//Slave task: read data coming into the port OR connect to device
int slvTask() {
    //if not connected, connect
    if(!BLEconnected) {
        
    }
    else { //read all data from buffer
        
    }
}

#define BAC_MAC_LEN 13
#define BAC_NAM_LEN 13
#define BAC_NUM_DEV 20
#define BAC_ID_LEN 9
int bacNumID = 0;

struct bacEntry {
    char    mac [BAC_MAC_LEN]={'\0'};
    int     rssi = 0;
    int     id  = 0;
    unsigned long t = 0;
    char    idStr[BAC_ID_LEN]={'\0'};
    char    rssiStr[5]={'\0'};
};

bacEntry bacData[BAC_NUM_DEV];

int bacStore(char *bacBuf, int len) {
    //make sure some response was given
    char *p = bacBuf;
    
    if(len<50) return 1;
    //parse and store
    //skip to +
    parseDISI(p,len);
}

char buf0[9]={'\0'};
char buf1[9]={'\0'};
char buf2[9]={'\0'};
char buf3[9]={'\0'};
char buf4[13]={'\0'};
char buf5[5]={'\0'};

void parseDISI(char *p, int len) {
    int i = 0;
    int id = 0;

    //skip OK+DISC:
    while(i<len) {
        //skip to start of useful data
        while(p[i]!='+'&&i<len) {
            i++;
        }
        i++;
        while(p[i]!=':') {
            i++;
        }
        i++;
        memcpy(buf0,p+i,8);//get manufacturer id
        Serial.println(buf0);
        //Serial.println(BLE_RES_FILTER);
        if(strcmp(buf0,BLE_RES_FILTER)) { //wrong id? skip
            Serial.println(strcmp(buf0,BLE_RES_FILTER));
            Serial.println("Skipped device");
        }
        else {
            i+=8;
            i++;//skip over :
            memcpy(buf0,p+i,8);
            i+=8;
            memcpy(buf1,p+i,8);
            i+=8;
            memcpy(buf2,p+i,8);
            i+=8;
            memcpy(buf3,p+i,8);
            i+=8;
            i+=10; //skip major minor
            i+=2;  //skip ::
            
            memcpy(buf4,p+i,12);//get mac
            i+=12;
            i++;//skip :
            memcpy(buf5,p+i,4);//get rssi
            i+=4;
            
            Serial.println(buf0);//id0
            Serial.println(buf1);//id1
            Serial.println(buf2);//id2
            Serial.println(buf3);//id3
            Serial.println(buf4);//mac
            Serial.println(buf5);//rssi
            if(strcmp(buf0,buf3)==0&&strcmp(buf1,buf2)==0) {
                addEntry();
            }
        }
    }
}

int addEntry() {
    int id = atoi(buf1);
    int i = 0;
    while(bacData[i].id != id && i<=bacNumID-1) {
        i++;
    }
    if(i == BAC_NUM_DEV) {
        Serial.println("No space for beacon data");
        return 1;
    }
    if(i == bacNumID) bacNumID++;
    if(strcmp(buf0,BEACON_ADV_UUID0)!=0) {
        Serial.println("Slave / unknown device found. Not logging");
        return -1;
    }
    memcpy(bacData[i].mac,buf4,12);
    bacData[i].id=id;
    bacData[i].rssi=atoi(buf5);
    bacData[i].t=millis();
    memcpy(bacData[i].rssiStr,buf5,4);
    memcpy(bacData[i].idStr,buf1,8);
    Serial.print("Logged at index");
    Serial.print(i);
    Serial.print('/');
    Serial.print(bacNumID);
    Serial.print("devs. ");
    Serial.print(" : ");
    Serial.print(bacData[i].id);
    Serial.write(' ');
    Serial.print(bacData[i].rssi);
    Serial.write(' ');
    Serial.println(bacData[i].t);

}

//Bacon task: read beacon information
int bacTask() {
    int ret = 0;
#ifdef DEBUG_TASK
    Serial.println("bacTask::");
#endif
    //Read previous scan results
    ret = bacRead(bacBuf); 
    
    //Start a new scan
    hmBacon.write("AT+DISI?");
    Serial.println( "" );

    //Update table
    bacStore(bacBuf,ret);
}

int wifiPrevState = WL_DISCONNECTED;
bool hostPrevConnected = false;

//Wifi task: upload data to cloud
int wifiTask() {
    int wifiState = WiFi.status();
    if(wifiState == WL_IDLE_STATUS) {
        Serial.print("Still attempting to connect to");
        Serial.println(ssid);
        wifiPrevState = wifiState;
        return -1;
    }
    if(wifiState == WL_CONNECT_FAILED) {
        Serial.println("Connection failed");
        wifiPrevState = wifiState;
        return 1;
    }
    if(wifiState != WL_CONNECTED) {
        if(wifiPrevState == WL_CONNECTED) {
            Serial.println("Wifi no connection. Reconnecting...");
        }
        wifiPrevState = WL_DISCONNECTED;
        wifiConnect();
        return 1;
    }
    if(wifiPrevState != WL_CONNECTED && wifiState == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connection established");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        wifiPrevState = WL_CONNECTED;
    }
    else if(wifiState == WL_CONNECTED && wifiPrevState == WL_CONNECTED) {
        Serial.println("Wifi connection maintained");
    }

    bool hostConnected = client.connected();
    if(hostConnected == false) {
        if(hostPrevConnected == false) {
            Serial.println("Starting new connection to server");
        }
        else {
            Serial.println("Lost connection. Reconnecting to server");
        }
        const int httpPort = 80;
        if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            hostPrevConnected = false;
            return 2;
        }
        Serial.print("Successfully connected to ");
        Serial.println(host);
        hostPrevConnected = true;
    }
    else {
        Serial.println("Host connection maintained");
    }
    
    //send advertisement packets if any 
    if(bacNumID>0) {
        Serial.println("Uploading beacon logs");
        for(int i=0;i<bacNumID;i++) {
            wifiSendBacPac(i);
        }
        bacNumID=0;
    }
    else {
        Serial.println("No beacon logs to upload");
    }

    return 0;
}

char contLenStr[11] = {'\0'};

int wifiSendBacPac(int i) {
    int contLen = 15+BAC_CONST_SIZE+50+19;
    sprintf(contLenStr,"%d",contLen); 
       
    client.print( String(
"PUT /tablestore1 HTTP/1.1")+"\r\n"+
"Host: "+host+":80\r\n"+
"Content-Type: application/json"+"\r\n"+
"Connection: close"+"\r\n"+
"Content-Length: "+contLenStr+"\r\n"+
"\r\n"+
"{"+"\r\n"+
"\""+KEY_GATEWAY_NAME+"\" : \""+GATEWAY_BASE_NAME+"\",\r\n"+
"\""+KEY_DEV_NAME+"\" : \""+String(bacData[i].mac)+WEARABLE_BASE_NAME+String(bacData[i].idStr)+"\",\r\n"+
"\"data_type\" : \"rssi\",\r\n"+
"\""+KEY_RSSI+"\" : \""+String(bacData[i].rssiStr)+"\"\r\n"+
"}\r\n\r\n");

while (client.available() == 0) {}

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    
    return 0;
}
int wifiConnect() {
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
}

void slvTaskChk() {
    if(millis()>slvSchedT) {
        slvSchedT=millis()+SLAVE_PERIOD;
        slvTask();
    }
}

void bacTaskChk() {
#ifdef DEBUG_TASK
    Serial.println("bacTaskChk::");
#endif
    if(millis()>bacSchedT) {
        bacSchedT=millis()+BACON_PERIOD;
        bacTask();
    }
}

void wifiTaskChk() {
    if(millis()>wifiSchedT) {
        wifiSchedT=millis()+WIFI_PERIOD;
        wifiTask();
    }
} 

//read values and update table
void bacReadTask() {
    int ret;
    //Read previous scan results
    ret = bacRead(bacBuf); 

    //Update table
    bacStore(bacBuf,ret);
}

void bacReadChk() {
    if(millis()>bacSchedR) {
        bacSchedR=millis()+BACON_READ_PERIOD;
        bacReadTask();
    }
} 

void loop() {
    //serialCmd();  
    slvTaskChk();
    bacTaskChk();
    bacReadChk();
    wifiTaskChk();
}
