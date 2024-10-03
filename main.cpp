#include <Arduino.h>
// /*
//   Rui Santos & Sara Santos - Random Nerd Tutorials
//   Complete project details at https://RandomNerdTutorials.com/esp32-web-bluetooth/
//   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
//   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// */
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// BLEServer* pServer = NULL;
// BLECharacteristic* pSensorCharacteristic = NULL;
// BLECharacteristic* pLedCharacteristic = NULL;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// uint32_t value = 0;

// const int ledPin = 2; // Use the appropriate GPIO pin for your setup

// // See the following for generating UUIDs:
// // https://www.uuidgenerator.net/
// #define SERVICE_UUID        "9b23de8e-d759-4327-9fd0-50935931ea09"
// #define SENSOR_CHARACTERISTIC_UUID "c4fc1687-6e0c-4e59-ab8a-815d91096ed3"
// #define LED_CHARACTERISTIC_UUID "a98b92ca-2453-461a-907f-9efb35623d91"

// class MyServerCallbacks: public BLEServerCallbacks {
//   void onConnect(BLEServer* pServer) {
//     deviceConnected = true;
//   };

//   void onDisconnect(BLEServer* pServer) {
//     deviceConnected = false;
//   }
// };

// class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic* pLedCharacteristic) {
//     std::string ledvalue  = pLedCharacteristic->getValue(); 
//     String value = String(ledvalue.c_str());
//     if (value.length() > 0) {
//       Serial.print("Characteristic event, written: ");
//       Serial.println(static_cast<int>(value[0])); // Print the integer value

//       int receivedValue = static_cast<int>(value[0]);
//       if (receivedValue == 1) {
//         digitalWrite(ledPin, HIGH);
//       } else {
//         digitalWrite(ledPin, LOW);
//       }
//     }
//   }
// };

// void setup() {
//   Serial.begin(115200);
//   pinMode(ledPin, OUTPUT);

//   // Create the BLE Device
//   BLEDevice::init("ESP32_Jorrit");

//   // Create the BLE Server
//   pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   // Create the BLE Service
//   BLEService *pService = pServer->createService(SERVICE_UUID);

//   // Create a BLE Characteristic
//   pSensorCharacteristic = pService->createCharacteristic(
//                       SENSOR_CHARACTERISTIC_UUID,
//                       BLECharacteristic::PROPERTY_READ   |
//                       BLECharacteristic::PROPERTY_WRITE  |
//                       BLECharacteristic::PROPERTY_NOTIFY |
//                       BLECharacteristic::PROPERTY_INDICATE
//                     );

//   // Create the ON button Characteristic
//   pLedCharacteristic = pService->createCharacteristic(
//                       LED_CHARACTERISTIC_UUID,
//                       BLECharacteristic::PROPERTY_WRITE
//                     );

//   // Register the callback for the ON button characteristic
//   pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

//   // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
//   // Create a BLE Descriptor
//   pSensorCharacteristic->addDescriptor(new BLE2902());
//   pLedCharacteristic->addDescriptor(new BLE2902());

//   // Start the service
//   pService->start();

//   // Start advertising
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(false);
//   pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
//   BLEDevice::startAdvertising();
//   Serial.println("Waiting a client connection to notify...");
// }

// void loop() {
//   // notify changed value
//   if (deviceConnected) {
//     pSensorCharacteristic->setValue(String(value).c_str());
//     pSensorCharacteristic->notify();
//     value++;
//     Serial.print("New value notified: ");
//     Serial.println(value);
//     delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
//   }
//   // disconnecting
//   if (!deviceConnected && oldDeviceConnected) {
//     Serial.println("Device disconnected.");
//     delay(500); // give the bluetooth stack the chance to get things ready
//     pServer->startAdvertising(); // restart advertising
//     Serial.println("Start advertising");
//     oldDeviceConnected = deviceConnected;
//   }
//   // connecting
//   if (deviceConnected && !oldDeviceConnected) {
//     // do stuff here on connecting
//     oldDeviceConnected = deviceConnected;
//     Serial.println("Device Connected");
//   }
// }

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define TRIGGER_PIN  D2  // Ultrasone sensor trigger pin
#define ECHO_PIN     D3 // Ultrasone sensor echo pin
#define MAX_DISTANCE 200 // Maximale meetafstand (in cm)

// BLE UUID's
#define SERVICE_UUID        "9b23de8e-d759-4327-9fd0-50935931ea09"
#define CHARACTERISTIC_UUID "a98b92ca-2453-461a-907f-9efb35623d91"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
unsigned long lastTime = 0;  // Tijd voor interval tussen metingen
const int interval = 1000;   // Verstuur om de seconde de meting

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    
    // Init BLE
    BLEDevice::init("ESP32_Jorrit");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    pCharacteristic->addDescriptor(new BLE2902());
    
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();
    
    Serial.println("Waiting for a client connection...");

    // Setup de ultrasone sensor
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

void loop() {
    if (deviceConnected) {
        unsigned long currentTime = millis();
        
        // Verzamel afstand elke seconde
        if (currentTime - lastTime >= interval) {
            lastTime = currentTime;

            // Lees de afstand van de ultrasone sensor
            long duration, distance;
            digitalWrite(TRIGGER_PIN, LOW);
            delayMicroseconds(2);
            digitalWrite(TRIGGER_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(TRIGGER_PIN, LOW);
            duration = pulseIn(ECHO_PIN, HIGH);
            distance = (duration * 0.034 / 2); // Omrekenen naar cm
            
            // Zorg ervoor dat de afstand binnen de grenzen valt
            if (distance > MAX_DISTANCE || distance < 0) {
                distance = MAX_DISTANCE; // Stel in op MAX_DISTANCE als er geen echo is
            }

            // Converteer naar string en stuur via BLE
            String distanceStr = String(distance) + " cm";
            Serial.println("Distance: " + distanceStr);
            pCharacteristic->setValue(distanceStr.c_str());
            pCharacteristic->notify();  // Stuur de waarde naar de client
        }
    }
}
