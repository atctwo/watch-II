# set up source files

set(LIB_ARDUINO_IRREMOTE_SRCS
    libraries/IRremoteESP8266/src/ir_Airwell.cpp
    libraries/IRremoteESP8266/src/ir_Aiwa.cpp
    libraries/IRremoteESP8266/src/ir_Amcor.cpp
    libraries/IRremoteESP8266/src/ir_Argo.cpp
    libraries/IRremoteESP8266/src/ir_Carrier.cpp
    libraries/IRremoteESP8266/src/ir_Coolix.cpp
    libraries/IRremoteESP8266/src/ir_Corona.cpp
    libraries/IRremoteESP8266/src/ir_Daikin.cpp
    libraries/IRremoteESP8266/src/ir_Delonghi.cpp
    libraries/IRremoteESP8266/src/ir_Denon.cpp
    libraries/IRremoteESP8266/src/ir_Dish.cpp
    libraries/IRremoteESP8266/src/ir_Doshisha.cpp
    libraries/IRremoteESP8266/src/ir_Ecoclim.cpp
    libraries/IRremoteESP8266/src/ir_Electra.cpp
    libraries/IRremoteESP8266/src/ir_EliteScreens.cpp
    libraries/IRremoteESP8266/src/ir_Epson.cpp
    libraries/IRremoteESP8266/src/ir_Fujitsu.cpp
    libraries/IRremoteESP8266/src/ir_GICable.cpp
    libraries/IRremoteESP8266/src/ir_GlobalCache.cpp
    libraries/IRremoteESP8266/src/ir_Goodweather.cpp
    libraries/IRremoteESP8266/src/ir_Gree.cpp
    libraries/IRremoteESP8266/src/ir_Haier.cpp
    libraries/IRremoteESP8266/src/ir_Hitachi.cpp
    libraries/IRremoteESP8266/src/ir_Inax.cpp
    libraries/IRremoteESP8266/src/ir_JVC.cpp
    libraries/IRremoteESP8266/src/ir_Kelvinator.cpp
    libraries/IRremoteESP8266/src/ir_Lasertag.cpp
    libraries/IRremoteESP8266/src/ir_Lego.cpp
    libraries/IRremoteESP8266/src/ir_LG.cpp
    libraries/IRremoteESP8266/src/ir_Lutron.cpp
    libraries/IRremoteESP8266/src/ir_Magiquest.cpp
    libraries/IRremoteESP8266/src/ir_Metz.cpp
    libraries/IRremoteESP8266/src/ir_Midea.cpp
    libraries/IRremoteESP8266/src/ir_MilesTag2.cpp
    libraries/IRremoteESP8266/src/ir_Mirage.cpp
    libraries/IRremoteESP8266/src/ir_Mitsubishi.cpp
    libraries/IRremoteESP8266/src/ir_MitsubishiHeavy.cpp
    libraries/IRremoteESP8266/src/ir_Multibrackets.cpp
    libraries/IRremoteESP8266/src/ir_MWM.cpp
    libraries/IRremoteESP8266/src/ir_NEC.cpp
    libraries/IRremoteESP8266/src/ir_Neoclima.cpp
    libraries/IRremoteESP8266/src/ir_Nikai.cpp
    libraries/IRremoteESP8266/src/ir_Panasonic.cpp
    libraries/IRremoteESP8266/src/ir_Pioneer.cpp
    libraries/IRremoteESP8266/src/ir_Pronto.cpp
    libraries/IRremoteESP8266/src/ir_RC5_RC6.cpp
    libraries/IRremoteESP8266/src/ir_RCMM.cpp
    libraries/IRremoteESP8266/src/ir_Samsung.cpp
    libraries/IRremoteESP8266/src/ir_Sanyo.cpp
    libraries/IRremoteESP8266/src/ir_Sharp.cpp
    libraries/IRremoteESP8266/src/ir_Sherwood.cpp
    libraries/IRremoteESP8266/src/ir_Sony.cpp
    libraries/IRremoteESP8266/src/ir_Symphony.cpp
    libraries/IRremoteESP8266/src/ir_Tcl.cpp
    libraries/IRremoteESP8266/src/ir_Technibel.cpp
    libraries/IRremoteESP8266/src/ir_Teco.cpp
    libraries/IRremoteESP8266/src/ir_Toshiba.cpp
    libraries/IRremoteESP8266/src/ir_Transcold.cpp
    libraries/IRremoteESP8266/src/ir_Trotec.cpp
    libraries/IRremoteESP8266/src/ir_Truma.cpp
    libraries/IRremoteESP8266/src/ir_Vestel.cpp
    libraries/IRremoteESP8266/src/ir_Voltas.cpp
    libraries/IRremoteESP8266/src/ir_Whirlpool.cpp
    libraries/IRremoteESP8266/src/ir_Whynter.cpp
    libraries/IRremoteESP8266/src/ir_Xmp.cpp
    libraries/IRremoteESP8266/src/ir_Zepeal.cpp
    libraries/IRremoteESP8266/src/IRac.cpp
    libraries/IRremoteESP8266/src/IRrecv.cpp
    libraries/IRremoteESP8266/src/IRsend.cpp
    libraries/IRremoteESP8266/src/IRtext.cpp
    libraries/IRremoteESP8266/src/IRtimer.cpp
    libraries/IRremoteESP8266/src/IRutils.cpp
)

set(LIB_ARDUINO_SRCS
    libraries/JC_Button/src/JC_Button.cpp
    libraries/Time/Time.cpp
    libraries/Time/DateStrings.cpp
    libraries/TimeAlarms/TimeAlarms.cpp
    libraries/TFT_eSPI/TFT_eSPI.cpp
    libraries/gumbo-parser/src/attribute.c
    libraries/gumbo-parser/src/char_ref.c
    libraries/gumbo-parser/src/error.c
    libraries/gumbo-parser/src/parser.c
    libraries/gumbo-parser/src/string_buffer.c
    libraries/gumbo-parser/src/string_piece.c
    libraries/gumbo-parser/src/tag.c
    libraries/gumbo-parser/src/tokenizer.c
    libraries/gumbo-parser/src/utf8.c
    libraries/gumbo-parser/src/util.c
    libraries/gumbo-parser/src/vector.c
    libraries/ESP32-audioI2S/src/aac_decoder/aac_decoder.cpp
    libraries/ESP32-audioI2S/src/mp3_decoder/mp3_decoder.cpp
    libraries/ESP32-audioI2S/src/flac_decoder/flac_decoder.cpp
    libraries/ESP32-audioI2S/src/Audio.cpp
    libraries/css-color-parser-cpp/csscolorparser.cpp
    libraries/tinyexpr/tinyexpr.c
    libraries/entities/entities.c
    libraries/tinywav/tinywav.c
    libraries/Adafruit_BusIO/Adafruit_BusIO_Register.cpp
    libraries/Adafruit_BusIO/Adafruit_I2CDevice.cpp
    libraries/Adafruit_BusIO/Adafruit_SPIDevice.cpp
    libraries/Adafruit-MCP23008-library/Adafruit_MCP23008.cpp
    libraries/Adafruit_MCP9808_Library/Adafruit_MCP9808.cpp
    libraries/Adafruit_LC709203F/Adafruit_LC709203F.cpp
    libraries/Adafruit_Sensor/Adafruit_Sensor.cpp
    libraries/Adafruit_SHTC3/Adafruit_SHTC3.cpp
    libraries/snake-game/snake.cpp
    icons/small_icons.cpp
)

set(WATCH_SRCS
    root_store.cpp
    watch2_system.cpp
    watch2_utility.cpp
    watch2_drawing.cpp
    watch2_images.cpp
    watch2_files.cpp
    watch2_dialogues.cpp
    watch2_wifi.cpp
    watch2_bluetooth.cpp
    watch2_audio.cpp
)

set(STATE_SRCS 
    states/state_alarms.cpp
    states/state_bt_remote.cpp
    states/state_calc.cpp
    states/state_image_viewer.cpp
    states/state_init.cpp
    states/state_ir_remote.cpp
    states/state_music_player.cpp
    states/state_nes.cpp
    states/state_notepad.cpp
    states/state_quiz.cpp
    states/state_radio.cpp
    states/state_recorder.cpp
    states/state_sdtest.cpp
    states/state_sensor.cpp
    states/state_settings.cpp
    states/state_snek.cpp
    states/state_state_menu.cpp
    states/state_stopwatch.cpp
    states/state_tetris.cpp
    states/state_timer.cpp
    states/state_watch_face.cpp
    states/state_weather.cpp
    states/state_wiki.cpp
    states/states.cpp
)

set(COMPONENT_SRCS 
    ${LIB_ARDUINO_SRCS}
    ${LIB_ARDUINO_IRREMOTE_SRCS}
    ${STATE_SRCS} 
    ${WATCH_SRCS} 
    "main.cpp"
)


# add includes

set(COMPONENT_INCLUDEDIRS			
    libraries/JC_Button/src/
    libraries/IRremoteESP8266/src/
    libraries/TFT_eSPI/ 			
    libraries/Time/ 				
    libraries/TimeAlarms/ 			
    libraries/tinyexpr/
    libraries/entities/
    libraries/tinywav/
    libraries/ESP32-audioI2S/src/
    libraries/Adafruit_BusIO/
    libraries/Adafruit-MCP23008-library/
    libraries/Adafruit_MCP9808_Library/
    libraries/Adafruit_LC709203F/
    libraries/Adafruit_Sensor/
    libraries/Adafruit_SHTC3/
    libraries/snake-game/
    #libraries/micropython/
)

idf_component_register(SRCS         ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_INCLUDEDIRS}
                        )

set_source_files_properties(
    "libraries/TFT_eSPI/TFT_eSPI.cpp"
    PROPERTIES COMPILE_FLAGS
    "-x c++ -std=gnu++17"
)

set_source_files_properties(
    "libraries/ESP32-audioI2S/src/Audio.cpp"
    PROPERTIES COMPILE_FLAGS
    "-DTAG='\"esp32 audio\"'"
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-value)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-function)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-narrowing)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-address)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-char-subscripts)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-but-set-variable)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-return-type)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-misleading-indentation)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-class-memaccess)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-sizeof-pointer-div)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)
target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)
target_compile_options(${COMPONENT_LIB} PRIVATE -D_DEBUG)
target_compile_options(${COMPONENT_LIB} PRIVATE -DUNICODE)
target_compile_options(${COMPONENT_LIB} PRIVATE -D_UNICODE)
target_compile_options(${COMPONENT_LIB} PRIVATE -DARDUINO=10900)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP_PLATFORM)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP32)
target_compile_options(${COMPONENT_LIB} PRIVATE -DARDUINO_ESP32_DEV)
target_compile_options(${COMPONENT_LIB} PRIVATE -DARDUINO_ARCH_ESP32)
target_compile_options(${COMPONENT_LIB} PRIVATE -DARDUINO_BOARD=\"ESP32_DEV\")
target_compile_options(${COMPONENT_LIB} PRIVATE -DMBEDTLS_CONFIG_FILE=\"mbedtls/esp_config.h\")
target_compile_options(${COMPONENT_LIB} PRIVATE -DHAVE_CONFIG_H)

# micropython
# add_prebuilt_library(micropython "${COMPONENT_PATH}/libraries/micropython/libmicropython.a")
# target_link_libraries(${COMPONENT_LIB} PUBLIC micropython)