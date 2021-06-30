
#include "states.h"
#include "../watch2.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_SHTC3.h>

Adafruit_SHTC3 *shtc3;
bool shtc3_present = false;

bool any_sensor_present = false;
sensors_event_t humidity, temp;         // sensor event objects
uint16_t sensor_poll_period = 500;      // the time between polling sensors (in ms)
uint32_t sensor_last_poll = 0;          // the most recent poll time

void state_func_sensor()
{

    if (!watch2::state_init)
    {
        ESP_LOGI(WATCH2_TAG, "Sensor Thing\nstarting sensor discovery...");

        // create sensor objects
        shtc3 = new Adafruit_SHTC3();

        // check if sensors are present
        if (shtc3->begin()) {
            ESP_LOGI(WATCH2_TAG, "found SHTC3");
            shtc3_present = true;
            any_sensor_present = true;
        } else shtc3_present = false;

        ESP_LOGI(WATCH2_TAG, "sensor discovery finished!");


        // if no sensors are present, print a message to the screen
        if (!any_sensor_present)
        {
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(0, watch2::top_thing_height);
            watch2::oled.println("Sensor Thing\nNo sensors present :(");
        }
    }



    // run every 500 ms (based on a variable)
    // for each sensor that exists
    // run a chunk of code to read the sensor and print the output

    if (millis() - sensor_last_poll > sensor_poll_period)
    {
        // set up screen
        watch2::oled.setTextColor(watch2::themecolour, BLACK);
        watch2::oled.setCursor(0, watch2::top_thing_height);

        // SHTC3
        if (shtc3_present)
        {
            // get temperature + humidity
            shtc3->getEvent(&humidity, &temp);

            // print sensor title
            watch2::oled.println("SHTC3");

            // print temperature
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.fillRect(watch2::oled.getCursorX(), watch2::oled.getCursorY(), SCREEN_WIDTH, watch2::oled.fontHeight(), BLACK);
            watch2::oled.print("Temperature: ");
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print(temp.temperature);
            watch2::oled.println("°C");

            // print humidity
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.fillRect(watch2::oled.getCursorX(), watch2::oled.getCursorY(), SCREEN_WIDTH, watch2::oled.fontHeight(), BLACK);
            watch2::oled.print("Humidity: ");
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.print(humidity.relative_humidity);
            watch2::oled.println("%");
        }

        // reset poll timer
        sensor_last_poll = millis();
    }



    if (dpad_left_active())
    {
        // delete sensor objects
        if (shtc3_present) delete shtc3;

        // reset sensor presence flag
        any_sensor_present = false;

        // return to state menu
        watch2::switchState(2);
    }
}