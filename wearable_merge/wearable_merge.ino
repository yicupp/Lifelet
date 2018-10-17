// MPU-6050 Short Example Sketch
// Public Domain
#include "Wire.h"
#include <dht.h>

dht DHT;
float temp = 0;
float humid = 0;
char strAm[10]={'\0'};
#define DHT11_PIN 7


const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
float vx = 0, vy = 0, vz = 0;
unsigned long sample_time = 0;
unsigned long t = 0;

//int data[STORE_SIZE][5]; //array for saving past data
//byte currentIndex=0; //stores current data array index (0-255)
boolean fall = false; //stores if a fall has occurred
boolean trigger1 = false; //stores if first trigger (lower threshold) has occurred
boolean trigger2 = false; //stores if second trigger (upper threshold) has occurred
boolean trigger3 = false; //stores if third trigger (orientation change) has occurred

byte trigger1count = 0; //stores the counts past since trigger 1 was set true
byte trigger2count = 0; //stores the counts past since trigger 2 was set true
byte trigger3count = 0; //stores the counts past since trigger 3 was set true
int angleChange = 0;

float Raw_AM = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
float AM = Raw_AM * 10;  // as values are within 0 to 1, I multiplied
  
unsigned long t_1 = 0;
unsigned long t_2 = 0;
unsigned long t_3 = 0;

unsigned long count=0;

#define FALL_RESET_TIME 15000
unsigned long fall_reset_timer = 0;

//Pedometer
#define PED_N 25
float ped_data[PED_N] = {'\0'};
int pedI=0;
float pedMax=0;;
int pedSteps=0;
float pedAvg=0;
float pedSum = 0;
#define PED_THRES_LOWER 
#define PED_THRES_UPPER 

//

//merging files
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

#define SLAVE_ADV_UUID0     "AAAAAAAA" //Type is slave
#define SLAVE_ADV_UUID1     "00000001" //id is 1
#define SLAVE_ADV_UUID2     "00000001"
#define SLAVE_ADV_UUID3     "AAAAAAAA"

#define SLAVE_ADV_MAJOR     "11111111"
#define SLAVE_ADV_MINOR     "00000001"

#define BEACON_ADV_UUID0    "BBBBBBBB" //type is beacon
#define BEACON_ADV_UUID1    "00000001" //id is 1
#define BEACON_ADV_UUID2    "00000001"
#define BEACON_ADV_UUID3    "BBBBBBBB"

#define BEACON_ADV_MAJOR    "22222222"
#define BEACON_ADV_MINOR    "00000001"

#define RENEW           
#define RENEW_DELAY     2000

//timeout values
#define TOS     500000
#define TOF     500

//HTTP keys
#define CODE_HUMIDITY       1
#define CHAR_HUMIDITY       'h'
#define  KEY_HUMIDITY       "humidity"

#define CODE_TEMP           2
#define CHAR_TEMP           't'
#define  KEY_TEMP           "temp"

#define CODE_SVM            3
#define CHAR_SVM            's'
#define  KEY_SVM            "svm"

#define CODE_VEL_MAG        4
#define  KEY_VEL_MAG        "vel_mag"

#define CODE_STEP_COUNT     5
#define CHAR_STEP_COUNT     'c'
#define  KEY_STEP_COUNT     "step_count"

#define CODE_FALL_DETECTED  6
#define CHAR_FALL_DETECTED  'f'
#define  KEY_FALL_DETECTED  "fall_detected"

#define CODE_GATEWAY_NAME   7
#define  KEY_GATEWAY_NAME   "gateway_name"
#define CODE_DEV_NAME       8
#define  KEY_DEV_NAME       "dev_name"
#define CODE_RSSI           9
#define  KEY_RSSI           "RSSI"

#define BLE_DATA_PERIOD     200
#define DATA_FIELD_NUM      6   //number of data fields 
#define DATA_ENTRY_NUM      8   //number of data entriess
#define DATA_ENTRY_SIZE     1   //number of bytes per data entry
#define DATA_SIZE           (2+DATA_ENTRY_NUM*DATA_ENTRY_SIZE)*DATA_FIELD_NUM


// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266

#define DEBUG

#define CMD_BUFF_SIZE   50
#define USR_BUFF_SIZE   100

char cmdBuff[CMD_BUFF_SIZE] = {'\0'};
char usrBuff[USR_BUFF_SIZE] = {'\0'};
unsigned long pack_time = 0;
bool ble_connected = false;

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
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);

  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  t_1 = millis();
  t_2 = millis();
  t_3 = millis();
  //count=millis()+7;

    char msg_buff[100] = {0};
    char msg_size = 100;

    pinMode(LEDPIN, OUTPUT);
 
    Serial.begin(9600);     // communication with the host computer
    Serial.println("Sensor setup complete");
    
    //Set up beacone
    hmBeacon.begin(9600);  

    //set up slave
    hmSlave.begin(9600);
    
#ifdef VERBOSE
    Serial.println("");
    Serial.println("Test\n");
    Serial.println(F("***********************************************\n"));
    Serial.println("");    
#endif
#ifdef DEBUG
    Serial.println( "Debug mode on!" );
#endif


    //initialise hmbeacon
    Serial.println(F("***********************************************\n"));
    Serial.println( F("Initialising  module" ));

    hmBeacon.listen();
    Serial.println( F("Finding Device MAC : " ));
    beaCmd( "ADDR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( F("Finding software version: " ));
    beaCmd( "VERR?", TOS, TOF );
    Serial.println( "" );

#ifdef RENEW
    Serial.println(F("Restoring to default settings"));
    beaCmd( "RENEW", TOS, TOF );
    Serial.println( "" );
    slvCmd( "RENEW", TOS, TOF );
    Serial.println( "" );
    delay(RENEW_DELAY);
#endif

    Serial.println( F("Setting power on mode" ));
    beaCmd( "IMME1", TOS, TOF );
    Serial.println( "" );
    
    Serial.print( F("Setting device name to: " ));
    Serial.println(BEACON_NAME);
    sprintf(cmdBuff, "NAME%s", BEACON_NAME);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println( "" );
    
    Serial.println( F("Setting BLE Slave mode" ));
    beaCmd( "ROLE0", TOS, TOF );
    Serial.println( "" );

    Serial.println( F("Setting advertising interval" ));
    beaCmd( "ADVI1", TOS, TOF );
    Serial.println( "" );

    Serial.println(F("Setting service uuid to:"));
    Serial.println(BEACON_SERV_ID);
    sprintf(cmdBuff, "UUID%s", BEACON_SERV_ID);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println(F("Setting characteristic uuid to:"));
    Serial.println(BEACON_CHAR_ID);
    sprintf(cmdBuff, "CHAR%s", BEACON_CHAR_ID);
    beaCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    

    Serial.println( F("**********************************************" ));
    Serial.println( F("BEACON INIT COMPLETE" ));
    Serial.println( F("**********************************************" ));

    Serial.println(F("***********************************************\n"));
    Serial.println( F("Initialising Peripheral module" ));

    hmSlave.listen();
    Serial.println( F("Finding Device MAC: " ));
    slvCmd( "ADDR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( F("Finding software version: " ));
    slvCmd( "VERR?", TOS, TOF );
    Serial.println( "" );

    Serial.println( F("Setting power on mode" ));
    slvCmd( "IMME1", TOS, TOF );
    Serial.println( "" );
    
    Serial.print( F("Setting device name to: " ));
    Serial.println(SLAVE_NAME);
    sprintf(cmdBuff, "NAME%s", SLAVE_NAME);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println( "" );
    
    Serial.println( F("Setting BLE Slave mode" ));
    slvCmd( "ROLE0", TOS, TOF );
    Serial.println( "" );

    Serial.println( F("Setting advertising interval" ));
    slvCmd( "ADVI1", TOS, TOF );
    Serial.println( "" );

    Serial.println(F("Setting service uuid to:"));
    Serial.println(SLAVE_SERV_ID);
    sprintf(cmdBuff, "UUID%s", SLAVE_SERV_ID);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println(F("Setting characteristic uuid to:"));
    Serial.println(SLAVE_CHAR_ID);
    sprintf(cmdBuff, "CHAR%s", SLAVE_CHAR_ID);
    slvCmd( cmdBuff, TOS, TOF );
    Serial.println("");

    Serial.println(F("Setting advertisement id for beacon"));
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
    
    Serial.println(F("Restarting beacon module"));
    beaCmd("RESET",TOS,TOF);
    Serial.println("");
    
    Serial.println(F("Seting ibeacon id for slave module"));
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

    Serial.println(F("Restarting slave"));
    slvCmd("RESET",TOS,TOF);
    Serial.println("");

    
    Serial.println( F("**********************************************" ));
    Serial.println( F("PERIPH INIT COMPLETE") );
    Serial.println( F("**********************************************" ));

    Serial.println(F("Starting loop"));
    Serial.println( F("+++++++++++++++++++++++++++++++++++++++++++++++" ));
    pack_time = millis();
    sample_time = millis();
    fall_reset_timer = millis();
}





void loop() {
  get_mpudata();
  AM_LT();
}

void dht11(){
  DHT.read11(DHT11_PIN);
 // Serial.print("Temperature = ");
#ifdef SENSOR_DEBUG
  Serial.print("Temperature : ");
#endif
  if( DHT.temperature != - 999){
    temp = DHT.temperature;
  };
  Serial.println(temp);
#ifdef SENSOR_DEBUG
  Serial.print("Humidity : ");
#endif
  if( DHT.humidity != - 999) {
    humid = DHT.humidity;
  };
  Serial.println(humid);
}
int AM_LT() {
  get_mpudata();
  if(AM <= 8) {
    t_1 = millis();
    trigger1 = 1;
    
    while(millis()-t_1 <= 700) {
      get_mpudata();
      if(AM_UT()) {trigger1=0;return 1;}
    }
    trigger1 = 0;
    return 0;
  }
  trigger1=0;
  return 0;
}

int AM_UT() {
  get_mpudata();
  if(AM >= 13) {
    t_2 = millis();
    trigger2 = 1;
    
    while(millis()-t_2 <= 700) {
      get_mpudata();
      if(AM_OC()) {trigger2=0;return 1;}
      if(millis()-t_1 <= 700) {trigger2=0;return 1;}
    }
    trigger2 = 0;
    return 0;
  }
  trigger2=0;
  return 0;
}

int AM_OC() {
  get_mpudata();
  angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5); //Serial.println(angleChange);
  t_3 = millis();
  trigger3 = 1;
  if (angleChange >= 30 && angleChange <= 400) { //if orientation changes by between 80-100 degrees
    while(millis()-t_3 <= 500) {
      get_mpudata();
      if(DEAD()) {trigger3=0;return 1;}
      if(millis()-t_2 <= 700) {trigger3=0;return 1;}
    }
    trigger3 = 0;
    return 0;
  }
  trigger3=0;
  return 0;
}

int DEAD() {
  unsigned long t_4 = millis();
  while(millis()-t_4 <= 200) {}
  while(millis()-t_4 <= 2000) {
    get_mpudata();
    angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5); //Serial.println(angleChange);
    Serial.println(angleChange);
    if(angleChange >= 35) return 1;
  }
  Serial.println("Fall detected");
  fall=true;
  /*while(1) {
    if(Serial.read() == '1') return 1;
  }*/
  fall_reset_timer = millis();
  return 0;
}

void get_mpudata() {
  mpu_read();
  //2050, 77, 1947 are values for calibration of accelerometer
  // values may be different
  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ + 3000) / 16384.00;
  //270, 351, 136 for gyroscope
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;
  //Serial.print(az);
  // calculating Amplitute vactor for 3 axis
  Raw_AM = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
  AM = Raw_AM * 10;  // as values are within 0 to 1, I multiplied
  // it by for using if else conditions
  //Serial.print(AM,5);
  if(trigger1) Serial.print(" 1 ");
  else Serial.print(" 0 ");
  if(trigger2) Serial.print(" 1 ");
  else Serial.print(" 0 ");
  if(trigger3) Serial.print(" 1 ");
  else Serial.print(" 0 ");
  step_count();
  
  
  if(fall == true && millis()-fall_reset_timer>FALL_RESET_TIME) {
    fall = false;
    fall_reset_timer = millis();
  }
#ifdef SENSOR_DEBUG
  Serial.println("");
  Serial.print("Accx :  ");
  Serial.print(ax);
  Serial.print(" Accy :  ");
  Serial.print(ay);
  Serial.print(" Accz :  ");
  Serial.print(az);
  Serial.println("");
  Serial.print("AM :  ");
  Serial.print(AM,5);
#endif
  send_data();
}
#define TIME_DEBUG
void send_data() {
    //send a packet once x seconds
    if(millis()-pack_time>=BLE_DATA_PERIOD) {
        Serial.println("Computation delay");
        Serial.println(millis()-t);
        Serial.println("Sending packet");
        t=millis();

        dtostrf(AM,6,2,strAm);
        step_count = 0;
        sprintf(usrBuff,"%c%d%c%d%c%s%c%d%c%d",CHAR_HUMIDITY,int(humid),CHAR_TEMP,int(temp)
        ,CHAR_SVM,strAm,CHAR_STEP_COUNT,step_count,CHAR_FALL_DETECTED,fall);
        hmSlave.write(usrBuff);
        Serial.println(usrBuff);
/*      
        //temp
        sprintf(usrBuff,"temp: %d\r\n",int(temp));
        hmSlave.write(usrBuff);
        
#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif
        
        //humidity
        sprintf(usrBuff,"humidity: %d\r\n",int(humid));
        hmSlave.write(usrBuff);
        
#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif   
     
        //svm
        dtostrf(AM,7,3,strAm);
        sprintf(usrBuff,"svm: %s\r\n",strAm);
        hmSlave.write(usrBuff);

#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif

        //vel_mag
        //dtostrf(pow((vx*vx+vy*vy+vz*vz),0.5),7,3,strAm);
        //sprintf(usrBuff,"vel_mag: %s\r\n",strAm);
        
        dtostrf(pow((gx*gx+gy*gy+gz*gz),0.5),7,3,strAm);
        sprintf(usrBuff,"vel_mag: %s\r\n",strAm);
        hmSlave.write(usrBuff);

#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif
        
        //step_count
        sprintf(usrBuff,"step_count: 0\r\n");
        hmSlave.write(usrBuff);

#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif
        
        //fall_detection
        sprintf(usrBuff,"fall_detection: %d\r\n",fall);
        hmSlave.write(usrBuff);
        pack_time = millis();
*/
#ifdef TIME_DEBUG 
Serial.println(millis()-t); 
#endif

        Serial.println("Packet sending delay");
        Serial.println(millis()-t);
        
        
    }
}

void mpu_read() {
  dht11();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  sample_time = millis()-sample_time;
  vx += sample_time*ax;
  vy += sample_time*ay;
  vz += sample_time*az;

  sample_time = millis();
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



void step_count() {
    float DataToDrop = ped_data[pedI];
    float DataToAdd = pow((ax*ax+ay*ay+az*az),0.5);
    ped_data[pedI] = DataToAdd;
    pedSum -= DataToDrop;
    pedSum += DataToAdd;
    pedI++;
    if(pedI == PED_N) pedI=0;
}
