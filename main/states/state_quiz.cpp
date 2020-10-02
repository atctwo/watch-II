#include "../watch2.h"

extern "C"
{
    #include "../libraries/entities/entities.h"
}

cJSON *getQuestion(uint8_t category, uint8_t difficulty)
{
    static HTTPClient http;

    Serial.println("[Quiz] getting new question");

    // set up url
    char server[100];
    char category_string[12];
    const char *difficulty_string;

    if (category != 0)
    {
        sprintf(category_string, "&category=%d", category);
    }
    else sprintf(category_string, "");

    switch(difficulty)
    {
        default:
        case 0: // any
            difficulty_string = "";
            break;
        case 1: // easy
            difficulty_string = "&difficulty=easy";
            break;
        case 2: // medium
            difficulty_string = "&difficulty=medium";
            break;
        case 3: // hard
            difficulty_string = "&difficulty=hard";
            break;
    }

    sprintf(server, "https://opentdb.com/api.php?amount=1%s%s&type=multiple", category_string, difficulty_string);
    
    Serial.println("[Quiz] connecting to server");
    watch2::wifi_client_secure.setCACert(root_ca_open_trivia_db);
    if (http.begin(watch2::wifi_client_secure, server))
    {
        Serial.print("[Quiz] connected to server");
        int http_code = http.GET();
        Serial.printf("[Quiz] http code: %d (%s)\n", http_code, http.errorToString(http_code));

        if (http_code > 0)
        {
            if (http_code == HTTP_CODE_OK)
            {
                String res = http.getString();
                Serial.println("[Quiz] response:");
                Serial.println(res);
                return cJSON_Parse(res.c_str());
            }
        }
        else Serial.println("[Quiz] ???");
        
    }
    else
    {
        Serial.println("[Quiz] failed to connect");
    }

    Serial.println("[Quiz] finished");
    return NULL;
}

void getCategories(std::vector<std::string> &category_names, std::vector<uint8_t> &category_ids)
{
    static HTTPClient http;

    Serial.println("[Quiz] getting categories");
    watch2::wifi_client_secure.setCACert(root_ca_open_trivia_db);
    
    Serial.println("[Quiz] connecting to server");
    if (http.begin(watch2::wifi_client_secure, "https://opentdb.com/api_category.php"))
    {
        Serial.print("[Quiz] connected to server");
        int http_code = http.GET();
        Serial.printf("[Quiz] http code: %d (%s)\n", http_code, http.errorToString(http_code));

        if (http_code > 0)
        {
            if (http_code == HTTP_CODE_OK)
            {
                // get and print response
                String res = http.getString();
                Serial.println("[Quiz] response:");
                Serial.println(res);
                
                // parse response
                cJSON *response_object = cJSON_Parse(res.c_str());

                if (response_object)
                {
                    // get categories array
                    cJSON *categories_array = cJSON_GetObjectItem(response_object, "trivia_categories");
                    if (categories_array)
                    {
                        for (uint8_t i = 0; i < cJSON_GetArraySize(categories_array); i++)
                        {
                            cJSON *category_object = cJSON_GetArrayItem(categories_array, i);
                            if (category_object)
                            {
                                const char* name = cJSON_GetObjectItem(category_object, "name")->valuestring;
                                int id = cJSON_GetObjectItem(category_object, "id")->valueint;
                                category_names.push_back(name);
                                category_ids.push_back(id);
                                Serial.printf("[Quiz] added category %s (id %d) at array pos %d\n", 
                                    category_names[category_names.size()-1].c_str(), category_ids[category_ids.size()-1], i
                                );
                            }
                            else Serial.printf("[Quiz] couldn't get category %d\n", i);
                        }
                    }
                    else Serial.println("[Quiz] failed to get categories array");
                }
                else Serial.println("[Quiz] failed to parse categories json");

                // free memory used by response
                cJSON_Delete(response_object);
            }
        }
        else Serial.println("[Quiz] ???");
        
    }
    else
    {
        Serial.println("[Quiz] failed to connect");
    }

    Serial.println("[Quiz] finished");
}

void state_func_quiz()
{
    static std::vector<std::string> category_names = {"Any Category"};
    static std::vector<uint8_t> category_ids = {0};
    static std::vector<std::string> difficulty_names = {"Any Difficulty", "Easy", "Medium", "Hard"};

    static uint8_t selected_menu_item = 2;
    static uint8_t selected_category = 0;
    static uint8_t selected_difficulty = 0;

    static uint16_t menu_y = 0;
    static uint16_t parameter_menu_y = 0;
    static uint8_t parameter_selection = 0;  // 0 means category selection, and 1 means difficulty selection
    static bool retrieved_categories = false;

    static uint8_t *selection;
    static std::vector<std::string> *parameter_options;

    static char temp_text[300] = "";
    static std::string current_question = "";
    static std::vector<std::string> answers = {"", "", "", ""};
    static uint8_t correct_answer = 0;
    static uint8_t selected_answer = 0;
    static bool answered = false;
    static bool answered_but_we_have_only_just_found_out = false;
    static uint16_t score = 0;

    switch(watch2::states[watch2::state].variant)
    {

        case 0: // menu

            if (!watch2::state_init)
            {
                // draw title
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::setFont(LARGE_FONT);
                watch2::oled.setTextDatum(TC_DATUM);
                watch2::oled.drawString("Quiz Game", SCREEN_WIDTH / 2, watch2::top_thing_height);
                watch2::oled.setTextDatum(TL_DATUM);
                menu_y = watch2::top_thing_height + watch2::oled.fontHeight();
                watch2::setFont(MAIN_FONT);

                if (watch2::wifi_state == 3)
                {
                    // get categories
                    if (!retrieved_categories) 
                    {
                        getCategories(category_names, category_ids);
                        retrieved_categories = true;
                    }
                }

                // reset score
                score = 0;
            }

            // update menu selection
            if (dpad_up_active())
            {
                if (selected_menu_item == 0) selected_menu_item = 2;
                else selected_menu_item--;
            }
            if (dpad_down_active())
            {
                if (selected_menu_item == 2) selected_menu_item = 0;
                else selected_menu_item++;
            }

            // draw menu
            draw(dpad_any_active(), {

                if (watch2::wifi_state == 3)
                {
                    watch2::drawMenu(
                        4, menu_y, SCREEN_WIDTH - 8, SCREEN_HEIGHT - menu_y, 
                        { category_names[selected_category], difficulty_names[selected_difficulty], "Start" },
                        selected_menu_item, false, true
                    );
                }
                else
                {
                    watch2::oled.setCursor(0, menu_y);
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.println("Please connect to wifi\n"
                                         "to use this app.  You can\n"
                                         "press enter to leave."
                    );
                }

            });

            // handle enter button
            if (dpad_enter_active())
            {
                switch(selected_menu_item)
                {
                    case 0: // category selection
                    case 1: // difficulty selection
                        if (watch2::wifi_state == 3)
                        {
                            parameter_selection = selected_menu_item;
                            watch2::switchState(watch2::state, 1);
                        }
                        else watch2::switchState(2);
                        break;

                    case 2: // quiz
                        if (watch2::wifi_state == 3) watch2::switchState(watch2::state, 2);
                        else watch2::switchState(2);
                        break;
                }
            }

            // return to state menu
            if (dpad_left_active())
            {
                watch2::switchState(2);
            }

            break;

        case 1: // parameter selection

            if (!watch2::state_init)
            {
                // draw title
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.print( parameter_selection ? "Difficulty" : "Category" );
                parameter_menu_y = watch2::top_thing_height + watch2::oled.fontHeight();

                if (parameter_selection == 0) 
                {
                    selection = &selected_category;
                    parameter_options = &category_names;
                }
                else
                {
                    selection = &selected_difficulty;
                    parameter_options = &difficulty_names;
                }
            }

            // handle changes to selected things
            if (dpad_up_active())
            {
                Serial.printf("[Quiz] selected thing: %d\n", *selection);
                Serial.printf("[Quiz] number of things: %d\n", parameter_options->size());
                if (*selection == 0) *selection = parameter_options->size() - 1;
                else (*selection)--;
            }
            if (dpad_down_active())
            {
                Serial.printf("[Quiz] selected thing: %d\n", *selection);
                Serial.printf("[Quiz] number of things: %d\n", parameter_options->size());
                if (*selection == parameter_options->size() - 1) *selection = 0;
                else (*selection)++;
            }

            // draw menu
            draw(dpad_any_active(), {

                watch2::drawMenu(
                    2, parameter_menu_y, SCREEN_WIDTH - 4, SCREEN_HEIGHT - parameter_menu_y, 
                    *parameter_options, *selection
                );

            });

            // return to menu
            if (dpad_enter_active()) watch2::switchState(watch2::state, 0);

            break;

        case 2: // quiz

            if (!watch2::state_init)
            {
                // reset things
                selected_answer = 0;
                answered = false;
                answered_but_we_have_only_just_found_out = false;

                // get a question
                cJSON *response_object = getQuestion(category_ids[selected_category], selected_difficulty);

                if (response_object)
                {
                    // get response code
                    int response_code = cJSON_GetObjectItem(response_object, "response_code")->valueint;
                    
                    if (response_code == 0) // success
                    {
                        // get results object
                        cJSON *results_object = cJSON_GetObjectItem(response_object, "results");
                        if (results_object)
                        {
                            // get question
                            cJSON *question_object = cJSON_GetArrayItem(results_object, 0);
                            if (question_object)
                            {
                                Serial.println("[Quiz] got question object");

                                // get actual question
                                //current_question = cJSON_GetObjectItem(question_object, "question")->valuestring;
                                decode_html_entities_utf8(temp_text, cJSON_GetObjectItem(question_object, "question")->valuestring);
                                current_question = temp_text;

                                // get answers
                                decode_html_entities_utf8(temp_text, cJSON_GetObjectItem(question_object, "correct_answer")->valuestring);
                                answers[0] = temp_text;
                                cJSON *incorrect_answers = cJSON_GetObjectItem(question_object, "incorrect_answers");
                                if (incorrect_answers)
                                {
                                    for (uint8_t i = 0; i < 3; i++)
                                    {
                                        decode_html_entities_utf8(temp_text, cJSON_GetArrayItem(incorrect_answers, i)->valuestring);
                                        answers[i+1] = temp_text;
                                    }
                                }
                                else Serial.println("[Quiz] couldn't get incorrect answers");

                                // store correct answer
                                decode_html_entities_utf8(temp_text, cJSON_GetObjectItem(question_object, "correct_answer")->valuestring);

                                // shuffle answers
                                std::srand(time(0));
                                std::random_shuffle(answers.begin(), answers.end());

                            }
                            else Serial.println("[Quiz] couldn't get question");
                        }
                        else Serial.println("[Quiz] couldn't get results object");

                    }
                    else if (response_code == 1) // no results
                    {
                        Serial.println("[Quiz] no results were returned");
                        watch2::oled.setCursor(2, watch2::top_thing_height);
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.println("Error 1\nno results were returned");
                    }
                    else if (response_code == 2) // invalid parameter
                    {
                        Serial.println("[Quiz] invalid parameter");
                        watch2::oled.setCursor(2, watch2::top_thing_height);
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.println("Error 2\ninvalid parameter");
                    }
                    else if (response_code == 3) // token not found
                    {
                        Serial.println("[Quiz] session token doesn't exist");
                        watch2::oled.setCursor(2, watch2::top_thing_height);
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.println("Error 3\ntoken not found");
                    }
                    else if (response_code == 4) // token empty
                    {
                        Serial.println("[Quiz] session token has returned all possible questions for the query");
                        watch2::oled.setCursor(2, watch2::top_thing_height);
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.println("Error 4\ntoken empty");
                    }   
                    
                }
                else Serial.println("[Quiz] failed to get response");

                // free the space used by the question
                cJSON_Delete(response_object);
            }

            // update selected answer
            if (dpad_up_active())
            {
                if (selected_answer == 0) selected_answer = answers.size() - 1;
                else selected_answer--;
            }
            if (dpad_down_active())
            {
                if (selected_answer == answers.size() - 1) selected_answer = 0;
                else selected_answer++;
            }

            if (dpad_enter_active()) // get a new question
            {
                if (!answered) 
                {
                    answered_but_we_have_only_just_found_out = true;
                    if (strcmp(temp_text, answers[selected_answer].c_str()) == 0) score++;
                }
            }

            draw(dpad_any_active(), {

                // print question
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.println(current_question.c_str());

                // print answers
                for (uint8_t i = 0; i < answers.size(); i++)
                {
                    if (answered || answered_but_we_have_only_just_found_out)
                    {
                        if (strcmp(temp_text, answers[i].c_str()) == 0) watch2::oled.setTextColor(TFT_GREEN, BLACK);
                        else 
                        {
                            if (selected_answer == i) watch2::oled.setTextColor(RED, BLACK);
                            else watch2::oled.setTextColor(WHITE, BLACK);
                        }
                    }
                    else
                    {
                        if (selected_answer == i) watch2::oled.setTextColor(watch2::themecolour, BLACK);
                        else                      watch2::oled.setTextColor(WHITE, BLACK);
                    }
                    
                    watch2::oled.printf("        %s\n", answers[i].c_str());
                }

            });

            if (dpad_enter_active()) // get a new question
            {
                if (answered)  watch2::switchState(watch2::state, 2);
            }
            if (dpad_left_active()) // return to the menu
            {
                score = 0;
                watch2::switchState(watch2::state, 0);
            }

            if (answered_but_we_have_only_just_found_out) answered = true;

            break;

    }

    watch2::drawTopThing();
}