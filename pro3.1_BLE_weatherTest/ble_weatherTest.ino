// pressure
#include <Arduino_LPS22HB.h>
// temperature and humidity
#include <Arduino_HTS221.h>
// light intensity
#include <Arduino_APDS9960.h>
// BLE
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>
// Interrupt timer
#include "NRF52_MBED_ISR_Timer.h"
#include "NRF52_MBED_TimerInterrupt.h"

// Define constants to be used later on
#define HW_TIMER_INTERVAL_MS 10
#define TIMER_INTERVAL_S 3

bool volatile is_interrupt_1_enabled = false;
float temperature, humidity, pressure;
int r, g, b, a;

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;

void enable_interrupt_1(){
    is_interrupt_1_enabled = true;
}

void TimerHandler(void){
    ISR_Timer.run();
}

// Create BLEService
BLEService sensorsService("19B10000-E8F2-537E-4F6C-D104768A1214"); 

// Create some characteristics to be added to the sensorsService
BLEIntCharacteristic ambientLightCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify);
BLEFloatCharacteristic humidityCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLENotify);
BLEFloatCharacteristic temperatureCharacteristic("19B10003-E8F2-537E-4F6C-D104768A1214", BLENotify);
BLEFloatCharacteristic pressureCharacteristic("19B10004-E8F2-537E-4F6C-D104768A1214", BLENotify);

void updateWeather(){
    temperature = HTS.readTemperature();
    humidity = HTS.readHumidity();
    pressure = BARO.readPressure();
    while(!APDS.colorAvailable()) delay(10);
    APDS.readColor(r, g, b, a);

    ambientLightCharacteristic.writeValue(a);
    humidityCharacteristic.writeValue(humidity);
    temperatureCharacteristic.writeValue(temperature);
    pressureCharacteristic.writeValue(pressure);

    Serial.println("------------For debugging-------------");
    Serial.print("Ambient light = ");
    Serial.println(a);
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" kPa");
}

void setup(){
    // Enable the BLE capabilities, Serial port, led and sensor
    Serial.begin(115200);
    while(!Serial);
    pinMode(LED_BUILTIN, OUTPUT);
    BLE.begin();
    BARO.begin();
    HTS.begin();
    APDS.begin();

    // Set up the timer and disable it
    ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
    ISR_Timer.setInterval(TIMER_INTERVAL_S * 1000, enable_interrupt_1);
    ISR_Timer.disableAll();

    // Set “Arduino Nano 33 BLE sense” as a device name and local name
    BLE.setDeviceName("Arduino Nano 33 BLE Sense");
    BLE.setLocalName("Arduino Nano 33 BLE Sense");

    // Add all the characteristic to the service
    sensorsService.addCharacteristic(ambientLightCharacteristic); 
    sensorsService.addCharacteristic(humidityCharacteristic); 
    sensorsService.addCharacteristic(temperatureCharacteristic); 
    sensorsService.addCharacteristic(pressureCharacteristic); 

    // Add the services to the device
    BLE.addService(sensorsService);

    // Start advertising
    BLE.advertise();
}

void initialize(){
    Serial.println("Everything is initialized.");
    digitalWrite(LED_BUILTIN, LOW);
    ISR_Timer.disableAll();
}

void loop(){
    BLE.advertise();
    initialize();

    while(!BLE.connected());
    String address = BLE.address();
    Serial.print("Connected and the Address is: ");
    Serial.println(address);
    digitalWrite(LED_BUILTIN, HIGH);
    ISR_Timer.enableAll();

    while(BLE.connected()){
        if (is_interrupt_1_enabled){
            updateWeather();
            is_interrupt_1_enabled = false;
        }
    } 
}