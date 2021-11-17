// BLE
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>

String DEVICE_ADDRESS = "72:3A:38:EB:BA:24";
// char* NUMCHAR_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";

void setup(){
    // Enable the BLE capabilities, Serial port, led and sensor
    Serial.begin(115200);
    while(!Serial);
    if (!BLE.begin()) {
        Serial.println("* Starting BLE module failed!");
        while (1);
    }

    Serial.println("Arduino Nano 33 BLE Sense (test 2)");
    Serial.println(" ");
    // BLE.scanForAddress(DEVICE_ADDRESS);
}

void loop(){
    static long previousMillis = 0;

    long interval = 1000;
    unsigned long currentMillis = millis();
    // Be aware this statement will be executed many times every interval until a BatteryMonitor is found
    // because the previousMillis is updated only under a condition
    if (currentMillis - previousMillis > interval){
        BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
        BLEDevice peripheral = BLE.available();

        if (peripheral){
            if (peripheral.localName() != "Arduino Nano 33 BLE Sense"){
                return;
            }
            previousMillis = currentMillis;
            BLE.stopScan();
            explorePeripheral( peripheral );
        }
    }
}

bool explorePeripheral(BLEDevice peripheral){
    if (!peripheral.connect()){
        return false;
    }

    if (!peripheral.discoverAttributes()){
        peripheral.disconnect();
        return false;
    }

    BLECharacteristic numTestCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

    if (numTestCharacteristic){
        byte value;
        numTestCharacteristic.readValue(value);
        Serial.println("Number: " + value);
    }

    peripheral.disconnect();
    return true;
}

void connectToPeripheral(){
    BLEDevice peripheral;

    Serial.println("- Discovering peripheral device...");

    do { peripheral = BLE.available(); } while (!peripheral);

    if (peripheral) {
        Serial.println("* Peripheral device found!");
        Serial.print("* Device MAC address: ");
        Serial.println(peripheral.address());
        Serial.print("* Device name: ");
        Serial.println(peripheral.localName());
        Serial.println();
    }
    BLE.stopScan();
    controlPeripheral(peripheral);
}

void controlPeripheral(BLEDevice peripheral) {
    Serial.println("- Connecting to peripheral device...");

    if (peripheral.connect()) {
        Serial.println("* Connected to peripheral device!");
        Serial.println(" ");
    } else {
        Serial.println("* Connection to peripheral device failed!");
        Serial.println(" ");
        return;
    }

    int characteristicCount = peripheral.characteristicCount();

    Serial.print(characteristicCount);
    Serial.println(" characteristis discovered");

    BLECharacteristic numTestCharacteristic = peripheral.characteristic("19b10001-e8f2-537E-4f6c-d104768a1214");

    if (numTestCharacteristic) {
        // use the characteristic
        byte value = 0;
        numTestCharacteristic.readValue(value);
        Serial.println("number is: " + value);
    } else {
        Serial.println("Peripheral does NOT have the characteristic");
    }

    while (peripheral.connected()){  }

    Serial.println("- Peripheral device disconnected!");
}