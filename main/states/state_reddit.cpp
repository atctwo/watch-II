
#include "states.h"
#include "base64.h"

enum reddit_post_types {

    REDDIT_POST_UNKNOWN,
    REDDIT_POST_TEXT,
    REDDIT_POST_IMAGE,
    REDDIT_POST_LINK

};

struct reddit_post {

    std::string id;                     // the id of the post
    reddit_post_types post_type;        // the type of post
    std::string title;                  // the title of the post
    std::string selftext;               // the text / main body of the post
    std::string author;                 // the author of the post
    std::string subreddit;              // the subreddit the post was posted in
    std::string url;                    // the url of the post if it is an image or link post
    uint32_t score;                     // the current score of the post
    float upvote_ratio;                 // the ratio of upvotes to downvotes
    uint32_t creation;                  // the date + time the post was created
    bool nsfw;                          // is the post nsfw?
    bool spoiler;                       // is the post a spoiler?
    bool locked;                        // is the post locked?

};

bool got_access_token = false;      // have we got an access token?
std::string access_token = "";      // the access token provided by reddit
uint32_t access_token_exp = 0;      // when the access token will expire
HTTPClient http;

// main menu
uint8_t menu_selected_item = 0;
std::vector<std::string> menu_items = {"Your Feed", "A Subreddit", "Exit"};
uint16_t menu_y = 0;

// post browser
bool subreddit_feed = false;    // if this is true, get the "best" posts instead of ones from a specific subreddit
std::string subreddit_name;     // the name of the subreddit to get posts from
std::string post_sort = "hot";  // how to sort the posts ("hot", "new", "top", "rising")
std::vector<reddit_post> retrieved_posts; // a store of retrived posts
uint8_t selected_post = 0;

// post viewer
reddit_post *viewing_post;
uint8_t viewer_page = 0;
uint8_t last_viewer_page = 255;
uint8_t viewer_post_scroll = 0;
uint8_t viewer_comments_scroll = 0;

// reddit authentication details
// TODO move these to api_keys.json
std::string reddit_username;
std::string reddit_password;
std::string reddit_app_id = "uiWicUkYCzMFSw";
std::string reddit_app_secret = "FWshU_DE-Mk24O93JoddQ7JWMkTisQ";

std::string getAccessToken()
{
    ESP_LOGI(WATCH2_TAG, "[reddit] getting access token");

    if (millis() > access_token_exp)
    {
        ESP_LOGI(WATCH2_TAG, "[reddit] access token expired or doesn't exist, getting new one");
        watch2::wifi_client_secure.setCACert(root_ca_reddit);
        if (http.begin(watch2::wifi_client_secure, "https://www.reddit.com/api/v1/access_token"))
        {
            ESP_LOGD(WATCH2_TAG, "[redit] connected to server");

            String auth = base64::encode(std::string(reddit_app_id + ":" + reddit_app_secret).c_str());
            http.addHeader("Authorization", "Basic " + auth);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            http.setUserAgent("watch2");

            int http_code = http.POST("grant_type=password&username=atctwo&password=Edt6d42!^");
            ESP_LOGI(WATCH2_TAG, "[reddit] http code: %d (%s)", http_code, http.errorToString(http_code));

            if (http_code > 0)
            {
                if (http_code == HTTP_CODE_OK)
                {
                    String res = http.getString();
                    ESP_LOGI(WATCH2_TAG, "response: %s", res.c_str());

                    cJSON *token = cJSON_Parse(res.c_str());
                    access_token = cJSON_GetObjectItem(token, "access_token")->valuestring;
                    access_token_exp = (cJSON_GetObjectItem(token, "expires_in")->valueint * 60) + millis();
                    got_access_token = true;
                    cJSON_free(token);

                    return access_token;
                }
            }
            else ESP_LOGD(WATCH2_TAG, "[reddit] ???");
            
        }
        else
        {
            ESP_LOGW(WATCH2_TAG, "[reddit] failed to connect");
        }

        return "";

    }
    else
    {
        ESP_LOGI(WATCH2_TAG, "[reddit] access token already exists :)");
        return access_token;
    }
}



cJSON *sendAPIgetRequest(std::string endpoint)
{
    ESP_LOGI(WATCH2_TAG, "[reddit] sending api request: %s", endpoint.c_str());

    char *page = (char *) ps_malloc(sizeof(char));
    int lengthHtml = 0;

    watch2::wifi_client_secure.setCACert(root_ca_reddit);
    if (http.begin(watch2::wifi_client_secure, std::string("https://oauth.reddit.com" + endpoint).c_str()))
    {
        ESP_LOGD(WATCH2_TAG, "[redit] connected to server");

        http.addHeader("Authorization", String(std::string("bearer " + getAccessToken()).c_str()));
        http.setUserAgent("watch2");

        int http_code = http.GET();
        ESP_LOGI(WATCH2_TAG, "[reddit] http code: %d (%s)", http_code, http.errorToString(http_code));

        Serial.println("headers:");
        for (int i = 0; i < http.headers(); i++)
        {
            Serial.printf("\t(%d) %s: %s\n", i, http.headerName(i).c_str(), http.header(i).c_str());
        }

        if (http_code > 0)
        {
            if (http_code == HTTP_CODE_OK)
            {

                // response processing code taken from https://learn.upesy.com/en/programmation/psram.html

                // Essaye de récupérer la taille du ficher ( -1 s'il n'y a pas de Content-Length dans l'header)
                int tempLength = http.getSize();
                Serial.println((String) "Content Length :" + tempLength);

                //Stockage de la page dans la PSRAM
                Serial.printf("Adresse mémoire de la page : %p \n", page);

                // Récupère le stream TCP
                WiFiClient *stream = http.getStreamPtr();

                //Initialisation position du buffer dans la PSRAM
                int position = 0;

                uint32_t currentTime = 0;
                uint8_t timeoutArboted = 1;

                // Récupère toute les données du fichier
                while(http.connected() && (tempLength > 0 || tempLength == -1))
                {

                    // Récupère les données disponibles (les données arrivent packets par packets)
                    size_t size = stream->available();
                    if (size){
                        page = (char*) ps_realloc(page, position + size + 1);
                        stream->readBytes(page+position, size);
                        position += size;

                        if (tempLength > 0){
                                tempLength -= size;
                        }

                        timeoutArboted = 1;
                        }else{
                                //Si on ne connaît pas la taille du fichier, on suppose que toutes les données sont recues avant le timeout
                                if(timeoutArboted){
                                        //Lance le timer
                                        currentTime = millis();
                                        timeoutArboted = 0;
                                }else{
                                        if(millis()-currentTime > 5000){
                                                //On a atteind le Timeout
                                                Serial.println("Timeout reached");
                                                break;
                                        }
                                }
                        }
                }

                *(page+position) = '\0';
                lengthHtml = position;
                Serial.println((String)"Downloaded " + lengthHtml + " Octets");
                
                cJSON *response = cJSON_Parse(page);
                free(page);
                return response;
            }
        }
        else ESP_LOGD(WATCH2_TAG, "[reddit] ???");
        
    }
    else
    {
        ESP_LOGW(WATCH2_TAG, "[reddit] failed to connect");
    }

    free(page);
    return NULL;

}

watch2::imageData getImageFromURL(const char *url)
{
    ESP_LOGI(WATCH2_TAG, "getting image from %s", url);

    char *page = (char *) ps_malloc(sizeof(char));
    int lengthHtml = 0;

    watch2::wifi_client_secure.setCACert(root_ca_reddit);
    if (http.begin(watch2::wifi_client_secure, url))
    {
        ESP_LOGD(WATCH2_TAG, "[redit] connected to server");

        int http_code = http.GET();
        ESP_LOGI(WATCH2_TAG, "[reddit] http code: %d (%s)", http_code, http.errorToString(http_code));

        if (http_code > 0)
        {
            if (http_code == HTTP_CODE_OK)
            {

                // response processing code taken from https://learn.upesy.com/en/programmation/psram.html

                // Essaye de récupérer la taille du ficher ( -1 s'il n'y a pas de Content-Length dans l'header)
                int tempLength = http.getSize();
                Serial.println((String) "Content Length :" + tempLength);

                //Stockage de la page dans la PSRAM
                Serial.printf("Adresse mémoire de la page : %p \n", page);

                // Récupère le stream TCP
                WiFiClient *stream = http.getStreamPtr();

                //Initialisation position du buffer dans la PSRAM
                int position = 0;

                uint32_t currentTime = 0;
                uint8_t timeoutArboted = 1;

                // Récupère toute les données du fichier
                while(http.connected() && (tempLength > 0 || tempLength == -1))
                {

                    // Récupère les données disponibles (les données arrivent packets par packets)
                    size_t size = stream->available();
                    if (size){
                        page = (char*) ps_realloc(page, position + size + 1);
                        stream->readBytes(page+position, size);
                        position += size;

                        if (tempLength > 0){
                                tempLength -= size;
                        }

                        timeoutArboted = 1;
                        }else{
                                //Si on ne connaît pas la taille du fichier, on suppose que toutes les données sont recues avant le timeout
                                if(timeoutArboted){
                                        //Lance le timer
                                        currentTime = millis();
                                        timeoutArboted = 0;
                                }else{
                                        if(millis()-currentTime > 5000){
                                                //On a atteind le Timeout
                                                Serial.println("Timeout reached");
                                                break;
                                        }
                                }
                        }
                }

                *(page+position) = '\0';
                lengthHtml = position;
                Serial.println((String)"Downloaded " + lengthHtml + " Octets");

                Serial.println("here we go");
                watch2::imageData img = watch2::getImageDataMemory(reinterpret_cast<const unsigned char*>(page), lengthHtml);
                
                free(page);
                return img;
            }
        }
        else ESP_LOGD(WATCH2_TAG, "[reddit] ???");
        
    }
    else
    {
        ESP_LOGW(WATCH2_TAG, "[reddit] failed to connect");
    }

    free(page);
}

void getSubredditPosts()
{
    std::string endpoint;

    if (subreddit_feed)
    {
        ESP_LOGI(WATCH2_TAG, "getting best posts");
        endpoint = "/best";
    }
    else
    {
        ESP_LOGI(WATCH2_TAG, "getting posts for r/%s", subreddit_name.c_str());
        endpoint += "/r/" + subreddit_name + "/" + post_sort;
    }

    cJSON *response = sendAPIgetRequest(endpoint);
    //Serial.print(cJSON_Print(response));
    if (response)
    {
        cJSON *page_info = cJSON_GetObjectItem(response, "data");
        cJSON *posts = cJSON_GetObjectItem(page_info, "children");
        int number_of_posts = cJSON_GetArraySize(posts);
        ESP_LOGI(WATCH2_TAG, "received %d posts", number_of_posts);

        for (int i = 0; i < number_of_posts; i++)
        {
            cJSON *post = cJSON_GetArrayItem(posts, i);
            if (post)
            {
                cJSON *post_data = cJSON_GetObjectItem(post, "data");
                if (post_data)
                {

                    cJSON *post_id = cJSON_GetObjectItem(post_data, "name");
                    cJSON *post_title = cJSON_GetObjectItem(post_data, "title");
                    cJSON *post_selftext = cJSON_GetObjectItem(post_data, "selftext");
                    cJSON *post_author = cJSON_GetObjectItem(post_data, "author");
                    cJSON *post_subreddit = cJSON_GetObjectItem(post_data, "subreddit");
                    cJSON *post_url = cJSON_GetObjectItem(post_data, "url");
                    cJSON *post_score = cJSON_GetObjectItem(post_data, "score");
                    cJSON *post_upvote_ratio = cJSON_GetObjectItem(post_data, "upvote_ratio");
                    cJSON *post_creation = cJSON_GetObjectItem(post_data, "creation");
                    cJSON *post_nsfw = cJSON_GetObjectItem(post_data, "nsfw");
                    cJSON *post_spoiler = cJSON_GetObjectItem(post_data, "spoiler");
                    cJSON *post_locked = cJSON_GetObjectItem(post_data, "locked");

                    reddit_post post_thing = (reddit_post){
                        post_id ? post_id->valuestring : "",
                        REDDIT_POST_UNKNOWN,
                        post_title ? post_title->valuestring : "",
                        post_selftext ? post_selftext->valuestring : "",
                        post_author ? post_author->valuestring : "",
                        post_subreddit ? post_subreddit->valuestring : "",
                        post_url ? post_url->valuestring : "",
                        post_score ? post_score->valueint : 0,
                        post_upvote_ratio ? post_upvote_ratio->valuedouble : 0.0,
                        post_creation ? post_creation->valueint : 0,
                        post_nsfw ? post_nsfw->valueint : 0,
                        post_spoiler ? post_spoiler->valueint : 0,
                        post_locked ? post_locked->valueint : 0
                    };

                    Serial.printf("%s from r/%s\n", post_thing.title.c_str(), post_thing.subreddit.c_str());


                    bool is_self = cJSON_GetObjectItem(post_data, "is_self")->valueint;
                    if (is_self) post_thing.post_type = REDDIT_POST_TEXT;
                    else {
                        std::string file_extension = watch2::file_ext(post_thing.url);
                        if (file_extension == "png" || file_extension == "jpeg" || file_extension == "jpg" || 
                            file_extension == "tga" || file_extension == "bmp") post_thing.post_type = REDDIT_POST_IMAGE;
                        else post_thing.post_type = REDDIT_POST_LINK;
                    }

                    retrieved_posts.push_back(post_thing);
                
                }
                else ESP_LOGW(WATCH2_TAG, "error getting post data");
            }
            else ESP_LOGW(WATCH2_TAG, "error getting post object");
        }
    }
    cJSON_free(response);
}

// https://github.com/Bodmer/TFT_eSPI/issues/558
void printSplitString(String text, uint16_t min_line, uint16_t max_line)
{
    int wordStart = 0;
    int wordEnd = 0;
    int line = 0;
    int cursor_x = 0;

    while ( (text.indexOf(' ', wordStart) >= 0) && ( wordStart <= text.length())) {
        wordEnd = text.indexOf(' ', wordStart + 1);
        uint16_t len = watch2::oled.textWidth(text.substring(wordStart, wordEnd));
        if (cursor_x + len >= watch2::oled.width()) {
            if (line >= min_line && line < max_line) watch2::oled.println();
            wordStart++;
            cursor_x = 0;
            line++;
        }
        if (line >= min_line && line < max_line) 
        {
            watch2::oled.setCursor(cursor_x, watch2::oled.getCursorY());
            watch2::oled.print(text.substring(wordStart, wordEnd));
        }
        cursor_x += len;
        wordStart = wordEnd;
    }
}

void state_func_reddit()
{
    if (watch2::states[watch2::state].variant == 0) // menu
    {
        if (!watch2::state_init)
        {
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.setTextColor(ORANGE, BLACK);

            menu_y = watch2::top_thing_height + 3;
            watch2::setFont(LARGE_FONT);
            menu_y += watch2::oled.fontHeight();
            watch2::oled.println("Reddit");

            watch2::setFont(MAIN_FONT);
            menu_y += watch2::oled.fontHeight();

            if (watch2::wifi_state == 3) // enabled, connected
            {
                getAccessToken();
                // cJSON *api_res = sendAPIgetRequest("/api/v1/me");
                const char *reddit_name = "hello";//cJSON_GetObjectItem(api_res, "name")->valuestring;
                // cJSON_free(api_res);
                watch2::oled.printf("Signed in as u/%s\n", reddit_name);
            }
            else
            {
                watch2::oled.printf("Not connected to Wifi...\nPress enter to return to the\napp menu");
            }
        }

        if (dpad_up_active())
        {
            if (menu_selected_item == 0) menu_selected_item = menu_items.size() - 1;
            else menu_selected_item--;
        }

        if (dpad_down_active())
        {
            if (menu_selected_item == menu_items.size() - 1) menu_selected_item = 0;
            else menu_selected_item++;
        }

        if (watch2::wifi_state == 3)
        {
            draw((dpad_any_active()), {

                watch2::drawMenu(2, menu_y, SCREEN_WIDTH - 4, SCREEN_HEIGHT - menu_y, menu_items, menu_selected_item, {}, false, true);

            });
        }

        if (dpad_enter_active())
        {
            if (menu_selected_item == 0) // your feed
            {
                subreddit_feed = true;
                watch2::switchState(watch2::state, 1);
            }

            else if (menu_selected_item == 1) // a subreddit
            {
                subreddit_feed = false;
                subreddit_name = watch2::textFieldDialogue("Subreddit", subreddit_name.c_str());
                if (!subreddit_name.empty())
                {
                    watch2::switchState(watch2::state, 1);
                }
            }

            else if (menu_selected_item == 2)
            {
                std::vector<reddit_post>().swap(retrieved_posts);
                watch2::switchState(2);
            }
        }

        watch2::drawTopThing();
    }


    else if (watch2::states[watch2::state].variant == 1) // post browser
    {
        if (!watch2::state_init)
        {
            if (watch2::last_variant == 0) // if the user is coming from the main menu
            {
                // get posts
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.print("Getting Posts\nThis may take a \nfew seconds");
                getSubredditPosts();
            }
        }

        if (dpad_up_active())
        {
            if (selected_post > 0) selected_post--;
        }

        if (dpad_down_active())
        {
            if (selected_post < retrieved_posts.size() - 1) selected_post++;
        }

        draw(dpad_any_active(), {

            watch2::oled.fillScreen(BLACK);

            // print posts
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(0, watch2::top_thing_height);
            watch2::oled.setTextWrap(true, false);

            uint8_t dot_width = watch2::oled.textWidth("...") * 2;
            // for (reddit_post post : retrieved_posts)
            for (int i = selected_post; i < selected_post + 4; i++)
            {
                if (i >= retrieved_posts.size()) break;

                reddit_post &post = retrieved_posts[i];
                //Serial.println(post.title.c_str());

                // print subreddit name
                watch2::oled.setTextColor(TFT_DARKGREY, BLACK);
                watch2::oled.print("r/");
                watch2::oled.println(post.subreddit.c_str());

                // print each character, but stop if the end of the screen is reached
                int char_index = 0;
                uint16_t char_x = 0, char_w = 0;
                uint8_t line = 0;
                watch2::oled.setTextColor((i == selected_post) ? watch2::themecolour : WHITE, BLACK);
                while(char_index < post.title.length())
                {
                    if (line == 1) // last line
                    {
                        char_w = watch2::oled.textWidth(String(post.title[char_index])) * 1.2;
                        watch2::oled.print(post.title[char_index]);
                        char_x += char_w;
                        char_index++;

                        if (char_x + char_w > SCREEN_WIDTH - dot_width)
                        {
                            watch2::oled.println("...");
                            break;
                        }
                        if (char_index == post.title.length())
                        {
                            watch2::oled.println("");
                            break;
                        }
                    }
                    else
                    {
                        size_t upcoming_space = post.title.find(" ", char_index);
                        std::string word = post.title.substr(char_index, upcoming_space - char_index);
                        uint8_t word_width = watch2::oled.textWidth(word.c_str());

                        if (upcoming_space == std::string::npos)
                        {
                            watch2::oled.println(word.c_str());
                            watch2::oled.println("");  // this is only on the first line, so print a newline to get to the next post area
                            break;
                        }
                        if (char_x + word_width > SCREEN_WIDTH - (dot_width * 2)) 
                        {
                            watch2::oled.println("");
                            line++;
                            char_x = 0;
                        }
                        else {
                            watch2::oled.printf(word.c_str());
                            watch2::oled.print(" ");
                            char_index += word.length() + 1;
                            char_x += word_width;
                        }
                    }
                    
                }
            }

            // draw dividing lines
            uint16_t post_height = watch2::oled.fontHeight() * 3;
            uint16_t line_y = watch2::top_thing_height;
            for (int i = 0; i < 4; i++)
            {
                line_y += post_height;
                watch2::oled.drawFastHLine(0, line_y, SCREEN_WIDTH, TFT_LIGHTGREY);
            }

        });

        watch2::drawTopThing();

        if (dpad_enter_active())
        {
            viewing_post = &retrieved_posts[selected_post];
            watch2::switchState(watch2::state, 2);
        }

        if (dpad_left_active())
        {
            watch2::switchState(watch2::state, 0);
        }
    }

    else if (watch2::states[watch2::state].variant == 2) // post viewer
    {
        if (!watch2::state_init)
        {
            if (viewing_post == NULL)
            {
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.print("No post loaded?");
            }
            else
            {
                watch2::oled.setTextWrap(false, false);
                viewer_page = 0;
                last_viewer_page = 255;
                viewer_post_scroll = 0;
                viewer_comments_scroll = 0;
            }
        }

        if (viewing_post != NULL)
        {
            if (dpad_up_active())
            {
                if (viewer_page == 0) if (viewer_post_scroll > 0) viewer_post_scroll -= 2;
                if (viewer_page == 1) if (viewer_comments_scroll > 0) viewer_comments_scroll -= 20;
            }

            if (dpad_down_active())
            {
                if (viewer_page == 0) viewer_post_scroll += 2;
                if (viewer_page == 1) viewer_comments_scroll += 20;
            }

            draw(dpad_any_active(), {

                if (viewer_page == 0) // post
                {
                    if (viewing_post->post_type == REDDIT_POST_TEXT)
                    {
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.setCursor(0, 0);
                        watch2::oled.fillScreen(BLACK);

                        int char_index = 0;
                        uint16_t char_x = 0, char_w = 0;
                        uint8_t line = 0;
                        uint8_t dot_width = watch2::oled.textWidth("...") * 2;
                        uint8_t number_of_lines = 8;

                        printSplitString(String(viewing_post->selftext.c_str()), viewer_post_scroll, viewer_post_scroll + number_of_lines);

                        // while(char_index < viewing_post->selftext.length())
                        // {
                        //     Serial.printf("processing char %d, line=%d\n", char_index, line);
                        //     size_t upcoming_space = viewing_post->selftext.find(" ", char_index);
                        //     std::string word = viewing_post->selftext.substr(char_index, upcoming_space - char_index);
                        //     uint8_t word_width = watch2::oled.textWidth(word.c_str());

                        //     Serial.printf("\tword = %s\n", word.c_str());
                        //     if (word.find("\n") != std::string::npos) line++;

                        //     if (upcoming_space == std::string::npos)
                        //     {
                        //         Serial.println("\tend of text");
                        //         if (line >= viewer_post_scroll && line <= (viewer_post_scroll + number_of_lines))
                        //         {
                        //             watch2::oled.println(word.c_str());
                        //             watch2::oled.println("");  // this is only on the first line, so print a newline to get to the next post area
                        //         }
                        //         break;
                        //     }
                        //     if (char_x + word_width > SCREEN_WIDTH - (dot_width * 2)) 
                        //     {
                        //         if (word_width > SCREEN_WIDTH)
                        //         {
                        //             Serial.println("\tword is longer than screen");
                        //             if (line >= viewer_post_scroll && line <= (viewer_post_scroll + number_of_lines))
                        //             {
                        //                 Serial.println("\tline is on screen, print word");

                        //                 watch2::oled.println("");
                        //                 line++;
                        //                 char_x = 0;

                        //                 watch2::oled.printf(word.c_str());
                        //                 watch2::oled.print(" ");
                        //             }
                        //             char_index += word.length() + 1;
                        //             char_x += word_width;
                        //         }

                        //         Serial.println("\tnewline");
                        //         if (line >= viewer_post_scroll && line <= (viewer_post_scroll + number_of_lines)) watch2::oled.println("");
                        //         line++;
                        //         char_x = 0;
                        //     }
                        //     else {
                        //         if (line >= viewer_post_scroll && line <= (viewer_post_scroll + number_of_lines))
                        //         {
                        //             Serial.println("\tline is on screen, print word");
                        //             watch2::oled.printf(word.c_str());
                        //             watch2::oled.print(" ");
                        //         }
                        //         char_index += word.length() + 1;
                        //         char_x += word_width;
                        //     }
                        // }
                    }
                    else if (viewing_post->post_type == REDDIT_POST_LINK)
                    {
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.setCursor(0, watch2::top_thing_height);
                        watch2::oled.printf("Link Post\n%s", viewing_post->url.c_str());
                    }
                    else if (viewing_post->post_type == REDDIT_POST_IMAGE  && (last_viewer_page != viewer_page))
                    {
                        Serial.println("getting image data");
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.setCursor(0, watch2::top_thing_height);
                        watch2::oled.println("Loading Image");
                        watch2::imageData img = getImageFromURL(viewing_post->url.c_str());

                        if (img.data)
                        {

                            // calculate scaling
                            float scaling = 1;
                            if (img.width >= img.height) // image is wider than it is tall
                            {
                                if (img.width > SCREEN_WIDTH) scaling = (float)img.width / SCREEN_WIDTH;
                                else scaling = 1.0;
                                ESP_LOGD(WATCH2_TAG, "[image viewer] image is wider, scaling factor is %f", scaling);
                            }
                            else // image is taller than it is wide
                            {
                                if (img.height > SCREEN_HEIGHT) scaling = (float)img.height / SCREEN_HEIGHT;
                                else scaling = 1.0;
                                ESP_LOGD(WATCH2_TAG, "[image viewer] image is taller, scaling factor is %f", scaling);
                            }

                            Serial.println("drawing image");

                            watch2::drawImage(img, 
                                (SCREEN_WIDTH / 2) -  ( (img.width / scaling)  / 2), 
                                (SCREEN_HEIGHT / 2) - ( (img.height / scaling) / 2), 
                            scaling, 0, watch2::oled, false);

                            Serial.println("freeing image");
                            watch2::freeImageData(img.data);

                        }
                        else
                        {
                            watch2::oled.setTextColor(WHITE, BLACK);
                            watch2::oled.setCursor(0, watch2::top_thing_height);
                            watch2::oled.printf("Error loading image: \n%s", img.error);
                        }
                    }
                }
                last_viewer_page = viewer_page;

            });
        }

        if (dpad_enter_active())
        {
            watch2::switchState(watch2::state, 1);
        }
    }


    
}