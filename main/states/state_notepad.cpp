#include "states.h"

void state_func_notepad()
{
    EXT_RAM_ATTR static int read = 0;                    //whether or not the selected file has been copied into memory
    EXT_RAM_ATTR static int yoffset = 0;                 //the offset to draw the file contents at
    EXT_RAM_ATTR static std::vector<int> filedata;       //a vector to store the file data
    EXT_RAM_ATTR static std::string filename;            //the name of the file to read
    
    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect("/");
        read = 0;
    }

    if (watch2::file_path == "canceled")
    {
        watch2::switchState(-1);
    }
    else
    {
    
        //read contents of file and store in memory
        if (read == 0)
        {

            ESP_LOGD(WATCH2_TAG, "reading file %s...", watch2::file_path.c_str());

            filedata.clear();
            yoffset = 0;
            fs::File f = SD.open(filename.c_str());

            int chr;
            while(chr != -1)
            {
                chr = f.read();
                Serial.print(chr);
                if (chr != -1) filedata.push_back(chr);
            }
            f.seek(0);
            f.close();

            ESP_LOGD(WATCH2_TAG, "\nfinished");

        }

        if (dpad_up_active()) yoffset = std::max(yoffset - 10, 0);
        if (dpad_down_active()) yoffset += 10;

        //if any button is pressed, or the file has just been read, print the contents
        //of the file
        if (dpad_any_active() || read == 0)
        {
            watch2::oled.fillScreen(BLACK);
            watch2::oled.setCursor(0, yoffset);
            for (int chr : filedata) watch2::oled.print((char) chr);
        }

        //if left is pressed, go back to the state selection screen
        if (dpad_left_active()) 
        {
            watch2::switchState(-1);
        }

        if (read == 0) read = 1;

    }
    
}