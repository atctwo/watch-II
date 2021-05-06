/**
 * @file watch2_files.cpp
 * @author atctwo
 * @brief functions for file system access
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // files
    EXT_RAM_ATTR int sd_state = 0;
    EXT_RAM_ATTR bool spiffs_state = 0;
    EXT_RAM_ATTR int file_select_status = 0;
    EXT_RAM_ATTR std::string file_path = "/";
    EXT_RAM_ATTR bool file_select_dir_list_init = false;
    
    //SdFat sdcard(&*vspi);
    //Adafruit_ImageReader reader(SD);

    int initSD(bool handleCS)
    {
        if (sd_state == 1)
        {
            // sd card is already initalised
            ESP_LOGW(WATCH2_TAG, "[initSD] sd is already mounted");
            return 1;
        }

        int no = 0;

        //enable the sd card
        digitalWrite(cs, HIGH);
        digitalWrite(sdcs, LOW);

        //initalise the sd card
        //if(!sdcard.begin(sdcs, SPISettings(9000000, MSBFIRST, SPI_MODE0))){
        if(!SD.begin(sdcs, *watch2::vspi, 4000000U)){

            //card couldn't mount
            ESP_LOGW(WATCH2_TAG, "[initSD] Couldn't mount SD card");
            // ESP_LOGD(WATCH2_TAG, "\tError code: ");
            // ESP_LOGD(WATCH2_TAG, "0x%x", sdcard.cardErrorCode());
            // ESP_LOGD(WATCH2_TAG, "\tError data: ");
            // ESP_LOGD(WATCH2_TAG, "0x%x", sdcard.cardErrorData());
            no = 0;

        }
        else
        {

            //card mounted successfully
            ESP_LOGD(WATCH2_TAG, "[initSD] Successfully mounted SD card");
            ESP_LOGD(WATCH2_TAG, "Card size: %u", SD.cardSize());
            //sdcard.ls(LS_R | LS_DATE | LS_SIZE);
            no = 1;

        }

        //if there is no sd card inserted, set no to 2
        //if (!digitalRead(sdcd)) no = 2;

        //set global sd state variable, and return
        sd_state = no;
        return no;
    }

    std::vector<std::string> getDirFiles(std::string path, std::vector<fs_icon> *icons)
    {
        //vector to store files in the directory
        std::vector<std::string> files;
        if (icons) icons->clear();

        //buffer to store filenames
        //char filename[255];

        //enable the sd card
        digitalWrite(cs, HIGH);
        digitalWrite(sdcs, LOW);

        //initalise the sd card (without changing any CS pin)
        if( initSD(false) == 0 ){
            
            //sd could not be accessed
            ESP_LOGW(WATCH2_TAG, "getDirFiles failed because no");
            return files;

        }
        else
        {

            digitalWrite(cs, HIGH);
            digitalWrite(sdcs, LOW);

            //open the dir at the path
            fs::File root = SD.open(path.c_str());

            //check that path is valid
            if (!SD.exists(path.c_str()))
            {
                //file path is invalid
                ESP_LOGD(WATCH2_TAG, "[getDirFiles] file path %s is invalid", path.c_str());
                return files;
            }

            //check that path is actually a dir
            if (!root.isDirectory())
            {
                //path is not a directory
                ESP_LOGW(WATCH2_TAG, "[getDirFiles] file path isn't a directory");
                return files;
            }

            while(true)
            {
                //ESP_LOGD(WATCH2_TAG, "f");

                //open next file in dir
                fs::File f;
                //bool thing = f.openNext(&root);
                f = root.openNextFile();

                //if there are no more files, break
                if (!f) 
                {
                    ESP_LOGD(WATCH2_TAG, "[getDirFiles] no more files");
                    break;
                }

                //get the name of the file
                //f.getName(filename, 255);
                const char *filename = watch2::file_name(f.name()).c_str();

                //add the name to the vector
                files.push_back(std::string(filename));

                // add the icon to the icons vector
                if (icons)
                {
                    std::string ext = file_ext(filename);
                    fs_icon icon;
                    if (f.isDirectory()) icon = FS_ICON_FOLDER;
                    else icon = fs_icon_ext_map[ext];
                    icons->push_back(icon);

                    //ESP_LOGD(WATCH2_TAG, "[getDirFiles] %s: %d", filename, icon);
                }

                f.close();
            }

            root.close();

            ESP_LOGD(WATCH2_TAG, "");

        }


        //disable the sd card
        digitalWrite(cs, LOW);
        digitalWrite(sdcs, HIGH);

        return files;
    }

    std::string beginFileSelect(std::string path)
    {
        //i was watching tiktoks while writing this, so it might be pretty awful

        // if path ends in a '/', remove the last character,
        // except if path == "/", because that's ok
        if (path != "/")
        {
            if (path[path.size() - 1] == '/') path.pop_back();
        }

        // accept "" as a valid path
        if (path == "") path = "/";

        // lock dpad
        for (uint16_t i = 0; i < 5; i++) 
        {
            //ESP_LOGD(WATCH2_TAG, "[button locks] set button %d to locked and not pressed", i);
            dpad_lock[i] = true;
            dpad_pressed[i] = false;
        }

        static int selected_icon = 0; //currently selected file
        static char filename[255]; //buffer to hold filename

        static std::vector<std::string> files2;      //vector of Strings representing names of files in directory
        static std::stack<int> selected_icon_stack; //stack for storing selected file indecies when navigating through subdirectories
        static std::vector<fs_icon> icons;

        //call once
        file_path = path;
        file_select_dir_list_init = false;

        while(true)
        {
            startLoop();

            //handle dpad up press
            if (dpad_up_active()) 
            {
                selected_icon--;
                if (selected_icon < 0) selected_icon = files2.size() - 1;
            }

            //handle dpad down press
            if (dpad_down_active())
            {
                selected_icon++;
                if (selected_icon > files2.size() - 1) selected_icon = 0;
            }

            //handle dpad enter press
            if (dpad_enter_active())
            {
                //if cancel was pressed
                if (files2[selected_icon] == "Cancel")
                {
                    //set the filename
                    file_path = "canceled";

                    //stop the file select menu being active
                    file_select_status = false;

                    //return to the calling state
                    switchState(state, states[state].variant, 10, 10, true);
                    return file_path;
                }
                else if (files2[selected_icon] == "..") //if parent directory was selected
                {
                    char path[file_path.length()];
                    strcpy(path, file_path.c_str());
                    char *pch;
                    file_path = "/";
                    
                    //get number of occurances of / character
                    int occurances = 0;
                    for (int i = 0; i < sizeof(path) / sizeof(char); i++) if (path[i] == '/') occurances++;
                    
                    //split the string
                    pch = strtok(path, "/");
                    
                    for (int i = 0; i < occurances - 2; i++)
                    {
                        file_path += pch;
                        file_path += "/";
                        pch = strtok(NULL, "/");
                    }

                    //reset the file select dialogue
                    file_select_dir_list_init = false;

                    //load selected icon index from the selected index stack
                    selected_icon = selected_icon_stack.top();
                    selected_icon_stack.pop();
                }
                else
                {
                    ESP_LOGD(WATCH2_TAG, "selected file or folder: ");
                    ESP_LOGD(WATCH2_TAG, "%s", file_path.c_str());
                    ESP_LOGD(WATCH2_TAG, " ");
                    ESP_LOGD(WATCH2_TAG, "%s", files2[selected_icon].c_str());

                    //determine whether selected path is a directory
                    fs::File selected_file;
                    std::string filename;
                    if(file_path == "/") filename = file_path + files2[selected_icon];
                    else filename = file_path + "/" + files2[selected_icon];
                    selected_file = SD.open(filename.c_str());
                                        
                    //if the path points to a directory
                    if (selected_file.isDirectory())
                    {
                        ESP_LOGD(WATCH2_TAG, "selected folder");
                        if(file_path == "/") file_path += files2[selected_icon];
                        else file_path += "/" + files2[selected_icon];
                        file_select_dir_list_init = false;
                        selected_icon_stack.push(selected_icon);
                        selected_icon = 0; //reset selected icon
                        ESP_LOGD(WATCH2_TAG, "new file path: %s", file_path.c_str());
                        selected_file.close();

                    }
                    else //otherwise, assume the path points to a file
                    {
                        ESP_LOGD(WATCH2_TAG, "selected file");
                        //set the file path
                        if(file_path == "/") file_path += files2[selected_icon];
                        else file_path += "/" + files2[selected_icon];

                        //reset selected icon
                        selected_icon = 0;

                        //clear the selected icon stack
                        for (int i = 0; i < selected_icon_stack.size(); i++) selected_icon_stack.pop();

                        //stop the file select menu being active
                        file_select_status = false;

                        ESP_LOGD(WATCH2_TAG, "new file path: %s", file_path.c_str());

                        //dim the screen and return to the calling state
                        selected_file.close();
                        switchState(state, states[state].variant, 10, 10, true);
                        return file_path;

                    }
                }
                
            }

            //if the file select list hasn't been initalised
            if (!file_select_dir_list_init)
            {
                ESP_LOGI(WATCH2_TAG, "[beginFileSelect] opening file dialogue for ");
                ESP_LOGI(WATCH2_TAG, "%s", file_path.c_str());

                //dim screen
                dimScreen(0, top_thing_height);
                oled.fillScreen(BLACK);

                //populate files2 with the contents of the selected directory
                files2.clear();
                files2 = getDirFiles(file_path, &icons);

                //if card isn't initalised, notify the user
                if (sd_state != 1)
                {
                    oled.setCursor(2, 36);
                    oled.print("[beginFileSelect] SD card not mounted");
                }
                else
                {
                    //if there are no files, notify the user
                    if (files2.size() == 0)
                    {
                        oled.setCursor(2, 36);
                        oled.print("This directory is empty");
                    }
                    //add back button if in a non-root directory
                    if (file_path != "/") 
                    {
                        files2.emplace(files2.begin(), ".."); 
                        icons.emplace(icons.begin(), FS_ICON_FOLDER);               
                    }
                }

                //add cancel option
                files2.emplace(files2.begin(), "Cancel");
                icons.emplace(icons.begin(), FS_ICON_CANCEL);

                dimScreen(1, 10);
            }

            //if file select list hasn't been initliased, or any button is pressed, redraw the menu
            if (watch2::forceRedraw || !file_select_dir_list_init || dpad_any_active())
            {
                std::vector<fs_icon> menu_icons = {};
                if (sd_state == 1 && files2.size() > 0) 
                {
                    menu_icons = icons;
                    //ESP_LOGD(WATCH2_TAG, "[beginFileSelect] using icon vector");
                }

                drawMenu(2, top_thing_height, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 12, files2, selected_icon, menu_icons, themecolour);
            };

            //finish file select list initilisation
            if (!file_select_dir_list_init) file_select_dir_list_init = true;

            drawTopThing();
            endLoop();

        }
    }

    std::string file_name(const char* filepath, bool extension)
    {
        // get icon name from filename
        std::string icon_name = filepath;       // copy filename
        size_t pos = icon_name.rfind("/");      // find last '/'
        icon_name.erase(0, pos + 1);            // remove path part

        if (!extension) 
        {
            pos = icon_name.rfind(".");             // find where the extension starts
            icon_name.erase(pos, icon_name.npos);   // remove extension
        }
        return icon_name;   
    }

    std::string dir_name(std::string file_path_thing)
    {
        std::string file_dir = file_path_thing;

        size_t pos = file_dir.rfind("/");       // find last '/'
        file_dir.erase(pos + 1, file_dir.npos); // erase filename part

        return file_dir;
    }

    std::string file_ext(std::string file_path_thing)
    {
        // adapted from https://stackoverflow.com/a/51992/9195285
        std::string::size_type idx;
        idx = file_path_thing.rfind('.');

        if(idx != std::string::npos)
        {
            std::string extension = file_path_thing.substr(idx+1);
            // adapted from https://stackoverflow.com/a/313990/9195285
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return std::tolower(c); });
            return extension;
        }
        else
        {
            // No extension found
            return "";
        }
    }

}