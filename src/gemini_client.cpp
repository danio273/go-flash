#include "gemini_client.h"

GeminiClient::GeminiClient(const char *apiKey)
{
    _apiKey = apiKey;
}

GeminiResult GeminiClient::generate(const String &query)
{
    GeminiResult result;
    result.success = false;
    result.httpCode = -1;

    if (WiFi.status() != WL_CONNECTED)
    {
        result.errorMessage = "WiFi not connected";
        return result;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    String url = "https://" + String(_host) + _endpoint + "?key=" + String(_apiKey);

    if (!http.begin(client, url))
    {
        result.errorMessage = "HTTP begin failed";
        return result;
    }

    http.addHeader("Content-Type", "application/json");

    http.setTimeout(10000);

    JsonDocument doc;

    JsonArray contents = doc["contents"].to<JsonArray>();
    JsonObject content = contents.add<JsonObject>();
    content["role"] = "user";

    JsonArray parts = content["parts"].to<JsonArray>();
    JsonObject textPart = parts.add<JsonObject>();
    textPart["text"] = query;

    JsonObject sysInst = doc["system_instruction"].to<JsonObject>();
    JsonArray sysParts = sysInst["parts"].to<JsonArray>();
    JsonObject sysText = sysParts.add<JsonObject>();
    sysText["text"] = system_instruction;

    JsonObject genConfig = doc["generationConfig"].to<JsonObject>();
    genConfig["temperature"] = _temperature;

    String payload;
    serializeJson(doc, payload);

    int httpCode = http.POST(payload);
    result.httpCode = httpCode;

    if (httpCode <= 0)
    {
        result.errorMessage = http.errorToString(httpCode);
        http.end();
        return result;
    }

    String response = http.getString();
    http.end();

    if (httpCode != 200)
    {
        return parseHttpError(httpCode, response);
    }

    return parseGeminiResponse(response);
}

GeminiResult GeminiClient::parseHttpError(int httpCode, const String &response)
{
    GeminiResult result;
    result.success = false;
    result.httpCode = httpCode;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, response);

    if (!err && doc["error"].is<JsonObject>())
    {
        result.errorMessage = doc["error"]["message"].as<String>();
        result.errorStatus = doc["error"]["status"].as<String>();
    }
    else
    {
        result.errorMessage = "HTTP error " + String(httpCode);
    }

    return result;
}

GeminiResult GeminiClient::parseGeminiResponse(const String &response)
{
    GeminiResult result;
    result.success = false;

    JsonDocument respDoc;
    DeserializationError error = deserializeJson(respDoc, response);

    if (error)
    {
        result.errorMessage = "JSON parse error";
        return result;
    }

    if (respDoc["error"].is<JsonObject>())
    {
        result.httpCode = respDoc["error"]["code"] | -1;
        result.errorMessage = respDoc["error"]["message"].as<String>();
        result.errorStatus = respDoc["error"]["status"].as<String>();
        return result;
    }

    result.success = true;
    result.text = respDoc["candidates"][0]["content"]["parts"][0]["text"];
    return result;
}
