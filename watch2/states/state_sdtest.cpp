#include "../globals.h"

void state_func_sdtest()
{
    if (file_path == "/") beginFileSelect("/");
    else
    {
        oled.setCursor(2, 42);
        oled.print(String(file_path.c_str()));

        drawTopThing();

        if (dpad_left_active()) 
        {
            file_path = "/";
            switchState(2);
        }
    }
}