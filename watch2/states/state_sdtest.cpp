#include "../watch2.h"
using namespace watch2;

void state_func_SDtest()
{
    static std::string filename;
    if (!state_init) 
    {
        filename = beginFileSelect();

        oled.setCursor(2, 42);
        oled.print(String(file_path.c_str()));
    }

    drawTopThing();

    if (dpad_left_active()) 
    {
        switchState(2);
    }
    
}