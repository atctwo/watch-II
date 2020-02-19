#include "../src/watch2.h"
#include "../src/csscolorparser/csscolorparser.hpp"
#include <IRremote.h>
#include <ArduinoJson.h>

struct cmdData {

    unsigned long data;
    int nbits;
    String protocol;

};

void populateProfileDataStructures(
    const char* filename, 
    int rows,
    int columns,
    int &pages,
    std::vector<String> &calc_buttons,
    std::vector<cmdData> &commands,
    std::unordered_map<int, unsigned char*> &icons,
    std::unordered_map<int, const char*> &colours
) {

    stbi_io_callbacks callbacks = {
        watch2::img_read,
        watch2::img_skip,
        watch2::img_eof
    };

    static DynamicJsonDocument doc(10000);
    static ImageReturnCode stat;
    static int page, col, row, index;

    //disable tft cs
    digitalWrite(cs, HIGH);

    //open and deserialize json file
    Serial.printf("Loading ir profile %s...\n", filename);
    File json = watch2::SD.open(filename);
    DeserializationError err = deserializeJson(doc, json);
    if (err)
    {
        Serial.print("\tdeserialisation unsuccessful, reason: ");
        Serial.print(err.c_str());
        Serial.println("\n\treturning to state menu...");
        watch2::switchState(2);
    }
    else
    {

        Serial.println("\tdeserialised successfully");

        //clear data structures
        Serial.println("\tconfiguring data structures");
        pages = doc["pages"];               //set number of pages
        calc_buttons.clear();
        commands.clear();
        //todo: free memory
        icons.clear();
        colours.clear();

        //resize data structutes
        calc_buttons.resize(rows * columns * pages, "");
        commands.resize(rows * columns * pages, (cmdData){0, 0, "none"});
        calc_buttons[0] = "dev";            //add devices button

        //add ir codes
        Serial.println("\tadding codes");
        JsonArray daisy = doc["codes"];     //get array of codes
        for (JsonObject code : daisy)
        {

            //get button page, row, and column
            Serial.print("\t(*) loading button location information, ");
            page = code["page"];
            row = code["row"];
            col = code["col"];
            index = (row * columns * pages) + (page * columns) + col;
            unsigned char *data = NULL;
            
            //if code has an icon, attempt to store it in memory
            Serial.print("loading button data, ");
            const char* icon = code["btn_icon"].as<const char*>();  //icon filename
            if (icon)
            {
                Serial.printf("icon: %s ", icon);

                //use stb_image to read the image data
                int img_w, img_h, img_n;
                File f = watch2::SD.open(icon);                                                 // open the image
                data = stbi_load_from_callbacks(&callbacks, &f, &img_w, &img_h, &img_n, 3);     // parse the image data
                f.close();                                                                      // close the image
                if (data != NULL) icons[index] = data;
                else
                {
                    Serial.print("icon load failed: ");
                    Serial.print(stbi_failure_reason());
                    Serial.print(", "); 
                }
            }
            //if no icon was specified, or the icon wasn't loaded
            const char *btn_text = code["btn_text"].as<char*>();
            if ((!icon || data == NULL) && btn_text)
            {
                Serial.printf("text: %s ", btn_text);

                //set button text
                calc_buttons[index] = String(btn_text);
            }
            else
            {
                //set button text
                calc_buttons[index] = String("idk");
            }

            //load colour
            const char* colour = code["colour"].as<const char*>();
            if (colour)
            {
                Serial.printf("colour: %s ", colour);
                colours[index] = colour;
            }

            //get ir code
            Serial.print("loading code information, ");
            unsigned long thingy = strtoul(code["code"].as<String>().c_str(), NULL, 16);

            Serial.printf("code: %ul ", thingy);

            //store code data in memory
            commands[index] = (cmdData){
                thingy,
                code["size"],
                code["protocol"].as<String>()
            };

            Serial.printf(" ram:[%2.0f%%]", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);

            Serial.println();

        }

        Serial.println("done!");
    }
    json.close();

}

std::map<std::string, std::string> getProfileNames() {

    static DynamicJsonDocument doc(10000);
    static ImageReturnCode stat;
    static int page, col, row, index;
    std::map<std::string, std::string> profile_data;

    //disable tft cs
    digitalWrite(cs, HIGH);

    //open and deserialize json file
    Serial.printf("Loading ir profiles\n");

    std::vector<std::string> profiles = watch2::getDirFiles("/ir/");
    for (std::string profile : profiles)
    {

        std::string filename = "/ir/" + profile;
        Serial.print("loading profile ");
        Serial.print(filename.c_str());
        Serial.print(", ");

        File json = watch2::SD.open(filename.c_str());
        json.setTimeout(5000);

        int chr = 0;
        while(chr != -1)
        {
            chr = json.read();
            Serial.print((char)chr);
        }
        json.rewind();

        DeserializationError err = deserializeJson(doc, json);

        Serial.printf("ram:[%2.0f%%]\n", ((float)ESP.getFreeHeap() / ESP.getHeapSize()) * 100);

        if (err)
        {
            Serial.print("failed, reason: ");
            Serial.println(err.c_str());
        }
        else
        {
            Serial.println("success");
            profile_data[std::string(doc["name"].as<char*>())] = profile;
        }

        json.close();

    }

    return profile_data;

}

void state_func_ir_remote()
{
    // ✓ have a pages key in json, set pages variable to this
    // ✓ init buttons vector to (4*5)*pages elements, w/ default value of ""
    // ✓ init command vector to (4*5)*pages elements, w/ default value of null
    // ✓ have page, row and col keys in json
    //   for each button, set buttons[(row * columns * pages) + (page * columns) + col] to the button text
    //   and set commands[(row * columns * pages) + (page * columns) + col] to the command

    static IRsend irsend;
    static IRrecv irrecv(IR_REC_PIN);
    static decode_results results;
    static int columns = 5;
    static int rows = 5;
    static int pages = 2;//doc["pages"].as<int>();
    static std::vector<String> calc_buttons;
    static std::vector<cmdData> commands;
    static std::unordered_map<int, unsigned char*> icons;
    static std::unordered_map<int, const char*> colours;
    static std::map<std::string, std::string> profiles;
    static std::vector<std::string> profile_names;
    static std::string profile_filename;
    static int selected_profile;
    static int icon_spacing = 6;
    static int radius = 10;
    static int no_icons = 0;
    static int input_colour = WHITE;
    static int selected_calc_button = 0;
    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static int32_t width = 0, height = 0;
    static int last_page_number = -1;

    static int icon_width = ( SCREEN_WIDTH - ( ( columns + 1 ) * icon_spacing ) ) / columns;
    static int icon_height = (2 * icon_spacing) + watch2::oled.fontHeight();

    int icon_xpos = icon_spacing;
    int icon_ypos = icon_spacing;
    int total_icons_per_row = columns * pages;

    if (watch2::states[watch2::state].variant == 0) // profile selection screen
    {
        if (!watch2::state_init)
        {
            //get profile names and paths
            profiles.clear();
            profiles = getProfileNames();

            //create array of profile names
            profile_names.clear();
            profile_names.push_back("Cancel");
            profile_names.push_back("Receiver");
            for (std::pair<std::string, std::string> bob : profiles)
            {
                profile_names.push_back(bob.first);
            }
        }

        //watch2::drawTopThing();

        if (dpad_up_active())
        {
            selected_profile--;
            if (selected_profile < 0) selected_profile = 2; //todo: get number of profiles
        }

        if (dpad_down_active())
        {
            selected_profile++;
            if (selected_profile > 2) selected_profile = 0;
        }

        if (dpad_any_active() || !watch2::state_init)
        {
            watch2::drawMenu(2, watch2::top_thing_height, SCREEN_WIDTH - 4, SCREEN_HEIGHT, profile_names, selected_profile, watch2::themecolour);
        }

        if (dpad_enter_active())
        {
            if (selected_profile == 0) //if cancel is selected
            {
                //clear button and code data structures
                calc_buttons.clear();
                commands.clear();
                //todo: free memory
                icons.clear();
                colours.clear();
                profiles.clear();

                //return to state menu
                watch2::switchState(2);
            }
            else if (selected_profile == 1) //if receiver is selected
            {
                //switch to receiver thing
                watch2::switchState(watch2::state, 2);
            }
            else
            {
                //set profile filename
                profile_filename = "/ir/" + profiles[profile_names[selected_profile]];

                //switch to remote grid
                watch2::switchState(watch2::state, 1);
            }
            
        }
    }
    else if (watch2::states[watch2::state].variant == 1) // ir remote
    {

        if (!watch2::state_init)
        {
            //populate the profile data structures with data from the selected json file
            populateProfileDataStructures(profile_filename.c_str(), rows, columns, pages, calc_buttons, commands, icons, colours);
            Serial.println(ESP.getFreeHeap());
        }

        if (dpad_left_active())
        {
            input_colour = WHITE;
            while(1)
            {
                if (selected_calc_button == 0)
                selected_calc_button = calc_buttons.size()-1;
                else selected_calc_button--;
                if (calc_buttons[selected_calc_button] != "" || icons.find(selected_calc_button) != icons.end()) break;
            }
        }

        if (dpad_right_active())
        {
            input_colour = WHITE;
            while(1)
            {
                selected_calc_button++;
                if (selected_calc_button == calc_buttons.size())
                selected_calc_button = 0;
                if (calc_buttons[selected_calc_button] != "" || icons.find(selected_calc_button) != icons.end()) break;
            }
        }

        if (dpad_up_active())
        {
            input_colour = WHITE;

            //get selected icon number
            int loop_limit = total_icons_per_row;
            int last = no_icons % total_icons_per_row;
            if (selected_calc_button < 4)
            {
                if (selected_calc_button < last) loop_limit = last;
                else loop_limit = total_icons_per_row + last;
            }

            // loop to calculated button
            for (int i=0; i < loop_limit; i++)
            {
                if (selected_calc_button == 0)
                {
                    selected_calc_button = calc_buttons.size();
                }
                selected_calc_button--;
            }

            // if button has no text or an icon, find next button with text or an icon
            if (calc_buttons[selected_calc_button] == "" && icons.find(selected_calc_button) == icons.end()) while(1)
            {
                if (selected_calc_button == 0)
                selected_calc_button = calc_buttons.size();
                else selected_calc_button--;
                if (calc_buttons[selected_calc_button] != "" || icons.find(selected_calc_button) != icons.end()) break;
            }
        }

        if (dpad_down_active())
        {
            input_colour = WHITE;

            //get selected icon number
            int loop_limit = total_icons_per_row;   
            int last = no_icons % total_icons_per_row;
            if (selected_calc_button >= (no_icons - total_icons_per_row))
            {
                if (selected_calc_button >= (no_icons - last)) loop_limit = last;
                else loop_limit = total_icons_per_row + last;
            }

            // loop to reach calculated button
            for (int i=0; i < loop_limit; i++)
            {
                selected_calc_button++;
                if (selected_calc_button == calc_buttons.size())
                selected_calc_button = 0;
            }
            //if button has no text or icon, find next button with text or an icon
            if (calc_buttons[selected_calc_button] == "" && icons.find(selected_calc_button) == icons.end()) while(1)
            {
                selected_calc_button++;
                if (selected_calc_button == calc_buttons.size())
                selected_calc_button = 0;
                if (calc_buttons[selected_calc_button] != "" || icons.find(selected_calc_button) != icons.end()) break;
            }
        }

        if (dpad_up_active() || dpad_down_active() || dpad_left_active() ||
            dpad_right_active() || dpad_enter_active() || !watch2::state_init)
        {
            //add space for top bar thing (watch2::top_thing_height) and input display (8 + (3 * icon_spacing))
            icon_ypos += watch2::top_thing_height;// + 8 + (3 * icon_spacing);

            //calculate page of selected item
            int selected_item_column = selected_calc_button % total_icons_per_row;
            int selected_page_number = floor( selected_item_column / columns );

            //clear matrix on page change
            if (selected_page_number != last_page_number)
            {
                watch2::oled.fillRect(0, icon_ypos, SCREEN_WIDTH, SCREEN_HEIGHT - icon_ypos, BLACK);
                last_page_number = selected_page_number;
            }

            Serial.println("h");

            //draw state icons
            for (int i = 0; i < calc_buttons.size(); i++)
            {
                //calculate page of current item
                int item_column = i % total_icons_per_row;
                int page_number = floor( item_column / columns );

                //if (calc_buttons[i] == "4") calculator_expression_thing += String(i) + String(" % ") + String(total_icons_per_row);

                if (page_number == selected_page_number) //if item is on same page as selected item
                {
                    // if item has an icon
                    if (icons.find(i) != icons.end())
                    {
                        //draw the icon
                        //todo: this
                        //icons[i].draw(watch2::oled, icon_xpos, icon_ypos);

                        //print outline
                        if (selected_calc_button == i)
                        {
                            watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, watch2::themecolour);
                        }
                        else watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, 0x4A49);
                    }
                    // if item has text
                    if (calc_buttons[i] != "")
                    {
                        //if a colour has been set for the text
                        if (colours.find(i) != colours.end())
                        {
                            const auto colour = CSSColorParser::parse(std::string(colours[i]));
                            if (colour)
                            {
                                watch2::oled.setTextColor(watch2::oled.color565(colour->r, colour->g, colour->b), BLACK);
                            }
                            else watch2::oled.setTextColor(WHITE, BLACK);
                            Serial.println();
                        }
                        else
                        {
                            watch2::oled.setTextColor(WHITE, BLACK);
                        }
                        

                        //print button text
                        watch2::oled.setCursor(icon_xpos + (int)(icon_spacing / 2), icon_ypos + (int)(icon_spacing / 2));
                        watch2::oled.print(calc_buttons[i]);

                        //print outline
                        if (selected_calc_button == i)
                        {
                            watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, watch2::themecolour);
                        }
                        else watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, 0x4A49);
                    }

                    //calculate position of next item
                    icon_xpos += icon_width + icon_spacing;
                    if ((item_column + 1) >= columns * (page_number + 1))
                    {
                        icon_xpos = icon_spacing;
                        icon_ypos += icon_height + icon_spacing;
                    }

                    //calculator_expression_thing += String(calc_buttons[i]) + String(" ");
                }

                if (!watch2::state_init) no_icons++;
            }

            Serial.println("i");
        }

        watch2::drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_calc_button == 0)
            {
                watch2::switchState(watch2::state, 0);
            }
            else
            {
                irsend.sendLG(
                    commands[selected_calc_button].data,
                    commands[selected_calc_button].nbits
                );
            }
        }

    }
    else if (watch2::states[watch2::state].variant == 2) //ir receiver
    {
        if (!watch2::state_init)
        {
            irrecv.enableIRIn();
        }

        if (irrecv.decode(&results))
        {
            Serial.print("decoded ir thing: ");
            Serial.print(results.decode_type);
            Serial.print(results.value, HEX);
            irrecv.resume();
        }   

        /*

        hey idiot

        the interrpits caused bu tje ir receivcer might be happeing when spi flash is being read or written
        so maybe don't draw 1st frame whem switching state, or delay before switching state
        or find esp32-specifici ir lib

        */

        //watch2::drawTopThing();

        if (dpad_left_active())
        {
            watch2::switchState(watch2::state, 0);
        }
    }
}