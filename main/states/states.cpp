#include "../watch2.h"
#include "state_init.cpp"
#include "state_watch_face.cpp"
#include "state_state_menu.cpp"
#include "state_settings.cpp"
// #include "state_calc.cpp"
// #include "state_stopwatch.cpp"
// #include "state_timer.cpp"
// #include "state_alarms.cpp"
// #include "state_sdtest.cpp"
// #include "state_notepad.cpp"
// #include "state_ir_remote.cpp"
// #include "state_image_viewer.cpp"
// #include "state_wiki.cpp"
// #include "state_quiz.cpp"
#include "state_nes.h"
// #include "state_music_player.cpp"
// #include "state_tetris.cpp"
// #include "state_weather.cpp"
// #include "state_radio.cpp"
// #include "state_bt_remote.cpp"
// #include "state_recorder.cpp"
// #include "state_cube.cpp"

namespace watch2
{

    std::vector<stateMeta> states = {

        stateMeta("Initial State", state_func_init, "init.bmp", 0, false, true),
        stateMeta("Watch Face", state_func_watch_face, "watch"),
        stateMeta("State Menu", state_func_state_menu, "menu", 0, false, true),
        stateMeta("Settings", state_func_settings, "settings"),

        // stateMeta("Calculator", state_func_calc, "calculator"),
        // stateMeta("Stopwatch", state_func_stopwatch, "stopwatch"),
        // stateMeta("Timer", state_func_timer, "timer"),
        // stateMeta("Alarms", state_func_alarms, "alarms"),
        // stateMeta("File Browser", state_func_SDtest, "file_browser"),
        // stateMeta("Notepad", state_func_notepad, "notepad"),
        // stateMeta("IR remote", state_func_ir_remote, "ir_remote"),
        // stateMeta("Image Viewer", state_func_image_viewer, "image_viewer"),
        // stateMeta("Weather", state_func_weather, "weather"),
        // stateMeta("Wikipedia", state_func_wiki, "wikipedia"),
        // stateMeta("Quiz Thingy", state_func_quiz, "quiz"),
        // stateMeta("Music Player", state_func_music_player, "music_player"),
        // stateMeta("Radio", state_func_radio, "radio"),
        // stateMeta("Recorder", state_func_recorder),
        // stateMeta("Bluetooth Remote", state_func_bt_remote, "ble_remote"),
        // stateMeta("Tetris", state_func_tetris, "ltris"),
        stateMeta("NES", state_func_nes, "nes")
        //stateMeta("c u b e", state_func_cube)

    };

}
