/**
 * @file watch2_wifi.cpp
 * @author atctwo
 * @brief functions for handling the wifi subsystem
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // wifi
    bool ntp_wakeup_connect = true;
    bool ntp_boot_connect = true;
    bool ntp_boot_connected = true;
    bool wifi_wakeup_reconnect = true;
    bool wifi_boot_reconnect = true;
    bool wifi_enabled = false;
    wifi_auth_mode_t wifi_encryption = WIFI_AUTH_MAX;
    uint8_t wifi_state = 0;
    uint8_t wifi_reconnect_attempts = 0;
    uint8_t initial_wifi_reconnect_attempts = 0;
    uint32_t wifi_connect_timeout = 15000;
    uint32_t wifi_connect_timeout_start = 0;

    WiFiClient wifi_client;
    WiFiClientSecure wifi_client_secure;

    void enable_wifi(bool connect)
    {
        ESP_LOGD(WATCH2_TAG, "[Wifi] enabling wifi");
        wifi_state = 1; // enabled, disconnected

        // tell the system to enable wifi on boot
        watch2::preferences.begin("watch2", false);
        watch2::preferences.putBool("wifi_en", true);
        watch2::preferences.end();

        // connect to an access point
        if (connect)
        {
            watch2::wifi_state = 4;
            watch2::initial_wifi_reconnect_attempts = 3;
            watch2::wifi_reconnect_attempts = watch2::initial_wifi_reconnect_attempts;
        }
    }

    void disable_wifi()
    {
        ESP_LOGD(WATCH2_TAG, "[Wifi] disconnecting from wifi");
        WiFi.disconnect();
        //WiFi.mode(WIFI_OFF);

        // tell the system to disable wifi on boot
        watch2::preferences.begin("watch2", false);
        watch2::preferences.putBool("wifi_en", false);
        watch2::preferences.end();
        wifi_state = 0; // disabled
    }

    void connectToWifiAP(const char *ssid, const char *password)
    {
        ESP_LOGD(WATCH2_TAG, "[WiFi] connecting to AP");
        WiFi.enableSTA(true);
        //WiFi.mode(WIFI_STA);
        WiFi.setSleep(true);
        WiFi.setHostname("watch ii");

        if (strcmp(ssid, "") == 0)
        {
            // automatically connect to internet
            if (wifi_reconnect_attempts == 0)
            {
                WiFi.disconnect();
                wifi_state = 1;
            }
            else
            {
                // get most recent profile
                uint8_t profile_index = initial_wifi_reconnect_attempts - wifi_reconnect_attempts;
                cJSON *profiles = getWifiProfiles();
                
                if (profiles)
                {
                    ESP_LOGD(WATCH2_TAG, "[Wifi] got profiles object");
                    cJSON *profile_array = cJSON_GetObjectItem(profiles, "profiles");
                    cJSON *access_index = cJSON_GetObjectItem(profiles, "access_index");

                    // if there are profiles
                    if (cJSON_GetArraySize(access_index) > 0)
                    {
                        if (access_index)
                        {
                            ESP_LOGD(WATCH2_TAG, "[Wifi] found %d profiles", cJSON_GetArraySize(profile_array));
                            ESP_LOGD(WATCH2_TAG, "[Wifi] found %d profiles in access index", cJSON_GetArraySize(access_index));

                            // if the profile index refers to a profile that doesn't exist
                            // (if the profile index is greater than the number of elements in the access index - 1)
                            if (profile_index > cJSON_GetArraySize(access_index) - 1)
                            {
                                // the profile list has been exhausted
                                ESP_LOGD(WATCH2_TAG, "[Wifi] tried to access a profile that doesn't exist");
                                WiFi.disconnect();
                                wifi_state = 1;
                                wifi_reconnect_attempts = 0;
                            }
                            else
                            {
                                // the profile index refers to an access index element that does exist, so get the information for that profile
                                ESP_LOGD(WATCH2_TAG, "[Wifi] access index %d points to an existing profile", profile_index);
                                const char *ssid = cJSON_GetArrayItem(access_index, profile_index)->valuestring;
                                cJSON *profile;
                                bool help = false;
                                for (int i = 0; i < cJSON_GetArraySize(profile_array); i++)
                                {
                                    profile = cJSON_GetArrayItem(profile_array, i);
                                    const char* profile_ssid = cJSON_GetObjectItem(profile, "ssid")->valuestring;
                                    ESP_LOGD(WATCH2_TAG, "checking ssid %s; profile ssid %s", ssid, profile_ssid);
                                    if (strcmp(ssid, profile_ssid) == 0) // if profile ssid matches ap ssid
                                    {
                                        help = true;
                                        break;
                                    }
                                }

                                if (help) // the profile actually exists
                                {
                                    ESP_LOGD(WATCH2_TAG, "[Wifi] the profile's SSID matches the access index's SSID :), connecting...");
                                    //WiFi._setStatus(WL_DISCONNECTED);
                                    WiFi.begin(
                                        cJSON_GetObjectItem(profile, "ssid")->valuestring,
                                        cJSON_GetObjectItem(profile, "password")->valuestring
                                    );
                                    wifi_connect_timeout_start = millis();
                                }
                                // otherwise, the AP name exists in the access index, but doesn't actually have a profile, so skip to the next AP
                                else 
                                {
                                    ESP_LOGD(WATCH2_TAG, "[Wifi] ssid was found in access index, but no matching profile was found");
                                    wifi_reconnect_attempts--;
                                }

                                wifi_state = 2;
                            }

                        }
                        else
                        {
                            ESP_LOGD(WATCH2_TAG, "[Wifi] couldn't access access index");
                            wifi_reconnect_attempts = 0;
                        }
                    }
                    else 
                    {
                        ESP_LOGD(WATCH2_TAG, "[Wifi] no profiles");
                        wifi_reconnect_attempts = 0;
                        WiFi.disconnect();
                        WiFi._setStatus(WL_CONNECT_FAILED);
                    }
                    
                    cJSON_Delete(profiles);

                }
                else
                {
                    ESP_LOGD(WATCH2_TAG, "[Wifi] couldn't access profile list");
                    wifi_reconnect_attempts = 0;
                }
            }
            
        }
        else
        {
            WiFi.begin(ssid, password);
            WiFi._setStatus(WL_DISCONNECTED);
            wifi_state = 2; // enabled, connecting
            wifi_connect_timeout_start = millis();
        }
        
        // the system will check if the wifi has connected to an AP in the endLoop() method.
    }

    cJSON *getWifiProfiles()
    {
        ESP_LOGD(WATCH2_TAG, "[Wifi profiles] getting profiles");
        if (spiffs_state == 1) // if spiffs is initalised
        {
            cJSON *profiles;

            // if profile file already exists
            if (SPIFFS.exists(WIFI_PROFILES_FILENAME))
            {
                ESP_LOGD(WATCH2_TAG, "[Wifi profiles] profile exists");

                // get handle to profiles file
                fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME);

                // get profiles list
                String profiles_list = profiles_file.readString();

                // parse profiles
                profiles = cJSON_Parse(profiles_list.c_str());

                if (profiles)
                {
                    ESP_LOGD(WATCH2_TAG, "[Wifi profiles] valid profile");
                }
                else
                {
                    ESP_LOGD(WATCH2_TAG, "[Wifi profiles] invalid profile list, returning minimal profile object");

                    // create profiles object
                    profiles = cJSON_CreateObject();
                    cJSON *profile_array = cJSON_AddArrayToObject(profiles, "profiles");
                    cJSON *access_index = cJSON_AddArrayToObject(profiles, "access_index");
                }

                // ESP_LOGD(WATCH2_TAG, "help");
                // ESP_LOGD(WATCH2_TAG, "%*", profiles_list);
                // ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profiles));

                // close file
                profiles_file.close();

                // return profiles
                return profiles;
            }
            //otherwise
            else
            {
                ESP_LOGD(WATCH2_TAG, "[Wifi profiles] profile doesn't exist, creating");

                // create profiles object
                profiles = cJSON_CreateObject();
                cJSON *profile_array = cJSON_AddArrayToObject(profiles, "profiles");
                cJSON *access_index = cJSON_AddArrayToObject(profiles, "access_index");

                // get handle to profiles file (it doesn't exist, so create it)
                fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME, "w");

                // write profiles object to file
                profiles_file.print(cJSON_Print(profiles));

                // close file
                profiles_file.close();

                // return profiles
                return profiles;
            }
        }
        else 
        {
            ESP_LOGW(WATCH2_TAG, "[Wifi profiles] can't access spiffs (state: %d)", spiffs_state);
            return nullptr;
        }
    }

    void setWifiProfiles(cJSON *profiles)
    {
        ESP_LOGD(WATCH2_TAG, "[Wifi profiles] setting profiles");
        if (spiffs_state == 1) // if spiffs has been initalised
        {
            // open profiles file.  if the file doesn't exist, it will be created, and if it does exist,
            // it will be truncated to zero bytes.
            fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME, "w");

            // get profile list as a string
            const char *profile_string = cJSON_Print(profiles);

            // write the profile object to the file
            profiles_file.write((uint8_t*)profile_string, strlen(profile_string));
            
            // close file
            profiles_file.close();
        }
    }

}