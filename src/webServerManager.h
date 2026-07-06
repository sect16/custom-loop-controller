#pragma once

#include <WebServer.h>

class WebServerManager
{
public:
    void begin();
    void handleClient();
    bool isOtaEnabled() const;

private:
    WebServer server{80};
    bool otaEnabled{false};

    void handleRoot();
    void handleReset();
    void handleOtaStart();

    void handleUpload();
    void handleUploadFinished();
};