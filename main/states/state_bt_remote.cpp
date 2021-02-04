
#include "../watch2.h"
#include <HIDKeyboardTypes.h>

void state_func_bt_remote()
{

    if (watch2::states[watch2::state].variant == 0)
    {
        if (watch2::bluetooth_state == 3) watch2::switchState(watch2::state, 1);
        else
        {
            if (!watch2::state_init)
            {
                Serial.println(watch2::bluetooth_state);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.println("Can't open remote\nbecause...");

                switch(watch2::bluetooth_state)
                {
                    case 0: // disabled
                        watch2::oled.println("Bluetooth Disabled");
                        break;

                    case 1: // enabling
                        watch2::oled.println("Still enabling\nBluetooth");
                        break;

                    case 2: // not connected
                        watch2::oled.println("Not Connected");
                        break;

                    case 3: // connected
                        watch2::oled.println("Connected...?\nsomething's wrong");
                }
            }
        }

        if (dpad_left_active())
        {
            watch2::switchState(2);
        }
    }

    if (watch2::states[watch2::state].variant == 1)
    {
        if (watch2::bluetooth_state != 3) watch2::switchState(watch2::state, 0);
        else
        {
            if (!watch2::state_init)
            {
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.println("Connected");
            }

            if (dpad_enter_active())
            {
                Serial.println("[ble remote] pressing play/pause");

                const char* hello = "Hello world from esp32 hid keyboard!!!\n";

                while(*hello){
                    
                    KEYMAP map = keymap[(uint8_t)*hello];
                    uint8_t msg[] = {map.modifier, 0x0, map.usage, 0x0,0x0,0x0,0x0,0x0};
                    watch2::ble_hid_send_report(msg, sizeof(msg));

                    hello++;
                    uint8_t msg1[] = {0x0, 0x0, 0x0, 0x0,0x0,0x0,0x0,0x0};
                    watch2::ble_hid_send_report(msg1, sizeof(msg1));
                    delay(10);
                }

                delay(500);
            }
            if (dpad_left_active())
            {
                Serial.println("[ble remote] pressing previous");
                watch2::ble_hid_send_media_key_report();
                delay(500);
            }
            // if (dpad_right_active())
            // {
            //     Serial.println("[ble remote] pressing next");
            //     watch2::ble_keyboard.write(KEY_MEDIA_NEXT_TRACK);
            //     delay(500);
            // }
            // if (dpad_up_active())
            // {
            //     Serial.println("[ble remote] pressing volume up");
            //     watch2::ble_keyboard.write(KEY_MEDIA_VOLUME_UP);
            //     delay(500);
            // }
            // if (dpad_down_active())
            // {
            //     Serial.println("[ble remote] pressing volume down");
            //     watch2::ble_keyboard.write(KEY_MEDIA_VOLUME_DOWN);
            //     delay(500);
            // }
        }

        if (watch2::btn_zero.wasPressed())
        {
            watch2::switchState(2);
        }
    }

}