#include <WiFi.h>

#define ENCRYPTION_METHOD "RSA" // Replace with the encryption method you're using!

const char *groundStations[] = {"A1", "A2", "A3", "A4", "A5", "A6",
                                "B1", "B2", "B3", "B4", "B5", "B6",
                                "C1", "C2", "C3", "C4", "C5", "C6",
                                "D1", "D2", "D3", "D4", "D5", "D6",
                                "E1", "E2", "E3", "E4", "E5", "E6",
                                "F1", "F2", "F3", "F4", "F5", "F6"};

const char *password = "12345678";
WiFiClient client;

int RSA_n = 3233, RSA_e = 17, RSA_d = 2753;

int modExp(int base, int exp, int mod) {
   TODO;
}

String RSA(String message, bool encrypt) {
    int key = encrypt ? RSA_e : RSA_d;
    String result = "";
    for (int i = 0; i < message.length(); i++) {
        int charCode = (int)message[i];
        int encryptedChar = modExp(charCode, key, RSA_n);
        result += String(encryptedChar) + " ";
    }
    return result;
}

String Caesar(String message, int shift) {
    TODO;
    return encryptedMessage;
}

uint16_t lfsr_state = 0xACE1;
String LFSR(String message) {
    TODO;
    return encryptedMessage;
}

String encryptMessage(String message) {
    if (String(ENCRYPTION_METHOD) == "RSA") return RSA(message, true);
    if (String(ENCRYPTION_METHOD) == "Caesar") return Caesar(message, 3);
    if (String(ENCRYPTION_METHOD) == "LFSR") return LFSR(message);
    return message;
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
}

void loop() {
    Serial.println("Scanning for ground stations...");
    int networksFound = WiFi.scanNetworks();

    for (int i = 0; i < networksFound; i++) {
        String detectedSSID = WiFi.SSID(i);
        Serial.println("Detected: " + detectedSSID);

        for (String station : groundStations) {
            if (detectedSSID == station) {
                Serial.println("Connecting to " + station);
                
                WiFi.begin(station.c_str(), password);
                if (WiFi.waitForConnectResult() == WL_CONNECTED) {
                    Serial.println("Connected to " + station);

                    Serial.println("Enter your message to send: ");
                    while (!Serial.available()) delay(100);
                    String userMessage = Serial.readStringUntil('\n');
                    userMessage.trim();

                    String encryptedMessage = encryptMessage(userMessage);

                    if (client.connect("192.168.4.1", 80)) {
                        client.println(encryptedMessage);
                        Serial.println("Sent encrypted message: " + encryptedMessage);
                        String response = client.readStringUntil('\r');
                        Serial.println("Received response: " + response);
                        client.stop();
                    } else {
                        Serial.println("Failed to connect to ground station.");
                    }

                    WiFi.disconnect();
                }
            }
        }
    }

    delay(5000);
}
