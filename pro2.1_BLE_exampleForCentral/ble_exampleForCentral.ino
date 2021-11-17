// BLE
#include <ArduinoBLE.h>

const char* deviceServiceUuid = "cb5054e8-3b3c-11ec-8d3d-0242ac130003";
const char* deviceServiceCharacteristicUuid = "cb505718-3b3c-11ec-8d3d-0242ac130003";

int32_t number = 0;
int32_t oldNumberValue = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    if (!BLE.begin()) {
        Serial.println("* Starting BLE module failed!");
        while (1);
    }
    
    BLE.setLocalName("Nano 33 BLE (Central)"); 

    Serial.println("Arduino Nano 33 BLE Sense (Central Device)");
    Serial.println(" ");
}

void loop() {
    connectToPeripheral();
}

void connectToPeripheral(){
    BLEDevice peripheral;
    
    Serial.println("- Discovering peripheral device...");

    do
    {
        BLE.scanForUuid(deviceServiceUuid);
        peripheral = BLE.available();
    } while (!peripheral);
    
    if (peripheral) {
        Serial.println("* Peripheral device found!");
        Serial.print("* Device MAC address: ");
        Serial.println(peripheral.address());
        Serial.print("* Device name: ");
        Serial.println(peripheral.localName());
        Serial.print("* Advertised service UUID: ");
        Serial.println(peripheral.advertisedServiceUuid());
        Serial.println(" ");
        BLE.stopScan();
        controlPeripheral(peripheral);
    }
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

    Serial.println("- Discovering peripheral device attributes...");
    if (peripheral.discoverAttributes()) {
        Serial.println("* Peripheral device attributes discovered!");
        Serial.println(" ");
    } else {
        Serial.println("* Peripheral device attributes discovery failed!");
        Serial.println(" ");
        peripheral.disconnect();
        return;
    }

    BLECharacteristic numberCharacteristic = peripheral.characteristic(deviceServiceCharacteristicUuid);
        
    if (!numberCharacteristic) {
        Serial.println("* Peripheral device does not have gesture_type characteristic!");
        peripheral.disconnect();
        return;
    } 
    
    while (peripheral.connected()) {
        numberCharacteristic.readValue(number);
        if (oldNumberValue != number) {
            oldNumberValue = number;
            Serial.print("* Reading value to number characteristic: ");
            Serial.println((int)number);
        }
    }
    Serial.println("- Peripheral device disconnected!");
    Serial.println("");
}