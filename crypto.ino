#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ==========================================================
//              CONFIGURABLE ENCRYPTION METHODS
// ==========================================================

// Define the available encryption methods.
enum EncryptionMethod {
  RSA_ENCRYPTION,
  CAESAR_CIPHER,
  LFSR_CIPHER
};

// Set the encryption method (default: RSA).
EncryptionMethod currentEncryptionMethod = RSA_ENCRYPTION;

// ------------------ RSA Parameters ------------------
// (RSA is applied per character and produces a 32-bit result per char.)
String andrewID = "INSERT_YOUR_ID";  // e.g., "fjahanian"
uint32_t RSA_N = 0;                  // RSA modulus 
uint32_t RSA_E = 0;                  // RSA public exponent 

// ------------------ Caesar Cipher Parameters ------------------
int CAESAR_SHIFT = 0;                // Shift value 

// ------------------ LFSR Parameters ------------------
uint8_t LFSR_SEED = 0;               // LFSR seed 
uint8_t lfsr;                        // Current state of the LFSR

// ==========================================================
//                GENERAL SETTINGS
// ==========================================================
#define FIXED_CHANNEL 1
const unsigned long SEND_INTERVAL = 3000; // milliseconds
unsigned long lastSendTime = 0;

// ==========================================================
//              RSA: Modular Exponentiation
// ==========================================================
uint32_t modExp(uint32_t base, uint32_t exp, uint32_t mod) {
  // TODO: Implement modular exponentiation.
  return 0; // placeholder return
}

// ==========================================================
//                LFSR: Step Function
// ==========================================================
uint8_t stepLFSR() {
  // TODO: Implement the LFSR step function
  return 0; // placeholder return
}

// ==========================================================
//                    STATION INFO
// ==========================================================
struct Station {
  const char* group;
  const char* mac_str;
  uint8_t mac[6];
};

Station stations[] = {
  {"A1", "DC:DA:0C:3B:67:88", {0xDC, 0xDA, 0x0C, 0x3B, 0x67, 0x88}},
  {"A2", "3C:84:27:C2:D5:D4", {0x3C, 0x84, 0x27, 0xC2, 0xD5, 0xD4}},
  {"A3", "3C:84:27:C3:3D:FC", {0x3C, 0x84, 0x27, 0xC3, 0x3D, 0xFC}},
  {"A4", "3C:84:27:C4:B5:C4", {0x3C, 0x84, 0x27, 0xC4, 0xB5, 0xC4}},
  {"A5", "3C:84:27:C2:A0:08", {0x3C, 0x84, 0x27, 0xC2, 0xA0, 0x08}},
  {"A6", "74:4D:BD:A1:E6:70", {0x74, 0x4D, 0xBD, 0xA1, 0xE6, 0x70}},
  {"B1", "3C:84:27:C3:2E:40", {0x3C, 0x84, 0x27, 0xC3, 0x2E, 0x40}},
  {"B2", "DC:DA:0C:3C:48:30", {0xDC, 0xDA, 0x0C, 0x3C, 0x48, 0x30}},
  {"B3", "DC:DA:0C:3C:37:E8", {0xDC, 0xDA, 0x0C, 0x3C, 0x37, 0xE8}},
  {"B4", "74:4D:BD:A1:E6:C4", {0x74, 0x4D, 0xBD, 0xA1, 0xE6, 0xC4}},
  {"B5", "3C:84:27:C6:E8:10", {0x3C, 0x84, 0x27, 0xC6, 0xE8, 0x10}},
  {"B6", "3C:84:27:C3:FE:DC", {0x3C, 0x84, 0x27, 0xC3, 0xFE, 0xDC}},
  {"C1", "3C:84:27:C3:B9:68", {0x3C, 0x84, 0x27, 0xC3, 0xB9, 0x68}},
  {"C2", "3C:84:27:C4:1D:C8", {0x3C, 0x84, 0x27, 0xC4, 0x1D, 0xC8}},
  {"C3", "3C:84:27:C2:E6:B4", {0x3C, 0x84, 0x27, 0xC2, 0xE6, 0xB4}},
  {"C4", "3C:84:27:C4:28:6C", {0x3C, 0x84, 0x27, 0xC4, 0x28, 0x6C}},
  {"C5", "3C:84:27:C2:9E:18", {0x3C, 0x84, 0x27, 0xC2, 0x9E, 0x18}},
  {"C6", "3C:84:27:C4:B6:FC", {0x3C, 0x84, 0x27, 0xC4, 0xB6, 0xFC}},
  {"D1", "3C:84:27:C3:2D:A8", {0x3C, 0x84, 0x27, 0xC3, 0x2D, 0xA8}},
  {"D2", "48:CA:43:2F:01:78", {0x48, 0xCA, 0x43, 0x2F, 0x01, 0x78}},
  {"D3", "48:CA:43:2F:05:B8", {0x48, 0xCA, 0x43, 0x2F, 0x05, 0xB8}},
  {"D4", "3C:84:27:C4:B3:94", {0x3C, 0x84, 0x27, 0xC4, 0xB3, 0x94}},
  {"D5", "3C:84:27:C5:46:3C", {0x3C, 0x84, 0x27, 0xC5, 0x46, 0x3C}},
  {"D6", "48:CA:43:2D:A8:44", {0x48, 0xCA, 0x43, 0x2D, 0xA8, 0x44}},
  {"E1", "74:4D:BD:A1:0D:50", {0x74, 0x4D, 0xBD, 0xA1, 0x0D, 0x50}},
  {"E2", "3C:84:27:C3:FD:F0", {0x3C, 0x84, 0x27, 0xC3, 0xFD, 0xF0}},
  {"E3", "48:CA:43:2E:FD:6C", {0x48, 0xCA, 0x43, 0x2E, 0xFD, 0x6C}},
  {"E4", "3C:84:27:C3:48:98", {0x3C, 0x84, 0x27, 0xC3, 0x48, 0x98}},
  {"E5", "3C:84:27:C3:2D:90", {0x3C, 0x84, 0x27, 0xC3, 0x2D, 0x90}},
  {"E6", "3C:84:27:C3:2E:F4", {0x3C, 0x84, 0x27, 0xC3, 0x2E, 0xF4}},
  {"F1", "48:CA:43:2F:03:CC", {0x48, 0xCA, 0x43, 0x2F, 0x03, 0xCC}},
  {"F2", "DC:DA:0C:3C:45:AC", {0xDC, 0xDA, 0x0C, 0x3C, 0x45, 0xAC}},
  {"F3", "3C:84:27:C3:3D:BC", {0x3C, 0x84, 0x27, 0xC3, 0x3D, 0xBC}},
  {"F4", "DC:DA:0C:3C:E6:E4", {0xDC, 0xDA, 0x0C, 0x3C, 0xE6, 0xE4}},
  {"F5", "3C:84:27:C3:41:6C", {0x3C, 0x84, 0x27, 0xC3, 0x41, 0x6C}},
  {"F6", "3C:84:27:C2:89:28", {0x3C, 0x84, 0x27, 0xC2, 0x89, 0x28}},
};

uint8_t selectedMAC[6];
int selectedChannel = FIXED_CHANNEL;

// ==========================================================
//                   ESP-NOW CALLBACKS
// ==========================================================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Data received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print("  Data: ");
  for (int i = 0; i < len; i++) {
    Serial.print(incomingData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// ==========================================================
//                SETUP FIXED PEER
// ==========================================================
bool setFixedPeer() {
  memcpy(selectedMAC, stations[0].mac, 6);
  selectedChannel = FIXED_CHANNEL;
  return true;
}

// ==========================================================
//               ENCRYPTION FUNCTION
// ==========================================================
// Encrypts the global 'andrewID' using the selected encryption method.
// The result is written into 'outputBuffer' and the number of bytes is returned.
size_t encryptAndrewID(uint8_t* outputBuffer, EncryptionMethod method) {
  size_t len = andrewID.length();
  size_t outLen = 0;

  switch(method) {
    case RSA_ENCRYPTION: {
      // TODO: For each character in andrewID:
      //   1. Compute RSA encryption using modExp:
      //         uint32_t cipher = modExp((uint8_t)andrewID.charAt(i), RSA_E, RSA_N);
      //   2. Store the 32-bit cipher as 4 bytes (big-endian) in outputBuffer
      break;
    }
    case CAESAR_CIPHER: {
      // TODO: For each character in andrewID:
      //   1. Shift the character value by CAESAR_SHIFT modulo 128.
      //   2. Store the result in outputBuffer.
      break;
    }
    case LFSR_CIPHER: {
      // TODO: Reset 'lfsr' to LFSR_SEED.
      // Then, for each character in andrewID:
      //   1. Call stepLFSR() to get the next keystream byte.
      //   2. XOR the character with the keystream byte.
      //   3. Store the result in outputBuffer.
      break;
    }
  }
  return outLen;
}

// ==========================================================
//            SERIAL COMMAND HANDLER
// ==========================================================
void checkSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    if (input == "1") {
      currentEncryptionMethod = RSA_ENCRYPTION;
      Serial.println("Switched encryption to RSA.");
    }
    else if (input == "2") {
      currentEncryptionMethod = CAESAR_CIPHER;
      Serial.println("Switched encryption to Caesar Cipher.");
    }
    else if (input == "3") {
      currentEncryptionMethod = LFSR_CIPHER;
      Serial.println("Switched encryption to LFSR Cipher.");
    }
    else if (input.startsWith("RSA_N ")) {
      String valueStr = input.substring(6);
      RSA_N = valueStr.toInt();
      Serial.print("Updated RSA_N to ");
      Serial.println(RSA_N);
    }
    else if (input.startsWith("RSA_E ")) {
      String valueStr = input.substring(6);
      RSA_E = valueStr.toInt();
      Serial.print("Updated RSA_E to ");
      Serial.println(RSA_E);
    }
    else if (input.startsWith("CAESAR_SHIFT ")) {
      String valueStr = input.substring(13);
      CAESAR_SHIFT = valueStr.toInt();
      Serial.print("Updated CAESAR_SHIFT to ");
      Serial.println(CAESAR_SHIFT);
    }
    else if (input.startsWith("LFSR_SEED ")) {
      String valueStr = input.substring(10);
      LFSR_SEED = (uint8_t)valueStr.toInt();
      Serial.print("Updated LFSR_SEED to ");
      Serial.println(LFSR_SEED);
    }
    else if (input.startsWith("ANDREW_ID ")) {
      String newID = input.substring(10);
      andrewID = newID;
      Serial.print("Updated ANDREW_ID to ");
      Serial.println(andrewID);
    }
    else {
      Serial.println("Unknown command. Use:");
      Serial.println("  1, 2, or 3 to switch encryption method.");
      Serial.println("  RSA_N <number>");
      Serial.println("  RSA_E <number>");
      Serial.println("  CAESAR_SHIFT <number>");
      Serial.println("  LFSR_SEED <number>");
      Serial.println("  ANDREW_ID <string>");
    }
  }
}

// ==========================================================
//                     SETUP FUNCTION
// ==========================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set up WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Set up a fixed peer (using the first station)
  if (setFixedPeer()) {
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(FIXED_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, selectedMAC, 6);
    peerInfo.channel = FIXED_CHANNEL;
    peerInfo.encrypt = false;
    if (!esp_now_is_peer_exist(selectedMAC)) {
      if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
      } else {
        Serial.print("Added peer with MAC: ");
        for (int i = 0; i < 6; i++) {
          Serial.printf("%02X", selectedMAC[i]);
          if (i < 5) Serial.print(":");
        }
        Serial.print(" on channel ");
        Serial.println(FIXED_CHANNEL);
      }
    }
  }

  Serial.println("Client setup complete.");
  Serial.println("Commands:");
  Serial.println("  1: RSA encryption");
  Serial.println("  2: Caesar cipher");
  Serial.println("  3: LFSR cipher");
  Serial.println("  RSA_N <number>");
  Serial.println("  RSA_E <number>");
  Serial.println("  CAESAR_SHIFT <number>");
  Serial.println("  LFSR_SEED <number>");
  Serial.println("  ANDREW_ID <string>");
}

// ==========================================================
//                     MAIN LOOP
// ==========================================================
void loop() {
  checkSerialInput();

  unsigned long now = millis();
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    uint8_t encryptedBuffer[512];
    size_t encryptedLen = encryptAndrewID(encryptedBuffer, currentEncryptionMethod);

    Serial.println("Sending encrypted Andrew ID:");
    for (size_t i = 0; i < encryptedLen; i++) {
      Serial.printf("%02X ", encryptedBuffer[i]);
    }
    Serial.println();

    esp_err_t result = esp_now_send(selectedMAC, encryptedBuffer, encryptedLen);
    if (result != ESP_OK) {
      Serial.println("Failed to send encrypted ID");
    }
  }
}
