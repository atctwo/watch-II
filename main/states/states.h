/**
 * @file states.h
 * @author atctwo
 * @brief header file for all the state functions
 * @version 0.1
 * @date 2021-02-08
 * 
 * @copyright Copyright (c) 2021 atctwo
 * 
 */

#include "../watch2.h"
#include "../libraries/gumbo-parser/src/gumbo.h"

// state_alarms
void state_func_alarms();

// state_blackjack
void state_func_blackjack();

// state_bt_remote
void state_func_bt_remote();

// state_calc
void state_func_calc();

// state_image_viewer
void state_func_image_viewer();

// state_init
void state_func_init();

// state_ir_remote
void state_func_ir_remote();

// state_music_player
void state_func_music_player();

// state_nes
static void *read_nes_rom(const char *filename, size_t *out_len);
void state_func_nes();

// state_notepad
void state_func_notepad();

// state_quiz
cJSON *getQuestion(uint8_t category, uint8_t difficulty);
void getCategories(std::vector<std::string> &category_names, std::vector<uint8_t> &category_ids);
void state_func_quiz();

// state_radio
void state_func_radio();

// state_recorder
void state_func_recorder();

// state_sdtest
void state_func_SDtest();

// state_sensor
void state_func_sensor();

// state_settings
void state_func_settings();

// state_snek
void state_func_snek();

// state_state_menu
void state_func_state_menu();

// state_stopwatch
void state_func_stopwatch();

// tetris
void state_func_tetris();

// state_timer
void state_func_timer();

// state_watch_face
void state_func_watch_face();

// state_weather
void getLatLong(double &latitude, double &longitude);
cJSON *getForecast(double latitude, double longitude);
cJSON *getDailyArray(cJSON *forecast);
void state_func_weather();

// state_wiki
void buildPageLines(GumboNode *node, std::vector<const char*> tags={});
static std::string cleantext(GumboNode* node);
void state_func_wiki();