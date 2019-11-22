void registerFileStates()
{

    registerState("SD Test", "what", [](){

        // Directory file.
        SdFile root;

        // Use for file creation in folders.
        SdFile file;

        oled.setCursor(0,19);
        oled.setFont(&SourceSansPro_Regular6pt7b);

        oled.print("sd dir test\n");
        //Serial.println("sd dir test");

        if (!state_init)
        {
            std::vector<File> files = getDirFiles("/");
            char filename[255];

            if (files.size() == 0) oled.print("error accessing files");
            else
            {
                for (int i = 0; i < files.size(); i++)
                {
                    files[i].getName(filename, 255);
                    oled.println(filename);
                }
            }
        }

        drawTopThing();

        if (dpad_left_active()) switchState(2);

    }, false);

    

    
}
