
#define SPI_SPEED SD_SCK_MHZ(4)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_bt.h"
#include "watch2.h"
#include "icons/small_icons.h"

// esp-idf logging tags
#define TAG_INIT        "watch2-init"
#define TAG_LOOP        "watch2-loop"

////////////////////////////////////////
// setup function
////////////////////////////////////////

void setup() {

    //set cpu Frequency
    //setCpuFrequencyMhz(80);

    // time for cpu to start up
    vTaskDelay(500);

    //begin serial
    Serial.begin(115200);
    ESP_LOGI(TAG_INIT, "\n\nwatch2 version %d\n", WATCH_VER);

    // set up psram
    Serial.println("setting up psram");
    psramInit();
    if (psramFound()) Serial.println("initalised psram");
    else Serial.println("failed to init psram");
    Serial.println("done");

    //set up oled
    ESP_LOGD(TAG_INIT, "setting up display: ");
    digitalWrite(cs, LOW);
    watch2::oled.setAttribute(PSRAM_ENABLE, true);
    watch2::oled.begin();
    watch2::oled.fillScreen(0);
    watch2::oled.writecommand(0x11); // sleep out
    watch2::oled.initDMA();

    watch2::oled.setTextDatum(MC_DATUM);

    //watch2::setFont(MAIN_FONT);
    watch2::oled.setTextSize(3);
    watch2::oled.drawString("loading...", SCREEN_WIDTH/2, (SCREEN_HEIGHT/2) + ((8 * 5) / 2));

    //watch2::setFont(REALLY_BIG_FONT);
    watch2::oled.setTextSize(5);
    watch2::oled.drawString("watch2", SCREEN_WIDTH/2, (SCREEN_HEIGHT/2) - ((8 * 5) / 2));

    watch2::oled.setTextDatum(TL_DATUM);
    watch2::oled.setTextSize(1);
    ESP_LOGD(TAG_INIT, "done");

    // configure gpio data direction registers
    pinMode(12, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(sdcs, OUTPUT);
    pinMode(tftbl, OUTPUT);
    pinMode(BATTERY_DIVIDER_PIN, INPUT);
    pinMode(IR_SEND_PIN, OUTPUT);

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);

    //set up spiffs
    ESP_LOGD(TAG_INIT, "setting up spiffs: ");
    if (!SPIFFS.begin())
    {
        ESP_LOGD(TAG_INIT, "spiffs init failed");
        watch2::spiffs_state = -1;
    }
    else
    {
        ESP_LOGD(TAG_INIT, "initalised spiffs successfully");
        watch2::spiffs_state = 1;
    }
    watch2::setFont(MAIN_FONT);

    // set up i2c
    ESP_LOGD(TAG_INIT, "setting up i2c devices: ");

    ESP_LOGD(TAG_INIT, "\tWire object: ");
    Wire.begin(I2C_SDA, I2C_SCL);
    ESP_LOGD(TAG_INIT, "done");

    Serial.print("\tMCP23008: ");
    if (watch2::mcp.begin(I2C_ADDRESS_MCP23008))
    {

        for (uint8_t i = 0; i < 7; i++) // set channels 0 to 6 as inputs with no pull up
        {
            watch2::mcp.pinMode(i, INPUT);
            watch2::mcp.pullUp(i, LOW);
        }

        watch2::mcp.pinMode(SHUTDOWN_PIN, OUTPUT); // set shutdown pin as an output w/ pull up
        watch2::mcp.pullUp(SHUTDOWN_PIN, LOW);
        watch2::mcp.digitalWrite(SHUTDOWN_PIN, 1);

        ESP_LOGD(TAG_INIT, "done");
    }
    else 
    {
        ESP_LOGD(TAG_INIT, "failed :(");
    }

    ESP_LOGD(TAG_INIT, "\tMCP9808: ");
    if (!watch2::temperature.begin(I2C_ADDRESS_MCP9098)) ESP_LOGW(TAG_INIT, "mcp9808 init failed");
    else 
    {
        watch2::temperature.setResolution(1);  // 0.25°C
        ESP_LOGD(TAG_INIT, "done");
    }

    ESP_LOGD(TAG_INIT, "\LC709203F: ");
    if (!watch2::fuel_gauge.begin()) ESP_LOGW(TAG_INIT, "LC709203F not found :(");
    else {
        watch2::fuel_gauge.setPackSize(LC709203F_APA_500MAH);
        watch2::fuel_gauge.setAlarmVoltage(3.4);
    }
    ESP_LOGD(TAG_INIT, "done");

    ESP_LOGD(TAG_INIT, "done setting up i2c");

    //set up SD card
    ESP_LOGD(TAG_INIT, "setting up sd card: ");
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);
    
    watch2::initSD();

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);
    ESP_LOGD(TAG_INIT, "done");
    
    //set up preferences
    ESP_LOGD(TAG_INIT, "setting up preferences: ");
    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
    watch2::timeout = watch2::preferences.getBool("timeout", false);
    watch2::short_timeout = watch2::preferences.getInt("short_timeout", 5000);
    watch2::long_timeout = watch2::preferences.getInt("long_timeout", 30000);
    watch2::themecolour = watch2::preferences.getInt("themecolour", BLUE);
    watch2::trans_mode = watch2::preferences.getUInt("trans_mode", 0);
    watch2::animate_watch_face = watch2::preferences.getBool("animate_time", true);
    watch2::screen_brightness = watch2::preferences.getUInt("brightness", (2^tftbl_resolution) / 2);
    watch2::timezone = watch2::preferences.getUInt("timezone", 0);
    watch2::ntp_wakeup_connect = watch2::preferences.getBool("ntp_wakeup", true);
    watch2::ntp_boot_connect = watch2::preferences.getBool("ntp_boot", true);
    watch2::wifi_wakeup_reconnect = watch2::preferences.getBool("wifi_wakeup", true);
    watch2::wifi_boot_reconnect = watch2::preferences.getBool("wifi_boot", true);
    watch2::wifi_enabled = watch2::preferences.getBool("wifi_en", false);
    watch2::weather_location = watch2::preferences.getString("weather_city", "");
    watch2::timer_music = watch2::preferences.getString("timer_music", "/music/alarms/CLASSI~1.mp3");
    watch2::alarm_music = watch2::preferences.getString("alarm_music", "/music/alarms/Meander.mp3");
    watch2::wfs = watch2::preferences.getString("wfs", "\x0\x0\x0\x0").c_str();
    watch2::preferences.end();
    ESP_LOGD(TAG_INIT, "done");

    //set up buttons
    ESP_LOGD(TAG_INIT, "setting up buttons: ");
    watch2::btn_dpad_up.begin();
    watch2::btn_dpad_down.begin();
    watch2::btn_dpad_left.begin();
    watch2::btn_dpad_right.begin();
    watch2::btn_dpad_enter.begin();
    ESP_LOGD(TAG_INIT, "done");

    //set up torch
    ESP_LOGD(TAG_INIT, "setting up torch: ");
    ledcAttachPin(TORCH_PIN, TORCH_PWM_CHANNEL);
    ledcSetup(TORCH_PWM_CHANNEL, 4000, 8);
    ledcWrite(TORCH_PWM_CHANNEL, 0);
    ESP_LOGD(TAG_INIT, "done");

    //set up tft backlight
    ESP_LOGD(TAG_INIT, "setting up backlight: ");
    ledcAttachPin(tftbl, TFTBL_PWM_CHANNEL);
    ledcSetup(TFTBL_PWM_CHANNEL, 4000, tftbl_resolution);
    ledcWrite(TFTBL_PWM_CHANNEL, 2^tftbl_resolution);
    ESP_LOGD(TAG_INIT, "done");

    //set up top thing
    ESP_LOGD(TAG_INIT, "setting up top thing: ");
    watch2::top_thing.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight() + 2);
    watch2::setFont(MAIN_FONT, watch2::top_thing);
    ESP_LOGD(TAG_INIT, "done");

    // set up icons
    ESP_LOGD(TAG_INIT, "setting up icons: ");

    // the icon maps are dynamically allocated using malloc() and "placement new"s.

    // allocate memory for maps using malloc; specifically allocate memory from PSRAM
    watch2::icons = (std::map<std::string, watch2::imageData>*) heap_caps_malloc(sizeof(std::map<std::string, watch2::imageData>), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    watch2::small_icons = (std::map<std::string, std::vector<unsigned char>>*) heap_caps_malloc(sizeof(std::map<std::string, std::vector<unsigned char>>), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    //ESP_LOGD(WATCH2_TAG, "spiram %s", (psramFound() ? "found" : "not found"));
    if (!watch2::icons) ESP_LOGW(TAG_INIT, "failed to allocate memory for icons!");
    if (!watch2::small_icons) ESP_LOGW(TAG_INIT, "failed to allocate memory for small icons!");

    // use placement new to construct objects
    new(watch2::icons) std::map<std::string, watch2::imageData>();
    new(watch2::small_icons) std::map<std::string, std::vector<unsigned char>>();

    // copy icons from flash to ram
    fs::File spi_root = SPIFFS.open("/");
    fs::File spi_file = spi_root.openNextFile();
    while(spi_file)
    {
        if ((watch2::file_ext(spi_file.name()).compare("bmp") == 0) || (watch2::file_ext(spi_file.name()).compare("png") == 0)) // the file extension is bmp
        {
            ESP_LOGD(TAG_INIT, "loading \"%s\" as \"%s\"", spi_file.name(), watch2::file_name(spi_file.name(), false).c_str());
            watch2::imageData loaded_icon = watch2::getImageDataSPIFFS(spi_file.name());
            if (loaded_icon.error) ESP_LOGW(TAG_INIT, "\tload error: %s", loaded_icon.error);
            watch2::registerIcon(watch2::file_name(spi_file.name(), false).c_str(), loaded_icon);
        }

        spi_file = spi_root.openNextFile();
    }
    registerSmallIcons();

    ESP_LOGD(TAG_INIT, "done");

    //set up framebuffer
    //watch2::framebuffer.createSprite(100, 100);
    //watch2::setFont(MAIN_FONT, watch2::framebuffer);

    // set up wifi
    ESP_LOGD(TAG_INIT, "setting up wifi...");
    if (watch2::wifi_enabled)
    {
        watch2::enable_wifi(false);
        if (watch2::wifi_boot_reconnect) 
        {
            watch2::enable_wifi();
        }
    }
    ESP_LOGD(TAG_INIT, "done!");

    // set up bluetooth
    ESP_LOGD(TAG_INIT, "setting up bluetooth...");
    esp_bt_mem_release(ESP_BT_MODE_CLASSIC_BT);     // disable classic bluetooth
    ESP_LOGD(TAG_INIT, "done!");



    //set up time
    ESP_LOGD(TAG_INIT, "setting up time: ");
    if (watch2::ntp_boot_connect) watch2::ntp_boot_connected = false;
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (watch2::boot_count == 0)
    {
        setTime(23, 58, 00, 28, 6, 2019);

    }
    ESP_LOGD(TAG_INIT, "done");

    // set up state menu
    ESP_LOGD(TAG_INIT, "setting up state menu: ");
    if (watch2::boot_count == 0)
    {
        //set selected menu icon to first non-hidden state
        watch2::selected_menu_icon = 0;
        while(1)
        {
            if (watch2::states[watch2::selected_menu_icon].hidden)
            {
                //check next state
                watch2::selected_menu_icon++;
            }
            else break;
        }
    }
    //else selected_menu_icon = states.find(selected_state);
    ESP_LOGD(TAG_INIT, "done");

    // setup i2s sort of
    // ESP_LOGD(WATCH2_TAG, "setting up i2s: ");
    // watch2::uninstall_i2s_driver();
    // ESP_LOGD(WATCH2_TAG, "done!");

    // set up fs icons
    ESP_LOGD(TAG_INIT, "setting up fs icon maps: ");
    watch2::setupFsIcons();
    ESP_LOGD(TAG_INIT, "done");

    // set up audio task
    ESP_LOGD(TAG_INIT, "setting up audio task: ");
    int x = 10;
    xTaskCreatePinnedToCore(watch2::audio_task, "audio", 10000, (void*)x, ESP_TASK_PRIO_MAX - 1, &watch2::audio_task_handle, 1);
    ESP_LOGD(TAG_INIT, "done");

    ESP_LOGD(TAG_INIT, "setting up watch face shortcuts: ");
    for (uint8_t i = 0; i < watch2::states.size(); i++)
    {
        if (watch2::states[i].stateName == "State Menu") 
        {
            ESP_LOGD(TAG_INIT, "state name, ");
            watch2::wfs[3] = i;
        }
        if (watch2::states[i].stateName == "Bluetooth Remote") 
        {
            ESP_LOGD(TAG_INIT, " ble remote, ");
            watch2::wfs[1] = i;
        }
    }
    ESP_LOGD(TAG_INIT, "done");

    //finish up
    watch2::boot_count++;
    ESP_LOGI(TAG_INIT, "setup finished!");

}

////////////////////////////////////////
// loop function
////////////////////////////////////////

void loop() {

    watch2::startLoop();

    //ESP_LOGD(WATCH2_TAG, "capacitive touch on gpio 0: %d", touchRead(0));

    //---------------------------------------
    // run current state
    //---------------------------------------

    if (watch2::timer_trigger_status == 0 && watch2::alarm_trigger_status == 0)
    {
        if ( watch2::state >= watch2::states.size() )
        {
            //dim screen
            uint8_t contrast = 0x0F;
            //watch2::oled.sendCommand(0xC7, &contrast, 1);

            digitalWrite(12, HIGH);

            watch2::oled.setCursor(5, 10);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("no state has been\nloaded");
            //watch2::oled.drawRGBBitmap(0, 29, coolcrab, 128, 55);
            if (dpad_any_active()) watch2::switchState(0);
        }
        else watch2::states[watch2::state].stateFunc();

    }

    //---------------------------------------
    // handle timers
    //---------------------------------------

    else if (watch2::timer_trigger_status != 0)
    {
        if (watch2::timer_trigger_status == 1 || watch2::forceRedraw)
        {
            //dim screen
            watch2::dimScreen(0, 10);
            watch2::oled.fillScreen(BLACK);

            //get time to display
            time_t duration = watch2::timers[watch2::timer_trigger_id].initial_duration;
            time_t time_left_hrs = floor(duration / 3600);
            time_t time_left_min = floor(duration % 3600 / 60);
            time_t time_left_sec = floor(duration % 3600 % 60);

            //draw message
            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.println("Time's up");

            watch2::setFont(REALLY_BIG_FONT);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.printf("%02d:%02d:%02d\n", time_left_hrs, time_left_min, time_left_sec);
            //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);

            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("have elapsed.  press any\nkey to continue");

            //brighten screen
            watch2::dimScreen(1, 10);

            // play music
            SD.begin(sdcs, *watch2::vspi, 4000000U);
            watch2::play_music(watch2::timer_music.c_str(), true);

            //set timer trigger status
            watch2::timer_trigger_status = 2;
        }

        watch2::drawTopThing();
        if (dpad_any_active())
        {
            watch2::timer_trigger_status = 0;
            watch2::stop_music();
            watch2::switchState(0);
        }
    }

    //---------------------------------------
    // handle alarms
    //---------------------------------------

    else if (watch2::alarm_trigger_status != 0)
    {

        static bool selected_alarm_action = 0;
        static bool last_time = 0;
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static uint16_t button_y;
        static uint16_t button_w = 60;
        static uint16_t button_h = 60;
        static uint16_t button_r = 7;
        EXT_RAM_ATTR static TFT_eSprite time_sprite = TFT_eSprite(&watch2::oled);
        static bool pause = false;
        //static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH, 20);
        //0 - dismiss
        //anything else - sleep

        if (watch2::alarm_trigger_status == 1 || watch2::forceRedraw)
        {
            //dim screen
            watch2::dimScreen(0, 10);
            watch2::oled.fillScreen(BLACK);

            //brighten screen
            watch2::dimScreen(1, 10);

            //set timer trigger status
            watch2::alarm_trigger_status = 2;

            //calculate button y pos
            watch2::setFont(REALLY_BIG_FONT);
            button_y = watch2::top_thing_height + watch2::oled.fontHeight() + (watch2::oled.fontHeight() / 4);

            //set up time buffer
            watch2::setFont(REALLY_BIG_FONT, time_sprite);
            time_sprite.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight());
            time_sprite.fillScreen(BLACK);

            uint16_t split_width = SCREEN_WIDTH / 4;

            //draw dismiss button
            watch2::drawImage((*watch2::icons)["dismiss"], split_width - (button_w / 2), button_y);
            
            //draw snooze button
            watch2::drawImage((*watch2::icons)["bed"], (split_width * 3) - (button_w / 2), button_y);
            
            //draw button text
            watch2::oled.fillRect(0, button_y + button_h + (button_h / 4), SCREEN_WIDTH, watch2::oled.fontHeight(), BLACK);
            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextDatum(TC_DATUM);
            String button = "< Dismiss     Snooze! >";
            watch2::oled.drawString(button, SCREEN_WIDTH / 2, button_y + button_h + (button_h / 4));
            watch2::oled.setTextDatum(TL_DATUM);

            watch2::alarm_trigger_status = 3;  //hack hack hack hack

            // play music
            SD.begin(sdcs, *watch2::vspi, 4000000U);
            watch2::play_music(watch2::alarm_music.c_str(), true);
        }

        if (now() != last_time)
        {
            watch2::setFont(REALLY_BIG_FONT, time_sprite);
            time_sprite.fillRect(0, 0, SCREEN_WIDTH, time_sprite.fontHeight(), BLACK);
            time_sprite.setTextColor(watch2::themecolour, BLACK);
            time_sprite.setCursor(2, 0);
            time_sprite.printf("%02d:%02d:%02d", hour(), minute(), second());
            time_sprite.pushSprite(0, watch2::top_thing_height);
            last_time = now();
        }

        watch2::drawTopThing();

        if (dpad_left_active())
        {
            ESP_LOGD(TAG_LOOP, "i wanna be a girl and i can't type");
            Alarm.write(watch2::alarms[watch2::alarm_trigger_id].alarm_id, watch2::alarms[watch2::alarm_trigger_id].initial_time);
        }
        if (dpad_right_active())
        {
            time_t time_alarm = Alarm.read(watch2::alarms[watch2::alarm_trigger_id].alarm_id);
            time_alarm += watch2::alarm_snooze_time; // if this is added to a time later than 23:55, then the alarm will be set to 24:something
            Alarm.write(watch2::alarms[watch2::alarm_trigger_id].alarm_id, time_alarm);
        }

        if (dpad_left_active() || dpad_right_active())
        {
            time_sprite.deleteSprite();
            watch2::alarm_trigger_status = 0;
            watch2::stop_music();
            watch2::switchState(0);
        }

    }

    watch2::endLoop();
    

}


////////////////////////////////////////
// esp32 entrypoint
////////////////////////////////////////

// this is the main entrypoint for the system when being compiled with ESP-IDF.
// this was stolen from the arduino esp32 core:
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/main.cpp

TaskHandle_t loopTaskHandle = NULL;
bool loopTaskWDTEnabled;

void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        if(loopTaskWDTEnabled){
            esp_task_wdt_reset();
        }
        loop();
        if (serialEventRun) serialEventRun();
    }
}

extern "C" void app_main()
{
    loopTaskWDTEnabled = false;
    initArduino();
    xTaskCreateUniversal(loopTask, "loopTask", 20000, NULL, 1, &loopTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
}


////////////////////////////////////////
// system functions
////////////////////////////////////////

extern "C"
{
    void vPortCleanUpTCB ( void *pxTCB ) {
        // place clean up code here
        // this isn't actually used.  this is just here to shut the compiler up if freertos is compiled with
        // static allocation enabled (which it isn;t by default, so if you haven't gone out of your way to
        // modify your esp32 core install, then this doesn't really do anything).
        // see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-freertos-enable-static-task-clean-up
        // for the deets.
    }
}

/*
void Adafruit_GFX::drawRainbowBitmap(int16_t x, int16_t y,
  const uint8_t bitmap[], int16_t w, int16_t h,
  uint16_t bg, int phase_difference) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

	float colour_parts = 360 / w;

    startWrite();
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            float R, G, B;
            if (!watch2::trans_mode)   watch2::HSVtoRGB(&R, &G, &B, fmodf(((((float) i) * colour_parts) + phase_difference), (float)360.0), 1.0, 1.0);
            else                                            watch2::getHeatMapColor(fmod(((float)i/(float)w) + (phase_difference/360.0), (float)1.0), &R, &G, &B);
            if(i & 7) byte <<= 1;
            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            writePixel(x+i, y, (byte & 0x80) ? watch2::oled.color565((int)(R*255.0), (int)(G*255.0), (int)(B*255.0)) : bg);
        }
    }
    endWrite();
}
*/