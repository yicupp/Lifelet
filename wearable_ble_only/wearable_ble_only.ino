#include <SoftwareSerial.h>

SoftwareSerial hmBeacon(10,11);     //RX,TX
SoftwareSerial hmSlave(4,5);        //Rx,Tx

int LEDPIN = 13;

#define SLAVE_NAME      "LLWearable01"
#define BEACON_NAME     "LIFELETB0001"

#define DEVICE_ID           1
#define DEVICE_NAME         "LLWearable01"

#define SLAVE_SERV_ID    "0x1234"
#define BEACON_SERV_ID   "0xABCD"

#define SLAVE_CHAR_ID       "0x6969"
#define BEACON_CHAR_ID      "0xDADA"

#define SLAVE_MEAS_POW      "0xC5"
#define BEACON_MEAS_POW     "0xC5"

#define SLAVE_ADV_UUID0     "69696969"
#define SLAVE_ADV_UUID1     "01020304"
#define SLAVE_ADV_UUID2     "40302010"
#define SLAVE_ADV_UUID3     "69696969"

#define SLAVE_ADV_MAJOR     "00000001"
#define SLAVE_ADV_MINOR     "00000001"

#define BEACON_ADV_UUID0    "AAAAAAAA"
#define BEACON_ADV_UUID1    "0A0B0C0D"
#define BEACON_ADV_UUID2    "D0C0B0A0"
#define BEACON_ADV_UUID3    "AAAAAAAA"

#define BEACON_ADV_MAJOR    "00000001"
#define BEACON_ADV_MINOR    "00000001"

#define RENEW           
#define RENEW_DELAY     2000

//timeout values
#define TOS     500000
#define TOF     500

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

#define DATA_FIELD_NUM      6   //number of data fields 
#define DATA_ENTRY_NUM      8   //number of data entriess
#define DATA_ENTRY_SIZE     1   //number of bytes per data entry
#define DATA_SIZE           (2+DATA_ENTRY_NUM*DATA_ENTRY_SIZE)*DATA_FIELD_NUM

unsigned char sensorData[DATA_FIELD_NUM][DATA_ENTRY_NUM] = {'\0','\0'};

// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266

#define DEBUG

#define CMD_BUFF_SIZE   50
#define USR_BUFF_SIZE   100

char cmdBuff[CMD_BUFF_SIZE] = {'\0'};
char usrBuff[USR_BUFF_SIZE] = {'\0'};

int AT_ok( char *str ) 
{
    if( str[0] == 'O' && str[1] == 'K' ) return 0;
    return 1;
}

void slvSend( char *cmd ) {
    char buf[50];
    sprintf( buf, "AT+%s", cmd);
    hmSlave.write( buf );
#ifdef DEBUG
    Serial.print( "AT_set ");
    Serial.println( buf );
#endif
}

void beaSend( char *cmd ) {
    char buf[50];
    sprintf( buf, "AT+%s", cmd);
    hmBeacon.write( buf );
#ifdef DEBUG
    Serial.print( "AT_set ");
    Serial.println( buf );
#endif
}

int slvCmd( char *cmd, int to_s, int to_f ) {
    char msg_buff[50] = {0};
    char msg_size = 50;
    delay(50);

#ifdef DEBUG
        Serial.print( "AT_cmd ");
        Serial.print( cmd );
        Serial.println( " sent");
#endif
     
    slvSend( cmd );
    slvGet( to_s, to_f, msg_buff, msg_size );

#ifdef DEBUG
        Serial.print( "AT_RET " );
        Serial.println( msg_buff );
#endif
    return 0;
}

int beaCmd( char *cmd, int to_s, int to_f ) {
    char msg_buff[50] = {0};
    char msg_size = 50;
    delay(50);
    
#ifdef DEBUG
        Serial.print( "AT_cmd ");
        Serial.print( cmd );
        Serial.println( " sent");
#endif
     
    beaSend( cmd );
    beaGet( to_s, to_f, msg_buff, msg_size );

#ifdef DEBUG
        Serial.print( "AT_RET ");
        Serial.println( msg_buff );
#endif
    return 0;
}

//search for a substring in a string
int ar_substr( char *str, char *sub ) {
    String sstr = String( str );
    if( sstr.indexOf( sub ) != -1) 
    {
        return 1;
    }
    return 0;
}

//compare two strings of size n
int ar_cmp( char* ar1, char* ar2, int n) 
{
    int x = 0;
    while( x < n ) {
        if( ar1[x] != ar2[x] ) return 1;
        x++;
    }
    return 0;
}

void setup() {
    char msg_buff[100] = {0};
    char msg_size = 100;

    pinMode(LEDPIN, OUTPUT);
 
    Serial.begin(9600);     // communication with the host computer
 
    //Set up beacone
    hmBeacon.begin(9600);  

    //set up slave
    hmSlave.begin(9600);
    
#ifdef VERBOSE
    Serial.println("");
    Serial.println("Test\n");
    Serial.println("***********************************************\n");
    Serial.println("");    
#endif
#ifdef DEBUG
    Serial.println( "Debug mode on!" );
#endif


    //initialise hmbeacon
    Serial.println("***********************************************\n");
    Serial.println( "Initialising  module" );

    hmBeacon.listen();
    Serial.println( "Finding Device MAC : " );
    beaCmd( "ADDR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Finding software version: " );
    beaCmd( "VERR?", TOS, TOF );
    Serial.println( "" );

#ifdef RENEW
    Serial.println("Restoring to default settings");
    beaCmd( "RENEW", TOS, TOF );
    Serial.println( "" );
    slvCmd( "RENEW", TOS, TOF );
    Serial.println( "" );
    delay(RENEW_DELAY);
#endif

    Serial.println( "Setting power on mode" );
    beaCmd( "IMME1", TOS, TOF );
    Serial.println( "" );
    
    Serial.print( "Setting device name to: " );
    Serial.println(BEACON_NAME);
    sprintf(cmdBuff, "NAME%s", BEACON_NAME);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println( "" );
    
    Serial.println( "Setting BLE Slave mode" );
    beaCmd( "ROLE0", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Setting advertising interval" );
    beaCmd( "ADVI1", TOS, TOF );
    Serial.println( "" );

    Serial.println("Setting service uuid to:");
    Serial.println(BEACON_SERV_ID);
    sprintf(cmdBuff, "UUID%s", BEACON_SERV_ID);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println("Setting characteristic uuid to:");
    Serial.println(BEACON_CHAR_ID);
    sprintf(cmdBuff, "CHAR%s", BEACON_CHAR_ID);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    

    Serial.println( "**********************************************" );
    Serial.println( "BEACON INIT COMPLETE" );
    Serial.println( "**********************************************" );

    Serial.println("***********************************************\n");
    Serial.println( "Initialising Peripheral module" );

    hmSlave.listen();
    Serial.println( "Finding Device MAC: " );
    slvCmd( "ADDR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Finding software version: " );
    slvCmd( "VERR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Setting power on mode" );
    slvCmd( "IMME1", TOS, TOF );
    Serial.println( "" );
    
    Serial.print( "Setting device name to: " );
    Serial.println(SLAVE_NAME);
    sprintf(cmdBuff, "NAME%s", SLAVE_NAME);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println( "" );
    
    Serial.println( "Setting BLE Slave mode" );
    slvCmd( "ROLE0", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Setting advertising interval" );
    slvCmd( "ADVI1", TOS, TOF );
    Serial.println( "" );

    Serial.println("Setting service uuid to:");
    Serial.println(SLAVE_SERV_ID);
    sprintf(cmdBuff, "UUID%s", SLAVE_SERV_ID);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println("Setting characteristic uuid to:");
    Serial.println(SLAVE_CHAR_ID);
    sprintf(cmdBuff, "CHAR%s", SLAVE_CHAR_ID);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println("Setting advertisement id for beacon");
    sprintf(cmdBuff, "IBE0%s", BEACON_ADV_UUID0);
    beaCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE1%s", BEACON_ADV_UUID1);
    beaCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE2%s", BEACON_ADV_UUID2);
    beaCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE3%s", BEACON_ADV_UUID3);
    beaCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "MARJ%s", BEACON_ADV_MAJOR);
    beaCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "MINO%s", BEACON_ADV_MINOR);
    beaCmd(cmdBuff,TOS,TOF);
    beaCmd("SHOW1",TOS,TOF);
    beaCmd("IBEA1",TOS,TOF);
    beaCmd("NOTI1",TOS,TOF);
    Serial.println("");
    
    Serial.println("Restarting beacon module");
    beaCmd("RESET",TOS,TOF);
    Serial.println("");
    
    Serial.println("Seting ibeacon id for slave module");
    sprintf(cmdBuff, "IBE0%s", SLAVE_ADV_UUID0);
    slvCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE1%s", SLAVE_ADV_UUID1);
    slvCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE2%s", SLAVE_ADV_UUID2);
    slvCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "IBE3%s", SLAVE_ADV_UUID3);
    slvCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "MARJ%s", SLAVE_ADV_MAJOR);
    slvCmd(cmdBuff,TOS,TOF);
    sprintf(cmdBuff, "MINO%s", SLAVE_ADV_MINOR);
    slvCmd(cmdBuff,TOS,TOF);
    slvCmd("SHOW1",TOS,TOF);
    slvCmd("IBEA1",TOS,TOF);
    slvCmd("NOTI1",TOS,TOF);
    Serial.println("");

    Serial.println("Restarting slave");
    slvCmd("RESET",TOS,TOF);
    Serial.println("");
/*
    Serial.println("Restarting beacon module");
    beaCmd("RESET",TOS,TOF);
    Serial.println("");
    
    Serial.println("Restarting slave module");
    slvCmd("RESET",TOS,TOF);
    Serial.println("");*/
    
    Serial.println( "**********************************************" );
    Serial.println( "PERIPH INIT COMPLETE" );
    Serial.println( "**********************************************" );

    Serial.println("Test data:");
    for(int x=0;x<DATA_FIELD_NUM;x++) {
        char start = 'a'+x*DATA_ENTRY_NUM;
        Serial.print(x);
        Serial.print('|');
        for(char y=0;y<DATA_ENTRY_NUM;y++) {
            sensorData[x][y]=start+y;
            Serial.print(char(sensorData[x][y]));
        }
        Serial.println();
    }

    Serial.println("Starting loop");
    Serial.println( "+++++++++++++++++++++++++++++++++++++++++++++++" );
}

//get msg from thing with a timeout from slave
int slvGet( int to_start, int to_finish, char *buff, int buff_size ) {
    int count = 0;
    int charc = 0;

    while( count < to_start && hmSlave.available() == 0) {
        count++;
    }
    if(count == to_start) {
        Serial.println("Msg Receive timed out");
        return 1;
    }
    
    while( count < to_finish && charc < buff_size - 1 ) {
        if( hmSlave.available() ) {
             //mySerial.read() ;
             buff[charc] = hmSlave.read();
             count = 0;
             charc++;
        }
        count++;
    }
    buff[charc] = '\0';
    return 0;
}

//get msg from thing with a timeout from slave
int beaGet( int to_start, int to_finish, char *buff, int buff_size ) {
    int count = 0;
    int charc = 0;

    while( count < to_start && hmBeacon.available() == 0) {
        count++;
    }
    if(count == to_start) {
        Serial.println("Msg Receive timed out");
        return 1;
    }
    
    while( count < to_finish && charc < buff_size - 1 ) {
        if( hmBeacon.available() ) {
             //mySerial.read() ;
             buff[charc] = hmBeacon.read();
             count = 0;
             charc++;
        }
        count++;
    }
    buff[charc] = '\0';
    return 0;
}

int prev = 0;
int to = 500;

void loop() 
{

    //Put your code here
    //Delete the stuff in the loop
    //They are for you to see the sending for yourself
    //Send data using 
    //hmSlave.write(str)
    
    // listen to the slave
    if ( hmSlave.available() )   { 
        while (hmSlave.available() > 0) {
            char inByte = hmSlave.read();
            Serial.write(inByte);
            if(inByte == 'S' || inByte == 's') {
                BLEsendData();
                BLEdisconnect();
            }
        } 
        Serial.write('\n');
    }

    //listen to beacon
    if ( hmBeacon.available() )   { 
        while (hmBeacon.available() > 0) {
            char inByte = hmBeacon.read();
            Serial.write(inByte);
        } 
        Serial.write('\n');
    }
 
    // listen for user input and send it to the slave or beacon
    if ( Serial.available() > 0) {   
        delay(50);//wait for all data to arrive  
        char *p = usrBuff;
        while(Serial.available() > 0) {
            *p = Serial.read();
            //Serial.print(*p);
            p++;
        }
        *p = '\0';
        if(usrBuff[0]=='s' || usrBuff[0]=='S') {
            hmSlave.write(usrBuff+1);
            Serial.print("To slave: ");
            Serial.println(usrBuff+1);
            hmSlave.listen();
        }
        else {
            hmBeacon.write(usrBuff);
            Serial.print("To beacon: ");
            Serial.println(usrBuff);
            hmBeacon.listen();
        }
    }
}

void BLEsendData() {
    //send id, entry size and name
    sprintf(usrBuff,"I%i|%i|%s",DEVICE_ID,DATA_ENTRY_SIZE,DEVICE_NAME);
    hmSlave.write(usrBuff);
    delay(10);
    
    //send data packets
    BLEformData();
}

//disconnect from the master
void BLEdisconnect() {
    delay(10);
    hmSlave.write('C'); //politely inform master of disconnection
    delay(25);
    hmSlave.write("AT"); //pull the plug on the hm10
}

void BLEformData() {
    //Packet structure
    //ID (1B) Type (1B) Data (7x 1B)    
    for(unsigned char i=0;i<DATA_FIELD_NUM;i++) {
        hmSlave.write(DEVICE_ID);
        hmSlave.write('|');
        hmSlave.write(i);
        hmSlave.write('|');
        hmSlave.write(sensorData[i],DATA_ENTRY_NUM*DATA_ENTRY_SIZE);
        delay(10);
    }
}
