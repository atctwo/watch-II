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

set (LIB_ESP8266AUDIO_SRCS
    libraries/ESP8266Audio/src/driver/SinglePinI2SDriver.cpp
    libraries/ESP8266Audio/src/AudioFileSourceBuffer.cpp
    libraries/ESP8266Audio/src/AudioFileSourceFS.cpp
    libraries/ESP8266Audio/src/AudioFileSourceHTTPStream.cpp
    libraries/ESP8266Audio/src/AudioFileSourceICYStream.cpp
    libraries/ESP8266Audio/src/AudioFileSourceID3.cpp
    libraries/ESP8266Audio/src/AudioFileSourcePROGMEM.cpp
    libraries/ESP8266Audio/src/AudioFileSourceSD.cpp
    libraries/ESP8266Audio/src/AudioFileSourceSPIRAMBuffer.cpp
    libraries/ESP8266Audio/src/AudioFileSourceSTDIO.cpp
    libraries/ESP8266Audio/src/AudioFileStream.cpp
    libraries/ESP8266Audio/src/AudioGeneratorAAC.cpp
    libraries/ESP8266Audio/src/AudioGeneratorFLAC.cpp
    libraries/ESP8266Audio/src/AudioGeneratorMIDI.cpp
    libraries/ESP8266Audio/src/AudioGeneratorMOD.cpp
    libraries/ESP8266Audio/src/AudioGeneratorMP3.cpp
    libraries/ESP8266Audio/src/AudioGeneratorMP3a.cpp
    libraries/ESP8266Audio/src/AudioGeneratorRTTTL.cpp
    libraries/ESP8266Audio/src/AudioGeneratorTalkie.cpp
    libraries/ESP8266Audio/src/AudioGeneratorWAV.cpp
    libraries/ESP8266Audio/src/AudioLogger.cpp
    libraries/ESP8266Audio/src/AudioOutputBuffer.cpp
    libraries/ESP8266Audio/src/AudioOutputFilterDecimate.cpp
    libraries/ESP8266Audio/src/AudioOutputI2S.cpp
    libraries/ESP8266Audio/src/AudioOutputI2SNoDAC.cpp
    libraries/ESP8266Audio/src/AudioOutputMixer.cpp
    libraries/ESP8266Audio/src/AudioOutputSerialWAV.cpp
    libraries/ESP8266Audio/src/AudioOutputSPDIF.cpp
    libraries/ESP8266Audio/src/AudioOutputSPIFFSWAV.cpp
    libraries/ESP8266Audio/src/AudioOutputSTDIO.cpp
    libraries/ESP8266Audio/src/libflac/bitmath.c
    libraries/ESP8266Audio/src/libflac/bitreader.c
    libraries/ESP8266Audio/src/libflac/cpu.c
    libraries/ESP8266Audio/src/libflac/crc.c
    libraries/ESP8266Audio/src/libflac/fixed.c
    libraries/ESP8266Audio/src/libflac/float.c
    libraries/ESP8266Audio/src/libflac/format.c
    libraries/ESP8266Audio/src/libflac/lpc.c
    libraries/ESP8266Audio/src/libflac/md5.c
    libraries/ESP8266Audio/src/libflac/memory.c
    libraries/ESP8266Audio/src/libflac/stream_decoder.c
    libraries/ESP8266Audio/src/libflac/window.c
    libraries/ESP8266Audio/src/libhelix-aac/aacdec.c
    libraries/ESP8266Audio/src/libhelix-aac/aactabs.c
    libraries/ESP8266Audio/src/libhelix-aac/bitstream.c
    libraries/ESP8266Audio/src/libhelix-aac/buffers.c
    libraries/ESP8266Audio/src/libhelix-aac/dct4.c
    libraries/ESP8266Audio/src/libhelix-aac/decelmnt.c
    libraries/ESP8266Audio/src/libhelix-aac/dequant.c
    libraries/ESP8266Audio/src/libhelix-aac/fft.c
    libraries/ESP8266Audio/src/libhelix-aac/filefmt.c
    libraries/ESP8266Audio/src/libhelix-aac/huffman.c
    libraries/ESP8266Audio/src/libhelix-aac/hufftabs.c
    libraries/ESP8266Audio/src/libhelix-aac/imdct.c
    libraries/ESP8266Audio/src/libhelix-aac/noiseless.c
    libraries/ESP8266Audio/src/libhelix-aac/pns.c
    libraries/ESP8266Audio/src/libhelix-aac/sbr.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrfft.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrfreq.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrhfadj.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrhfgen.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrhuff.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrimdct.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrmath.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrqmf.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrside.c
    libraries/ESP8266Audio/src/libhelix-aac/sbrtabs.c
    libraries/ESP8266Audio/src/libhelix-aac/stproc.c
    libraries/ESP8266Audio/src/libhelix-aac/tns.c
    libraries/ESP8266Audio/src/libhelix-aac/trigtabs.c
    libraries/ESP8266Audio/src/libhelix-mp3/bitstream.c
    libraries/ESP8266Audio/src/libhelix-mp3/buffers.c
    libraries/ESP8266Audio/src/libhelix-mp3/dct32.c
    libraries/ESP8266Audio/src/libhelix-mp3/dequant.c
    libraries/ESP8266Audio/src/libhelix-mp3/dqchan.c
    libraries/ESP8266Audio/src/libhelix-mp3/huffman.c
    libraries/ESP8266Audio/src/libhelix-mp3/hufftabs.c
    libraries/ESP8266Audio/src/libhelix-mp3/imdct.c
    libraries/ESP8266Audio/src/libhelix-mp3/mp3dec.c
    libraries/ESP8266Audio/src/libhelix-mp3/mp3tabs.c
    libraries/ESP8266Audio/src/libhelix-mp3/polyphase.c
    libraries/ESP8266Audio/src/libhelix-mp3/scalfact.c
    libraries/ESP8266Audio/src/libhelix-mp3/stproc.c
    libraries/ESP8266Audio/src/libhelix-mp3/subband.c
    libraries/ESP8266Audio/src/libhelix-mp3/trigtabs.c
    libraries/ESP8266Audio/src/libmad/bit.c
    libraries/ESP8266Audio/src/libmad/decoder.c
    libraries/ESP8266Audio/src/libmad/fixed.c
    libraries/ESP8266Audio/src/libmad/frame.c
    libraries/ESP8266Audio/src/libmad/huffman.c
    libraries/ESP8266Audio/src/libmad/layer3.c
    libraries/ESP8266Audio/src/libmad/stream.c
    libraries/ESP8266Audio/src/libmad/synth.c
    libraries/ESP8266Audio/src/libmad/timer.c
    libraries/ESP8266Audio/src/libmad/version.c
)

set(LIB_ARDUINO_SRCS
    libraries/JC_Button/src/JC_Button.cpp
    libraries/Time/Time.cpp
    libraries/Time/DateStrings.cpp
    libraries/TimeAlarms/TimeAlarms.cpp
    libraries/TFT_eSPI/TFT_eSPI.cpp
    libraries/ESP32-BLE-Keyboard/BleKeyboard.cpp
    libraries/ESP32-BLE-Keyboard/BleConnectionStatus.cpp
    libraries/ESP32-BLE-Keyboard/KeyboardOutputCallbacks.cpp
)

set(WATCH_SRCS
    libraries/css-color-parser-cpp/csscolorparser.cpp
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
    libraries/tinyexpr/tinyexpr.c
    libraries/entities/entities.c
    root_store.cpp
    watch2.cpp
    #SdFatFSWrapper.cpp
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
    ${LIB_ARDUINO_SRCS}
    ${LIB_ARDUINO_IRREMOTE_SRCS} 
    ${LIB_SDFAT_SRCS} 
    ${LIB_ESP8266AUDIO_SRCS}
    ${STATE_SRCS} 
    ${WATCH_SRCS} 
    "main.cpp"
)


# add includes

set(COMPONENT_INCLUDEDIRS 
    libraries/SdFat/src/ 			
    libraries/JC_Button/src/ 	
    libraries/Arduino-IRremote/
    libraries/TFT_eSPI/ 			
    libraries/Time/ 				
    libraries/TimeAlarms/ 			
    libraries/tinyexpr/
    libraries/entities/
    libraries/ESP32-BLE-Keyboard/
    libraries/ESP8266Audio/src/
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