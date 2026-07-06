#include "WebServerManager.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <WiFi.h>

static const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Maintenance</title>
</head>
<body>

<h1>ESP32 Maintenance</h1>

<h2>Firmware Upload</h2>

<form method=input type="file" name="firmware">
    <input type="submit" value="Upload">
</form>

<hr>

<button onclick="enableOTA()">
Enable PlatformIO OTA
</button>

<button onclick="restartESP()">
Restart ESP32
</button>

<script>
function enableOTA()
{
    fetch('/ota/start', {method:'POST'})
        .then(r => r.text())
        .then(alert);
}

function restartESP()
{
    fetch('/reset', {method:'POST'})
        .then(r => r.text())
        .then(alert);
}
</script>

</body>
</html>
)rawliteral";

void WebServerManager::begin()
{
    // [this]() allows the lambda to access WebServerManager's member functions
    server.on("/", HTTP_GET, [this]() {
        handleRoot();
    });
    
    server.on("/reset", HTTP_POST, [this]() {
        handleReset();
    });

    server.on("/ota/start", HTTP_POST, [this]() {
        handleOtaStart();
    });

    // Both the upload-finished callback and the chunk-handling callback need the capture
    server.on("/upload", HTTP_POST, 
        [this]() {
            handleUploadFinished();
        }, 
        [this]() {
            handleUpload();
        }
    );

    server.begin();

    Serial.println("HTTP server started");
}
void WebServerManager::handleClient()
{
    server.handleClient();
}

bool WebServerManager::isOtaEnabled() const
{
    return otaEnabled;
}

void WebServerManager::handleRoot()
{
    server.send(200, "text/html", PAGE);
}

void WebServerManager::handleOtaStart()
{
    if (!otaEnabled)
    {
        ArduinoOTA.begin();
        otaEnabled = true;
    }

    server.send(200, "text/plain", "ArduinoOTA enabled");
}

void WebServerManager::handleReset()
{
    server.send(200, "text/plain", "Restarting...");

    delay(500);

    ESP.restart();
}

void WebServerManager::handleUpload()
{
    HTTPUpload& upload = server.upload();

    switch (upload.status)
    {
        case UPLOAD_FILE_START:

            Serial.printf("Upload start: %s\n",
                          upload.filename.c_str());

            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            {
                Update.printError(Serial);
            }

            break;

        case UPLOAD_FILE_WRITE:

            if (Update.write(upload.buf,
                             upload.currentSize) != upload.currentSize)
            {
                Update.printError(Serial);
            }

            break;

        case UPLOAD_FILE_END:

            if (!Update.end(true))
            {
                Update.printError(Serial);
            }

            break;

        case UPLOAD_FILE_ABORTED:

            Update.end();
            Serial.println("Upload aborted");

            break;

        default:
            break;
    }
}

void WebServerManager::handleUploadFinished()
{
    bool success = !Update.hasError();

    server.send(
        success ? 200 : 500,
        "text/plain",
        success
            ? "Update successful. Rebooting..."
            : "Update failed");

    if (success)
    {
        delay(1000);
        ESP.restart();
    }
}