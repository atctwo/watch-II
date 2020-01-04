#include "../src/watch2.h"

void registerSmallIcons()
{
    watch2::registerSmallIcon("small_sd", {

        // 'small_sd, 11x8px
	    0x00, 0x00, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x0d, 0xc0, 0x00, 0x00

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

        0xc3, 0xe7, 0x7e, 0x3c, 0x3c, 0x7e, 0xe7, 0xc3

    });

    watch2::registerSmallIcon("pause", {

        0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00

    });

    watch2::registerSmallIcon("play", {

        0x00, 0x60, 0x78, 0x7e, 0x7e, 0x78, 0x60, 0x00

    });

    watch2::registerSmallIcon("add", {

        0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18

    });

    watch2::registerSmallIcon("back", {

        0x00, 0x18, 0x78, 0xff, 0xff, 0x78, 0x18, 0x00

    });




}
