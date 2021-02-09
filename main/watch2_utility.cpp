/**
 * @file watch2_utility.cpp
 * @author atctwo
 * @brief system functions that are sort of specific to a few applications
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // util
    EXT_RAM_ATTR int8_t timezone = 0;
    String weather_location = "";

    std::string getApiKey(const char *service, const char *field)
    {
        Serial.printf("[api keys] getting api key: %s:%s\n", service, field);
        std::string api_key = "";

        if (SPIFFS.exists(API_KEYS_FILENAME))
        {
            // open keys file
            fs::File api_keys_file = SPIFFS.open(API_KEYS_FILENAME);

            // parse json
            cJSON *api_keys_json = cJSON_Parse(api_keys_file.readString().c_str());

            cJSON *service_json = cJSON_GetObjectItem(api_keys_json, service);
            if (service_json)
            {
                cJSON *field_json = cJSON_GetObjectItem(service_json, field);
                if (field_json)
                {
                    const char *value = field_json->valuestring;
                    Serial.printf("[api keys] value: %s\n", value);
                    uint8_t pos = 0;
                    while(1)
                    {
                        api_key += value[pos];

                        pos++;
                        if (value[pos] == '\0') break;
                    }
                }
                else Serial.println("[api keys] field doesn't exist");
            }
            else Serial.println("[api keys] service doesn't exist");

            // free json memory
            cJSON_Delete(api_keys_json);
        }
        else
        {
            Serial.println("[api keys] file doesn't exist");
        }

        return api_key;
    }

    void getCurrentWeather(uint16_t &weather, time_t &sunrise, time_t &sunset)
    {
        weather = 0;
        sunrise = 0;
        sunset = 0;

        if (wifi_state != 3) // wifi not connected
        {
            Serial.println("[weather] not connected to wifi");
        }
        else // wifi is connected
        {
            Serial.println("[weather] getting current weather");

            HTTPClient *http = new HTTPClient();
            char server[150];
            std::string api_key = getApiKey("openweather");
            Serial.printf("[weather] key: %s\n", api_key.c_str());
            sprintf(server, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", weather_location.c_str(), api_key.c_str());
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
                            // get weather array
                            cJSON *weather_array = cJSON_GetObjectItem(weather_data, "weather");
                            if (weather_array)
                            {
                                // get the primary weather condition
                                cJSON *weather_status = cJSON_GetArrayItem(weather_array, 0);
                                if (weather_status)
                                {
                                    // get the weather code
                                    weather = cJSON_GetObjectItem(weather_status, "id")->valueint;
                                    Serial.printf("[weather] got primary weather code: %d\n", weather);
                                }
                                else
                                {
                                    Serial.println("[weather] failed to get primary weather condition");
                                }
                            }
                            else
                            {
                                Serial.println("[weather] failed to get weather array");
                            }

                            // get time info
                            cJSON *time_info = cJSON_GetObjectItem(weather_data, "sys");
                            if (time_info)
                            {
                                // get sunrise
                                sunrise = cJSON_GetObjectItem(time_info, "sunrise")->valueint;
                                Serial.printf("[weather] got sunrise time: %d\n", sunrise);

                                // get sunset
                                sunset = cJSON_GetObjectItem(time_info, "sunset")->valueint;
                                Serial.printf("[weather] got sunset time: %d\n", sunset);
                            }
                            else
                            {
                                Serial.println("[weather] failed to get time info");
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

    void getTimeFromNTP()
    {
        Serial.println("[NTP] setting time using ntp");
        configTime(watch2::timezone * 60 * 60, 0, NTP_SERVER);

        struct tm timeinfo;
        getLocalTime(&timeinfo);
        Serial.println(&timeinfo, "[NTP] retrieved time: %A, %B %d %Y %H:%M:%S");
        setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
    }

    // stolen from https://gist.github.com/dgoguerra/7194777
    const char *humanSize(uint64_t bytes)
    {
        char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
        char length = sizeof(suffix) / sizeof(suffix[0]);

        int i = 0;
        double dblBytes = bytes;

        if (bytes > 1024) {
            for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
                dblBytes = bytes / 1024.0;
        }

        static char output[200];
        sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
        return output;
    }

    // https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
    double ReadVoltage(byte pin){
    double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
    if(reading < 1 || reading > 4095) return 0;
    return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
    // return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
    } // Added an improved polynomial, use either, comment out as required


}