#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

class WebManager {
public:
    bool init();
    void loop();
    void broadcast(const String &json);

    IPAddress getIP() const;

private:
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
    bool mountSPIFFS();
    void registerStatic(const char *uri, const char *path, const char *mime);
    void serveFile(const char *path, const char *mime);

    WebServer http_{HTTP_PORT};
    WebSocketsServer ws_{WEBSOCKET_PORT};
    bool spiffsOk_ = false;
};
