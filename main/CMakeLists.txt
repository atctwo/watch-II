# set up source files

set(LIB_ARDUINO_IRREMOTE_SRCS
    libraries/Arduino-IRremote/ir_Aiwa.cpp
    #libraries/Arduino-IRremote/ir_BoseWave.cpp
    libraries/Arduino-IRremote/ir_Denon.cpp
    libraries/Arduino-IRremote/ir_Dish.cpp
    libraries/Arduino-IRremote/ir_JVC.cpp
    libraries/Arduino-IRremote/ir_Lego_PF.cpp
    libraries/Arduino-IRremote/ir_LG.cpp
    #libraries/Arduino-IRremote/ir_MagiQuest.cpp
    libraries/Arduino-IRremote/ir_Mitsubishi.cpp
    libraries/Arduino-IRremote/ir_NEC.cpp
    libraries/Arduino-IRremote/ir_Panasonic.cpp
    libraries/Arduino-IRremote/ir_RC5_RC6.cpp
    libraries/Arduino-IRremote/ir_Samsung.cpp
    libraries/Arduino-IRremote/ir_Sanyo.cpp
    libraries/Arduino-IRremote/ir_Sharp.cpp
    #libraries/Arduino-IRremote/ir_Sharp_alt.cpp
    libraries/Arduino-IRremote/ir_Sony.cpp
    libraries/Arduino-IRremote/ir_Template.cpp
    libraries/Arduino-IRremote/ir_Whynter.cpp
    libraries/Arduino-IRremote/irPronto.cpp
    libraries/Arduino-IRremote/irRecv.cpp
    libraries/Arduino-IRremote/IRremote.cpp
    libraries/Arduino-IRremote/irSend.cpp
    #libraries/Arduino-IRremote/src/sam.cpp
    #libraries/Arduino-IRremote/src/esp32.cpp
)

set(LIB_SDFAT_SRCS
    libraries/SdFat/src/MinimumSerial.cpp
    libraries/SdFat/src/FatLib/FatFile.cpp
    libraries/SdFat/src/FatLib/FatFileLFN.cpp
    libraries/SdFat/src/FatLib/FatFilePrint.cpp
    libraries/SdFat/src/FatLib/FatFileSFN.cpp
    libraries/SdFat/src/FatLib/FatVolume.cpp
    libraries/SdFat/src/FatLib/FmtNumber.cpp
    libraries/SdFat/src/FatLib/fstream.cpp
    libraries/SdFat/src/FatLib/istream.cpp
    libraries/SdFat/src/FatLib/ostream.cpp
    libraries/SdFat/src/FatLib/StdioStream.cpp
    libraries/SdFat/src/SdCard/SdioCardEX.cpp
    libraries/SdFat/src/SdCard/SdioTeensy.cpp
    libraries/SdFat/src/SdCard/SdSpiCard.cpp
    libraries/SdFat/src/SdCard/SdSpiCardEX.cpp
    libraries/SdFat/src/SpiDriver/SdSpiESP8266.cpp
    libraries/SdFat/src/SpiDriver/SdSpiParticle.cpp
    libraries/SdFat/src/SpiDriver/SdSpiSAM3X.cpp
    libraries/SdFat/src/SpiDriver/SdSpiSTM32.cpp
    libraries/SdFat/src/SpiDriver/SdSpiTeensy3.cpp
)

set(LIB_NOFRENDO_SRCS 
	libraries/nofrendo/cpu/dis6502.c
	libraries/nofrendo/cpu/nes6502.c
	libraries/nofrendo/libsnss/libsnss.c
	libraries/nofrendo/nes/mmclist.c
	libraries/nofrendo/nes/nes_mmc.c
	libraries/nofrendo/nes/nes_pal.c
	libraries/nofrendo/nes/nes_ppu.c
	libraries/nofrendo/nes/nes_rom.c
	libraries/nofrendo/nes/nes.c
	libraries/nofrendo/nes/nesinput.c
	libraries/nofrendo/nes/nesstate.c
	libraries/nofrendo/sndhrdw/fds_snd.c
	libraries/nofrendo/sndhrdw/mmc5_snd.c
	libraries/nofrendo/sndhrdw/nes_apu.c
	libraries/nofrendo/sndhrdw/vrcvisnd.c
	libraries/nofrendo/mappers/mapvrc.c
	libraries/nofrendo/mappers/map000.c
	libraries/nofrendo/mappers/map001.c
	libraries/nofrendo/mappers/map002.c
	libraries/nofrendo/mappers/map003.c
	libraries/nofrendo/mappers/map004.c
	libraries/nofrendo/mappers/map005.c
	libraries/nofrendo/mappers/map007.c
	libraries/nofrendo/mappers/map008.c
	libraries/nofrendo/mappers/map009.c
	libraries/nofrendo/mappers/map011.c
	libraries/nofrendo/mappers/map015.c
	libraries/nofrendo/mappers/map016.c
	libraries/nofrendo/mappers/map018.c
	libraries/nofrendo/mappers/map019.c
	libraries/nofrendo/mappers/map024.c
	libraries/nofrendo/mappers/map032.c
	libraries/nofrendo/mappers/map033.c
	libraries/nofrendo/mappers/map034.c
	libraries/nofrendo/mappers/map040.c
	libraries/nofrendo/mappers/map041.c
	libraries/nofrendo/mappers/map042.c
	libraries/nofrendo/mappers/map046.c
	libraries/nofrendo/mappers/map050.c
	libraries/nofrendo/mappers/map064.c
	libraries/nofrendo/mappers/map065.c
	libraries/nofrendo/mappers/map066.c
	libraries/nofrendo/mappers/map070.c
	libraries/nofrendo/mappers/map073.c
	libraries/nofrendo/mappers/map075.c
	libraries/nofrendo/mappers/map078.c
	libraries/nofrendo/mappers/map079.c
	libraries/nofrendo/mappers/map085.c
	libraries/nofrendo/mappers/map087.c
	libraries/nofrendo/mappers/map093.c
	libraries/nofrendo/mappers/map094.c
	libraries/nofrendo/mappers/map099.c
	libraries/nofrendo/mappers/map160.c
	libraries/nofrendo/mappers/map229.c
	libraries/nofrendo/mappers/map231.c
	libraries/nofrendo/bitmap.c
	libraries/nofrendo/config.c
	libraries/nofrendo/event.c
	libraries/nofrendo/gui_elem.c
	libraries/nofrendo/gui.c
	libraries/nofrendo/intro.c
	libraries/nofrendo/log.c
	libraries/nofrendo/memguard.c
	libraries/nofrendo/nofrendo.c
	libraries/nofrendo/pcx.c
	libraries/nofrendo/vid_drv.c
)

set(LIB_ARDUINO_SRCS
    libraries/JC_Button/src/JC_Button.cpp
    libraries/JC_Button_MCP23008/src/JC_Button_MCP23008.cpp
    libraries/Time/Time.cpp
    libraries/Time/DateStrings.cpp
    libraries/TimeAlarms/TimeAlarms.cpp
    libraries/TFT_eSPI/TFT_eSPI.cpp
    libraries/ESP32-BLE-Keyboard/BleKeyboard.cpp
    libraries/ESP32-BLE-Keyboard/BleConnectionStatus.cpp
    libraries/ESP32-BLE-Keyboard/KeyboardOutputCallbacks.cpp
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
    libraries/ESP32-audioI2S/src/Audio.cpp
    libraries/css-color-parser-cpp/csscolorparser.cpp
    libraries/tinyexpr/tinyexpr.c
    libraries/entities/entities.c
    libraries/Adafruit_BusIO/Adafruit_BusIO_Register.cpp
    libraries/Adafruit_BusIO/Adafruit_I2CDevice.cpp
    libraries/Adafruit_BusIO/Adafruit_SPIDevice.cpp
    libraries/Adafruit-MCP23008-library/Adafruit_MCP23008.cpp
    states/state_nes.cpp
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
    watch2_battery.cpp
)

set(STATE_SRCS 
    states/state_alarms.cpp
    states/state_calc.cpp
    states/state_image_viewer.cpp
    states/state_init.cpp
    states/state_ir_remote.cpp
    states/state_notepad.cpp
    states/state_sdtest.cpp
    states/state_settings.cpp
    states/state_state_menu.cpp
    states/state_stopwatch.cpp
    states/state_timer.cpp
    states/state_watch_face.cpp
    states/state_wiki.cpp
    states/states.cpp
)

set(COMPONENT_SRCS 
    ${LIB_ARDUINO_IRREMOTE_SRCS} 
    ${LIB_SDFAT_SRCS}
    ${LIB_NOFRENDO_SRCS}
    ${LIB_ARDUINO_SRCS}
    ${STATE_SRCS} 
    ${WATCH_SRCS} 
    "main.cpp"
)


# add includes

set(COMPONENT_INCLUDEDIRS 
    libraries/SdFat/src/ 			
    libraries/JC_Button/src/ 	
    libraries/JC_Button_MCP23008/src/ 	
    libraries/Arduino-IRremote/
    libraries/TFT_eSPI/ 			
    libraries/Time/ 				
    libraries/TimeAlarms/ 			
    libraries/tinyexpr/
    libraries/entities/
    libraries/ESP32-BLE-Keyboard/
    libraries/ESP32-audioI2S/src/
    libraries/Adafruit_BusIO/
    libraries/Adafruit-MCP23008-library/
    #libraries/micropython/
    libraries/nofrendo/cpu/
    libraries/nofrendo/libsnss/
    libraries/nofrendo/nes/
    libraries/nofrendo/sndhrdw/
    libraries/nofrendo/
)

idf_component_register(SRCS         ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_INCLUDEDIRS}
                        )

set_source_files_properties(
    "libraries/TFT_eSPI/TFT_eSPI.cpp"
    PROPERTIES COMPILE_FLAGS
    "-x c++ -std=gnu++17"
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

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=char-subscripts)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=stringop-truncation)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=attributes)
#target_compile_options(${COMPONENT_LIB} PRIVATE -DNOFRENDO_DEBUG)

# micropython
# add_prebuilt_library(micropython "${COMPONENT_PATH}/libraries/micropython/libmicropython.a")
# target_link_libraries(${COMPONENT_LIB} PUBLIC micropython)