#include "../globals.h"

void state_func_state_menu()
{
    static int columns = 4;
    static int icon_size = 27;
    static int icon_spacing = 4;
    static int no_icons = 0;
    static std::vector<int> menu_positions;

    int icon_xpos = icon_spacing;
    int icon_ypos = icon_spacing;
    static int last_yoffset = -1;

    if (!state_init)
    {
        no_icons = 0;
        menu_positions.clear();
    }

    if (dpad_left_active())
    {
        if (selected_menu_icon == menu_positions[0])
        {
            selected_menu_icon = states.size();
        }
        while(1)
        {
            selected_menu_icon--;
            if (!states[selected_menu_icon].hidden) break;
        }
    }

    if (dpad_right_active())
    {
        while(1)
        {
            selected_menu_icon++;
            if (selected_menu_icon == states.size())
            selected_menu_icon = 0;
            if (!states[selected_menu_icon].hidden) break;
        }
    }

    if (dpad_up_active())
    {
        //get selected icon number
        int loop_limit = columns;
        std::vector<int>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), selected_menu_icon);
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
            if (selected_menu_icon == menu_positions[0])
            {
                selected_menu_icon = states.size();
            }
            while(1)
            {
                selected_menu_icon--;
                if (!states[selected_menu_icon].hidden) break;
            }
        }
    }

    if (dpad_down_active())
    {
        //get selected icon number
        int loop_limit = columns;
        std::vector<int>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), selected_menu_icon);
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
                selected_menu_icon++;
                if (selected_menu_icon == states.size())
                selected_menu_icon = 0;
                if (!states[selected_menu_icon].hidden) break;
            }
        }
    }



    if (dpad_up_active() || dpad_down_active() || dpad_left_active() ||
        dpad_right_active() || dpad_enter_active() || !state_init)
    {
        oled.fillRect(0, 86, SCREEN_WIDTH, 10, BLACK); //clear state name text
        icon_ypos += 10;                               //add space for top bar thing

        //determine row of selected icon by means of an overengineered 2d-ish linear search
        int selected_icon_row;
        int row = 0;
        int col = 0;
        for (int i = 0; i < states.size(); i++)
        {
            stateMeta stateinfo = states[i];
            if (!stateinfo.hidden)
            {
                //if current icon is selected
                if (selected_menu_icon == i)
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
            oled.fillScreen(BLACK);
            last_yoffset = icon_yoffset;
        }

        //draw state icons
        for (int i = 0; i < states.size(); i++)
        {
            stateMeta stateinfo = states[i];
            if (!stateinfo.hidden)
            {
                //draw app icon
                oled.drawRGBBitmap(icon_xpos, icon_ypos - icon_yoffset, icons[stateinfo.stateIcon].data(),
                                    icon_size, icon_size);

                //if current app is selected, draw an outline around it
                if (selected_menu_icon == i)
                {
                    oled.drawRoundRect(icon_xpos-1, icon_ypos-1 - icon_yoffset, icon_size+1, icon_size+1, 3, themecolour);
                }
                //otherwise, clear any outline around it
                else oled.drawRoundRect(icon_xpos-1, icon_ypos-1 - icon_yoffset, icon_size+1, icon_size+1, 3, BLACK);

                icon_xpos += icon_size + icon_spacing;
                if ((icon_xpos+icon_size) > SCREEN_WIDTH)
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_size + icon_spacing;
                }

                if (!state_init)
                {
                    menu_positions.push_back(i);
                }

                if (!state_init) no_icons++;
            }
        }

        //draw name of selected icon
        oled.setCursor(2, 94);
        oled.setTextColor(WHITE);
        oled.fillRect(0, 94 - 10, SCREEN_WIDTH, SCREEN_HEIGHT - (94 - 10), BLACK);
        oled.print(states[selected_menu_icon].stateName.c_str());
    }

    drawTopThing();

    if (dpad_enter_active())
    {
        switchState(selected_menu_icon);
    }
}