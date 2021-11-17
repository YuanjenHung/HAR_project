// BLE
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

// Create BLEService
BLEService sensorsService("19b10000-e8f2-537e-4f6c-d104768a1214"); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic numTestCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLENotify | BLERead);

void updateData(){
    numTestCharacteristic.writeValue(2);
}

void setup(){
    // Enable the BLE capabilities, Serial port, led and sensor
    pinMode(LED_BUILTIN, OUTPUT);
    BLE.begin();

    // Set “Arduino Nano 33 BLE sense” as a device name and local name
    BLE.setDeviceName("Arduino Nano 33 BLE Sense");
    BLE.setLocalName("Arduino Nano 33 BLE Sense");

    // Add all the characteristic to the service
    sensorsService.addCharacteristic(numTestCharacteristic); 

    // Add the services to the device
    BLE.addService(sensorsService);

    // Start advertising
    BLE.advertise();
}

void loop(){
    BLE.advertise();
    digitalWrite(LED_BUILTIN, LOW);

    while(!BLE.connected());
    digitalWrite(LED_BUILTIN, HIGH);

    while(BLE.connected()){
        updateData();
    } 
}