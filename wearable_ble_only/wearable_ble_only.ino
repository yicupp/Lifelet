#include <SoftwareSerial.h>

SoftwareSerial hmBeacon(10,11);     //RX,TX
SoftwareSerial hmSlave(4,5);        //Rx,Tx

int LEDPIN = 13;

#define SLAVE_NAME      "LIFELETS0001"
#define BEACON_NAME     "LIFELETB0001"

// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266

#define DEBUG


int AT_ok( char *str ) 
{
    if( str[0] == 'O' && str[1] == 'K' ) return 0;
    return 1;
}

void AT_set( char *cmd ) {
    char buf[50];
    sprintf( buf, "AT+%s", cmd);
    mySerial.write( buf );
#ifdef DEBUG
    Serial.print( "AT_set ");
    Serial.println( buf );
#endif
}

void AT_get( char *cmd ) {
    char buf[50];
    sprintf( buf, "AT+%s?", cmd);
    mySerial.write( buf );
    if( debug ) {
        Serial.print( "AT_set ");
        Serial.println( buf );
    }
}

int AT_cmd( char *cmd, int to_s, int to_f ) {
    char msg_buff[50] = {0};
    char msg_size = 50;

#ifdef DEBUG
        Serial.print( "AT_cmd ");
        Serial.print( cmd );
        Serial.println( " sent");
#endif
     
    AT_set( cmd );
    getmsg_blk( to_s, to_f, msg_buff, msg_size );

#ifdef DEBUG
        Serial.print( "BLE " );
        Serial.print( cmd );
        Serial.print( " returned " );
        Serial.print( msg_buff );
#endif
    
    if( msg_buff[0] == 'O' && msg_buff[1] == 'K' ) 
    {
#ifdef DEBUG        
        Serial.println( " **** OK" );
#endif
        return 0;
    }
    else 
    {
#ifdef DEBUG
        Serial.println( " **** not OK" );
#endif
        return 1;
    }
}

int AT_scan_for_mac(char *mac, int to_s, int to_f) 
{
    char buf[50] = {'\0'};
    if( debug ) 
    {
        Serial.print( "Starting iBeacon scan for mac " );
        Serial.println( mac );
    }
    AT_get( "DISI" );
    getmsg_blk( buf, to_s, to_f, 50 );
    if( AT_ok( buf ) == 0 ) {
#ifdef DEBUG
        Serial.println( "Scan started" );
#endif
        while( getmsg_blk( buf, to_s, to_f, 50 ) == 0) {
            if( ar_substr(buf, wrb_mac) ) 
            {
#ifdef DEBUG                
                Serial.println( "Found device !" );
#endif
                while( getmsg_blk( buf, to_s, to_f, 50 ) == 0 ) if( ar_cmp( buf, "OK+DISCE", 8 ) == 0 ) break;
                return 0;
            }
        }
        if( ar_cmp( buf, "OK+DISCE", 8 ) == 0 ) 
        {
#ifdef DEBUG
            Serial.println( "Failed to find device" );
#endif            
            return 1;
        }
    }
    else {
#ifdef DEBUG
        Serial.println( "Scan failed to start" );
#endif        
        return 2;
    }
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
    if( debug ) Serial.println( "Debug mode on!" );
#endif


    //initialise hmbeacon
    Serial.println("***********************************************\n");
    Serial.println( "Initialising BLE Module" );
    
    Serial.println( "Checking BLE Module status" );
 

    
    mySerial.write( "AT" );
    getmsg_blk( 50000, 500, msg_buff, msg_size );
    if( msg_buff == "OK" ) 
    {
        Serial.println( "BLE Module OK" );
    }
    Serial.println( "" );

    Serial.println( "Setting power on mode" );
    AT_cmd( "IMME1", 500000, 500 );
    Serial.println( "" );
    
    Serial.println( "Finding Device name: " );
    AT_cmd( "NAME?", 500000, 500 );
    Serial.println( "" );

    Serial.println( "Finding Device MAC: " );
    AT_cmd( "ADDR?", 500000, 500 );
    Serial.println( "" );

    Serial.println( "Finding software version: " );
    AT_cmd( "VERR?", 500000, 500 );
    Serial.println( "" );
    
    Serial.println( "Setting BLE Server mode" );
    AT_cmd( "ROLE0", 500000, 500 );
    Serial.println( "" );

    Serial.println( "**********************************************" );
    Serial.println( "BLE INIT COMPLETE" );
    Serial.println( "**********************************************" );
/*
    Serial.println( "\n\n" );
    Serial.println( "**********************************************" );
    Serial.print( "Beginning discovery for " );
    Serial.println( wrb_mac );
    Serial.println( "**********************************************" );

    Serial.println("Discovery complete");
*/
    Serial.println("Starting loop");
    Serial.println( "+++++++++++++++++++++++++++++++++++++++++++++++" );
}

//get msg from thing with a timeout 
int getmsg_blk( int to_start, int to_finish, char *buff, int buff_size ) {
    int count = 0;
    int charc = 0;

    while( count < to_start && mySerial.available() == 0) {
        count++;
    }
    if(count == to_start) {
        Serial.println("Msg Receive timed out");
        return 1;
    }
    
    while( count < to_finish && charc < buff_size - 1 ) {
        if( mySerial.available() ) {
             //mySerial.read() ;
             buff[charc] = mySerial.read();
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
    // listen for communication from the ESP8266 and then write it to the serial monitor
    if ( mySerial.available() )   
    { 
        prev = 0;
        Serial.write( mySerial.read() );  
    }
    else 
    {
        if(prev > to) 
        {
            Serial.println("");
            prev = -1;
        }
        if(prev != -1) {
            prev ++;
        }
    }
 
    // listen for user input and send it to the ESP8266
    if ( Serial.available() )       {  mySerial.write( Serial.read() );  }

    //mySerial.write("AT");
}
