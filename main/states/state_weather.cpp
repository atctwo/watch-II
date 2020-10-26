#include "../watch2.h"

void getLatLong(double &latitude, double &longitude)
{
    latitude = 0;
    longitude = 0;

    if (watch2::wifi_state != 3) // wifi not connected
    {
        Serial.println("[weather] not connected to wifi");
    }
    else // wifi is connected
    {
        Serial.println("[weather] getting current weather");

        HTTPClient *http = new HTTPClient();
        char server[150];
        std::string api_key = watch2::getApiKey("openweather");
        Serial.printf("[weather] key: %s\n", api_key.c_str());
        sprintf(server, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", watch2::weather_location.c_str(), api_key.c_str());
        Serial.printf("[weather] server request: %s\n", server);

        // connect to server
        Serial.println("[weather] connecting to server");
        http->begin(watch2::wifi_client, server);
        int http_code = http->GET();
        if (http_code)
        {
            Serial.println("[weather] connected to server");
            Serial.printf("[weather] http code: %d (%s)\n", http_code, http->errorToString(http_code));

            if (http_code > 0)
            {
                if (http_code == HTTP_CODE_OK)
                {
                    // get server response
                    String res = http->getString();
                    Serial.println("[weather] response:");
                    Serial.println(res);

                    // parse returned json
                    cJSON *weather_data = cJSON_Parse(res.c_str());
                    if (weather_data)
                    {
                        // get coord object
                        cJSON *coord_object = cJSON_GetObjectItem(weather_data, "coord");
                        if (coord_object)
                        {
                            // get the latitude and longitude
                            latitude = cJSON_GetObjectItem(coord_object, "lat")->valuedouble;
                            longitude = cJSON_GetObjectItem(coord_object, "lon")->valuedouble;
                        }
                        else
                        {
                            Serial.println("[weather] failed to get coord object");
                        }                        
                    }
                    else
                    {
                        Serial.println("[weather] failed to parse response");
                    }
                    
                    // free memory used by parsed json
                    cJSON_Delete(weather_data);
                }
            }
            else Serial.println("[weather] ???");
            
        }
        else
        {
            Serial.println("[weather] failed to connect");
        }

        delete http;
        Serial.println("[weather] finished");
    }
    
}

cJSON *getForecast(double latitude, double longitude)
{
    cJSON *forecast = NULL;

    if (watch2::wifi_state != 3) // wifi not connected
    {
        Serial.println("[weather] not connected to wifi");
    }
    else // wifi is connected
    {
        Serial.println("[weather] getting current weather");

        HTTPClient *http = new HTTPClient();
        char server[175];
        std::string api_key = watch2::getApiKey("openweather");
        Serial.printf("[weather] key: %s\n", api_key.c_str());
        sprintf(server, "http://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=minutely,current,hourly,alerts&appid=%s", latitude, longitude, api_key.c_str());
        Serial.printf("[weather] server request: %s\n", server);

        // connect to server
        Serial.println("[weather] connecting to server");
        http->begin(watch2::wifi_client, server);
        int http_code = http->GET();
        if (http_code)
        {
            Serial.println("[weather] connected to server");
            Serial.printf("[weather] http code: %d (%s)\n", http_code, http->errorToString(http_code));

            if (http_code > 0)
            {
                if (http_code == HTTP_CODE_OK)
                {
                    // get server response
                    String res = http->getString();
                    Serial.println("[weather] response:");
                    Serial.println(res);

                    // parse returned json
                    forecast = cJSON_Parse(res.c_str());
                    if (forecast)
                    {
                        Serial.println("[weather] parsed response successfully");                      
                    }
                    else
                    {
                        Serial.println("[weather] failed to parse response");
                    }
                }
            }
            else Serial.println("[weather] ???");
            
        }
        else
        {
            Serial.println("[weather] failed to connect");
        }

        delete http;
        Serial.println("hi");
    }

    Serial.println("[weather] finished");
    Serial.println("aaa");
    return forecast;
    
}

cJSON *getDailyArray(cJSON *forecast)
{
    cJSON *daily_array;

    if (forecast)
    {
        daily_array = cJSON_GetObjectItem(forecast, "daily");
        if (!daily_array) Serial.println("[weather] couldn't get daily array");
    }
    else
    {
        Serial.println("[weather] forecast is null");
    }

    return daily_array;
}

void state_func_weather()
{

    static cJSON *forecast;
    static cJSON *daily_array;
    static double latitude = 0;
    static double longitude = 0;
    static uint8_t selected_x = 0;
    static uint8_t selected_y = 0;

    static uint8_t padding = 10;
    static uint8_t button_icon_size = 20;
    static uint16_t day_width = (SCREEN_WIDTH / 2) - (padding * 1.5);
    static uint16_t day_height = (SCREEN_HEIGHT - watch2::top_thing_height - button_icon_size - (padding * 5)) / 4;
    static uint16_t day_radius = 7;

    if (watch2::states[watch2::state].variant == 0) // 8 day forecast
    {
        if (!watch2::state_init)
        {
            if (watch2::weather_location.isEmpty()) // if no weather location has been set
            {
                Serial.println("[weather] location has not been set");
            }
            else
            {
                // get latitude and longitude
                getLatLong(latitude, longitude);
                Serial.printf("[weather] latitude: %f, longitude: %f\n", latitude, longitude);

                // get forecast
                forecast = getForecast(latitude, longitude);
                daily_array = getDailyArray(forecast);
            }
        }

        if (dpad_left_active())
        {
            if (selected_x == 0) selected_x = 1;
            else selected_x--;
        }
        if (dpad_right_active())
        {
            if (selected_x == 1) selected_x = 0;
            else selected_x++;
        }
        if (dpad_up_active())
        {
            if (selected_y == 0) selected_y = 4;
            else selected_y--;
        }
        if (dpad_down_active())
        {
            if (selected_y == 4) selected_y = 0;
            else selected_y++;
        }

        if (dpad_enter_active())
        {
            if (selected_x == 0 && selected_y == 0) // update location
            {
                std::string location = watch2::textFieldDialogue("Location", watch2::weather_location.c_str());
                if (location.empty())
                {
                    // do nothing
                }
                else
                {
                    watch2::weather_location = String(location.c_str());
                    watch2::preferences.begin("watch2");
                    watch2::preferences.putString("weather_city", watch2::weather_location); 
                    watch2::preferences.end();
                }
            }

            if (selected_x == 1 && selected_y == 0) // exit app
            {
                if (forecast) cJSON_Delete(forecast);
                watch2::switchState(2);
            }
        }

        draw(dpad_any_active(), {

            // draw location
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(0, watch2::top_thing_height);
            watch2::oled.print(watch2::weather_location);

            // draw icons
            watch2::oled.drawBitmap(
                SCREEN_WIDTH - button_icon_size, watch2::top_thing_height, 
                (*watch2::small_icons)["key_cancel"].data(), button_icon_size, button_icon_size, (selected_y == 0 && selected_x == 1)?watch2::themecolour:WHITE
            );
            watch2::oled.drawBitmap(
                SCREEN_WIDTH - (button_icon_size * 2) - padding, watch2::top_thing_height, 
                (*watch2::small_icons)["key_symbols"].data(), button_icon_size, button_icon_size, (selected_y == 0 && selected_x == 0)?watch2::themecolour:WHITE
            );

            // draw days
            uint16_t x = padding, y = watch2::top_thing_height + button_icon_size + padding;
            for (uint8_t i = 0; i < 8; i++)
            {
                // numbers
                uint8_t current_x = i % 2;
                uint8_t current_y = i / 2;
                uint16_t colour = ( ( current_x == selected_x && current_y == (selected_y - 1)) ? watch2::themecolour : WHITE );

                uint16_t x_position = x + ( (day_width + padding)  * current_x);
                uint16_t y_position = y + ( (day_height + padding) * current_y);

                // draw outline
                watch2::oled.drawRoundRect(x_position, y_position, day_width, day_height, day_radius, colour);

                // draw name and weather
                if (daily_array)
                {
                    cJSON *day_object = cJSON_GetArrayItem(daily_array, i);
                    if (day_object)
                    {
                        // draw day
                        time_t dt = cJSON_GetObjectItem(day_object, "dt")->valueint;
                        watch2::oled.setTextDatum(CL_DATUM);
                        watch2::oled.setTextColor(WHITE, BLACK);
                        watch2::oled.drawString(dayShortStr(weekday(dt)), x_position + padding, y_position + (day_height / 2));
                        watch2::oled.setTextDatum(TL_DATUM);

                        // draw weather icon
                        cJSON *weather_array = cJSON_GetObjectItem(day_object, "weather");
                        if (weather_array)
                        {
                            cJSON *primary_weather = cJSON_GetArrayItem(weather_array, 0);
                            int weather_id = cJSON_GetObjectItem(primary_weather, "id")->valueint;

                            // this assumes all the weather icons are the same dimensions
                            // if the icons were to be different sizes, the next two lines would have to be called
                            // in each switch statement case
                            uint16_t weather_icon_x = (x_position + day_width) - padding - (*watch2::icons)["rain"].width;
                            uint16_t weather_icon_y = (y_position + (day_height / 2)) - ((*watch2::icons)["rain"].height / 2);

                            switch(weather_id / 100)
                            {
                                case 2: // thunder
                                    drawImage((*watch2::icons)["thunder"], weather_icon_x, weather_icon_y);
                                    break;
                                    
                                case 3: // drizzle
                                    drawImage((*watch2::icons)["sun_rain"], weather_icon_x, weather_icon_y);
                                    break;

                                case 5: // rain
                                    drawImage((*watch2::icons)["rain"], weather_icon_x, weather_icon_y);
                                    break;

                                case 6: // snow
                                    drawImage((*watch2::icons)["snow"], weather_icon_x, weather_icon_y);
                                    break;

                                case 7: // atmosphere
                                    drawImage((*watch2::icons)["wind"], weather_icon_x, weather_icon_y);
                                    break;

                                case 8: // clear / clouds
                                    if (weather_id == 800) // clear
                                    {
                                        drawImage((*watch2::icons)["sun"], weather_icon_x, weather_icon_y);
                                    }
                                    else
                                    {
                                        drawImage((*watch2::icons)["sun_cloud"], weather_icon_x, weather_icon_y);
                                    }
                                    break;

                                default:
                                    drawImage((*watch2::icons)["weather_unknown"], weather_icon_x, weather_icon_y);
                                    break;
                            }
                        }
                        else Serial.println("[weather] failed to get weather array");
                    }
                    else Serial.printf("[weather] couldn't get information for day %d\n", i);

                    
                }
            }

        });
    }

    watch2::drawTopThing();

}