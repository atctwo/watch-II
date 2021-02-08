
#include "../watch2.h"

#include <FS.h>
#include <SD.h>
#include "driver/i2s.h"
#include <Audio.h>

void state_func_radio()
{
    static std::vector<std::string> station_names;
    static std::vector<std::string> station_urls;
    static uint8_t selected_station = 0;
    
    // station selection
    if (watch2::states[watch2::state].variant == 0)
    {

        if (!watch2::state_init)
        {
            watch2::initSD();
            Serial.printf("wifi %d, sd %d\n", watch2::wifi_state, watch2::sd_state);
            if (watch2::wifi_state == 3 && watch2::sd_state == 1)
            {
                station_names.clear();
                station_urls.clear();

                watch2::setup_audio_for_playback();

                // get stations from file
                fs::File radio_file = SD.open(RADIO_FILENAME);
                while(radio_file.available())
                {
                    // get station name and url
                    String station_name;
                    String station_url;

                    station_name = radio_file.readStringUntil(';');
                    station_url = radio_file.readStringUntil('\n');

                    // add entry to stations
                    station_names.push_back(station_name.c_str());
                    station_urls.push_back(station_url.c_str());

                    // std::string &name_string = station_names[station_names.size() - 1];
                    // name_string.erase(std::remove(name_string.begin(), name_string.end(), ';'), name_string.end());

                    std::string &url_string = station_urls[station_urls.size() - 1];
                    url_string.erase(std::remove(url_string.begin(), url_string.end(), '\n'), url_string.end());
                }

                // print stations
                Serial.println("read stations: ");
                for (uint8_t i = 0; i < station_names.size(); i++)
                {
                    Serial.printf("%s: %s\n", station_names[i].c_str(), station_urls[i].c_str());
                }

                // close file
                radio_file.close();
            }
            else
            {
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.println("Cannot play because:");

                if (watch2::wifi_state != 3) watch2::oled.println("- not connected to Wifi");
                if (watch2::sd_state != 1) watch2::oled.println("- SD card not mounted");
            }
            
        }

        if (dpad_up_active())
        {
            if (selected_station == 0) selected_station = station_names.size() - 1;
            else selected_station--;
        }
        if (dpad_down_active())
        {
            if (selected_station == station_names.size() - 1) selected_station == 0;
            else selected_station++;
        }

        draw(dpad_up_active() || dpad_down_active(), {

            if (station_names.size() > 0)
            {
                watch2::drawMenu(
                    2, watch2::top_thing_height, SCREEN_WIDTH - 4, SCREEN_HEIGHT - watch2::top_thing_height,
                    station_names, selected_station
                );
            }
        });

        watch2::drawTopThing();

        if (dpad_enter_active())
        {
            if (watch2::wifi_state == 3 && watch2::sd_state == 1) watch2::switchState(watch2::state, 1);
        }

        if (dpad_left_active())
        {
            station_names.clear();
            station_urls.clear();
            watch2::switchState(2);
        }

    }

    // radio player
    if (watch2::states[watch2::state].variant == 1)
    {
        if (!watch2::state_init)
        {
            watch2::oled.setCursor(0, watch2::top_thing_height);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println(station_names[selected_station].c_str());
        
            watch2::play_music(station_urls[selected_station].c_str(), false, NULL);
        }

        if (dpad_enter_active())
        {
            watch2::audio.pauseResume();
        }
        if (dpad_up_active())
        {
            watch2::speaker_volume = std::min(watch2::speaker_volume + 1, 21);
            watch2::audio.setVolume(watch2::speaker_volume);
        }
        if (dpad_down_active())
        {
            watch2::speaker_volume = std::max(watch2::speaker_volume - 1, 0);
            watch2::audio.setVolume(watch2::speaker_volume);
        }

        if (dpad_left_active())
        {
            watch2::stop_music();
            watch2::switchState(watch2::state, 0);
        }
    }

}