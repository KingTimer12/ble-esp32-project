#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define SERVICE_UUID "3f631a64-f7bd-4f29-bb89-60af02aa1d9e"
#define CHARACTERISTIC_UUID "bc7960b8-6987-4302-b3f8-18d239cb9743"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
BLEAdvertising* pAdvertising = nullptr;

bool deviceConnected = false;
bool ledState = false;

const char* ssid = "Esp32";
const char* password = "modelo12";
const char* serverIP = "192.168.43.177";

HTTPClient http;

void sendPostRequest(const char* endpoint) {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "http://" + String(serverIP) + endpoint;
    http.begin(url);

    int httpResponseCode = http.POST("");

    if (httpResponseCode > 0) {
      Serial.printf("POST OK: %d\n", httpResponseCode);
    } else {
      Serial.println("POST Erro");
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado!");
  }
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Cliente conectado");
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Cliente desconectado");
    // Restart advertising
    pAdvertising->start();
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0) {
      if (value == "ON") {
        ledState = true;
        sendPostRequest("/leg/on");
      } else if (value == "OFF") {
        ledState = false;
        sendPostRequest("/leg/off");
      } else if (value == "TOGGLE") {
        ledState = !ledState;
        sendPostRequest(ledState ? "/leg/on" : "/leg/off");
      }
    }
  }
};

void setupBLEService() {
  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  BLEDevice::init("ESP32-BEACON");
  setupBLEService();
  pAdvertising->start();

  Serial.println("Pronto!");
}

void loop() {
  Serial.printf("Status: LED=%s, Conectado=%s\n", ledState ? "ON" : "OFF",
                deviceConnected ? "SIM" : "NAO");
  delay(5000);
}
