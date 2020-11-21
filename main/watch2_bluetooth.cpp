/**
 * @file watch2_bluetooth.cpp
 * @author atctwo
 * @brief functions to handle the bluetooth subsystem
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"
#include "esp_bt.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

namespace watch2 {

    // bluetooth
    uint8_t bluetooth_state = 0;
    bool ble_set_up = false;
    BleKeyboard ble_keyboard("watch2", "atctwo");

    void enable_bluetooth()
    {
        Serial.println("[Bluetooth] enabling bluetooth");
        bluetooth_state = 1; // enabling

        // enable ble keyboard
        Serial.println("[Bluetooth] starting BLE keyboard");
        ble_keyboard.begin();

        // Serial.println("[BLE] ble device init");
        // BLEDevice::init("watch2");

        if (ble_set_up)
        {
            Serial.println("[BLE] ble already set up, restarting bt");
            btStart();
        }
        else
        {
            // Serial.println("[BLE] create server");
            // BLEServer *pServer = BLEDevice::createServer();

            // Serial.println("[BLE] set up services");
            // BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
            // BLECharacteristic *pCharacteristic = pService->createCharacteristic(
            //                                         "beb5483e-36e1-4688-b7f5-ea07361b26a8",
            //                                         BLECharacteristic::PROPERTY_READ |
            //                                         BLECharacteristic::PROPERTY_WRITE
            //                                     );

            // pCharacteristic->setValue("Hello World says Neil");
            // pService->start();

            // Serial.println("[BLE] set up advertising");
            // // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
            // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
            // pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
            // pAdvertising->setScanResponse(true);
            // pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
            // pAdvertising->setMinPreferred(0x12);
            ble_set_up = true;
        }

        //Serial.println("[BLE] starting advertising");
        //BLEDevice::startAdvertising();
        

        Serial.println("[Bluetooth] finished enabling bluetooth");
        bluetooth_state = 2;
    }

    void disable_bluetooth()
    {
        Serial.println("[Bluetooth] disabling bluetooth");

        // disable ble keyboard
        Serial.println("[Bluetooth] ending BLE keyboard");
        ble_keyboard.end();

        Serial.println("[BLE] stopping advertising");
        BLEDevice::stopAdvertising();

        Serial.println("[BLE] de-init ble device");
        BLEDevice::deinit(false);

        Serial.println("[Bluetooth] finished disabling bluetooth");
        bluetooth_state = 0;
    }

}