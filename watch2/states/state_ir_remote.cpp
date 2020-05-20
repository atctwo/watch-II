#include "../src/watch2.h"
#include "../src/csscolorparser/csscolorparser.hpp"
#include <IRremote.h>

void state_func_ir_remote()
{

    //----------------------------------------------------
    // variables
    //----------------------------------------------------

    // profile selection menu
    static std::vector<std::string> profile_names;
    static uint8_t selected_profile = 0;
    static std::string selected_profile_filename = "";

    // ir receiver
    static IRAM_ATTR IRrecv irrecv(IR_REC_PIN);
    static IRAM_ATTR decode_results ir_recv_results;
    static bool ir_enabled = false;

    // ir remote
    static IRsend irsend;
    static cJSON *profile;
    static cJSON *codes;
    static char *json_contents;
    static std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int>>> code_indices;
    static uint8_t columns = 5;
    static uint8_t rows = 5;
    static uint8_t pages = 2;
    static uint8_t icon_spacing = 6;
    static uint8_t radius = 4;
    static uint8_t no_icons = 0;
    static uint8_t selected_row = 0;
    static uint8_t selected_col = 0;
    static uint8_t last_page_number = 255;
    static uint8_t this_page_number = 0;

    static uint8_t icon_width = ( SCREEN_WIDTH - ( ( columns + 1 ) * icon_spacing ) ) / columns;
    static uint8_t icon_height = (2 * icon_spacing) + watch2::oled.fontHeight();

    static uint8_t icon_xpos = icon_spacing;
    static uint8_t icon_ypos = icon_spacing;
    static uint8_t total_icons_per_row = columns * pages;
    static uint16_t no_icons_separate_from_the_other_no_icons = rows * columns;

    //----------------------------------------------------
    // 0 - profile selection menu
    //----------------------------------------------------

    if (watch2::states[watch2::state].variant == 0)
    {
        // on state init
        if (!watch2::state_init)
        {
            // get the names of each profile in the ir directory of the sd card
            profile_names = watch2::getDirFiles("/ir/");
            // add other ir remote modes
            profile_names.insert( profile_names.begin(), "Serial" );
            profile_names.insert( profile_names.begin(), "Receiver" );
            profile_names.insert( profile_names.begin(), "Cancel" );
        }

        // if up or down is pressed, update selected profile
        if (dpad_down_active())
        {
            selected_profile++;
            if (selected_profile >= profile_names.size()) selected_profile = 0;
        }

        if (dpad_up_active())
        {
            selected_profile--;
            if (selected_profile < 0) selected_profile = profile_names.size() - 1;
        }

        // on state init, or if any key is pressed
        if (!watch2::state_init || dpad_any_active())
        {
            // draw profile menu
            watch2::drawMenu(
                2,
                watch2::top_thing_height,
                SCREEN_WIDTH - 2,
                SCREEN_HEIGHT - watch2::top_thing_height,
                profile_names,
                selected_profile
            );
        }

        // draw top thing
        watch2::drawTopThing();

        // handle pressing enter
        if (dpad_enter_active())
        {
            switch(selected_profile)
            {
                case 0: // cancel button
                    watch2::switchState(2);
                    break;

                case 1: // receiver
                    watch2::switchState(watch2::state, 2);
                    break;

                case 2: // serial
                    watch2::switchState(watch2::state, 3);
                    break;

                default: // profile
                    selected_profile_filename = "/ir/";
                    selected_profile_filename += profile_names[selected_profile];
                    watch2::switchState(watch2::state, 1);
                    break;
            }
        }

    } // 0 - profile selection menu



    //----------------------------------------------------
    // 1 - ir button menu
    //----------------------------------------------------

    if (watch2::states[watch2::state].variant == 1)
    {

        if (!watch2::state_init)
        {
            // print stuff
            Serial.print("loading ir profile: ");

            // get a handle (???) to the selected file
            File json = watch2::SD.open(selected_profile_filename.c_str());

            // allocate memory for the file's contents
            json_contents = (char*) malloc(json.fileSize());

            // load the contents of the selected file into memory
            json.read(json_contents, json.fileSize());

            // close the json file handle
            json.close();

            // parse the json file
            profile = cJSON_Parse(json_contents);

            // print the json tree
            //Serial.println(cJSON_Print(profile));

            // print the profile name
            cJSON *twist = cJSON_GetObjectItem(profile, "name");
            if (twist) Serial.println(twist->valuestring);
            else Serial.println("profile not named");

            // determine the number of pages
            cJSON *p = cJSON_GetObjectItem(profile, "pages");
            pages = p->valueint;
            total_icons_per_row = columns * pages;
            
            // get an array of codes
            codes = cJSON_GetObjectItem(profile, "codes");

            // iterate through codes and save the indices to the 3d array thing based on their position
            cJSON *code;
            uint8_t code_index = 0;
            cJSON_ArrayForEach(code, codes)
            {
                Serial.println(cJSON_Print(code));

                uint8_t page = cJSON_GetObjectItem(code, "page")->valueint;
                uint8_t row  = cJSON_GetObjectItem(code, "row")->valueint;
                uint8_t col  = cJSON_GetObjectItem(code, "col")->valueint;

                code_indices[page][row][col] = code_index;
                code_index++;
            }

            // add devices button
            cJSON *devices_btn = cJSON_CreateObject();

            cJSON *devices_btn_text = cJSON_CreateString("dev");
            cJSON_AddItemToObjectCS(devices_btn, "text", devices_btn_text);

            cJSON_AddItemToArray(codes, devices_btn);
            code_indices[0][0][0] = code_index;

            // set page number
            last_page_number = 255;

            // print icon width + height
            Serial.printf("icon width:  %d\nicon height: %d\n", icon_width, icon_height);
        }

        watch2::drawTopThing();

        if (dpad_left_active())
        {
            selected_col--;
            if (selected_col == 255)
            {
                selected_col = columns - 1;
                this_page_number--;
                if (this_page_number == 255) this_page_number = pages - 1;
            }
        }

        if (dpad_right_active())
        {
            selected_col++;
            if (selected_col >= columns)
            {
                selected_col = 0;
                this_page_number++;
                if (this_page_number >= pages) this_page_number = 0;
            }
        }

        if (dpad_up_active())
        {
            selected_row--;
            if (selected_row == 255)
            {
                selected_row = rows - 1;
            }
        }

        if (dpad_down_active())
        {
            selected_row++;
            if (selected_row >= rows)
            {
                selected_row = 0;
            }
        }

        // draw button contents
        if (this_page_number != last_page_number)
        {
            if (watch2::state_init) watch2::dimScreen(false, 100);
            icon_ypos = watch2::top_thing_height;

            // clear page
            int matrix_width = (columns * icon_spacing) + (columns * icon_width);
            int matrix_height = (rows * icon_spacing) + (rows * icon_height);
            watch2::oled.fillRect(icon_spacing, icon_ypos, matrix_width, matrix_height, BLACK);
            last_page_number = this_page_number;
        
            //draw state icons
            for (int i = no_icons_separate_from_the_other_no_icons * this_page_number; i < no_icons_separate_from_the_other_no_icons * (this_page_number + 1); i++)
            {
                Serial.printf("button %d\n", i);

                // get grid position
                uint8_t relative_index = i - (this_page_number * rows * columns);
                uint8_t row = div(relative_index, columns).quot;
                uint8_t col = relative_index % columns;

                Serial.printf("\tpage: %d\n", this_page_number);
                Serial.printf("\trow:  %d\n", row);
                Serial.printf("\tcol:  %d\n", col);
                Serial.printf("\tcode index: %d\n", code_indices[this_page_number][row][col]);

                //get button data
                cJSON *btn = cJSON_GetArrayItem(codes, code_indices[this_page_number][row][col]);
                if (btn)
                {

                    Serial.println("\tfound button entry in json file");

                    // check if the button has an icon
                    cJSON *btn_icon_object = cJSON_GetObjectItem(btn, "icon");
                    if (btn_icon_object)
                    {
                        // get image data
                        Serial.println(btn_icon_object->valuestring);
                        watch2::imageData data = watch2::getImageData(btn_icon_object->valuestring);

                        // if image loaded successfully
                        if (data.data != NULL)
                        {
                            watch2::drawImage(data, icon_xpos, icon_ypos);
                        }
                        else 
                        {
                            Serial.print("\t");
                            Serial.println(data.error);
                        }
                    }
                    // no icon was found, so use text
                    else
                    {

                        // print the button text
                        cJSON *btn_text_object = cJSON_GetObjectItem(btn, "text");
                        if (btn_text_object)
                        {
                            // determine button colour
                            cJSON *btn_colour_object = cJSON_GetObjectItem(btn, "colour");
                            uint16_t btn_colour = WHITE;
                            if (btn_colour_object)
                            {
                                CSSColorParser::optional<CSSColorParser::Color> btn_colour_css = CSSColorParser::parse(btn_colour_object->valuestring);
                                if (btn_colour_css) btn_colour = watch2::oled.color565(btn_colour_css->r, btn_colour_css->g, btn_colour_css->b);
                                Serial.printf("\tcolour: %s (0x%x)\n", btn_colour_object->valuestring, btn_colour);
                            }
                            //free(btn_colour_object);

                            // print button text
                            watch2::oled.setTextDatum(MC_DATUM);
                            watch2::oled.setTextColor(btn_colour, BLACK);
                            watch2::oled.drawString(btn_text_object->valuestring, icon_xpos + (icon_width / 2), icon_ypos + (icon_height / 2));
                            Serial.printf("\tbutton text: %s\n", btn_text_object->valuestring);
                        }
                        //free(btn_text_object);

                    }
                }
                else Serial.println("\tdidn't find button in json file");
                //free(btn);
                

                //calculate position of next item
                icon_xpos += icon_width + icon_spacing;
                if ((col + 1) >= columns)
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_height + icon_spacing;
                }
                

                if (!watch2::state_init) no_icons++;

            }
            watch2::dimScreen(true, 100);
        }

        // draw button outlines
        if (dpad_any_active() || !watch2::state_init)
        {
            icon_ypos = watch2::top_thing_height;
            for (int i = no_icons_separate_from_the_other_no_icons * this_page_number; i < no_icons_separate_from_the_other_no_icons * (this_page_number + 1); i++)
            {

                // get grid position
                uint8_t relative_index = i - (this_page_number * rows * columns);
                uint8_t row = div(relative_index, columns).quot;
                uint8_t col = relative_index % columns;

                //print outline
                if (selected_row == row && selected_col == col)
                {
                    watch2::oled.drawRoundRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, radius, watch2::themecolour);
                }
                else watch2::oled.drawRoundRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, radius, 0x4A49);

                //calculate position of next item
                icon_xpos += icon_width + icon_spacing;
                if ((col + 1) >= columns)
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_height + icon_spacing;
                }

            }

        }

        if (dpad_enter_active())
        {

            if (selected_row == 0 && selected_col == 0 && this_page_number == 0) // if device button is selected
            {

                // free the memory used the the cJSON object
                cJSON_Delete(profile);

                // free the memory that the json file in memory file uses i shouldn't write code when watching tik toks
                free(json_contents);

                // free the memory used by the code indices
                code_indices.clear();

                // return to profile selection menu
                watch2::switchState(watch2::state, 0);

            }
            else
            {

                Serial.println("sending ir code: ");

                // get code object
                cJSON *code = cJSON_GetArrayItem(codes, code_indices[this_page_number][selected_row][selected_col]);
                if (code)
                {

                    // get ir protocol
                    cJSON *protocol_object = cJSON_GetObjectItem(code, "protocol");
                    cJSON *ir_code_object = cJSON_GetObjectItem(code, "code");
                    cJSON *ir_code_size = cJSON_GetObjectItem(code, "size");

                    if (protocol_object && ir_code_object && ir_code_size)
                    {
                        // get protocol string
                        char *protocol = protocol_object->valuestring;
                        int protocol_length = sizeof(protocol) / sizeof(char);

                        // get protocol as lowercase
                        for (int i = 0; i < protocol_length; i++) protocol[i] = tolower(protocol[i]);

                        // get code as a value
                        unsigned long ir_code = strtoul(ir_code_object->valuestring, NULL, 0);

                        Serial.printf("\tprotocol: %s\n", protocol);
                        Serial.printf("\tcode:     %x\n", ir_code);
                        Serial.printf("\tstr code: %s\n", ir_code_object->valuestring);
                        Serial.printf("\tsize:     %d\n", ir_code_size->valueint);

                        //send ir code
                        if (strcmp(protocol, "rc5") == 0)                irsend.sendRC5(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "rc6") == 0)                irsend.sendRC6(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "nec") == 0)                irsend.sendNEC(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "sony") == 0)               irsend.sendSony(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "panasonic") == 0)          irsend.sendPanasonic(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "jvc") == 0)                irsend.sendJVC(ir_code, ir_code_size->valueint, false);
                        if (strcmp(protocol, "samsung") == 0)            irsend.sendSAMSUNG(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "whynter") == 0)            irsend.sendWhynter(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "aiwa rc t501") == 0)       irsend.sendAiwaRCT501(ir_code);
                        if (strcmp(protocol, "lg") == 0)                 irsend.sendLG(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "sanyo") == 0)              irsend.sendSanyo(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "mitsubishi") == 0)         irsend.sendMitsubishi(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "dish") == 0)               irsend.sendDISH(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "sharp") == 0)              irsend.sendSharp(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "denon") == 0)              irsend.sendDenon(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "pronto") == 0)             irsend.sendPronto(ir_code_object->valuestring, false, false);
                        if (strcmp(protocol, "lego pf") == 0)            irsend.sendLegoPowerFunctions(ir_code, false);
                    }
                    else
                    {
                        if (!protocol_object) Serial.println("invalid protocol");
                        if (!ir_code_object) Serial.println("invalid ir code");
                        if (!ir_code_size) Serial.println("invalid ir code size");
                    }
                    
                }
            }
        }

    } // 1 - ir button menu




    //----------------------------------------------------
    // 2 - receiver
    //----------------------------------------------------

    if (watch2::states[watch2::state].variant == 2)
    {

        if (!watch2::state_init)
        {
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.print("IR Receiver\ncurrently very broken\nuse with caution\npress enter to enable ir\nreception.  results will be \nprinted over serial.  if you \nexit this screen after \nenabling reception, the \nwatch will crash\nsorry!");  
            watch2::setFont(LARGE_FONT);        
        }

        if (dpad_enter_active())
        {
            // start the receiver
            irrecv.enableIRIn();
            ir_enabled = true;  
        }

        if (ir_enabled)
        if (irrecv.decode(&ir_recv_results))
        {
            //watch2::oled.setCursor(2, watch2::top_thing_height + watch2::oled.fontHeight());
            //watch2::oled.println(ir_recv_results.value, HEX);

            // print code info
            Serial.print("Protocol: ");
            switch(ir_recv_results.decode_type)
            {
                case -1: Serial.println("Unknown");              break;
                case 0:  Serial.println("Unused");               break;
                case 1:  Serial.println("RC5");                  break;
                case 2:  Serial.println("RC6");                  break;
                case 3:  Serial.println("NEC");                  break;
                case 4:  Serial.println("Sony");                 break;
                case 5:  Serial.println("Panasonic");            break;
                case 6:  Serial.println("JVC");                  break;
                case 7:  Serial.println("Samsung");              break;
                case 8:  Serial.println("Whynter");              break;
                case 9:  Serial.println("Aiwa RC T501");         break;
                case 10: Serial.println("LG");                   break;
                case 11: Serial.println("Sanyo");                break;
                case 12: Serial.println("Mitsubishi");           break;
                case 13: Serial.println("Dish");                 break;
                case 14: Serial.println("Sharp");                break;
                case 15: Serial.println("Denon");                break;
                case 16: Serial.println("Pronto");               break;
                case 17: Serial.println("LEGO Power Functions"); break;
            }
            Serial.print("Value:    0x");
            Serial.println(ir_recv_results.value, HEX);
            Serial.print("No. bits: ");
            Serial.println(ir_recv_results.bits);
            Serial.println();
            irrecv.resume();
        }

        //watch2::drawTopThing();

        if (dpad_left_active())
        {
            watch2::setFont(MAIN_FONT);
            watch2::switchState(watch2::state, 0);
        }

    } // 2 - receiver






    //----------------------------------------------------
    // 3 - serial
    //----------------------------------------------------

    if (watch2::states[watch2::state].variant == 3)
    {
        if (!watch2::state_init)
        {
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.print("Serial thing");
        }

        watch2::drawTopThing();

        if (dpad_any_active())
        {
            watch2::switchState(watch2::state, 0);
        }
    } // 3 - serial

}