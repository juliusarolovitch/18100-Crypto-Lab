#include <WiFiClientSecure.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// ====== Setup Variables ======
const char SSID[]        = "CMU-DEVICE";
const char PASS[]        = "";
const char DEVICE_KEY[]  = "INSERT";
const char DEVICE_ID[]   = "INSERT";
#define MAX_ESP_NOW_PAYLOAD 200

const char* GROUP_ID = "INSERT";  // SET THIS TO GROUP ID

uint8_t pendingMac[6];
String pendingAndrewID;
bool hasPendingRequest = false;

// ====== Arduino Cloud Connection Handler ======
WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
String visits;  // Cloud variable

void onVisitsChange() {
  Serial.print("Cloud update: visits = ");
  Serial.println(visits);
}

void initProperties() {
  ArduinoCloud.setBoardId(DEVICE_ID);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(visits, READWRITE, ON_CHANGE, onVisitsChange);
}

// ====== Encryption/Decryption Method Selection ======
enum EncryptionMethodType {
  CAESAR,
  LFSR,
  RSA
};

EncryptionMethodType encryptionMethod = RSA;  

// ====== Caesar Cipher ======
const int CAESAR_SHIFT = 10;
String caesarEncrypt(const String &input) {
  String result = "";
  for (char c : input)
    // TODO
  return result;
}
String caesarDecrypt(const uint8_t* data, int len) {
  String result = "";
  for (int i = 0; i < len; i++)
    // TODO
  return result;
}

// ====== LFSR ======
uint8_t LFSR_SEED = 0xA5;  // Set your seed here
uint8_t lfsr;
uint8_t stepLFSR() {
  uint8_t fb = // TODO;
  lfsr = // TODO;
  return lfsr;
}
String lfsrEncrypt(const String &input) {
  lfsr = LFSR_SEED;
  String result = "";
  for (char c : input)
    result += // TODO;
  return result;
}
String lfsrDecrypt(const uint8_t* data, int len) {
  lfsr = LFSR_SEED;
  String result = "";
  for (int i = 0; i < len; i++)
    result += TODO;
  return result;
}

// ====== RSA ======
const uint32_t RSA_N = 3233;   // Modulus
const uint32_t RSA_E = 17;     // Public exponent
const uint32_t RSA_D = 2753;   // Private exponent

uint32_t modExp(uint32_t base, uint32_t exp, uint32_t mod) {
  uint32_t result = 1;
  base %= mod;
  while (exp > 0) {
    if (exp & 1)
      result = %
    exp >>= 1;
    base = // TODO
  }
  return result;
}

String rsaEncrypt(const String &input) {
  String result = "";
  // Encrypt each character into 4 bytes
  for (char c : input) {
    uint32_t cipher = // TODO
    // TODO
  }
  return result;
}

String rsaDecrypt(const uint8_t* data, int len) {
  if (len % 4 != 0) return "(Malformed RSA data)";
  String result = "";
  for (int i = 0; i < len; i += 4) {
    uint32_t block = // TODO
    char plainChar = (char) // TODO
    result += plainChar;
  }
  return result;
}

// ====== General Encrypt/Decrypt Functions ======
String encryptMessage(const String &msg) {
  switch(encryptionMethod) {
    case CAESAR: return caesarEncrypt(msg);
    case LFSR:   return lfsrEncrypt(msg);
    case RSA:    return rsaEncrypt(msg);
  }
  return msg;
}

String decryptMessage(const uint8_t* data, int len) {
  switch(encryptionMethod) {
    case CAESAR: return caesarDecrypt(data, len);
    case LFSR:   return lfsrDecrypt(data, len);
    case RSA:    return rsaDecrypt(data, len);
  }
  return "";
}

// ====== Sending Chunked Message ======
void sendChunkedMessage(const uint8_t *mac, const String &message) {
  int totalLen = message.length();
  int offset = 0;
  while (offset < totalLen) {
    int chunkSize = min(MAX_ESP_NOW_PAYLOAD, totalLen - offset);
    String chunk = encryptMessage(message.substring(offset, offset + chunkSize));
    esp_now_send(mac, (uint8_t*)chunk.c_str(), chunk.length());
    Serial.print("Sent chunk: ");
    Serial.println(chunk);
    offset += chunkSize;
    delay(50);
  }
}

// ====== Wait Until Station is Enabled ======
bool waitForStationEnable() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  Serial.println("Waiting for station to be enabled via server...");
  while (true) {
    http.begin(client, "https://18100cryptolab.duckdns.org/station_status");
    http.setTimeout(2000); 
    int code = http.GET();
    if (code == 200) {
      String payload = http.getString();
      StaticJsonDocument<128> doc;
      DeserializationError err = deserializeJson(doc, payload);
      if (!err && doc.containsKey("enabled")) {
        bool enabled = doc["enabled"];
        if (enabled) {
          Serial.println("Station enabled by server.");
          return true;
        } else {
          Serial.println("Station disabled. Retrying in 10s...");
        }
      } else {
        Serial.println("Invalid JSON while checking status.");
      }
    } else {
      Serial.printf("Failed to check status. HTTP %d. Retrying in 10s...\n", code);
    }
    http.end();
    delay(10000);
  }
}

// ====== Hint Fetching ======
const char* HINT_API_ENDPOINT = "https://18100cryptolab.duckdns.org/random_hint";

String getHintFromServer() {
  WiFiClientSecure client;
  client.setInsecure(); 
  HTTPClient http;
  http.begin(client, HINT_API_ENDPOINT);
  http.setTimeout(10000);  
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err && doc.containsKey("group_id") && doc.containsKey("hint") && doc.containsKey("method")) {
      String groupID = doc["group_id"].as<String>();
      String hint = doc["hint"].as<String>();
      String method = doc["method"].as<String>();
      Serial.println("Random Hint Fetched:");
      Serial.print("→ Group: ");
      Serial.println(groupID);
      Serial.print("→ Hint: ");
      Serial.println(hint);
      Serial.print("→ Method: ");
      Serial.println(method);
      return hint + " [" + method + "]";
    } else {
      Serial.println("JSON parse error or missing fields.");
    }
  } else {
    Serial.printf("Failed to fetch hint. HTTP code: %d\n", httpCode);
  }
  return "No hint available. Try again later.";
}

// ====== ESP-NOW Receive Callback ======
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Received data from: ");
  Serial.println(macStr);

  String decryptedMessage = decryptMessage(incomingData, len);
  Serial.print("Decrypted message: ");
  Serial.println(decryptedMessage);

  if (decryptedMessage.length() <= 10) {
    Serial.print("Valid Andrew ID: ");
    Serial.println(decryptedMessage);
    visits = decryptedMessage;

    memcpy(pendingMac, mac, 6);
    pendingAndrewID = decryptedMessage;
    hasPendingRequest = true;
  } else {
    Serial.println("Invalid Andrew ID (too long)");
  }
}

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  delay(1000);
  randomSeed(analogRead(0));

  // Connect to WiFi and start SoftAP
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  int wifiChannel = WiFi.channel();
  Serial.print("Connected WiFi channel: ");
  Serial.println(wifiChannel);

  if (!WiFi.softAP("CryptoStation", "", wifiChannel)) {
    Serial.println("SoftAP failed to start");
  } else {
    Serial.print("SoftAP started on channel ");
    Serial.println(wifiChannel);
  }

  if (!waitForStationEnable()) {
    Serial.println("Station never enabled.");
    return;
  }

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  Serial.println("Arduino Cloud started");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW server initialized");
}

// ====== Main Loop ======
void loop() {
  ArduinoCloud.update();

  if (hasPendingRequest) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, pendingMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("Client added as peer");

      String email = pendingAndrewID + "@andrew.cmu.edu";
      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
               pendingMac[0], pendingMac[1], pendingMac[2], pendingMac[3], pendingMac[4], pendingMac[5]);
      String url = "https://18100cryptolab.duckdns.org/record_station_visit/" + email + "/" + String(macStr) + "/" + String(GROUP_ID);

      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.begin(client, url);
      int httpCode = http.GET();
      if (httpCode == 200) {
        String payload = http.getString();
        Serial.println("Station visit recorded successfully.");
        Serial.println(payload);
      } else {
        Serial.printf("Failed to record station visit. HTTP code: %d\n", httpCode);
      }
      http.end();

      String finalHint = getHintFromServer();
      Serial.print("Fetched hint on-demand: ");
      Serial.println(finalHint);

      String processingMsg = "Processing your request...";
      String encryptedProcessingMsg = encryptMessage(processingMsg);
      esp_now_send(pendingMac, (uint8_t*)encryptedProcessingMsg.c_str(), encryptedProcessingMsg.length());

      sendChunkedMessage(pendingMac, finalHint);

      esp_now_del_peer(pendingMac);
      Serial.println("Client peer removed");
    } else {
      Serial.println("Failed to add peer");
    }
    hasPendingRequest = false;
  }
}
