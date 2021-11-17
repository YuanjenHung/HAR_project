// BLE
#include <ArduinoBLE.h>

uint16_t ambient; 
// byte temperature, humidity;


const char* bathroomServiceUuid = "2ba69d4c-409d-11ec-973a-0242ac130003";
const char* bathroomAmbientCharacteristicUuid = "2ba69f40-409d-11ec-973a-0242ac130003";
const char* bathroomHumidityCharacteristicUuid = "2ba6a198-409d-11ec-973a-0242ac130003";
const char* bathroomTemperatureCharacteristicUuid = "2ba6a26a-409d-11ec-973a-0242ac130003";

const char* deviceServiceUuid = "56f0165a-46f5-11ec-81d3-0242ac130003";
const char* ambientLightCharacteristicUuid = "56f01880-46f5-11ec-81d3-0242ac130003";
const char* humidityCharacteristicUuid = "56f01966-46f5-11ec-81d3-0242ac130003";
const char* temperatureCharacteristicUuid = "50ad77e2-46f5-11ec-81d3-0242ac130003";

// Create BLEService
BLEService sensorsService(deviceServiceUuid); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic ambientLightCharacteristic(ambientLightCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(humidityCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic temperatureCharacteristic(temperatureCharacteristicUuid, BLERead | BLENotify);

void setup() {
    // Enable the BLE capabilities, Serial port, led and sensor
    Serial.begin(115200);
    while(!Serial);
    pinMode(LED_BUILTIN, OUTPUT);
    if (!BLE.begin()) while (1);

    // Set device name, local name and advertised service
    BLE.setLocalName("Hopping For Bathroom");
    BLE.setDeviceName("Hopping For Bathroom");
    BLE.setAdvertisedService(sensorsService);

    // Add all the characteristic to the service
    sensorsService.addCharacteristic(ambientLightCharacteristic); 
    sensorsService.addCharacteristic(humidityCharacteristic); 
    sensorsService.addCharacteristic(temperatureCharacteristic);

    // Add the services to the device
    BLE.addService(sensorsService);
    
    // Start advertising
    BLE.advertise();
}

void loop() {
    connectToPeripheral();
}

void connectToPeripheral(){
    BLEDevice peripheral;
    
    Serial.println("- Discovering peripheral device...");

    do
    {
        BLE.scanForUuid(bathroomServiceUuid);
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

    BLECharacteristic bathroomAmbientCharacteristic = peripheral.characteristic(bathroomAmbientCharacteristicUuid);
    // BLECharacteristic bathroomHumidityCharacteristic = peripheral.characteristic(bathroomHumidityCharacteristicUuid);
    // BLECharacteristic bathroomTemperatureCharacteristic = peripheral.characteristic(bathroomTemperatureCharacteristicUuid);
        
    if (!bathroomAmbientCharacteristic) {
        Serial.println("* Peripheral device does not have such characteristic!");
        peripheral.disconnect();
        return;
    } 

    if (!bathroomAmbientCharacteristic.subscribe()) {
        Serial.println("Subscription failed!");
        peripheral.disconnect();
        return;
    }
    while (peripheral.connected()) {
        if (bathroomAmbientCharacteristic.valueUpdated()) {
            bathroomAmbientCharacteristic.readValue(ambient);
            // ambientLightCharacteristic.writeValue(ambient);
            Serial.println("Ambient: " + ambient);
        }
        // if (bathroomHumidityCharacteristic.valueUpdated()) {
        //     bathroomHumidityCharacteristic.readValue(humidity);
        //     // humidityCharacteristic.writeValue(humidity);
        //     Serial.println("Humidity: " + humidity);
        // }
        // if (bathroomTemperatureCharacteristic.valueUpdated()) {
        //     bathroomTemperatureCharacteristic.readValue(temperature);
        //     // temperatureCharacteristic.writeValue(temperature);
        //     Serial.println("Temperature: " + temperature);
        // }
        if (!BLE.central()) {
            BLE.advertise();
            digitalWrite(LED_BUILTIN, LOW);
        } else {
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    Serial.println("- Peripheral device disconnected!");
    Serial.println("");
}