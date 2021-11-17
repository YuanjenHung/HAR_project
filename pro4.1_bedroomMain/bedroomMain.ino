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

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* ambientLightCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* humidityCharacteristicUuid = "19b10002-e8f2-537e-4f6c-d104768a1214";
const char* temperatureCharacteristicUuid = "19b10003-e8f2-537e-4f6c-d104768a1214";

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

void updateWeather(){
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
    // Enable the BLE capabilities, Serial port, led and sensor
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
    ISR_Timer.disableAll();

    // Set device name, local name and advertised service
    BLE.setLocalName("Bedroom Main");
    BLE.setDeviceName("Bedroom Main");
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

void loop(){
    BLE.advertise();
    digitalWrite(LED_BUILTIN, LOW);
    ISR_Timer.disableAll();

    while(!BLE.connected());
    digitalWrite(LED_BUILTIN, HIGH);
    ISR_Timer.enableAll();

    while(BLE.connected()){
        if (is_interrupt_1_enabled){
            updateWeather();
            is_interrupt_1_enabled = false;
        }
    } 
}