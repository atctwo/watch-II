#include "../src/watch2.h"
#include "state_init.cpp"
#include "state_watch_face.cpp"
#include "state_state_menu.cpp"
#include "state_settings.cpp"
#include "state_calc.cpp"
#include "state_stopwatch.cpp"
#include "state_timer.cpp"
#include "state_alarms.cpp"
#include "state_SDtest.cpp"
#include "state_notepad.cpp"
#include "state_ir_remote.cpp"

namespace watch2
{

    std::vector<stateMeta> states = {

        stateMeta("Initial State", state_func_init, "init", 0, true),
        stateMeta("Watch Face", state_func_watch_face, "watch"),
        stateMeta("State Menu", state_func_state_menu, "menu", 0, true),
        stateMeta("Settings", state_func_settings, "settings"),

        stateMeta("Calculator", state_func_calc, "calculator"),
        stateMeta("Stopwatch", state_func_stopwatch, "stopwatch"),
        stateMeta("Timer", state_func_timer, "timer"),
        stateMeta("Alarms", state_func_alarms, "alarms"),
        stateMeta("SD test", state_func_SDtest),
        stateMeta("Notepad", state_func_notepad, "notepad"),
        stateMeta("IR remote", state_func_ir_remote, "ir remote")

    };

}
