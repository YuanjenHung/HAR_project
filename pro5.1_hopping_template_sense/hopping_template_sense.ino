// BLE
#include <ArduinoBLE.h>

// Interrupt timer
#include "NRF52_MBED_ISR_Timer.h"
#include "NRF52_MBED_TimerInterrupt.h"

// Sensor - pressure, temperature, humidity, light
#include <Arduino_LPS22HB.h>
#include <Arduino_HTS221.h>
#include <Arduino_APDS9960.h>
      
// Define constants to be used later on
#define HW_TIMER_INTERVAL_MS 10
#define TIMER_INTERVAL_S 5

bool volatile is_interrupt_1_enabled = false;
uint16_t force;
unsigned char humidity[4], temperature[4];

const char* hoppingServiceUuid = "2ba69d4c-409d-11ec-973a-0242ac130003";
const char* hoppingAmbientCharacteristicUuid = "2ba69f40-409d-11ec-973a-0242ac130003";
const char* hoppingHumidityCharacteristicUuid = "2ba6a198-409d-11ec-973a-0242ac130003";
const char* hoppingTemperatureCharacteristicUuid = "2ba6a26a-409d-11ec-973a-0242ac130003";

const char* deviceServiceUuid = "710c9972-4561-11ec-81d3-0242ac130003";
const char* ambientLightCharacteristicUuid = "710c9c10-4561-11ec-81d3-0242ac130003";
const char* humidityCharacteristicUuid = "710c9d46-4561-11ec-81d3-0242ac130003";
const char* temperatureCharacteristicUuid = "710c9e22-4561-11ec-81d3-0242ac130003";


/*
710c9972-4561-11ec-81d3-0242ac130003
710c9c10-4561-11ec-81d3-0242ac130003
710c9d46-4561-11ec-81d3-0242ac130003
710c9e22-4561-11ec-81d3-0242ac130003
710c9ee0-4561-11ec-81d3-0242ac130003
*/

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;

// Create BLEService
BLEService sensorsService(deviceServiceUuid); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic ambientLightCharacteristic(ambientLightCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(humidityCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic temperatureCharacteristic(temperatureCharacteristicUuid, BLERead | BLENotify);

void enable_interrupt_1(){
    is_interrupt_1_enabled = true;
}

void TimerHandler(void){
    ISR_Timer.run();
}

float parse_float(unsigned char b[]){
    float f;
    unsigned char buffer[] = {b[0], b[1], b[2], b[3]};
    memcpy(&f, &buffer, sizeof(f));
    return f;
}

void setup() {
    // Enable the BLE capabilities, Serial port, led and sensor
    Serial.begin(115200);
    while(!Serial);
    pinMode(LED_BUILTIN, OUTPUT);
    BLE.begin();
    if (!BLE.begin()) while (1);

    // Set up the timer and disable it
    ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
    ISR_Timer.setInterval(TIMER_INTERVAL_S * 1000, enable_interrupt_1);
    ISR_Timer.disableAll();

    // Set device name, local name and advertised service
    BLE.setLocalName("Kitchen Main");
    BLE.setDeviceName("Kitchen Main");
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
    // if (enableHopping) {
    //     connectToPeripheral();
    // } else {
    //     BLE.advertise();
    //     digitalWrite(LED_BUILTIN, LOW);
    //     ISR_Timer.disableAll();

    //     while(!BLE.connected());
    //     digitalWrite(LED_BUILTIN, HIGH);
    //     ISR_Timer.enableAll();

    //     while(BLE.connected()){
    //         if (is_interrupt_1_enabled){
    //             update();
    //             is_interrupt_1_enabled = false;
    //         }
    //     } 
    // }
}

void connectToPeripheral(){
    BLEDevice peripheral;
    
    Serial.println("- Discovering peripheral device...");

    do
    {
        BLE.scanForUuid(hoppingServiceUuid);
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

    BLECharacteristic hoppingAmbientCharacteristic = peripheral.characteristic(hoppingAmbientCharacteristicUuid);
    BLECharacteristic hoppingHumidityCharacteristic = peripheral.characteristic(hoppingHumidityCharacteristicUuid);
    BLECharacteristic hoppingTemperatureCharacteristic = peripheral.characteristic(hoppingTemperatureCharacteristicUuid);
        
    if (!hoppingAmbientCharacteristic || !hoppingHumidityCharacteristic || !hoppingTemperatureCharacteristic) {
        Serial.println("* Peripheral device does not have such characteristic!");
        peripheral.disconnect();
        return;
    } 

    if (!hoppingAmbientCharacteristic.subscribe() || !hoppingHumidityCharacteristic.subscribe() || !hoppingTemperatureCharacteristic.subscribe()) {
        Serial.println("Subscription failed!");
        peripheral.disconnect();
        return;
    }
    
    while (peripheral.connected()) {
        if (hoppingAmbientCharacteristic.valueUpdated()) {
            hoppingAmbientCharacteristic.readValue(force);
            Serial.print("* light: ");
            Serial.println((int)force);
            // forceCharacteristic.writeValue(force);
        }
        if (hoppingHumidityCharacteristic.valueUpdated()) {
            hoppingHumidityCharacteristic.readValue(humidity, 4);
            Serial.print("* humidity: ");
            Serial.println(parse_float(humidity));
            // forceCharacteristic.writeValue(force);
        }
        if (hoppingTemperatureCharacteristic.valueUpdated()) {
            hoppingTemperatureCharacteristic.readValue(temperature, 4);
            Serial.print("* temperature: ");
            Serial.println(parse_float(temperature));
            // forceCharacteristic.writeValue(force);
        }
        if (!BLE.central()) {
            BLE.advertise();
            digitalWrite(LED_BUILTIN, LOW);
            // ISR_Timer.disableAll();
        } else {
            digitalWrite(LED_BUILTIN, HIGH);
            // ISR_Timer.enableAll();
            // if (is_interrupt_1_enabled){
            //     update();
            //     is_interrupt_1_enabled = false;
            // }
        }
    }
    Serial.println("- Peripheral device disconnected!");
    Serial.println("");
}