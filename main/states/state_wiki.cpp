/*

    watch 2 wikipedia app

    this is really buggy at the minute.  all of the errors seem to stem from issues
    that HTTPCient has when connecting to Wikipedia over HTTPS (the issues are dpad_up_active
    to the server rejecting connections, supposedly).

*/

#include "states.h"
#include "../libraries/gumbo-parser/src/gumbo.h"

struct article_lines{

    std::string                 text;      // the contents of the line
    std::vector<const char*>    styles;     // tags that define how the line should be styled

};

// adapted from function below
void buildPageLines(GumboNode *node, std::vector<const char*> tags)
{
    if (node->type == GUMBO_NODE_TEXT)
    {
        ESP_LOGD(WATCH2_TAG, "text: %s", node->v.text.text);
        if (!tags.empty()) for (const char* tag : tags) ESP_LOGD(WATCH2_TAG, "tags: %s, ", tag);
        ESP_LOGD(WATCH2_TAG, "\n");
    }
    else if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag != GUMBO_TAG_SCRIPT && node->v.element.tag != GUMBO_TAG_STYLE)
    {
        GumboVector *children = &node->v.element.children;
        for (uint32_t i = 0; i < children->length; i++)
        {
            buildPageLines((GumboNode*) children->data[i]);
        }
    }
}

// from https://github.com/google/gumbo-parser/blob/master/examples/clean_text.cc
static std::string cleantext(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return std::string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      const std::string text = cleantext((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }
    return contents;
  } else {
    return "";
  }
}

void state_func_wiki()
{
    static std::string search_query = "";
    static uint32_t pageid = 0;
    static bool connected_to_wikipeda = false;

    switch(watch2::states[watch2::state].variant)
    {

        case 0: // menu

            static std::vector<std::string> wiki_menu_options = {"Search Wikipedia", "Exit"};
            static uint8_t wiki_selected_option = 0;
            static HTTPClient http;

            if (dpad_up_active() || dpad_down_active())
            {
                if (wiki_selected_option == 0) wiki_selected_option = 1;
                else wiki_selected_option = 0;
            }

            if (!watch2::state_init && !connected_to_wikipeda)
            {
                // ESP_LOGD(WATCH2_TAG, "[Wiki] connecting to wikipedia");
                // watch2::wifi_client_secure.setCACert(root_ca_wikipedia);
                // if (!watch2::wifi_client_secure.connect("en.wikipedia.org", PORT_HTTPS))
                // {
                //     ESP_LOGD(WATCH2_TAG, "[Wiki] couldn't connect");
                // }
                // else connected_to_wikipeda = true;
                connected_to_wikipeda = true;
            }

            draw(dpad_any_active(), {
                watch2::drawImage((*watch2::icons)["wikipedia_text"], 0, watch2::top_thing_height);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setTextDatum(TC_DATUM);
                watch2::oled.drawString("The Free Encyclopedia", SCREEN_WIDTH / 2, watch2::top_thing_height + 40);
                watch2::oled.setTextDatum(TL_DATUM);

                uint16_t menu_y = watch2::top_thing_height + 40 + watch2::oled.fontHeight() + 10;
                if (watch2::wifi_state != 3)
                {
                    watch2::oled.setCursor(1, menu_y);
                    watch2::oled.print("Not connected to Wifi\nPress enter to go back\nto the app menu");
                }
                else if (!connected_to_wikipeda)
                {
                    watch2::oled.setCursor(1, menu_y);
                    watch2::oled.print("Could not connect\nto Wikipedia's\nservers.  Press\nenter to go back\nto the state menu");
                }
                else 
                {
                    watch2::drawMenu(2, menu_y, SCREEN_WIDTH - 4, SCREEN_HEIGHT - menu_y, wiki_menu_options, wiki_selected_option, {}, false);
                    watch2::oled.setCursor(0, menu_y + 24 + (watch2::oled.fontHeight() * 2));
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.print("Work in progress\n_very_ buggy");
                }
                
                watch2::forceRedraw = false;
            });

            if (dpad_enter_active())
            {
                if (wiki_selected_option == 1 || watch2::wifi_state != 3 || !connected_to_wikipeda) // exit
                {
                    // go back to the state menu
                    watch2::switchState(-1);
                }
                else if (wiki_selected_option == 0) // Search
                {
                    search_query = watch2::textFieldDialogue("Search Query", search_query.c_str());
                    if (!search_query.empty()) watch2::switchState(watch2::state, 1); // go to search results
                }
            }

            break;

        case 1: // search results

            static std::vector<std::string> wiki_search_options = {"Back"};
            static uint8_t wiki_selected_search_result = 0;
            static uint16_t request_status = 102; // processing
            static std::string flavour_text = "";
            static uint16_t flavour_h = watch2::oled.fontHeight() * 4;
            static uint16_t flavour_y = SCREEN_WIDTH - flavour_h;
            static cJSON *search_response;
            static cJSON *search_query_obj;
            static cJSON *search_search;

            if (dpad_up_active())
            {
                if (wiki_selected_search_result == 0) wiki_selected_search_result = wiki_search_options.size() - 1;
                else wiki_selected_search_result--;
            }

            if (dpad_down_active())
            {
                if (wiki_selected_search_result == wiki_search_options.size() - 1) wiki_selected_search_result = 0;
                else wiki_selected_search_result++;
            }

            if (!watch2::state_init)
            {
                // actually search

                // this check is required so that the program doesn't reperform the search every time this state is switched in to.
                // we only want it to search if it is being switched into from the menu, but not if it's from the page variant.  When we
                // leave this variant to go back to the menu, we set request_status to 102 (processing), so when this variant is switched
                // back into, the following if statement evaluates as true, and the search is performed.  we don't reset the request_status
                // when switching to the page variant, because that means when we switch from the page variant to this variant, the
                // request_status will still be HTTP_CODE_OK, and the program won't resend the search.
                if (request_status != HTTP_CODE_OK)
                {
                    // set up search results vector
                    wiki_search_options.clear();
                    wiki_search_options.push_back("Back");

                    ESP_LOGD(WATCH2_TAG, "[Wiki] sending search for ");
                    ESP_LOGD(WATCH2_TAG, "%s", search_query.c_str());

                    //https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=" + String(search_query.c_str()) + "&format=json
                    watch2::wifi_client_secure.setCACert(root_ca_wikipedia);
                    if(http.begin(watch2::wifi_client_secure, "https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=" + String(search_query.c_str()) + "&format=json"))
                    {
                        ESP_LOGD(WATCH2_TAG, "[Wiki] connected to wikipedia");
                        int http_code= http.GET();

                        if (http_code > 0)
                        {
                            ESP_LOGD(WATCH2_TAG, "[Wiki] got response");
                            ESP_LOGD(WATCH2_TAG, "       response code: %d", http_code);
                            if (http_code == HTTP_CODE_OK)
                            {
                                search_response = cJSON_Parse(http.getString().c_str());
                                search_query_obj = cJSON_GetObjectItem(search_response, "query");
                                search_search = cJSON_GetObjectItem(search_query_obj, "search");

                                // add results to menu
                                for (int i = 0; i < cJSON_GetArraySize(search_search); i++)
                                {
                                    cJSON *search_result = cJSON_GetArrayItem(search_search, i);
                                    wiki_search_options.push_back(cJSON_GetObjectItem(search_result, "title")->valuestring);
                                }

                            }
                        }
                        else ESP_LOGD(WATCH2_TAG, "[Wiki] GET failed, error: %s", http.errorToString(http_code).c_str());

                        request_status = http_code;
                        http.end();
                    }
                    else ESP_LOGW(WATCH2_TAG, "[Wiki] unable to connect to wikipedia");
                }
            }

            draw(dpad_any_active(), {
                // set flavour text
                if (wiki_selected_search_result == 0) flavour_text = "";
                else {
                    if (search_search)
                    {
                        // get search result
                        cJSON *result = cJSON_GetArrayItem(search_search, wiki_selected_search_result - 1);
                        
                        // get article snippet
                        const char* snippet = cJSON_GetObjectItem(result, "snippet")->valuestring;

                        // parse snippet
                        GumboOutput *snippet_html = gumbo_parse(snippet);

                        // save cleaned snippet
                        flavour_text = cleantext(snippet_html->root);

                        // free up memory taken by html
                        gumbo_destroy_output(&kGumboDefaultOptions, snippet_html);
                    }
                }

                // draw title
                std::string thing = "Results for " + search_query;
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.print(thing.c_str());

                watch2::drawMenu(
                    2, watch2::top_thing_height + watch2::oled.fontHeight(), 
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - watch2::oled.fontHeight() - watch2::top_thing_height - flavour_h, 
                    wiki_search_options, wiki_selected_search_result
                );

                // draw response
                if (request_status != HTTP_CODE_OK)
                {
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.setCursor(0, SCREEN_HEIGHT * 0.4);
                    watch2::oled.print("Request failed, error:\n");
                    watch2::oled.print(http.errorToString(request_status).c_str());
                }

                // draw flavour text
                if (request_status == HTTP_CODE_OK)
                {
                    watch2::oled.fillRect(0, flavour_y, SCREEN_WIDTH, flavour_h, BLACK);
                    watch2::oled.drawFastHLine(0, flavour_y, SCREEN_WIDTH, watch2::themecolour);
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.setCursor(0, flavour_y + 2);
                    watch2::oled.print(flavour_text.c_str());
                }
                
                watch2::forceRedraw = false;
            });

            if (dpad_enter_active())
            {
                if (wiki_selected_search_result == 0)
                {
                    // go back to menu
                    watch2::switchState(watch2::state, 0);
                    if (request_status == HTTP_CODE_OK) cJSON_Delete(search_response);
                    request_status = 102;
                }
                else
                {
                    // get search result
                    cJSON *result = cJSON_GetArrayItem(search_search, wiki_selected_search_result - 1);
                    
                    // get page id
                    pageid = cJSON_GetObjectItem(result, "pageid")->valueint;

                    // go to page
                    watch2::switchState(watch2::state, 3);
                }
            }

            break;

        case 2: // contents
            break;

        case 3: // page

            static uint16_t page_request_status = 102;
            static std::vector<article_lines> lines;
            static uint16_t line_offset = 0;
            static uint16_t total_lines = 0;
            static cJSON *page_response;
            static cJSON *page_query;
            static cJSON *page_pages;
            static cJSON *page_object;

            if (!watch2::state_init)
            {
                line_offset = 0;

                // actually get the page
                ESP_LOGD(WATCH2_TAG, "[Wiki] getting page with id ");
                ESP_LOGD(WATCH2_TAG, "%lu", pageid);

                //https://en.wikipedia.org/w/api.php?action=query&list=search&srsearch=" + String(search_query.c_str()) + "&format=json
                watch2::wifi_client_secure.setCACert(root_ca_wikipedia);
                String url = "https://en.wikipedia.org/w/api.php?action=query&format=json&prop=extracts&formatversion=2&explaintext=1&pageids=" + String(pageid);
                ESP_LOGD(WATCH2_TAG, "[Wiki] request url: ");
                ESP_LOGD(WATCH2_TAG, "%s", url.c_str());
                if(http.begin(watch2::wifi_client_secure, url))
                {
                    ESP_LOGD(WATCH2_TAG, "[Wiki] connected to wikipedia");
                    int http_code= http.GET();

                    if (http_code > 0)
                    {
                        ESP_LOGD(WATCH2_TAG, "[Wiki] got response");
                        ESP_LOGD(WATCH2_TAG, "       response code: %d", http_code);
                        if (http_code == HTTP_CODE_OK)
                        {
                            // get article contents
                            String page_response_json = http.getString();
                            ESP_LOGD(WATCH2_TAG, "[Wiki] json response: ");
                            ESP_LOGD(WATCH2_TAG, "%s", page_response_json.c_str());

                            page_response = cJSON_Parse(page_response_json.c_str());
                            page_query = cJSON_GetObjectItem(page_response, "query");
                            page_pages = cJSON_GetObjectItem(page_query, "pages");
                            page_object = cJSON_GetArrayItem(page_pages, 0);

                            if (page_object)
                            {
                                // parse article
                                char *article_text = cJSON_GetObjectItem(page_object, "extract")->valuestring;
                                // ESP_LOGD(WATCH2_TAG, "[Wiki] successful response:\n");
                                // ESP_LOGD(WATCH2_TAG, "%*", article_text);
                                // ESP_LOGD(WATCH2_TAG, "%*", );
                                
                                // split article into lines
                                lines.clear();
                                lines.push_back( (article_lines) {"", {""}} );
                                char *pch;
                                uint16_t line_width = 0;
                                uint16_t word_width = 0;
                                uint16_t current_line = 0;
                                pch = strtok(article_text, " ");
                                while(pch != NULL)
                                {
                                    word_width = watch2::oled.textWidth(pch);
                                    if ((line_width + word_width) > (SCREEN_WIDTH * 0.85))
                                    {
                                        // word wrap
                                        // remove newlines from current line
                                        lines[current_line].text.erase(std::remove(lines[current_line].text.begin(), lines[current_line].text.end(), '\n'), lines[current_line].text.end());

                                        // add new line
                                        lines.push_back( (article_lines) {"", {""}} );

                                        // increment line counter
                                        current_line++;
                                        total_lines++;

                                        // add word to new line
                                        lines[current_line].text.append(pch);
                                        lines[current_line].text.append(" ");

                                        // add word width to line width
                                        line_width = word_width;
                                    }
                                    else
                                    {
                                        // word can be added to the current line without going over the line
                                        line_width += word_width;
                                        lines[current_line].text.append(pch);
                                        lines[current_line].text.append(" "); // add space to line
                                    }
                                    pch = strtok(NULL, " ");
                                }
                            }
                            else
                            {
                                ESP_LOGD(WATCH2_TAG, "[Wiki] error parsing response");
                                lines.clear();
                                lines.push_back((article_lines){"error parsing response", {}});
                            }
                        }
                        else
                        {
                            ESP_LOGD(WATCH2_TAG, "[Wiki] Request not ok, error: %s", http.errorToString(http_code).c_str());
                            lines.clear();
                            lines.push_back((article_lines){"Request not ok, error:", {}});
                            lines.push_back((article_lines){http.errorToString(http_code).c_str(), {}});
                        }
                    }
                    else 
                    {
                        ESP_LOGD(WATCH2_TAG, "[Wiki] GET failed, error: %s", http.errorToString(http_code).c_str());
                        lines.clear();
                        lines.push_back((article_lines){"GET failed, error:", {}});
                        lines.push_back((article_lines){http.errorToString(http_code).c_str(), {}});
                    }

                    page_request_status = http_code;
                    http.end();
                }
                else 
                {
                    ESP_LOGW(WATCH2_TAG, "[Wiki] unable to connect to wikipedia");
                    lines.clear();
                    lines.push_back((article_lines){"unable to connect to wikipedia", {}});
                }
                
            }

            if (dpad_up_active())
            {
                if ((line_offset - 1) > 0) line_offset--;
            }

            if (dpad_down_active())
            {
                if ((line_offset + 1) < total_lines) line_offset++;
            }

            draw(dpad_any_active(), {
                // ???
                uint16_t lines_that_can_fit_on_screen_at_one_time = (SCREEN_HEIGHT / watch2::oled.fontHeight());
                uint16_t text_height = 0;

                // print lines
                watch2::oled.fillScreen(0);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);
                for (uint16_t i = line_offset; i < (line_offset + lines_that_can_fit_on_screen_at_one_time); i++)
                {
                    if (i < lines.size()) // if i refers to a line that actually exists
                    if (text_height < (SCREEN_HEIGHT))
                    {
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.println(lines[i].text.c_str());
                        text_height += watch2::oled.fontHeight();
                    }
                }
            });

            if (dpad_left_active())
            {
                // go back to search page
                page_request_status = HTTP_CODE_PROCESSING;
                watch2::switchState(watch2::state, 1);
            }

            break;
    }

    watch2::drawTopThing();

}