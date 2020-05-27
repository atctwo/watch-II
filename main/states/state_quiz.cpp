#include "../watch2.h"

cJSON *getQuestion()
{
    static HTTPClient http;

    Serial.println("[Quiz] getting new question");
    const char *server = "postman-echo.com";
    watch2::wifi_client.setCACert(root_ca_open_trivia_db);
    
    Serial.println("[Quiz] connecting to server");
    if (!http.begin(watch2::wifi_client, "https://opentdb.com/api.php?amount=10"))
    {
        Serial.print("[Quiz] connected to server");
        int http_code = http.GET();
        Serial.printf("[Quiz] http code: %d (%s)\n", http_code, http.errorToString(http_code));

        if (http_code > 0)
        {
            if (http_code == HTTP_CODE_OK)
            {
                Serial.println("[Quiz] response:");
                Serial.println(http.getString().c_str());
            }
        }
        else Serial.println("[Quiz] ???");
        
    }
    else
    {
        Serial.println("[Quiz] failed to connect");
    }

    Serial.println("[Quiz] finished");
}

void state_func_quiz()
{
    if (!watch2::state_init)
    {
        cJSON *question = getQuestion();
    }

    if (dpad_left_active())
    {
        watch2::switchState(2);
    }
}