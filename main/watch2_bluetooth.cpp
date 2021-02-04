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

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


namespace watch2 {

    

    // bluetooth
    uint8_t bluetooth_state = 0;
    bool ble_set_up = false;
    //BleKeyboard ble_keyboard("watch2", "atctwo");

    BLEServer *pServer;
    BLEHIDDevice *ble_hid;
    BLECharacteristic *input_keyboard;
    BLECharacteristic *output_keyboard;
    BLECharacteristic *input_media_keys;

    // Report IDs:
    #define KEYBOARD_ID 0x01
    #define MEDIA_KEYS_ID 0x02

    static const uint8_t _hidReportDescriptor[] = {
        USAGE_PAGE(1),      0x01,       // Generic Desktop Ctrls
        USAGE(1),           0x06,       // Keyboard
        COLLECTION(1),      0x01,       // Application

        REPORT_ID(1),       0x01,        //   Report ID (1)
        USAGE_PAGE(1),      0x07,       //   Kbrd/Keypad
        USAGE_MINIMUM(1),   0xE0,
        USAGE_MAXIMUM(1),   0xE7,
        LOGICAL_MINIMUM(1), 0x00,
        LOGICAL_MAXIMUM(1), 0x01,
        REPORT_SIZE(1),     0x01,       //   1 byte (Modifier)
        REPORT_COUNT(1),    0x08,
        HIDINPUT(1),           0x02,       //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position
        REPORT_COUNT(1),    0x01,       //   1 byte (Reserved)
        REPORT_SIZE(1),     0x08,
        HIDINPUT(1),           0x01,       //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
        REPORT_COUNT(1),    0x06,       //   6 bytes (Keys)
        REPORT_SIZE(1),     0x08,
        LOGICAL_MINIMUM(1), 0x00,
        LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
        USAGE_MINIMUM(1),   0x00,
        USAGE_MAXIMUM(1),   0x65,
        HIDINPUT(1),           0x00,       //   Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position
        REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
        REPORT_SIZE(1),     0x01,
        USAGE_PAGE(1),      0x08,       //   LEDs
        USAGE_MINIMUM(1),   0x01,       //   Num Lock
        USAGE_MAXIMUM(1),   0x05,       //   Kana
        HIDOUTPUT(1),          0x02,       //   Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
        REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
        REPORT_SIZE(1),     0x03,
        HIDOUTPUT(1),          0x01,       //   Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile
        END_COLLECTION(0),
        // ------------------------------------------------- Media Keys
        USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
        USAGE(1),           0x01,          // USAGE (Consumer Control)
        COLLECTION(1),      0x01,          // COLLECTION (Application)
        REPORT_ID(1),       MEDIA_KEYS_ID, //   REPORT_ID (3)
        USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
        LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
        LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
        REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
        REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
        USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
        USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
        USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
        USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
        USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
        USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
        USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
        USAGE(2),           0x23, 0x02,    //   Usage (WWW Home)            ; bit 7: 128
        USAGE(2),           0x94, 0x01,    //   Usage (My Computer) ; bit 0: 1
        USAGE(2),           0x92, 0x01,    //   Usage (Calculator)  ; bit 1: 2
        USAGE(2),           0x2A, 0x02,    //   Usage (WWW fav)     ; bit 2: 4
        USAGE(2),           0x21, 0x02,    //   Usage (WWW search)  ; bit 3: 8
        USAGE(2),           0x26, 0x02,    //   Usage (WWW stop)    ; bit 4: 16
        USAGE(2),           0x24, 0x02,    //   Usage (WWW back)    ; bit 5: 32
        USAGE(2),           0x83, 0x01,    //   Usage (Media sel)   ; bit 6: 64
        USAGE(2),           0x8A, 0x01,    //   Usage (Mail)        ; bit 7: 128
        HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        END_COLLECTION(0)                  // END_COLLECTION
    };

    class watch2_ble_server_callbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* pServer){
            bluetooth_state = 3;
            BLE2902* desc = (BLE2902*)input_keyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
            desc->setNotifications(true);

            desc = (BLE2902*)input_media_keys->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
            desc->setNotifications(true);

            Serial.println("[bluetooth] connected to device");
        }

        void onDisconnect(BLEServer* pServer){
            bluetooth_state = 2;
            BLE2902* desc = (BLE2902*)input_keyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
            desc->setNotifications(false);

            desc = (BLE2902*)input_media_keys->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
            desc->setNotifications(false);

            Serial.println("[bluetooth] disconnected from device");
        }
    };

    void ble_server_task()
    {
        // set up test server
        BLEDevice::init("watch2");
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new watch2_ble_server_callbacks());

        // set up example service
        BLEService *pService = pServer->createService(SERVICE_UUID);
        BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                                CHARACTERISTIC_UUID,
                                                BLECharacteristic::PROPERTY_READ |
                                                BLECharacteristic::PROPERTY_WRITE
                                            );

        pCharacteristic->setValue("Hello World says Neil");
        pService->start();

        // set up ble hid
        ble_hid = new BLEHIDDevice(pServer);
        ble_hid->manufacturer()->setValue("atctwo");
        ble_hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
        ble_hid->hidInfo(0x00, 0x01);

        input_keyboard = ble_hid->inputReport(0x01);
        output_keyboard = ble_hid->outputReport(0x01);
        input_media_keys = ble_hid->inputReport(0x02);

        ble_hid->reportMap((uint8_t*) _hidReportDescriptor, sizeof(_hidReportDescriptor));
        ble_hid->startServices();


        // set up ble security
        BLESecurity *pSecurity = new BLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);


        // set up advertising
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->addServiceUUID(ble_hid->hidService()->getUUID());
        pAdvertising->setAppearance(ESP_BLE_APPEARANCE_GENERIC_WATCH);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
        pAdvertising->setMinPreferred(0x12);
        pAdvertising->start();

        Serial.println("[bluetooth] started BLE server task");
        vTaskDelay(portMAX_DELAY);
    
    }

    void ble_hid_send_report(uint8_t* report, uint16_t size)
    {
        Serial.print("[bluetooth] sending hid report: ");
        for (int i = 0; i < size; i++) Serial.printf("0x%2x, ", report[i]);
        Serial.println("");

        input_keyboard->setValue(report, size);
        input_keyboard->notify();
    }

    void ble_hid_send_media_key_report()
    {
        uint8_t media_key_report[3] = {0x08, 0x00, 0x00}; // send play / pause
        input_media_keys->setValue(media_key_report, 3);
        input_media_keys->notify();

        uint8_t media_key_report1[3] = {0, 0, 0};
        input_media_keys->setValue(media_key_report1, 3);
        input_media_keys->notify();
    }

    void enable_bluetooth()
    {
        Serial.println("[Bluetooth] enabling bluetooth");
        bluetooth_state = 1; // enabling

        // enable ble keyboard
        //Serial.println("[Bluetooth] starting BLE keyboard");
        //ble_keyboard.begin();

        // Serial.println("[BLE] ble device init");
        // BLEDevice::init("watch2");

        if (ble_set_up)
        {
            Serial.println("[BLE] ble already set up, restarting bt");
        }
        else
        {
            xTaskCreate((TaskFunction_t) ble_server_task, "ble-server", 20000, NULL, 5, NULL);
            ble_set_up = true;
        }

        //Serial.println("[bluetooth] starting advertising");
        //BLEDevice::startAdvertising();
        

        Serial.println("[Bluetooth] finished enabling bluetooth");
        bluetooth_state = 3;
    }

    void disable_bluetooth()
    {
        Serial.println("[Bluetooth] disabling bluetooth");

        // disable ble keyboard
        //Serial.println("[Bluetooth] ending BLE keyboard");
        //ble_keyboard.end();

        Serial.println("[BLE] stopping advertising");
        BLEDevice::stopAdvertising();

        Serial.println("[BLE] de-init ble device");
        BLEDevice::deinit(false);

        btStop();
        delete pServer;

        Serial.println("[Bluetooth] finished disabling bluetooth");
        bluetooth_state = 0;
    }

}