#include "../src/watch2.h"

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

        //dim screen
        uint8_t contrast = 0x00;
        //watch2::oled.sendCommand(0xC7, &contrast, 1);
    }

    //variant 1
    //this will give the user a menu, which will allow them to wipe any saved system settings
    else if (watch2::states[watch2::state].variant == 1)
    {
        //settings clear mode
        if (!watch2::state_init)
        {
            watch2::oled.setCursor(0,10);
            watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
            watch2::oled.setTextColor(WHITE);
            watch2::oled.print("Do you want to clear\nall saved settings?");
        }

        if (dpad_up_active() || dpad_down_active())
        {
            selected_option = !selected_option;
        }

        if (dpad_any_active() || !watch2::state_init)
        {
            watch2::drawMenu(2, 37, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 37, {"No", "Yes"}, selected_option, RED);
        }

        if (dpad_enter_active())
        {
            if (selected_option == 1) watch2::preferences.clear();
            watch2::switchState(1);
        }
    }

    //check down button for settings clearing thing
    if (watch2::states[watch2::state].variant == 0)
    {
        if (digitalRead(dpad_down))
        {
            watch2::switchState(watch2::state, 1);
        }
        else
        {
            //switch state
            watch2::switchState(1);
        }
    }
}