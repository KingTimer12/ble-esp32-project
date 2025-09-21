#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define LED_PIN 23
#define SERVICE_UUID "3f631a64-f7bd-4f29-bb89-60af02aa1d9e"
#define CHARACTERISTIC_UUID "bc7960b8-6987-4302-b3f8-18d239cb9743"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
BLEAdvertising* pAdvertising = nullptr;

bool deviceConnected = false;
bool ledState = false;

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
      Serial.print("Comando recebido: ");
      Serial.println(value.c_str());

      if (value == "ON") {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ligado");
      } else if (value == "OFF") {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED desligado");
      } else if (value == "TOGGLE") {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        Serial.println(ledState ? "LED ligado" : "LED desligado");
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
  Serial.println("Iniciando ESP32 iBeacon + BLE Service...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize BLE
  BLEDevice::init("ESP32-BEACON");

  // Setup BLE Service for LED control
  setupBLEService();

  // Start advertising
  pAdvertising->start();

  Serial.println("ESP32 pronto!");
  Serial.println("- Broadcasting como iBeacon");
  Serial.println("- Aguardando conexões para controle do LED");
  Serial.printf("- UUID: %s\n", SERVICE_UUID);
}

void loop() {
  Serial.printf("Status: LED=%s, Conectado=%s\n", ledState ? "ON" : "OFF",
                deviceConnected ? "SIM" : "NAO");
  delay(5000);
}
