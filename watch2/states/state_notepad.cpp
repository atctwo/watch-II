#include "../watch2.h"
using namespace watch2;

void state_func_notepad()
{
    static int read = 0;                    //whether or not the selected file has been copied into memory
    static int yoffset = 0;                 //the offset to draw the file contents at
    static std::vector<int> filedata;       //a vector to store the file data
    static std::string filename;            //the name of the file to read
    
    if (!state_init) 
    {
        filename = beginFileSelect("/");
        read = 0;
    }

    if (file_path == "canceled")
    {
        switchState(2);
    }
    else
    {
    
        //read contents of file and store in memory
        if (read == 0)
        {

            Serial.printf("reading file %s...\n", file_path.c_str());

            filedata.clear();
            yoffset = 0;
            File f = SD.open(filename.c_str());

            int chr;
            while(chr != -1)
            {
                chr = f.read();
                Serial.print(chr);
                if (chr != -1) filedata.push_back(chr);
            }
            f.rewind();
            f.close();

            Serial.println("\nfinished");

        }

        if (dpad_up_active()) yoffset = std::max(yoffset - 10, 0);
        if (dpad_down_active()) yoffset += 10;

        //if any button is pressed, or the file has just been read, print the contents
        //of the file
        if (dpad_any_active() || read == 0)
        {
            oled.fillScreen(BLACK);
            oled.setCursor(0, 8 - yoffset);
            for (int chr : filedata) oled.print((char) chr);
        }

        //if left is pressed, go back to the state selection screen
        if (dpad_left_active()) 
        {
            switchState(2);
        }

        if (read == 0) read = 1;

    }
    
}