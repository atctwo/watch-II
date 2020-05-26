#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

ARDUINO_ULIBRARIES_LIST := $(patsubst $(COMPONENT_PATH)/libraries/%,%,$(wildcard $(COMPONENT_PATH)/libraries/*))
ARDUINO_SINGLE_ULIBRARY_FILES = $(patsubst $(COMPONENT_PATH)/%,%,$(sort $(dir $(wildcard $(COMPONENT_PATH)/libraries/$(MODULE)/*)) $(dir $(wildcard $(COMPONENT_PATH)/libraries/$(MODULE)/src/*/)) $(dir $(wildcard $(COMPONENT_PATH)/libraries/$(MODULE)/src/*/*/)) $(dir $(wildcard $(COMPONENT_PATH)/libraries/$(MODULE)/src/*/*/*/)) $(dir $(wildcard $(COMPONENT_PATH)/libraries/$(MODULE)/src/*/*/*/*/))))
ARDUINO_USER_LIBS := $(foreach MODULE,$(ARDUINO_ULIBRARIES_LIST), $(ARDUINO_SINGLE_ULIBRARY_FILES))

# ARDUINO_USER_LIBS should include the libraries, but I'm getting "something.h: No such file or directory" errors without
# manually adding the libraries to the include dirs variable
COMPONENT_ADD_INCLUDEDIRS := $(ARDUINO_USER_LIBS) main . 	\
							 libraries/SdFat/src/ 			\
							 libraries/JC_Button/src/ 		\
							 libraries/Arduino-IRremote/ 	\
							 libraries/TFT_eSPI/ 			\
							 libraries/Time/ 				\
							 libraries/TimeAlarms/ 			\
							 libraries/tinyexpr/src/
COMPONENT_SRCDIRS := $(ARDUINO_USER_LIBS) main .

CFLAGS+=-Wno-error=unused-value -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=format=
CXXFLAGS+=-Wno-error=unused-value -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=format=