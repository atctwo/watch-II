#include "states.h"
#include "../libraries/css-color-parser-cpp/csscolorparser.hpp"
#include <IRremote.h>

void state_func_ir_remote()
{

    //----------------------------------------------------
    // variables
    //----------------------------------------------------

    // profile selection menu
    static std::vector<std::string> profile_names;
    EXT_RAM_ATTR static uint8_t selected_profile = 0;
    static std::string selected_profile_filename = "";

    // ir receiver
    static IRAM_ATTR IRrecv irrecv(IR_REC_PIN);
    static IRAM_ATTR decode_results ir_recv_results;
    static bool ir_enabled = false;

    // ir remote
    EXT_RAM_ATTR static IRsend irsend;
    EXT_RAM_ATTR static cJSON *profile;
    EXT_RAM_ATTR static cJSON *codes;
    EXT_RAM_ATTR static char *json_contents;
    EXT_RAM_ATTR static std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, int>>> code_indices;
    static uint8_t columns = 5;
    static uint8_t rows = 5;
    static uint8_t pages = 2;
    static uint8_t icon_spacing = 6;
    static uint8_t radius = 4;
    static uint8_t no_icons = 0;
    EXT_RAM_ATTR static uint8_t selected_row = 0;
    EXT_RAM_ATTR static uint8_t selected_col = 0;
    static uint8_t last_page_number = 255;
    EXT_RAM_ATTR static uint8_t this_page_number = 0;

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
            profile_names = watch2::getDirFiles("/ir");
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
        draw(dpad_any_active(), {
            // draw profile menu
            watch2::drawMenu(
                2,
                watch2::top_thing_height,
                SCREEN_WIDTH - 2,
                SCREEN_HEIGHT - watch2::top_thing_height,
                profile_names,
                selected_profile
            );
        });

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
            ESP_LOGD(WATCH2_TAG, "loading ir profile: ");

            // get a handle (???) to the selected file
            fs::File json = SD.open(selected_profile_filename.c_str());

            // allocate memory for the file's contents
            json_contents = (char*) malloc(json.size());

            // load the contents of the selected file into memory
            json.readBytes(json_contents, json.size());

            // close the json file handle
            json.close();

            // parse the json file
            profile = cJSON_Parse(json_contents);

            // print the json tree
            //ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profile));

            // print the profile name
            cJSON *twist = cJSON_GetObjectItem(profile, "name");
            if (twist) ESP_LOGD(WATCH2_TAG, "%s", twist->valuestring);
            else ESP_LOGD(WATCH2_TAG, "profile not named");

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
                ESP_LOGD(WATCH2_TAG, "%s", cJSON_Print(code));

                uint8_t page = cJSON_GetObjectItem(code, "page")->valueint;
                uint8_t row  = cJSON_GetObjectItem(code, "row")->valueint;
                uint8_t col  = cJSON_GetObjectItem(code, "col")->valueint;

                code_indices[page][row][col] = code_index + 1;
                code_index++;
            }

            // add devices button
            cJSON *devices_btn = cJSON_CreateObject();

            cJSON *devices_btn_text = cJSON_CreateString("dev");
            cJSON_AddItemToObjectCS(devices_btn, "text", devices_btn_text);

            cJSON_AddItemToArray(codes, devices_btn);
            code_indices[0][0][0] = code_index + 1;

            // set page number
            last_page_number = 255;

            // print icon width + height
            ESP_LOGD(WATCH2_TAG, "icon width:  %d\nicon height: %d", icon_width, icon_height);
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
        if ((this_page_number != last_page_number) || watch2::forceRedraw)
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
                // ESP_LOGD(WATCH2_TAG, "button %d", i);

                // get grid position
                uint8_t relative_index = i - (this_page_number * rows * columns);
                uint8_t row = div(relative_index, columns).quot;
                uint8_t col = relative_index % columns;
                int code_index = code_indices[this_page_number][row][col];

                // ESP_LOGD(WATCH2_TAG, "\tpage: %d", this_page_number);
                // ESP_LOGD(WATCH2_TAG, "\trow:  %d", row);
                // ESP_LOGD(WATCH2_TAG, "\tcol:  %d", col);
                // ESP_LOGD(WATCH2_TAG, "\tcode index: %d", code_index - 1);

                //get button data
                if (code_index > 0)
                {
                    cJSON *btn = cJSON_GetArrayItem(codes, code_index - 1);
                    if (btn)
                    {

                        ESP_LOGD(WATCH2_TAG, "\tfound button entry in json file");

                        // check if the button has an icon
                        cJSON *btn_icon_object = cJSON_GetObjectItem(btn, "icon");
                        if (btn_icon_object)
                        {
                            // get image data
                            // ESP_LOGD(WATCH2_TAG, "%*", btn_icon_object->valuestring);
                            watch2::imageData data = watch2::getImageData(btn_icon_object->valuestring);

                            // if image loaded successfully
                            if (data.data != NULL)
                            {
                                watch2::drawImage(data, icon_xpos, icon_ypos);
                            }
                            else 
                            {
                                // ESP_LOGD(WATCH2_TAG, "\t");
                                // ESP_LOGD(WATCH2_TAG, "%*", data.error);
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
                                    // ESP_LOGD(WATCH2_TAG, "\tcolour: %s (0x%x)", btn_colour_object->valuestring, btn_colour);
                                }
                                //free(btn_colour_object);

                                // print button text
                                watch2::oled.setTextDatum(MC_DATUM);
                                watch2::oled.setTextColor(btn_colour, BLACK);
                                watch2::oled.drawString(btn_text_object->valuestring, icon_xpos + (icon_width / 2), icon_ypos + (icon_height / 2));
                                // ESP_LOGD(WATCH2_TAG, "\tbutton text: %s", btn_text_object->valuestring);
                            }
                            //free(btn_text_object);

                        }
                    }
                    // else ESP_LOGD(WATCH2_TAG, "\tcode index doesn't point to a button");
                }
                // else ESP_LOGD(WATCH2_TAG, "\tbutton doesn't have any IR data");
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
        draw(dpad_any_active(), {
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

        });

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

                ESP_LOGD(WATCH2_TAG, "sending ir code: ");

                // get code object
                int code_index = code_indices[this_page_number][selected_row][selected_col];
                cJSON *code = cJSON_GetArrayItem(codes, code_index - 1);
                if ((code_index > 0) && code)
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

                        ESP_LOGD(WATCH2_TAG, "\tprotocol: %s", protocol);
                        ESP_LOGD(WATCH2_TAG, "\tcode:     %x", ir_code);
                        ESP_LOGD(WATCH2_TAG, "\tstr code: %s", ir_code_object->valuestring);
                        ESP_LOGD(WATCH2_TAG, "\tsize:     %d", ir_code_size->valueint);

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
                        if (strcmp(protocol, "sharp") == 0)              irsend.sendSharpRaw(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "sharp alt") == 0)          irsend.sendSharpAltRaw(ir_code, ir_code_size->valueint);
                        if (strcmp(protocol, "denon") == 0)              irsend.sendDenon(ir_code, ir_code_size->valueint);
                        //if (strcmp(protocol, "pronto") == 0)             irsend.sendPronto(ir_code_object->valuestring, false, false);
                        if (strcmp(protocol, "lego pf") == 0)            irsend.sendLegoPowerFunctions(ir_code, false);
                        //if (strcmp(protocol, "bose wave") == 0)          irsend.sendBoseWave(ir_code);
                        //if (strcmp(protocol, "magiquest") == 0)          irsend.sendMagiQuest()
                    }
                    else
                    {
                        if (!protocol_object) ESP_LOGD(WATCH2_TAG, "invalid protocol");
                        if (!ir_code_object) ESP_LOGD(WATCH2_TAG, "invalid ir code");
                        if (!ir_code_size) ESP_LOGD(WATCH2_TAG, "invalid ir code size");
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

        if (dpad_enter_active())
        {
            if (!ir_enabled)
            {
                // start the receiver
                watch2::oled.fillScreen(BLACK);
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.print("IR Receiver    ");
                watch2::oled.setTextColor(GREEN, BLACK);
                watch2::oled.println("enabled");

                irrecv.enableIRIn();
                ir_enabled = true;  
            }
            else
            {
                // disable the receiver
                irrecv.disableIRIn();
                ir_enabled = false;
            }
        }

        draw((!ir_enabled && dpad_enter_active()), {
            watch2::oled.fillScreen(BLACK);
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print("IR Receiver    ");
            watch2::oled.setTextColor(RED, BLACK);
            watch2::oled.println("disabled");
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("Press Enter to enable the \n"
                                "IR receiver.  After you \n"
                                "enable the receiver, you\n"
                                "will need to disable it\n"
                                "before you can leave \n"
                                "the app");
        });

        if (ir_enabled)
        if (irrecv.decode(&ir_recv_results) || watch2::forceRedraw)
        {
            irrecv.disableIRIn();
            delay(100);
            uint16_t code_info_y = watch2::top_thing_height + watch2::oled.fontHeight();
            watch2::oled.fillRect( 0, code_info_y, SCREEN_WIDTH, SCREEN_HEIGHT - code_info_y, BLACK);

            // print code info
            watch2::oled.setCursor(0, code_info_y);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print("\nProtocol: ");
            watch2::oled.setTextColor(WHITE, BLACK);
            switch(ir_recv_results.decode_type)
            {
                default: watch2::oled.printf("Unknown (%d)\n", ir_recv_results.decode_type); break;
                case 0:  watch2::oled.println("Unused");               break;
                case 1:  watch2::oled.println("RC5");                  break;
                case 2:  watch2::oled.println("RC6");                  break;
                case 3:  watch2::oled.println("NEC");                  break;
                case 4:  watch2::oled.println("Sony");                 break;
                case 5:  watch2::oled.println("Panasonic");            break;
                case 6:  watch2::oled.println("JVC");                  break;
                case 7:  watch2::oled.println("Samsung");              break;
                case 8:  watch2::oled.println("Whynter");              break;
                case 9:  watch2::oled.println("Aiwa RC T501");         break;
                case 10: watch2::oled.println("LG");                   break;
                case 11: watch2::oled.println("Sanyo");                break;
                case 12: watch2::oled.println("Mitsubishi");           break;
                case 13: watch2::oled.println("Dish");                 break;
                case 14: watch2::oled.println("Sharp");                break;
                case 15: watch2::oled.println("Denon");                break;
                case 16: watch2::oled.println("Pronto");               break;
                case 17: watch2::oled.println("LEGO Power Functions"); break;
            }

            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print("Value:    ");
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("0x");
            watch2::oled.println(ir_recv_results.value, HEX);

            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print("No. bits: ");
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println(ir_recv_results.bits);
            

            if (ir_recv_results.decode_type == PANASONIC || ir_recv_results.decode_type == SHARP)
            {
                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.print("Address:  ");
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.print("0x");
                watch2::oled.println(ir_recv_results.address);
            }
            // if (ir_recv_results.decode_type == MAGIQUEST)
            // {
            //     watch2::oled.setTextColor(watch2::themecolour, BLACK);
            //     watch2::oled.print("Magnitude:");
            //     watch2::oled.setTextColor(WHITE, BLACK);
            //     watch2::oled.print("0x");
            //     watch2::oled.println(ir_recv_results.magnitude);
            // }
            watch2::oled.println();

            
            irrecv.enableIRIn();
            irrecv.resume();
        }

        //watch2::drawTopThing();

        if (!ir_enabled && dpad_left_active())
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
        draw(false, {
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.println("Serial thing");
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println("Send commands over\n"
                                 "serial to send them \n"
                                 "using the IR LED.\n"
                                 "Use the format\n"
                                 "cmd;protocol;code;size\n");
        });

        watch2::drawTopThing();

        if (Serial.available())
        {

            //ESP_LOGD(WATCH2_TAG, "received data: 0x%x", Serial.read());
            
            const char *protocol;
            uint32_t code;
            uint8_t size;
            char *pch;

            // get thing from serial
            char thing[50];// = "cmd;nec;0xfe50af;32";
            Serial.readBytesUntil(0x0A, thing, 50); // terminator is line feed
            ESP_LOGD(WATCH2_TAG, "received string: %s", thing);

            if (thing[0] == 'c' && thing[1] == 'm' && thing[2] == 'd')
            {

                // split string
                char *str = strdup(thing);
                pch = strtok(str, ";");
                pch = strtok(NULL, ";"); // skip "cmd" header

                protocol = strdup(pch); // store protocol
                ESP_LOGD(WATCH2_TAG, "protocol: %s", protocol);
                pch = strtok(NULL, ";");

                code = strtoul(pch, NULL, 0); // store code
                ESP_LOGD(WATCH2_TAG, "code: 0x%x", code);
                ESP_LOGD(WATCH2_TAG, "raw code: %s", pch);
                pch = strtok(NULL, ";");

                size = strtol(pch, NULL, 0); // store size
                ESP_LOGD(WATCH2_TAG, "size: %d bits", size);
                ESP_LOGD(WATCH2_TAG, "raw size: %s", pch);

                // print details
                uint16_t code_info_y = watch2::top_thing_height + watch2::oled.fontHeight();
                watch2::oled.fillRect( 0, code_info_y, SCREEN_WIDTH, SCREEN_HEIGHT - code_info_y, BLACK);

                watch2::oled.setCursor(0, code_info_y);
                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.print("Protocol: ");
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.println(protocol);

                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.printf("Code:      0x");
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.println(code, HEX);

                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.printf("Size:       ");
                watch2::oled.printf("%d bits\n", size);

                // send code
                delay(500);
                if (strcmp(protocol, "rc5") == 0)                irsend.sendRC5(code, size);
                if (strcmp(protocol, "rc6") == 0)                irsend.sendRC6(code, size);
                if (strcmp(protocol, "nec") == 0)                irsend.sendNEC(0xfe50af, 24);
                if (strcmp(protocol, "sony") == 0)               irsend.sendSony(code, size);
                if (strcmp(protocol, "panasonic") == 0)          irsend.sendPanasonic(code, size);
                if (strcmp(protocol, "jvc") == 0)                irsend.sendJVC(code, size, false);
                if (strcmp(protocol, "samsung") == 0)            irsend.sendSAMSUNG(code, size);
                if (strcmp(protocol, "whynter") == 0)            irsend.sendWhynter(code, size);
                if (strcmp(protocol, "aiwa rc t501") == 0)       irsend.sendAiwaRCT501(code);
                if (strcmp(protocol, "lg") == 0)                 irsend.sendLG(code, size);
                //if (strcmp(protocol, "sanyo") == 0)              irsend.sendSanyo(code, size);
                //if (strcmp(protocol, "mitsubishi") == 0)         irsend.sendMitsubishi(code, size);
                if (strcmp(protocol, "dish") == 0)               irsend.sendDISH(code, size);
                if (strcmp(protocol, "sharp") == 0)              irsend.sendSharpRaw(code, size);
                //if (strcmp(protocol, "sharp alt") == 0)          irsend.sendSharpAltRaw(code, size);
                if (strcmp(protocol, "denon") == 0)              irsend.sendDenon(code, size);
                //if (strcmp(protocol, "pronto") == 0)             irsend.sendPronto(ir_code_object->valuestring, false, false);
                if (strcmp(protocol, "lego pf") == 0)            irsend.sendLegoPowerFunctions(code, false);
                //if (strcmp(protocol, "bose wave") == 0)          irsend.sendBoseWave(code);
                //if (strcmp(protocol, "magiquest") == 0)          irsend.sendMagiQuest(code, size);

            }
        }

        if (dpad_left_active())
        {
            watch2::switchState(watch2::state, 0);
        }
    } // 3 - serial

}