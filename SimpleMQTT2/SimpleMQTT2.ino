/**
 * A simple Azure IoT example for sending telemetry.
 */

#include <WiFi.h>
#include "Esp32MQTTClient.h"

// Please input the SSID and password of WiFi
//const char* ssid     = "Terrortown";
//const char* password = "aaaaaaaa";

//const char* ssid     = "OPTUS_755C9E";
//const char* password = "coredsouse20745";

const char* ssid     = "yicup";
const char* password = "aaaaaaaa";

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "HostName=Lifelet.azure-devices.net;DeviceId=gateway1;SharedAccessKey=DysRZD7S3gc16BOB01mRFdVdHr66Yo+CsJimJJnpjrU=";

static bool hasIoTHub = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting connecting WiFi.");
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
}
int x = 0;
void loop() {
  x++;
  Serial.println("start sending events.");
  if (hasIoTHub)
  {
    char buff[128];

    // replace the following line with your data sent to Azure IoTHub
    snprintf(buff, 128, "{\"data\":\"%i\"}", x);
    
    if (Esp32MQTTClient_SendEvent(buff))
    {
      Serial.println("Sending data succeed");
    }
    else
    {
      Serial.println("Failure...");
    }
    delay(5000);
  }
}
