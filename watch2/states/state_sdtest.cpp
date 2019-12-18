#include "../globals.h"

void state_func_sdtest()
{
    if (file_path == "/") beginFileSelect("/");
    else
    {
        oled.setCursor(2, 42);
        oled.print(file_path);

        drawTopThing();

        if (dpad_left_active()) 
        {
            file_path = "/";
            switchState(2);
        }
    }
}