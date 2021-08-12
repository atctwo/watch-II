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
        ESP_LOGD(WATCH2_TAG, "[api keys] getting api key: %s:%s", service, field);
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
                    ESP_LOGD(WATCH2_TAG, "[api keys] value: %s", value);
                    uint8_t pos = 0;
                    while(1)
                    {
                        api_key += value[pos];

                        pos++;
                        if (value[pos] == '\0') break;
                    }
                }
                else ESP_LOGD(WATCH2_TAG, "[api keys] field doesn't exist");
            }
            else ESP_LOGD(WATCH2_TAG, "[api keys] service doesn't exist");

            // free json memory
            cJSON_Delete(api_keys_json);
        }
        else
        {
            ESP_LOGW(WATCH2_TAG, "[api keys] file doesn't exist");
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
            ESP_LOGD(WATCH2_TAG, "[weather] not connected to wifi");
        }
        else // wifi is connected
        {
            ESP_LOGD(WATCH2_TAG, "[weather] getting current weather");

            HTTPClient *http = new HTTPClient();
            char server[150];
            std::string api_key = getApiKey("openweather");
            ESP_LOGD(WATCH2_TAG, "[weather] key: %s", api_key.c_str());
            sprintf(server, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", weather_location.c_str(), api_key.c_str());
            ESP_LOGD(WATCH2_TAG, "[weather] server request: %s", server);

            // connect to server
            ESP_LOGD(WATCH2_TAG, "[weather] connecting to server");
            http->begin(watch2::wifi_client, server);
            int http_code = http->GET();
            if (http_code)
            {
                ESP_LOGD(WATCH2_TAG, "[weather] connected to server");
                ESP_LOGD(WATCH2_TAG, "[weather] http code: %d (%s)", http_code, http->errorToString(http_code));

                if (http_code > 0)
                {
                    if (http_code == HTTP_CODE_OK)
                    {
                        // get server response
                        String res = http->getString();
                        ESP_LOGD(WATCH2_TAG, "[weather] response:");
                        ESP_LOGD(WATCH2_TAG, "%s", res.c_str());

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
                                    ESP_LOGD(WATCH2_TAG, "[weather] got primary weather code: %d", weather);
                                }
                                else
                                {
                                    ESP_LOGD(WATCH2_TAG, "[weather] failed to get primary weather condition");
                                }
                            }
                            else
                            {
                                ESP_LOGD(WATCH2_TAG, "[weather] failed to get weather array");
                            }

                            // get time info
                            cJSON *time_info = cJSON_GetObjectItem(weather_data, "sys");
                            if (time_info)
                            {
                                // get sunrise
                                sunrise = cJSON_GetObjectItem(time_info, "sunrise")->valueint;
                                ESP_LOGD(WATCH2_TAG, "[weather] got sunrise time: %d", sunrise);

                                // get sunset
                                sunset = cJSON_GetObjectItem(time_info, "sunset")->valueint;
                                ESP_LOGD(WATCH2_TAG, "[weather] got sunset time: %d", sunset);
                            }
                            else
                            {
                                ESP_LOGD(WATCH2_TAG, "[weather] failed to get time info");
                            }
                            
                        }
                        else
                        {
                            ESP_LOGD(WATCH2_TAG, "[weather] failed to parse response");
                        }
                        
                        // free memory used by parsed json
                        cJSON_Delete(weather_data);
                    }
                }
                else ESP_LOGD(WATCH2_TAG, "[weather] ???");
                
            }
            else
            {
                ESP_LOGD(WATCH2_TAG, "[weather] failed to connect");
            }

            delete http;
            ESP_LOGD(WATCH2_TAG, "[weather] finished");
        }
        
    }

    void getTimeFromNTP()
    {
        ESP_LOGI(WATCH2_TAG, "[NTP] setting time using ntp");
        configTime(watch2::timezone * 60 * 60, 0, NTP_SERVER);

        struct tm timeinfo;
        getLocalTime(&timeinfo);
        ESP_LOGD(WATCH2_TAG, "[NTP] retrieved time: %s", asctime(&timeinfo));

        // set system time
        setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);

        // set ds1337 time
        struct ds1337_time_t time_thing;
        make_time(&time_thing, timeinfo.tm_year - 100, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        if (ds1337_write_time(&time_thing)) ESP_LOGW(WATCH2_TAG, "error writing time to DS1337");
        else ESP_LOGI(WATCH2_TAG, "wrote time to DS1337");
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

    void print_memory_details(Print &output)
    {
        // print memory info
        output.printf("[memory] used internal memory (heap):  %d (%s) (%0.2f%%)\n", 
            ESP.getHeapSize() - ESP.getFreeHeap(), watch2::humanSize(ESP.getHeapSize() - ESP.getFreeHeap()), 
            ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100
        );
        output.printf("[memory] free internal memory (heap):  %d (%s)\n", ESP.getFreeHeap(), watch2::humanSize(ESP.getFreeHeap()));
        output.printf("[memory] total internal memory (heap): %d (%s)\n", ESP.getHeapSize(), watch2::humanSize(ESP.getHeapSize()));

        output.printf("[memory] used external memory (heap):  %d (%s) (%0.2%%)\n", 
            ESP.getPsramSize() - ESP.getFreePsram(), watch2::humanSize(ESP.getPsramSize() - ESP.getFreePsram()), 
            ((float)(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize()) * 100
        );
        output.printf("[memory] free external memory (heap):  %d (%s)\n", ESP.getFreePsram(), watch2::humanSize(ESP.getFreePsram()));
        output.printf("[memory] total external memory (heap): %d (%s)\n", ESP.getPsramSize(), watch2::humanSize(ESP.getPsramSize()));
    }

    void print_task_details(Print &output)
    {
        // print task info
        // table header taken from http://exploreembedded.com/wiki/Read_Task_Info_%3A_vTaskList()

        char pcWriteBuffer[40 * (uxTaskGetNumberOfTasks() + 1)];
        vTaskList(pcWriteBuffer);
        output.println("Task Info:");
        output.println("Task  State   Prio    Stack    Num");
        output.print(pcWriteBuffer);
    }

    // https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
    double ReadVoltage(byte pin){
    double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
    if(reading < 1 || reading > 4095) return 0;
    return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
    // return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
    } // Added an improved polynomial, use either, comment out as required


}