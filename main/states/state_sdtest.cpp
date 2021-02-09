#include "states.h"

void state_func_SDtest()
{
    EXT_RAM_ATTR static std::string filename;

    static uint8_t action_padding = 4;
    static uint16_t action_w = (SCREEN_WIDTH / 2) - (action_padding * 3);
    static uint16_t action_h = (action_padding * 2) + watch2::oled.fontHeight();

    static uint8_t action_radius = action_h / 2;
    static bool flag_thing = true;
    static int selected_button = 0;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();

        // if file select was cancelled, return to the state menu
        if (filename.compare("canceled") == 0) watch2::switchState(2);
        else {

            flag_thing = true;

            // print file name
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::setFont(SLIGHTLY_BIGGER_FONT);
            watch2::oled.println(watch2::file_name(filename.c_str()).c_str());
            watch2::setFont(MAIN_FONT);

            // open file
            fs::File f = SD.open(filename.c_str());

            // print file size
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("Size: ");
            watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            watch2::oled.println(watch2::humanSize(f.size()));

            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("Cre:");
            watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            //f.printCreateDateTime(&watch2::oled);
            watch2::oled.println("");

            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("Mod:");
            watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            //f.printModifyDateTime(&watch2::oled);
            watch2::oled.println("");

            // print file hidden flag
            // watch2::oled.setTextColor(WHITE, BLACK);
            // watch2::oled.print("Hidden: ");
            // watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            // watch2::oled.println(f.isHidden() ? "yes" : "no");

            // // print file read only flag
            // watch2::oled.setTextColor(WHITE, BLACK);
            // watch2::oled.print("Read Only: ");
            // watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            // watch2::oled.println(f.isReadOnly() ? "yes" : "no");

            // // print file hidden flag
            // watch2::oled.setTextColor(WHITE, BLACK);
            // watch2::oled.print("System: ");
            // watch2::oled.setTextColor(watch2:: themecolour, BLACK);
            // watch2::oled.println(f.isSystem() ? "yes" : "no");

            // close file
            f.close();

        }
    }

    if (dpad_left_active())     selected_button -= 1;
    if (dpad_right_active())    selected_button += 1;
    if (dpad_up_active())       selected_button -= 2;
    if (dpad_down_active())     selected_button += 2;
    if (dpad_any_active()) {
        if (selected_button > 3) selected_button -= 4;
        if (selected_button < 0) selected_button += 4;
    }

    draw(dpad_any_active() || flag_thing, {

        // draw rename button (2)
        watch2::oled.fillRoundRect(
            action_padding, SCREEN_HEIGHT - (action_padding * 3) - watch2::oled.fontHeight(),
            action_w, action_h, action_radius, (selected_button == 2 ? watch2::themecolour : BLACK)
        );
        watch2::oled.drawRoundRect(
            action_padding, SCREEN_HEIGHT - (action_padding * 3) - watch2::oled.fontHeight(),
            action_w, action_h, action_radius, watch2::themecolour
        );
        watch2::oled.setTextColor(selected_button == 2 ? BLACK : watch2::themecolour, selected_button == 2 ? watch2::themecolour : BLACK);
        watch2::oled.drawString("Rename", action_padding * 3, SCREEN_HEIGHT - (action_padding * 2) - watch2::oled.fontHeight());

        // draw copy button (0)
        watch2::oled.fillRoundRect(
            action_padding, SCREEN_HEIGHT - (action_padding * 6) - (watch2::oled.fontHeight() * 2),
            action_w, action_h, action_radius, selected_button == 0 ? watch2::themecolour : BLACK
        );
        watch2::oled.drawRoundRect(
            action_padding, SCREEN_HEIGHT - (action_padding * 6) - (watch2::oled.fontHeight() * 2),
            action_w, action_h, action_radius, watch2::themecolour
        );
        watch2::oled.setTextColor(selected_button == 0 ? BLACK : watch2::themecolour, selected_button == 0 ? watch2::themecolour : BLACK);
        watch2::oled.drawString("Exit", action_padding * 3, SCREEN_HEIGHT - (action_padding * 5) - (watch2::oled.fontHeight() * 2));

        // draw delete button (3)
        watch2::oled.fillRoundRect(
            (SCREEN_WIDTH / 2) + action_padding, SCREEN_HEIGHT - (action_padding * 3) - watch2::oled.fontHeight(),
            action_w, action_h, action_radius, selected_button == 3 ? watch2::themecolour : BLACK
        );
        watch2::oled.drawRoundRect(
            (SCREEN_WIDTH / 2) + action_padding, SCREEN_HEIGHT - (action_padding * 3) - watch2::oled.fontHeight(),
            action_w, action_h, action_radius, watch2::themecolour
        );
        watch2::oled.setTextColor(selected_button == 3 ? BLACK : watch2::themecolour, selected_button == 3 ? watch2::themecolour : BLACK);
        watch2::oled.drawString("Delete", (SCREEN_WIDTH / 2) + action_padding + (action_padding * 2), SCREEN_HEIGHT - (action_padding * 2) - watch2::oled.fontHeight());

        // draw move button (1)
        watch2::oled.fillRoundRect(
            (SCREEN_WIDTH / 2) + action_padding, SCREEN_HEIGHT - (action_padding * 6) - (watch2::oled.fontHeight() * 2),
            action_w, action_h, action_radius, selected_button == 1 ? watch2::themecolour : BLACK
        );
        watch2::oled.drawRoundRect(
            (SCREEN_WIDTH / 2) + action_padding, SCREEN_HEIGHT - (action_padding * 6) - (watch2::oled.fontHeight() * 2),
            action_w, action_h, action_radius, watch2::themecolour
        );
        watch2::oled.setTextColor(selected_button == 1 ? BLACK : watch2::themecolour, selected_button == 1 ? watch2::themecolour : BLACK);
        watch2::oled.drawString("", (SCREEN_WIDTH / 2) + action_padding + (action_padding * 2), SCREEN_HEIGHT - (action_padding * 5) - (watch2::oled.fontHeight() * 2));

        flag_thing = false;

    });

    watch2::drawTopThing();

    if (dpad_enter_active()) 
    {
        if (selected_button == 0) // copy
        {
            watch2::switchState(watch2::state);
        }
        else if (selected_button == 1) // move
        {
            
        }
        else if (selected_button == 2) // rename
        {
            std::string new_name = watch2::textFieldDialogue("New Name", filename.c_str());
            if (new_name.compare("") != 0) SD.rename(filename.c_str(), new_name.c_str());
        }
        else if (selected_button == 3) // delete
        {
            bool confirmation = watch2::messageBox("Are you sure?", {"No", "Yes"});
            if (confirmation) {
                SD.remove(filename.c_str());
                watch2::switchState(watch2::state);
            }
        }

        //watch2::switchState(watch2::state);
    }
    
}