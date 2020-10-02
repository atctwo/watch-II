#include "../watch2.h"

void registerSmallIcons()
{
    watch2::registerSmallIcon("internet_time", {

        // 'internet_time', 30x30px
		0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x01, 0xff, 0xfe, 0x00, 
		0x03, 0x98, 0x67, 0x00, 0x07, 0x30, 0x33, 0x80, 0x0c, 0x30, 0x31, 0xc0, 0x0c, 0x60, 0x10, 0xc0, 
		0x18, 0x60, 0x18, 0x60, 0x1f, 0xff, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xf0, 0x30, 0xc0, 0x08, 0x30, 
		0x20, 0xc0, 0x0c, 0x30, 0x20, 0xc0, 0x00, 0x10, 0x20, 0xc0, 0x00, 0x10, 0x20, 0xc0, 0x0f, 0x10, 
		0x20, 0xc0, 0x3f, 0xc0, 0x30, 0xc0, 0x7f, 0xe0, 0x3f, 0xfc, 0xff, 0x60, 0x1f, 0xfc, 0xfe, 0x70, 
		0x18, 0x60, 0xf8, 0xf0, 0x0c, 0x60, 0xf9, 0xf0, 0x0e, 0x30, 0xfb, 0xf0, 0x07, 0x30, 0xff, 0xf0, 
		0x03, 0x98, 0xff, 0xe0, 0x00, 0xfe, 0x7f, 0xe0, 0x00, 0x7f, 0x3f, 0xc0, 0x00, 0x0f, 0x0f, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_letters", {

        // 'letters', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x80, 0x00, 0x00, 0x80, 0x00, 0x38, 0xb0, 0xe0, 0x44, 0x89, 0x10, 0x04, 0x89, 0x00, 0x34, 0x80, 
        0x00, 0x44, 0x81, 0x00, 0x44, 0x89, 0x10, 0x34, 0xb0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_symbols", {

        // 'symbols', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 
        0xc2, 0x40, 0x22, 0x22, 0x40, 0x22, 0x2f, 0xe0, 0x20, 0x22, 0x40, 0x20, 0x44, 0x80, 0x20, 0x8f, 
        0xe0, 0x20, 0x84, 0x80, 0x00, 0x04, 0x80, 0x20, 0x84, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_clear", {

        // 'ce', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x07, 0x1f, 0x80, 0x18, 0x90, 0x00, 0x10, 0x10, 0x00, 0x10, 0x1f, 0x80, 0x10, 0x10, 
        0x00, 0x10, 0x50, 0x00, 0x18, 0x90, 0x00, 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_cancel", {

        // 'cancel', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0e, 0x00, 0x03, 
        0x0c, 0x00, 0x03, 0x98, 0x00, 0x01, 0xb8, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xf0, 
        0x00, 0x01, 0xf0, 0x00, 0x01, 0x98, 0x00, 0x03, 0x1c, 0x00, 0x07, 0x0c, 0x00, 0x06, 0x06, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_tick", {

        // 'tick', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x03, 0x80, 0x00, 0x07, 0x00, 0x00, 
        0x0e, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x38, 0x00, 0x00, 0x30, 0x00, 0x00, 0x70, 
        0x00, 0x0c, 0xe0, 0x00, 0x1c, 0xc0, 0x00, 0x1f, 0xc0, 0x00, 0x0f, 0x80, 0x00, 0x0f, 0x00, 0x00, 
        0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_move_right", {

        // 'move_right', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x80, 0x00, 0x03, 0xc0, 0x7f, 0xff, 
        0xe0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0x80, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_move_left", {

        // 'move_left', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7f, 0xff, 0xe0, 0x3c, 0x00, 
        0x00, 0x1c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_caps_off", {

        // 'caps_off', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x90, 0x00, 0x01, 0x08, 0x00, 0x02, 
        0x04, 0x00, 0x07, 0x0e, 0x00, 0x01, 0x08, 0x00, 0x01, 0x08, 0x00, 0x01, 0x08, 0x00, 0x01, 0x08, 
        0x00, 0x01, 0x08, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_caps_on", {

        // 'caps_on', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xf0, 0x00, 0x01, 0xf8, 0x00, 0x03, 
        0xfc, 0x00, 0x07, 0xfe, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 
        0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x01, 0xf8, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_space", {

        // 'space', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0xc0, 0x30, 0x00, 0xc0, 0x30, 0x00, 0xc0, 0x3f, 0xff, 0xc0, 
        0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("key_backspace", {
        // 'backspace', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x03, 0xff, 0x00, 0x07, 0x7b, 0x80, 0x0f, 0xb7, 0x80, 0x1f, 0xcf, 0x80, 0x1f, 0xcf, 
        0x80, 0x0f, 0xb7, 0x80, 0x07, 0x7b, 0x80, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    });

    watch2::registerSmallIcon("small_ram", {

        // 'ram', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xc0, 0x66, 0x66, 0x60, 0x66, 0x66, 0x60, 0x7f, 0xff, 
        0xe0, 0x3f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_battery", {

        // 'battery', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x01, 0xf8, 0x00, 0x03, 0xfc, 0x00, 0x03, 
        0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 
        0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x0c, 0x00, 
        0x03, 0xfc, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_wifi_3", {

        // 'wifi_small_3', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x0f, 0xff, 0x00, 0x38, 0x01, 0xc0, 0x60, 
        0x00, 0x60, 0xc0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_wifi_2", {

        // 'wifi_small_2', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x60, 0x00, 0x03, 0xfc, 0x00, 0x0c, 0x03, 0x00, 0x18, 0x01, 0x80, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_wifi_1", {

        // 'wifi_small_1', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x01, 0xf8, 0x00, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_wifi_0", {

        // 'wifi_small_0', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 
        0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_wifi_complete", {

        // 'wifi_small_complete', 20x20px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x0f, 0xff, 0x00, 0x38, 0x01, 0xc0, 0x60, 
        0x00, 0x60, 0xc0, 0x60, 0x30, 0x03, 0xfc, 0x00, 0x0c, 0x03, 0x00, 0x18, 0x01, 0x80, 0x00, 0x00, 
        0x00, 0x01, 0xf8, 0x00, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 
        0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("small_sd", {

        // 'small_sd, 20x20px
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x3f, 
        0xff, 0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 
        0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0x80, 0x3f, 0x38, 0x00, 0x1f, 0x30, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("wifi", {

        // 'wifi', 30x30px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x7f, 0xf8, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x07, 0xff, 0xff, 0x80, 0x1f, 0xc0, 0x0f, 0xe0, 
        0x3e, 0x00, 0x01, 0xf0, 0x78, 0x00, 0x00, 0x78, 0xf0, 0x00, 0x00, 0x3c, 0xe0, 0x1f, 0xe0, 0x1c, 
        0x00, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x03, 0xe0, 0x1f, 0x00, 0x07, 0x80, 0x07, 0x80, 
        0x07, 0x00, 0x03, 0x80, 0x00, 0x07, 0x80, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x3f, 0xf0, 0x00, 
        0x00, 0x38, 0x70, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 
        0x00, 0x07, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("sun", {

        // 'sun', 30x30px
        0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 
        0x0c, 0x03, 0x00, 0xc0, 0x0e, 0x00, 0x01, 0xc0, 0x07, 0x00, 0x03, 0x80, 0x03, 0x80, 0x07, 0x00, 
        0x01, 0x07, 0x82, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x60, 0x18, 0x00, 
        0x00, 0xc0, 0x0c, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0xf8, 0xc0, 0x0c, 0x7c, 0xf8, 0xc0, 0x0c, 0x7c, 
        0x00, 0xc0, 0x0c, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x30, 0x30, 0x00, 
        0x00, 0x1f, 0xe0, 0x00, 0x01, 0x0f, 0xc2, 0x00, 0x03, 0x80, 0x07, 0x00, 0x07, 0x00, 0x03, 0x80, 
        0x0e, 0x00, 0x01, 0xc0, 0x0c, 0x03, 0x00, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 
        0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00

    });

    watch2::registerSmallIcon("bluetooth", {

        // 'bluetooth', 30x30px
        0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x03, 0xf0, 0x00, 
        0x00, 0x03, 0xf8, 0x00, 0x00, 0x03, 0xbc, 0x00, 0x01, 0x03, 0x9e, 0x00, 0x03, 0x83, 0x8f, 0x00, 
        0x01, 0xc3, 0x8f, 0x00, 0x00, 0xe3, 0x9e, 0x00, 0x00, 0x73, 0xbc, 0x00, 0x00, 0x3b, 0xf8, 0x00, 
        0x00, 0x1f, 0xf0, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x07, 0xc0, 0x00, 
        0x00, 0x0f, 0xe0, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x3b, 0xf8, 0x00, 0x00, 0x73, 0xbc, 0x00, 
        0x00, 0xe3, 0x9e, 0x00, 0x01, 0xc3, 0x8f, 0x00, 0x03, 0x83, 0x8f, 0x00, 0x01, 0x03, 0x9e, 0x00, 
        0x00, 0x03, 0xbc, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x03, 0xe0, 0x00, 
        0x00, 0x03, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00

    });

    watch2::registerSmallIcon("torch_but_smaller", {

        // 'torch_but_smaller', 30x30px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x20, 
        0x00, 0x00, 0x00, 0x40, 0x00, 0x01, 0x80, 0x80, 0x00, 0x02, 0x41, 0x00, 0x00, 0x04, 0x20, 0x00, 
        0x00, 0x0c, 0x10, 0x18, 0x00, 0x0e, 0x08, 0x20, 0x00, 0x03, 0x04, 0x00, 0x00, 0x11, 0x82, 0x00, 
        0x00, 0x10, 0xc1, 0x00, 0x00, 0x20, 0x60, 0x80, 0x00, 0x20, 0x30, 0x80, 0x00, 0x40, 0x19, 0x00, 
        0x00, 0x80, 0x0e, 0x00, 0x01, 0x80, 0x0c, 0x00, 0x02, 0x40, 0x60, 0x00, 0x04, 0x21, 0x80, 0x00, 
        0x09, 0xd2, 0x00, 0x00, 0x11, 0x4c, 0x00, 0x00, 0x32, 0xc8, 0x00, 0x00, 0x49, 0x10, 0x00, 0x00, 
        0x84, 0x20, 0x00, 0x00, 0x82, 0x40, 0x00, 0x00, 0xc1, 0x80, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00, 
        0x32, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("volume", {

        // 'volume', 30x30px
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xc0, 0x00, 0x07, 0x80, 0xc0, 0x00, 0x0f, 0x86, 0x60, 
        0x00, 0x1f, 0x86, 0x70, 0x00, 0x3f, 0x83, 0x30, 0x3f, 0xff, 0xb3, 0x30, 0x7f, 0xff, 0xb9, 0x98, 
        0x7f, 0xff, 0x99, 0x98, 0x7f, 0xff, 0x99, 0x98, 0x7f, 0xff, 0x99, 0x98, 0x7f, 0xff, 0x89, 0x98, 
        0x7f, 0xff, 0x99, 0x98, 0x7f, 0xff, 0x99, 0x98, 0x7f, 0xff, 0x99, 0x98, 0x3f, 0xff, 0xb1, 0xb0, 
        0x00, 0x7f, 0x83, 0x30, 0x00, 0x3f, 0x83, 0x30, 0x00, 0x1f, 0x86, 0x60, 0x00, 0x0f, 0x80, 0x60, 
        0x00, 0x03, 0x80, 0xc0, 0x00, 0x01, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("x", {

        // 'small_x', 10x10px
        0x00, 0x00, 0x61, 0x80, 0x73, 0x80, 0x3f, 0x00, 0x1e, 0x00, 0x1e, 0x00, 0x3f, 0x00, 0x73, 0x80, 
        0x61, 0x80, 0x00, 0x00

    });

    watch2::registerSmallIcon("pause", {

        // 'small_pause', 10x10px
        0x00, 0x00, 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 0x33, 0x00, 
        0x33, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("play", {

        // 'small_play', 10x10px
        0x00, 0x00, 0x30, 0x00, 0x38, 0x00, 0x3c, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x38, 0x00, 
        0x30, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("add", {

        // 'small_add', 10x10px
        0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x7f, 0x80, 0x7f, 0x80, 0x0c, 0x00, 0x0c, 0x00, 
        0x0c, 0x00, 0x00, 0x00

    });

    watch2::registerSmallIcon("back", {

        // 'small_back', 10x10px
        0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x38, 0x00, 0x7f, 0x80, 0x7f, 0x80, 0x38, 0x00, 0x18, 0x00, 
        0x00, 0x00, 0x00, 0x00

    });




}
