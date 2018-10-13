#include <SoftwareSerial.h>

SoftwareSerial hmBeacon(10,11);     //RX,TX
SoftwareSerial hmSlave(4,5);        //Rx,Tx

int LEDPIN = 13;

#define SLAVE_NAME      "LIFELETS0002"
#define BEACON_NAME     "LIFELETB0002"

#define SLAVE_SERV_ID    "0x1234"
#define BEACON_SERV_ID   "0xABCD"

#define SLAVE_CHAR_ID       "0x6969"
#define BEACON_CHAR_ID      "0xDADA"

#define SLAVE_MEAS_POW      "0xC5"
#define BEACON_MEAS_POW     "0xC5"

#define RENEW           
#define RENEW_DELAY     2500

//timeout values
#define TOS     500000
#define TOF     500

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
    Serial.println( "Initialising Beacon module" );

    hmBeacon.listen();
    Serial.println( "Finding Device MAC: " );
    beaCmd( "ADDR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( "Finding software version: " );
    beaCmd( "VERR?", TOS, TOF );
    Serial.println( "" );

#ifdef RENEW
    Serial.println("Restoring to default settings");
    beaCmd( "RENEW", TOS, TOF );
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

    Serial.println("Restarting module");
    beaCmd("RESET",TOS,TOF);
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

#ifdef RENEW
    Serial.println("Restoring to default settings");
    slvCmd( "RENEW", TOS, TOF );
    Serial.println( "" );
    delay(RENEW_DELAY);
#endif

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

    Serial.println("Restarting module");
    slvCmd("RESET",TOS,TOF);
    Serial.println("");

    Serial.println( "**********************************************" );
    Serial.println( "PERIPH INIT COMPLETE" );
    Serial.println( "**********************************************" );

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

    while( count < to_start && hmSlave.available() == 0) {
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
