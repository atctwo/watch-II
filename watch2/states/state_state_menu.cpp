#include "../src/watch2.h"

void state_func_state_menu()
{
    static int columns = 4;
    static int icon_size = 47;
    static int icon_spacing = 10;
    static int no_icons = 0;
    static std::vector<int> menu_positions;

    int icon_xpos = icon_spacing;
    int icon_ypos = 0;//icon_spacing;
    static int last_yoffset = -1;

    if (!watch2::state_init)
    {
        no_icons = 0;
        menu_positions.clear();
    }

    if (dpad_left_active())
    {
        if (watch2::selected_menu_icon == menu_positions[0])
        {
            watch2::selected_menu_icon = watch2::states.size();
        }
        while(1)
        {
            watch2::selected_menu_icon--;
            if (!watch2::states[watch2::selected_menu_icon].hidden) break;
        }
    }

    if (dpad_right_active())
    {
        while(1)
        {
            watch2::selected_menu_icon++;
            if (watch2::selected_menu_icon == watch2::states.size())
            watch2::selected_menu_icon = 0;
            if (!watch2::states[watch2::selected_menu_icon].hidden) break;
        }
    }

    if (dpad_up_active())
    {
        //get selected icon number
        int loop_limit = columns;
        std::vector<int>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), watch2::selected_menu_icon);
        if (selected_pos != menu_positions.end())
        {
            int index = std::distance(menu_positions.begin(), selected_pos) ;
            int last = no_icons % columns;
            if (index < columns)
            {
                if (index < last) loop_limit = last;
                else loop_limit = last + columns;
            }
            //oled.printf("%d", loop_limit);
        }

        for (int i=0; i < loop_limit; i++)
        {
            if (watch2::selected_menu_icon == menu_positions[0])
            {
                watch2::selected_menu_icon = watch2::states.size();
            }
            while(1)
            {
                watch2::selected_menu_icon--;
                if (!watch2::states[watch2::selected_menu_icon].hidden) break;
            }
        }
    }

    if (dpad_down_active())
    {
        //get selected icon number
        int loop_limit = columns;
        std::vector<int>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), watch2::selected_menu_icon);
        if (selected_pos != menu_positions.end())
        {
            int index = std::distance(menu_positions.begin(), selected_pos) ;
            int last = no_icons % columns;
            if (index + 1 > ( no_icons - 4 ))
            {
                if (index + 1 > ( no_icons - last) ) loop_limit = last;
                else loop_limit = last + columns;
            }
            //oled.printf("%d", loop_limit);
        }

        for (int i=0; i < loop_limit; i++)
        {
            while(1)
            {
                watch2::selected_menu_icon++;
                if (watch2::selected_menu_icon == watch2::states.size())
                watch2::selected_menu_icon = 0;
                if (!watch2::states[watch2::selected_menu_icon].hidden) break;
            }
        }
    }



    if (dpad_any_active() || !watch2::state_init || watch2::forceRedraw)
    {
        watch2::oled.fillRect(0, 86, SCREEN_WIDTH, 10, BLACK); //clear state name text
        icon_ypos += watch2::top_thing_height;                 //add space for top bar thing

        //determine row of selected icon by means of an overengineered 2d-ish linear search
        int selected_icon_row;
        int row = 0;
        int col = 0;
        for (int i = 0; i < watch2::states.size(); i++)
        {
            watch2::stateMeta stateinfo = watch2::states[i];
            if (!stateinfo.hidden)
            {
                //if current icon is selected
                if (watch2::selected_menu_icon == i)
                {
                    selected_icon_row = row;
                    break;
                }
                else
                {
                    col++;
                    if (col > 3)
                    {
                        col = 0;
                        row++;
                    }
                }
            }
        }
        
        int selected_icon_ypos = icon_ypos + (selected_icon_row * (icon_size + icon_spacing));
        int yoffset_threshold = SCREEN_HEIGHT - (icon_size + icon_spacing);
        int icon_yoffset = (selected_icon_ypos > yoffset_threshold) ? (icon_size + icon_spacing) * (selected_icon_row - 1) : 0;

        //if yoffset has changed, redraw screen
        if (icon_yoffset != last_yoffset)
        {
            watch2::oled.fillScreen(BLACK);
            last_yoffset = icon_yoffset;
        }

        //draw state icons
        for (int i = 0; i < watch2::states.size(); i++)
        {
            watch2::stateMeta stateinfo = watch2::states[i];
            if (!stateinfo.hidden)
            {
                //draw app icon
                watch2::oled.setSwapBytes(true);
                watch2::oled.pushImage(icon_xpos, icon_ypos - icon_yoffset, icon_size, icon_size, watch2::icons[stateinfo.stateIcon].data());
                /*std::string icon_path = "/" + stateinfo.stateIcon;
                if (SPIFFS.exists(icon_path.c_str()))
                {
                    watch2::drawBmp(icon_path.c_str(), icon_xpos, icon_ypos - icon_yoffset);
                }
                else
                {
                    Serial.print("[error] ");
                    Serial.print(icon_path.c_str());
                    Serial.println(" does not exist");
                }*/

                //if current app is selected, draw an outline around it
                if (watch2::selected_menu_icon == i)
                {
                    watch2::oled.drawRoundRect(icon_xpos-1, icon_ypos-1 - icon_yoffset, icon_size+1, icon_size+1, 10, watch2::themecolour);
                }
                //otherwise, clear any outline around it
                else watch2::oled.drawRoundRect(icon_xpos-1, icon_ypos-1 - icon_yoffset, icon_size+1, icon_size+1, 10, BLACK);

                icon_xpos += icon_size + icon_spacing;
                if ((icon_xpos+icon_size) > SCREEN_WIDTH)
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_size + icon_spacing;
                }

                if (!watch2::state_init)
                {
                    menu_positions.push_back(i);
                }

                if (!watch2::state_init) no_icons++;
            }
        }

        //draw name of selected icon
        watch2::oled.setCursor(2, SCREEN_HEIGHT - 4 - watch2::oled.fontHeight());
        watch2::oled.setTextColor(WHITE, BLACK);
        watch2::oled.fillRect(0, SCREEN_HEIGHT - watch2::oled.fontHeight() - 8, SCREEN_WIDTH, watch2::oled.fontHeight() + 8, BLACK);
        watch2::oled.print(watch2::states[watch2::selected_menu_icon].stateName.c_str());
        watch2::forceRedraw = false;
    }

    watch2::drawTopThing();

    if (dpad_enter_active())
    {
        watch2::switchState(watch2::selected_menu_icon);
    }
}