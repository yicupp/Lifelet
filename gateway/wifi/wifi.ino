/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>

const char* ssid     = "Terrortown";
const char* password = "aaaaaaaa";

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
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
}

int value = 0;

void loop()
{
    delay(5000);
    ++value;

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
    String url = "/sys/a1Vywf8EbI6/esp32gate/thing/event/property/post";
    //url += streamId;
    //url += "?private_key=";
    //url += privateKey;
    //url += "&value=";
    //url += value;

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    /*client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");*/

    client.print(   String("POST /auth HTTP/1.1\r\n") + 
                    "Host: iot-as-http.cn-shanghai.aliyuncs.com\r\n" +
                    "Content-Type: application/json\r\n" +
                    "body: {\"version\":\"default\",\"clientId\":\"esp32gate1\",\"signmethod\":\"hmacsha1\",\"sign\":\"4870141D4067227128CBB4377906C3731CAC221C\",\"productKey\":\"ZG1EvTEa7NN\",\"deviceName\":\"NlwaSPXsCpTQuh8FxBGH\"}\r\n");
    unsigned long timeout = millis();
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
    }

    Serial.println();
    Serial.println("closing connection");
}
