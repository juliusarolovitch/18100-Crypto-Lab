#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "time.h"

#define CUSTOM_MSG ""  // Insert riddle here!

const char *ssid = "A1";  
const char *password = "12345678";

WiFiServer server(80);

String connectionHistory;
int RSA_n = 3233, RSA_e = 17, RSA_d = 2753;

const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec = 0;
int daylightOffset_sec = 3600;

int modExp(int base, int exp, int mod) {
    int result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

String RSA(String encryptedMessage) {
    String decryptedMessage = "";
    int start = 0;
    while (start < encryptedMessage.length()) {
        int end = encryptedMessage.indexOf(' ', start);
        if (end == -1) end = encryptedMessage.length();
        String numStr = encryptedMessage.substring(start, end);
        int encryptedVal = numStr.toInt();
        char decryptedChar = modExp(encryptedVal, RSA_d, RSA_n);
        decryptedMessage += decryptedChar;
        start = end + 1;
    }
    return decryptedMessage;
}

void logConnection(String groupID) {
    struct tm timeinfo;
    char timeString[30];
    if (getLocalTime(&timeinfo)) {
        strftime(timeString, 30, "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
        strcpy(timeString, "Unknown Time");
    }

    connectionHistory += "Group " + groupID + " @ " + String(timeString) + "\n";
}

void setup() {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    server.begin();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        String encryptedMessage = client.readStringUntil('\r');
        Serial.println("Received encrypted: " + encryptedMessage);

        String groupID = RSA(encryptedMessage);
        Serial.println("Decrypted group ID: " + groupID);

        logConnection(groupID);

        client.println("Success " + groupID + " logged. " + CUSTOM_MSG);
        client.stop();
    }

    ArduinoCloud.update();
}
