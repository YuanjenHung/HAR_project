#include <ArduinoBLE.h>

uint32_t light;
unsigned char humidity[4], temperature[4];

const char* hoppingServiceUuid = "3291138a-409d-11ec-973a-0242ac130003";
const char* hoppingAmbientCharacteristicUuid = "329115ce-409d-11ec-973a-0242ac130003";
const char* hoppingHumidityCharacteristicUuid = "329116b4-409d-11ec-973a-0242ac130003";
const char* hoppingTemperatureCharacteristicUuid = "3291177c-409d-11ec-973a-0242ac130003";

const char* deviceServiceUuid = "56f0165a-46f5-11ec-81d3-0242ac130003";
const char* ambientLightCharacteristicUuid = "56f01880-46f5-11ec-81d3-0242ac130003";
const char* humidityCharacteristicUuid = "56f01966-46f5-11ec-81d3-0242ac130003";
const char* temperatureCharacteristicUuid = "50ad77e2-46f5-11ec-81d3-0242ac130003";

/*
56f0165a-46f5-11ec-81d3-0242ac130003
56f01880-46f5-11ec-81d3-0242ac130003
56f01966-46f5-11ec-81d3-0242ac130003
56f01a38-46f5-11ec-81d3-0242ac130003
56f01e98-46f5-11ec-81d3-0242ac130003
56f01f74-46f5-11ec-81d3-0242ac130003
56f02028-46f5-11ec-81d3-0242ac130003
56f020e6-46f5-11ec-81d3-0242ac130003
*/

// Create BLEService
BLEService sensorsService(deviceServiceUuid); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic ambientLightCharacteristic(ambientLightCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(humidityCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic temperatureCharacteristic(temperatureCharacteristicUuid, BLERead | BLENotify);


float parse_float(unsigned char b[]){
    float f;
    unsigned char buffer[] = {b[0], b[1], b[2], b[3]};
    memcpy(&f, &buffer, sizeof(f));
    return f;
}

void setup() {
    // Serial.begin(115200);
    // while (!Serial);
    pinMode(LED_BUILTIN, OUTPUT);

    // begin initialization
    if (!BLE.begin()) {
        // Serial.println("starting BLE failed!");

        while (1);
    }

    // Serial.println("BLE Central scan");

    // Set device name, local name and advertised service
    BLE.setLocalName("Hopping Bathroom");
    BLE.setDeviceName("Hopping Bathroom");
    BLE.setAdvertisedService(sensorsService);

    // Add all the characteristic to the service
    sensorsService.addCharacteristic(ambientLightCharacteristic); 
    sensorsService.addCharacteristic(humidityCharacteristic); 
    sensorsService.addCharacteristic(temperatureCharacteristic);

    // Add the services to the device
    BLE.addService(sensorsService);
    
    // Start advertising
    BLE.advertise();
    
    // start scanning for peripheral
    BLE.scanForUuid(hoppingServiceUuid);
}

void loop() {
    checkPeripheral();   
}

void checkPeripheral() {
    BLEDevice peripheral = BLE.available();
    if (peripheral) {
        // discovered a peripheral
        // Serial.println("Discovered a peripheral");
        // Serial.println("-----------------------");

        // print address
        // Serial.print("Address: ");
        // Serial.println(peripheral.address());

        // print the local name, if present
        if (peripheral.hasLocalName()) {
            // Serial.print("Local Name: ");
            // Serial.println(peripheral.localName());
        }

        // print the advertised service UUIDs, if present
        if (peripheral.hasAdvertisedServiceUuid()) {
            // Serial.print("Service UUIDs: ");
            for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
                // Serial.print(peripheral.advertisedServiceUuid(i));
                // Serial.print(" ");
            }
            // Serial.println();
        }

        // print the RSSI
        // Serial.print("RSSI: ");
        // Serial.println(peripheral.rssi());

        // Serial.println();

        BLE.stopScan();
        connectToPeripheral(peripheral);
    }
}

void connectToPeripheral(BLEDevice peripheral) {
    // Serial.println("- Connecting to peripheral device...");

    if (peripheral.connect()) {
        // Serial.println("* Connected to peripheral device!");
        // Serial.println(" ");
    } else {
        // Serial.println("* Connection to peripheral device failed!");
        // Serial.println(" ");
    }
    subscribeToPeripheral(peripheral);
}

void subscribeToPeripheral(BLEDevice peripheral) {
    
    // Serial.println("- Discovering peripheral device attributes...");
    if (peripheral.discoverAttributes()) {
        // Serial.println("* Peripheral device attributes discovered!");
        // Serial.println(" ");
    } else {
        // Serial.println("* Peripheral device attributes discovery failed!");
        // Serial.println(" ");
        peripheral.disconnect();
        connectToPeripheral(peripheral);
    }

    BLECharacteristic hoppingAmbientCharacteristic = peripheral.characteristic(hoppingAmbientCharacteristicUuid);
    BLECharacteristic hoppingHumidityCharacteristic = peripheral.characteristic(hoppingHumidityCharacteristicUuid);
    BLECharacteristic hoppingTemperatureCharacteristic = peripheral.characteristic(hoppingTemperatureCharacteristicUuid);
        
    if (!hoppingAmbientCharacteristic || !hoppingHumidityCharacteristic || !hoppingTemperatureCharacteristic) {
        // Serial.println("* Peripheral device does not have such characteristic!");
        peripheral.disconnect();
        connectToPeripheral(peripheral);
    } 

    if (!hoppingAmbientCharacteristic.subscribe() || !hoppingHumidityCharacteristic.subscribe() || !hoppingTemperatureCharacteristic.subscribe()) {
        // Serial.println("Subscription failed!");
        peripheral.disconnect();
        connectToPeripheral(peripheral);
    }
    
    while (peripheral.connected()) {
        if (hoppingAmbientCharacteristic.valueUpdated()) {
            hoppingAmbientCharacteristic.readValue(light);
            // Serial.print("* light: ");
            // Serial.println((int)light);
            ambientLightCharacteristic.writeValue(light);
        }
        if (hoppingHumidityCharacteristic.valueUpdated()) {
            hoppingHumidityCharacteristic.readValue(humidity, 4);
            // Serial.print("* humidity: ");
            // Serial.println(parse_float(humidity));
            humidityCharacteristic.writeValue(parse_float(humidity));
        }
        if (hoppingTemperatureCharacteristic.valueUpdated()) {
            hoppingTemperatureCharacteristic.readValue(temperature, 4);
            // Serial.print("* temperature: ");
            // Serial.println(parse_float(temperature));
            temperatureCharacteristic.writeValue(parse_float(temperature));
        }
        if (!BLE.central()) {
            digitalWrite(LED_BUILTIN, LOW);
        } else {
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    // Serial.println("- Peripheral device disconnected!");
    // Serial.println("");
}