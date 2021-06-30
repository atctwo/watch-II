/**
 * @file watch2_drawing.cpp
 * @author atctwo
 * @brief functions that can be used to draw things to the screen.  (also, icon stuff)
 * @version 0.1
 * @date 2020-11-21
 * this file doesn't contain any image functions; those can be found in watch2_images.cpp.
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // drawing
    uint8_t top_thing_height = oled.fontHeight() + 20;
    EXT_RAM_ATTR bool forceRedraw = false;
    EXT_RAM_ATTR bool forceRedrawLooped = false;
    EXT_RAM_ATTR uint16_t trans_mode = 0;
    EXT_RAM_ATTR bool animate_watch_face = false;
    int themecolour = BLUE;

    TFT_eSPI oled = TFT_eSPI();
    EXT_RAM_ATTR TFT_eSprite top_thing = TFT_eSprite(&oled);
    EXT_RAM_ATTR TFT_eSprite framebuffer = TFT_eSprite(&oled);

    EXT_RAM_ATTR std::map<std::string, imageData> *icons;
    EXT_RAM_ATTR std::map<std::string, std::vector<unsigned char>> *small_icons;

    // fs icon maps
    std::unordered_map<std::string, fs_icon> fs_icon_ext_map;
    std::unordered_map<fs_icon, std::string> fs_icon_name_map;

    void drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, std::vector<fs_icon> icons, bool scroll, bool centre, int colour)
    {
        static int16_t x1, y1;
        static uint16_t w=0, w2=0, h=0, h2=0;
        static int padding = 4;
        static int radius = 4;
        int original_y = y;
        
        //clear screen where menu will be drawn
        oled.fillRect(x, y, width, height, BLACK);

        x += padding;

        //get text dimensions
        w = oled.textWidth(items[0].c_str());
        h = oled.fontHeight();

        //get total height of button (incl. padding)
        int ht = h + padding + padding + padding;
        
        //calculate how many items are onscreen (not considering y offset)
        int onscreen_items = height / ht;

        //calculate how many items are onscreen after the offset threshold but before the height of the menu
        int threshold_items = onscreen_items - ( (height - ht) / ht );

        //calculate y index of selected item
        int selected_y_index = y + ((selected + 1) * ht);

        //calculate offset threshold based on selected item
        //if the selected item has a y index greater than this value, the offset will be non-zero
        int y_offset = 0;
        if (scroll) 
        {
            if (selected_y_index > ((y + height) - (ht))) 
            {
                y_offset = ht * ((selected + 2) - onscreen_items);
            }
        }

        //print each menu item
        int fridgebuzz = 0; // onscreen icon number
        for (const std::string &item : items)
        {
            //calculate the item's y position
            int item_ypos = y - y_offset;

            //if item is within the area in which the menu is to be drawn

            // Serial.println(item.c_str());
            // Serial.printf("\ty:                 %d\n", y);
            // Serial.printf("\ty_offset:          %d\n", y_offset);
            // Serial.printf("\titem_ypos:         %d\n", item_ypos);
            // Serial.printf("\tcheck 1 value:     %d\n", original_y - ht);
            // Serial.printf("\tcheck 1 outcome:   %d\n", item_ypos >= ( original_y - ht));
            // Serial.printf("\tcheck 2 value:     %d\n", (original_y + height + ht));
            // Serial.printf("\tcheck 2 outcome:   %d\n", item_ypos < (original_y + height + ht));
            // Serial.printf("\tdraw?              %d\n", item_ypos >= ( original_y - ht) && item_ypos < (original_y + height + ht));
            // Serial.println("");

            if (item_ypos >= ( original_y - ht) && item_ypos < (original_y + height + ht))
            {

                //draw the item rounded rectangle

                //if the current item is the selected item
                if (fridgebuzz == selected)
                {
                    //draw a filled in rounded rectangle
                    oled.fillRoundRect(x, y - y_offset, width - (padding*2), h + padding + padding, radius, colour);
                    oled.setTextColor(BLACK, colour);
                }
                else
                {
                    //draw the outline of a rounded rectangle
                    oled.drawRoundRect(x, y - y_offset, width - (padding*2), h + padding + padding, radius, colour);
                    oled.setTextColor(colour, BLACK);
                }

                // draw the icon
                uint16_t icon_space = 0;
                if (fridgebuzz < icons.size())
                {
                    std::string icon_name = fs_icon_name_map[icons[fridgebuzz]];

                    //ESP_LOGD(WATCH2_TAG, "%s: icon \"%s\" (item no %d, icon no %d)", item.c_str(), icon_name.c_str(), fridgebuzz, icons[fridgebuzz]);

                    if (!icon_name.empty())
                    {
                        oled.drawBitmap(
                            x + padding, y + padding - y_offset,
                            (*small_icons)[icon_name].data(),
                            20, 20, (fridgebuzz == selected ? BLACK : WHITE)
                        );
                    }
                    icon_space = 20 + padding; // set to 0 if no icon is being drawn
                    
                }

                //draw the item text
                String itemtext = "";

                //get the length of the text
                //watch2::getTextBounds(String(item.c_str()), x + padding, y + h + padding - y_offset, &x1, &y1, &w, &h2);
                w = watch2::oled.textWidth(item.c_str());

                //if the text is too long for the item button
                if (w > (width - (padding * 6)))
                {

                    //this is a _really_ inefficient idea
                    //iterate through each letter until the length of the button is reached

                    //find the width of the string "..." and store it in w2
                    w2 = oled.textWidth("...");
                    h = oled.fontHeight();

                    //running value of item text length
                    int text_length = 0;

                    //for each letter
                    for (int i = 0; i < item.length(); i++)
                    {
                        //get width of character
                        w = oled.textWidth(String(item[i]).c_str());

                        //add width to running value
                        //really, the character width should be added to this value,
                        //but for some reason, the character width calculated by watch2::getTextBounds()
                        //above isn't correct
                        text_length += w;

                        //if the text would be too long (idk im tired)
                        //the limit is the width - padding - the length of "..."
                        if (text_length > (width - (padding * 10) - w2 - icon_space))
                        {
                            //add "..." to the item text, and break the loop
                            itemtext += "...";
                            break;
                        }
                        else
                        {
                            //add the character to the item text
                            itemtext += String(item[i]);
                        }
                        
                    }

                }
                else itemtext = String(item.c_str());

                //print the text
                if (centre)
                {
                    oled.setTextDatum(TC_DATUM);
                    oled.drawString(itemtext, x + (width / 2) + icon_space, y + padding - y_offset);
                    oled.setTextDatum(TL_DATUM);
                }
                else
                {
                    oled.setCursor(x + padding + icon_space, y + padding - y_offset);
                    oled.print(itemtext);
                }

            }

            y += ht;
            fridgebuzz++;
        }
    }

    void drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour)
    {
        static String last_selected_value = "";
        static int last_selected_element = -1;
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static int padding = 4;
        static int spacing = 1;
        static int radius = 4;
        int ypos = padding;               //y position of cursor (considering multiline settings)


        for (int i = 0; i < items.size(); i++)
        {
            int text_height = 0;
            int cursor_y_pos = y + ypos;
            int outline_width;
            int outlinecolour;

            // draw settings title
            oled.setCursor(x, cursor_y_pos);
            watch2::getTextBounds(String(items[i].setting_title), x, cursor_y_pos, &x1, &y1, &w, &h);
            oled.print(items[i].setting_title);
            text_height = h;

            // determine settings value
            String final_setting_value;
            if (items[i].setting_value < items[i].text_map.size()) final_setting_value = items[i].text_map[items[i].setting_value];
            else final_setting_value = String(items[i].setting_value);

            // clear previous settings value
            if (last_selected_element == i && last_selected_value != final_setting_value)
            {
                digitalWrite(12, HIGH);
                watch2::getTextBounds(last_selected_value, x, cursor_y_pos, &x1, &y1, &w, &h);
                outline_width = std::max(items[i]. min_field_width, w + (2 * padding));

                oled.fillRect( (x + (width - padding - outline_width)) - padding,
                                cursor_y_pos - padding,
                                outline_width,
                                h + (2 * padding),
                                BLACK
                );
            }
            else digitalWrite(12, LOW);

            // draw settings value
            watch2::getTextBounds(final_setting_value, x, cursor_y_pos, &x1, &y1, &w, &h);
            outline_width = std::max(items[i]. min_field_width, w + (2 * padding));
            outlinecolour = BLACK;

            oled.setCursor(x + (width - padding - outline_width), cursor_y_pos);
            oled.print(final_setting_value);

            //draw outline around value if element is selected
            if (selected == i) outlinecolour = colour;

            oled.drawRoundRect( (x + (width - padding - outline_width)) - padding,
                                cursor_y_pos - padding,
                                outline_width,
                                h + (2 * padding),
                                radius,
                                outlinecolour
            );

            if (selected == i)
            {
                last_selected_value = final_setting_value;
                last_selected_element = selected;
            }
            ypos += text_height + ( 2 * padding );
        }
    }

    void drawTopThing(bool light)
    {
        static char text[4];
        static double batteryVoltage = 4.2;
        static uint8_t batteryPercentage = 0;
        static int last_battery_reading = millis() - 1000;
        static uint16_t icon_size = 20;
        static uint16_t icon_padding = 5;
        uint16_t icon_xpos = SCREEN_WIDTH;
        uint16_t milliseconds = 0;

        top_thing.fillRect(0, 0, top_thing.width(), top_thing.height(), BLACK);

        if (!light)
        {
            //top_thing_height = top_thing.fontHeight() + 10;
            top_thing.drawFastHLine(0, top_thing.fontHeight(), SCREEN_WIDTH, themecolour);
            top_thing.drawFastHLine(0, top_thing.fontHeight() + 1, SCREEN_WIDTH, themecolour);
            top_thing.drawFastHLine(0, top_thing.fontHeight() + 2, SCREEN_WIDTH, themecolour);

            top_thing.setCursor(1,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.setTextSize(1);
            //top_thing.setFreeFont(&SourceSansPro_Regular6pt7b);
            top_thing.printf("%02d:%02d", hour(), minute());

            if ( millis() - last_battery_reading > 10000)
            {
                //batteryVoltage = ( (ReadVoltage(BATTERY_DIVIDER_PIN) * 3.3 ) / 4095.0 ) * 2;
                //batteryVoltage = ReadVoltage(BATTERY_DIVIDER_PIN) * BATTERY_VOLTAGE_SCALE;
                //batteryPercentage = 69.0; ( batteryVoltage / BATTERY_VOLTAGE_MAX ) * 100.0;
                batteryPercentage = round(watch2::fuel_gauge.cellPercent());

                // write the battery percentage to the ble battery level service
                if (bluetooth_state == 2 || bluetooth_state == 3) ble_hid->setBatteryLevel(batteryPercentage);

                last_battery_reading = millis();
            }

            // print battery
            sprintf(text, "%.0d", batteryPercentage);
            icon_xpos -= (watch2::oled.textWidth(text) + icon_padding);
            top_thing.setCursor(icon_xpos,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.printf(text);

            icon_xpos -= (icon_size + icon_padding);
            top_thing.drawBitmap(
                icon_xpos,
                1,
                (*small_icons)["small_battery"].data(),
                icon_size,
                icon_size,
                WHITE
            );
            
            // print ram usage
            sprintf(text, "%2.0f", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);
            icon_xpos -= (watch2::oled.textWidth(text) + icon_padding);
            top_thing.setCursor(icon_xpos,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.printf(text);

            icon_xpos -= (icon_size + icon_padding);
            top_thing.drawBitmap(
                icon_xpos,
                1,
                (*small_icons)["small_ram"].data(),
                icon_size,
                icon_size,
                WHITE
            );

        }

        //draw sd card status
        int sd_colour = ORANGE;
        switch(sd_state)
        {
            case 0: sd_colour = RED; break;    //didn't mount successfully
            case 1: sd_colour = 0x0660; break; //mounted successfully
            case 2: sd_colour = BLUE;          //sd card not present
        }

        icon_xpos -= (icon_size + icon_padding);
        top_thing.drawBitmap(
            icon_xpos,
            1,
            (*small_icons)["small_sd"].data(),
            icon_size,
            icon_size,
            sd_colour
        );

        // draw wifi icon
        if (wifi_state > 0) // if wifi is enabled
        {
            icon_xpos -= (icon_size + icon_padding);
            switch(wifi_state)
            {
                case 1: // enabled, disconnected
                    
                    top_thing.drawBitmap(
                        icon_xpos,
                        1,
                        (*small_icons)["small_wifi_complete"].data(),
                        icon_size,
                        icon_size,
                        0x4A69
                    );
                    break;

                case 2: // enabled, connecting
                    
                    milliseconds = millis() % 1000;

                    // bar 0
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_0"].data(), icon_size, icon_size, (0 <= milliseconds && milliseconds <= 250) ? WHITE : 0x6B4D);
                    
                    // bar 1
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_1"].data(), icon_size, icon_size, (251 <= milliseconds && milliseconds <= 500) ? WHITE : 0x6B4D);

                    // bar 2
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_2"].data(), icon_size, icon_size, (501 <= milliseconds && milliseconds <= 750) ? WHITE : 0x6B4D);

                    // bar 3
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_3"].data(), icon_size, icon_size, (751 <= milliseconds && milliseconds <= 1000) ? WHITE : 0x6B4D);

                    break;

                case 3: // enabled, connected
                    
                    // draw the wifi symbol, but hightlight each "bar" depending on the wifi signal strength

                    // bar 0
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_0"].data(), icon_size, icon_size, (WiFi.RSSI() >= -80) ? WHITE : 0x6B4D);
                    
                    // bar 1
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_1"].data(), icon_size, icon_size, (WiFi.RSSI() >= -70) ? WHITE : 0x6B4D);
                    
                    // bar 2
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_2"].data(), icon_size, icon_size, (WiFi.RSSI() >= -67) ? WHITE : 0x6B4D);
                    
                    // bar 3
                    top_thing.drawBitmap(icon_xpos, 1, (*small_icons)["small_wifi_3"].data(), icon_size, icon_size, (WiFi.RSSI() >= -30) ? WHITE : 0x6B4D);
                    break;
            }
        }

        top_thing.pushSprite(0, 0);
    }

    void setFont(const char* font, TFT_eSPI &tft, fs::FS &ffs)
    {
        if (ffs.exists("/" + String(font) + ".vlw"))
        {
            tft.loadFont(String(font));
        }
        else 
        {
            ESP_LOGW(WATCH2_TAG, "[error] font %s doesn't exist", font);
            tft.setFreeFont(NULL);
        }
    }

    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        std::string help(string);
        int newlines = std::count(help.begin(), help.end(), '\n') + 1;

        *x1 = x;
        *y1 = y;
        *w = oled.textWidth(string);
        *h = oled.fontHeight() * newlines;
    }

    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        std::string help(str.c_str());
        int newlines = std::count(help.begin(), help.end(), '\n') + 1;

        *x1 = x;
        *y1 = y;
        *w = oled.textWidth(str);
        *h = oled.fontHeight() * newlines;
    }

    bool registerIcon(std::string iconName, imageData icon)
    {
        icons->emplace( iconName, icon );

        return false;
    }

    bool registerSmallIcon(std::string iconName, std::vector<unsigned char> icon)
    {
        small_icons->emplace( iconName, icon );

        return false;
    }

    void setupFsIcons()
    {
        watch2::fs_icon_ext_map["png"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["jpg"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["jpeg"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["gif"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["tiff"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["tif"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["bmp"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["psd"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["tga"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["hdr"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["pic"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["pnm"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["ico"] = watch2::FS_ICON_FILE_IMAGE;
        watch2::fs_icon_ext_map["webp"] = watch2::FS_ICON_FILE_IMAGE;

        watch2::fs_icon_ext_map["aac"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["flac"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["midi"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["mid"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["rtttl"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["wav"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["mp3"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["mod"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["m3u"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["icy"] = watch2::FS_ICON_FILE_AUDIO;
        watch2::fs_icon_ext_map["ogg"] = watch2::FS_ICON_FILE_AUDIO;

        watch2::fs_icon_ext_map["mp4"] = watch2::FS_ICON_FILE_VIDEO;
        watch2::fs_icon_ext_map["avi"] = watch2::FS_ICON_FILE_VIDEO;
        watch2::fs_icon_ext_map["wmv"] = watch2::FS_ICON_FILE_VIDEO;
        watch2::fs_icon_ext_map["mov"] = watch2::FS_ICON_FILE_VIDEO;
        watch2::fs_icon_ext_map["webm"] = watch2::FS_ICON_FILE_VIDEO;

        watch2::fs_icon_ext_map["zip"] = watch2::FS_ICON_FILE_COMPRESSED;
        watch2::fs_icon_ext_map["7z"] = watch2::FS_ICON_FILE_COMPRESSED;
        watch2::fs_icon_ext_map["tar"] = watch2::FS_ICON_FILE_COMPRESSED;
        watch2::fs_icon_ext_map["gz"] = watch2::FS_ICON_FILE_COMPRESSED;
        watch2::fs_icon_ext_map["lz"] = watch2::FS_ICON_FILE_COMPRESSED;
        watch2::fs_icon_ext_map["rar"] = watch2::FS_ICON_FILE_COMPRESSED;

        watch2::fs_icon_ext_map["json"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["xml"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["csv"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["c"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["cpp"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["cxx"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["h"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["hpp"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["hxx"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["py"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["js"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["html"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["htm"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["css"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["less"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["ts"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["java"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["ino"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["pde"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["lua"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["php"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["bat"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["sh"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["ps1"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["rs"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["rb"] = watch2::FS_ICON_FILE_CODE;
        watch2::fs_icon_ext_map["sql"] = watch2::FS_ICON_FILE_CODE;

        // watch2::fs_icon_ext_map["dat"] = watch2::FS_ICON_FILE_DATABASE;
        // watch2::fs_icon_ext_map["db"] = watch2::FS_ICON_FILE_DATABASE;
        // watch2::fs_icon_ext_map["sqlite"] = watch2::FS_ICON_FILE_DATABASE;

        watch2::fs_icon_ext_map["abw"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["odt"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["doc"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["docx"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["tex"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["pdf"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["epub"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["docm"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["ppt"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["pptx"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["rtf"] = watch2::FS_ICON_FILE_DOCUMENT;
        watch2::fs_icon_ext_map["txt"] = watch2::FS_ICON_FILE_DOCUMENT;

        watch2::fs_icon_ext_map["ttf"] = watch2::FS_ICON_FILE_FONT;
        watch2::fs_icon_ext_map["otf"] = watch2::FS_ICON_FILE_FONT;
        watch2::fs_icon_ext_map["woff"] = watch2::FS_ICON_FILE_FONT;

        // watch2::fs_icon_ext_map["exe"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["out"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["app"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["apk"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["ipa"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["jar"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["class"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["elf"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["com"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["o"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["a"] = watch2::FS_ICON_FILE_EXECUTABLE;
        // watch2::fs_icon_ext_map["so"] = watch2::FS_ICON_FILE_EXECUTABLE;

        watch2::fs_icon_name_map[watch2::FS_ICON_BLANK] = "file";
        watch2::fs_icon_name_map[watch2::FS_ICON_CANCEL] = "cancel";
        watch2::fs_icon_name_map[watch2::FS_ICON_FOLDER] = "folder";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_GENERIC] = "file";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_IMAGE] = "file_image";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_AUDIO] = "file_audio";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_VIDEO] = "file_video";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_COMPRESSED] = "file_compressed";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_CODE] = "file_code";
        //watch2::fs_icon_name_map[watch2::FS_ICON_FILE_DATABASE] = "file";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_DOCUMENT] = "file_document";
        watch2::fs_icon_name_map[watch2::FS_ICON_FILE_FONT] = "file_font";
        //watch2::fs_icon_name_map[watch2::FS_ICON_FILE_EXECUTABLE] = "file";
    }

    // adapted from https://stackoverflow.com/a/9069480/9195285
    void colour888(uint16_t colour, float *r, float *g, float *b)
    {
        uint16_t red   = (colour & 0xf800) >> 11;
        uint16_t green = (colour & 0x07e0) >> 5;
        uint16_t blue  = (colour & 0x001f);

        *r = ( red   * 255 ) / 31;
        *g = ( green * 255 ) / 63;
        *b = ( blue  * 255 ) / 31;
    }

    // from https://www.cs.rit.edu/~ncs/color/t_convert.html#RGB%20to%20HSV%20&%20HSV%20to%20RGB
    void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
    {
        int i;
        float f, p, q, t;
        if( s == 0 ) {
            // achromatic (grey)
            *r = *g = *b = v;
            return;
        }
        h /= 60;			// sector 0 to 5
        i = floor( h );
        f = h - i;			// factorial part of h
        p = v * ( 1 - s );
        q = v * ( 1 - s * f );
        t = v * ( 1 - s * ( 1 - f ) );
        switch( i ) {
            case 0:
                *r = v;
                *g = t;
                *b = p;
                break;
            case 1:
                *r = q;
                *g = v;
                *b = p;
                break;
            case 2:
                *r = p;
                *g = v;
                *b = t;
                break;
            case 3:
                *r = p;
                *g = q;
                *b = v;
                break;
            case 4:
                *r = t;
                *g = p;
                *b = v;
                break;
            default:		// case 5:
                *r = v;
                *g = p;
                *b = q;
                break;
        }
    }

    //from http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
    void getHeatMapColor(float value, float *red, float *green, float *blue, std::vector<std::array<float, 3>> heatmap)
    {
        // A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

        int idx1;        // |-- Our desired color will be between these two indexes in "color".
        int idx2;        // |
        float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

        if(value <= 0)      {  idx1 = idx2 = 0;            }    // accounts for an input <=0
        else if(value >= 1)  {  idx1 = idx2 = heatmap.size() - 1; }    // accounts for an input >=0
        else
        {
            value = value * (heatmap.size() - 1);        // Will multiply value by 3.
            idx1  = floor(value);                  // Our desired color will be after this index.
            idx2  = idx1+1;                        // ... and before this index (inclusive).
            fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
        }

        *red   = (heatmap[idx2][0] - heatmap[idx1][0])*fractBetween + heatmap[idx1][0];
        *green = (heatmap[idx2][1] - heatmap[idx1][1])*fractBetween + heatmap[idx1][1];
        *blue  = (heatmap[idx2][2] - heatmap[idx1][2])*fractBetween + heatmap[idx1][2];
    }

}