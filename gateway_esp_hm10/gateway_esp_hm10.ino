#include <HardwareSerial.h>

//timeout values
#define TOS_BAC     1000
#define TOF_BAC     250
#define TOS_SLV     1000
#define TOF_SLV     250

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

#define RENEW           
#define RENEW_DELAY     2000
#define DEBUG

HardwareSerial hmSlave(1);
HardwareSerial hmBacon(2);

#define CMD_BUF_LEN 50
static char cmdBuf[CMD_BUF_LEN] = {'\0'};

void bacCmd(char * cmd, unsigned long tos, unsigned long tof) {
    int i = 0;
    //send at command
    sprintf(cmdBuf,"AT+%s",cmd);
    hmBacon.write(cmdBuf);
/*    while(cmdBuf[i]!='\0') {
        hmBacon.write(cmdBuf[i]);
        Serial.write(cmdBuf[i]);
        Serial.println(millis());
        i++;
    }*/
    //hmBacon.write(cmdBuf);
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
/*    while(cmdBuf[i]!='\0') {
        hmSlave.write(cmdBuf[i]);
        i++;
    }*/
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
/*while(1){
    while (hmSlave.available() > 0) {
        Serial.write(hmSlave.read());Serial.println(millis());        
    }
    while(hmBacon.available() > 0) {
        Serial.write(hmBacon.read());Serial.println(millis());
    }
    while(Serial.available() > 0) {
        char in = Serial.read();Serial.println(millis());
        if(in == 'b') hmBacon.write(in);
        else if(in == 'B') {bacCmd( "ADDR?", TOS_BAC, TOF_BAC );}
        else if(in == 'c') {hmBacon.write('A');hmBacon.write('T');}
        else if(in == 'C') {hmSlave.write('A');hmSlave.write('T');}
        else hmSlave.write(in);
    }
}*/

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
}

unsigned long tl = 0;
char usrBuf[50]={'\0'};

void loop() {
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
    while(hmBacon.available() > 0 && hmBacon.available()>0) {
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
