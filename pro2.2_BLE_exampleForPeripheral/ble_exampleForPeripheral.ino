// BLE
#include <ArduinoBLE.h>

// Interrupt timer
#include "NRF52_MBED_ISR_Timer.h"
#include "NRF52_MBED_TimerInterrupt.h"
      
// Define constants to be used later on
#define HW_TIMER_INTERVAL_MS 10
#define TIMER_INTERVAL_S 5

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

bool volatile is_interrupt_1_enabled = false;

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;

long number = random(1, 6);

BLEService provideNumberService(deviceServiceUuid); 
BLEIntCharacteristic numberCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLENotify);

void enable_interrupt_1(){
    is_interrupt_1_enabled = true;
}

void TimerHandler(void){
    ISR_Timer.run();
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (!BLE.begin()) {
        while (1);
    }

    // Set up the timer and disable it
    ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler);
    ISR_Timer.setInterval(TIMER_INTERVAL_S * 1000, enable_interrupt_1);
    ISR_Timer.disableAll();

    BLE.setLocalName("Arduino Nano 33 BLE (Peripheral)");
    BLE.setAdvertisedService(provideNumberService);
    provideNumberService.addCharacteristic(numberCharacteristic);
    BLE.addService(provideNumberService);
    numberCharacteristic.writeValue(number);
    BLE.advertise();

    // Serial.println("Nano 33 BLE (Peripheral Device)");
    // Serial.println(" ");
}

void loop() {
    ISR_Timer.disableAll();

    while(!BLE.connected());
    digitalWrite(LED_BUILTIN, HIGH);
    ISR_Timer.enableAll();

    while(BLE.connected()){
        if (is_interrupt_1_enabled){
            number = random(1, 6);
            numberCharacteristic.writeValue(number);
            is_interrupt_1_enabled = false;
        }
    }
    BLE.advertise();
    digitalWrite(LED_BUILTIN, LOW);
}