#include "states.h"

void state_func_init()
{
    //if variant is 0, this state just switches to state 1 (watch face) without
    //doing much.  if the down button is held, this state switches to variant 1,
    //which will allow the user to wipe their settings

    static bool selected_option = false;

    //variant 0
    //set up system after waking up from sleep
    //most setup is done in the deep sleep function, but code in the deep sleep function
    //won't be executed if the watch is being turned on for the first time (whereas this
    //code will)
    if (watch2::states[watch2::state].variant == 0)
    {
        //clear screen
        watch2::oled.fillScreen(0);

        if (watch2::mcp.digitalRead(dpad_down))
        {
            Serial.println("[init] switching to clear settings menu");
            watch2::switchState(watch2::state, 1);
        }
        else
        {
            //switch state
            Serial.println("[init] switching to watch face");
            watch2::switchState(1, 0);
        }
    }

    //variant 1
    //this will give the user a menu, which will allow them to wipe any saved system settings
    else if (watch2::states[watch2::state].variant == 1)
    {
        //settings clear mode
        if (!watch2::state_init)
        {
            Serial.println("[init] entered settings clear menu");
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setTextDatum(TC_DATUM);
            watch2::oled.drawString("Do you want to clear\nall saved settings?", SCREEN_WIDTH / 2,watch2::top_thing_height + 5);
            watch2::oled.setTextDatum(TL_DATUM);
        }

        if (dpad_up_active() || dpad_down_active())
        {
            selected_option = !selected_option;
        }

        draw(dpad_any_active(), {
            watch2::drawMenu(2, 100, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 37, {"No", "Yes"}, selected_option, {}, false, true, RED);
        });

        if (dpad_enter_active())
        {
            if (selected_option == 1) 
            {
                Serial.println("[init] clearing settings");
                watch2::preferences.begin("watch2");
                watch2::preferences.clear();
                watch2::preferences.end();
            }

            Serial.println("[init] switching to watch face");
            watch2::switchState(1);
        }
    }
}