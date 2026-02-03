#pragma once

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "disk_config.h"

struct GeminiResult
{
    bool success;

    const char *text;

    int httpCode;
    String errorMessage;
    String errorStatus;
};

class GeminiClient
{
public:
    GeminiClient(const char *apiKey);

    GeminiResult generate(const String &query);

private:
    const char *_apiKey;
    const float _temperature = 0.3;

    String system_instruction = "You are a school assistant. Rules: "
                                "1. Respond in the user's language. "
                                "2. Plain text only, no markdown, no code blocks. "
                                "3. IMPORTANT: Response must be under " +
                                String(DATA_BUF_SIZE - 1000) + " chars. " +
                                "4. If it is code: only core logic, no comments. "
                                "5. Prioritize brevity over politeness. ";

    const char *_host = "generativelanguage.googleapis.com";
    const int _httpsPort = 443;
    const String _endpoint = "/v1beta/models/gemini-2.5-flash:generateContent";

    GeminiResult parseHttpError(int httpCode, const String &response);
    GeminiResult parseGeminiResponse(const String &response);
};
