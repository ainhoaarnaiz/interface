#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUIDs Personalizados (¡Deben ser idénticos en el código web!)
// Genera tus propios UUIDs con una herramienta online (ej: uuidgenerator.net)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
uint8_t dataValue = 0; // Contador que enviaremos

// Callback para manejar eventos de conexión/desconexión
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected.");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected. Advertising...");
      // Inicia la publicidad de nuevo para permitir la reconexión
      BLEDevice::startAdvertising(); 
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server...");

  // Inicializa el dispositivo BLE y configura el nombre
  BLEDevice::init("ESP32-S3-Talker");

  // Crea el servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crea el servicio personalizado
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crea la característica (permite lectura, notificaciones y escritura)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY 
                    );
  
  // Agrega la Descriptor 2902 (Necesario para Notificaciones)
  pCharacteristic->addDescriptor(new BLE2902());

  // Inicia el servicio
  pService->start();

  // Inicia la publicidad (visible para otros dispositivos)
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // Velocidad de conexión más rápida
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Incrementa el valor a enviar
    dataValue++; 

    // Prepara el payload como un array de bytes
    uint8_t payload[1];
    payload[0] = dataValue;
    
    // Envía la notificación al cliente (la interfaz web)
    pCharacteristic->setValue(payload, 1);
    pCharacteristic->notify(); 

    Serial.print("Notifying value: ");
    Serial.println(dataValue);
  }
  delay(1000); // Envía un nuevo valor cada segundo
}