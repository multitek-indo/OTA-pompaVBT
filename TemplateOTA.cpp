#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32httpUpdate.h>
#include <WiFiManager.h>

#define FIRMWARE_VERSION 1.0
#define UPDATE_URL "https://dumpkids.github.io/ota.github.io/update.json"

t_httpUpdate_return updateOverHttp(String url_update);

void setup()
{
    Serial.begin(9600);

    // Menghubungkan ke Wi-Fi menggunakan Wi-Fi Manager
    connectWifi();
    
    // Cek update setelah koneksi Wi-Fi berhasil
    checkUpdate();
}

void loop()
{
    // Cek update secara berkala
    checkUpdate();
    delay(5000);
}

void connectWifi()
{
    Serial.println("Connecting to WiFi using WiFiManager");

    WiFiManager wifiManager;

    // Menyambungkan ke Wi-Fi atau membuat portal konfigurasi
    if (!wifiManager.autoConnect("ESP32_AP"))
    {
        Serial.println("Failed to connect and hit timeout");
        ESP.restart();  // Restart jika gagal
    }
    
    // Jika koneksi sukses, tampilkan IP
    Serial.println("Connected to WiFi!");
    Serial.println("IP Address: " + WiFi.localIP().toString());
}

void checkUpdate()
{
    Serial.println("Checking update");
    HTTPClient http;
    String response;
    String url = UPDATE_URL;
    http.begin(url);
    int httpCode = http.GET();
    
    // Hanya proses jika response OK
    if (httpCode == HTTP_CODE_OK)
    {
        response = http.getString();
        Serial.println(response);
        
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, response);
        JsonObject obj = doc.as<JsonObject>();
        
        String version = obj["version"];
        String url_update = obj["url"];
        
        Serial.println("Version: " + version);
        Serial.println("Update URL: " + url_update);
        
        // Bandingkan versi firmware
        if (version.toDouble() > FIRMWARE_VERSION)
        {
            Serial.println("Update Available");
            if (updateOverHttp(url_update) == HTTP_UPDATE_OK)
            {
                Serial.println("Update Success");
            }
            else
            {
                Serial.println("Update Failed");
            }
        }
        else
        {
            Serial.println("No Update Available");
        }
    }
    else
    {
        Serial.printf("HTTP request failed with code %d\n", httpCode);
    }
    http.end(); // Jangan lupa untuk menutup koneksi HTTP
}

t_httpUpdate_return updateOverHttp(String url_update)
{
    t_httpUpdate_return ret;

    if (WiFi.status() == WL_CONNECTED)
    {
        ret = ESPhttpUpdate.update(url_update);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            return ret;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            return ret;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            return ret;
        }
    }
    else
    {
        Serial.println("WiFi not connected, update failed");
    }
}
