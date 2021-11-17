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
#define TIMER_INTERVAL_S 60

bool volatile is_interrupt_1_enabled = false;
float temperature, humidity;
int r, g, b, a;
// uint16_t force;


const char* hoppingServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* hoppingCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

const char* deviceServiceUuid = "735a587c-4561-11ec-81d3-0242ac130003";
const char* ambientLightCharacteristicUuid = "735a5a8e-4561-11ec-81d3-0242ac130003";
const char* humidityCharacteristicUuid = "735a5b74-4561-11ec-81d3-0242ac130003";
const char* temperatureCharacteristicUuid = "735a5c50-4561-11ec-81d3-0242ac130003";
// const char* forceCharacteristicUuid = "735a5d0e-4561-11ec-81d3-0242ac130003";

/*
735a587c-4561-11ec-81d3-0242ac130003
735a5a8e-4561-11ec-81d3-0242ac130003
735a5b74-4561-11ec-81d3-0242ac130003
735a5c50-4561-11ec-81d3-0242ac130003
735a5d0e-4561-11ec-81d3-0242ac130003
735a5dd6-4561-11ec-81d3-0242ac130003
735a64c0-4561-11ec-81d3-0242ac130003
735a65ce-4561-11ec-81d3-0242ac130003
*/

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;

// Create BLEService
BLEService sensorsService(deviceServiceUuid); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic ambientLightCharacteristic(ambientLightCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic humidityCharacteristic(humidityCharacteristicUuid, BLERead | BLENotify);
BLEFloatCharacteristic temperatureCharacteristic(temperatureCharacteristicUuid, BLERead | BLENotify);
// BLEIntCharacteristic forceCharacteristic(forceCharacteristicUuid, BLERead | BLENotify);

void enable_interrupt_1(){
    is_interrupt_1_enabled = true;
}

void TimerHandler(void){
    ISR_Timer.run();
}

void update(){
    temperature = HTS.readTemperature();
    humidity = HTS.readHumidity();
    while(!APDS.colorAvailable()) delay(10);
    APDS.readColor(r, g, b, a);

    ambientLightCharacteristic.writeValue(a);
    humidityCharacteristic.writeValue(humidity);
    temperatureCharacteristic.writeValue(temperature);

    // Serial.println("------------For debugging-------------");
    // Serial.print("Ambient light = ");
    // Serial.println(a);
    // Serial.print("Temperature = ");
    // Serial.print(temperature);
    // Serial.println(" °C");
    // Serial.print("Humidity = ");
    // Serial.print(humidity);
    // Serial.println(" %");
}

void setup() {
    // Enable the BLE capabilities, port, led and sensor
    // Serial.begin(115200);
    // while(!Serial);
    pinMode(LED_BUILTIN, OUTPUT);
    BLE.begin();
    HTS.begin();
    APDS.begin();
    if (!BLE.begin()) while (1);

    // Set up the timer and disable it
    ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
    ISR_Timer.setInterval(TIMER_INTERVAL_S * 1000, enable_interrupt_1);
    ISR_Timer.enableAll();

    // Set device name, local name and advertised service
    BLE.setLocalName("Kitchen Main");
    BLE.setDeviceName("Kitchen Main");
    BLE.setAdvertisedService(sensorsService);

    // Add all the characteristic to the service
    sensorsService.addCharacteristic(ambientLightCharacteristic); 
    sensorsService.addCharacteristic(humidityCharacteristic); 
    sensorsService.addCharacteristic(temperatureCharacteristic);
    // sensorsService.addCharacteristic(forceCharacteristic);

    // Add the services to the device
    BLE.addService(sensorsService);
    
    // Start advertising
    BLE.advertise();
}

void loop() {
    // connectToPeripheral();

    if (BLE.connected()) {
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        digitalWrite(LED_BUILTIN, LOW);
        BLE.advertise();
    }

    if (is_interrupt_1_enabled){
        update();
        is_interrupt_1_enabled = false;
    }
}

void connectToPeripheral(){
    BLEDevice peripheral;
    
    // Serial.println("- Discovering peripheral device...");

    do
    {
        BLE.scanForUuid(hoppingServiceUuid);
        peripheral = BLE.available();
    } while (!peripheral);
    
    if (peripheral) {
        // Serial.println("* Peripheral device found!");
        // Serial.print("* Device MAC address: ");
        // Serial.println(peripheral.address());
        // Serial.print("* Device name: ");
        // Serial.println(peripheral.localName());
        // Serial.print("* Advertised service UUID: ");
        // Serial.println(peripheral.advertisedServiceUuid());
        // Serial.println(" ");
        BLE.stopScan();
        controlPeripheral(peripheral);
    }
}

void controlPeripheral(BLEDevice peripheral) {
    // Serial.println("- Connecting to peripheral device...");

    if (peripheral.connect()) {
        // Serial.println("* Connected to peripheral device!");
        // Serial.println(" ");
    } else {
        // Serial.println("* Connection to peripheral device failed!");
        // Serial.println(" ");
        return;
    }

    // Serial.println("- Discovering peripheral device attributes...");
    if (peripheral.discoverAttributes()) {
        // Serial.println("* Peripheral device attributes discovered!");
        // Serial.println(" ");
    } else {
        // Serial.println("* Peripheral device attributes discovery failed!");
        // Serial.println(" ");
        peripheral.disconnect();
        return;
    }

    BLECharacteristic hoppingCharacteristic = peripheral.characteristic(hoppingCharacteristicUuid);
        
    if (!hoppingCharacteristic) {
        // Serial.println("* Peripheral device does not have such characteristic!");
        peripheral.disconnect();
        return;
    } 

    if (!hoppingCharacteristic.subscribe()) {
        // Serial.println("Subscription failed!");
        peripheral.disconnect();
        return;
    }
    
    while (peripheral.connected()) {
        if (hoppingCharacteristic.valueUpdated()) {
            // hoppingCharacteristic.readValue(force);
            // Serial.print("* Update value from force sensor: ");
            // Serial.println((int)force);
            // forceCharacteristic.writeValue(force);
        }
        if (!BLE.central()) {
            BLE.advertise();
            digitalWrite(LED_BUILTIN, LOW);
            ISR_Timer.disableAll();
        } else {
            digitalWrite(LED_BUILTIN, HIGH);
            ISR_Timer.enableAll();
            if (is_interrupt_1_enabled){
                update();
                is_interrupt_1_enabled = false;
            }
        }
    }
    // Serial.println("- Peripheral device disconnected!");
    // Serial.println("");
}