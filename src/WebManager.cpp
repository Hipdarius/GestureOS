#include "WebManager.h"
#include "config.h"
#include <WiFi.h>
#include <SPIFFS.h>

static WebManager *g_self = nullptr;

bool WebManager::mountSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("[WebManager] SPIFFS mount failed");
        return false;
    }
    Serial.println("[WebManager] SPIFFS mounted");
    return true;
}

void WebManager::serveFile(const char *path, const char *mime) {
    if (!SPIFFS.exists(path)) {
        http_.send(404, "text/plain", "Not found");
        return;
    }
    File f = SPIFFS.open(path, "r");
    http_.streamFile(f, mime);
    f.close();
}

void WebManager::registerStatic(const char *uri, const char *path, const char *mime) {
    http_.on(uri, HTTP_GET, [this, path, mime]() { serveFile(path, mime); });
}

bool WebManager::init() {
    g_self = this;

    WiFi.mode(WIFI_AP);
    bool ap = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    if (!ap) {
        Serial.println("[WebManager] softAP failed");
        return false;
    }
    Serial.print("[WebManager] AP '");
    Serial.print(WIFI_SSID);
    Serial.print("' at ");
    Serial.println(WiFi.softAPIP());

    spiffsOk_ = mountSPIFFS();

    registerStatic("/",              "/index.html",  "text/html");
    registerStatic("/index.html",    "/index.html",  "text/html");
    registerStatic("/style.css",     "/style.css",   "text/css");
    registerStatic("/app.js",        "/app.js",      "application/javascript");
    registerStatic("/three.min.js",  "/three.min.js","application/javascript");

    http_.onNotFound([this]() {
        http_.send(404, "text/plain", "Not found");
    });

    http_.begin();
    Serial.println("[WebManager] HTTP started on :80");

    ws_.begin();
    ws_.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        if (g_self) g_self->handleWebSocketEvent(num, type, payload, length);
    });
    Serial.println("[WebManager] WebSocket started on :81");

    return true;
}

void WebManager::loop() {
    http_.handleClient();
    ws_.loop();
}

void WebManager::broadcast(const String &json) {
    ws_.broadcastTXT(json);
}

IPAddress WebManager::getIP() const {
    return WiFi.softAPIP();
}

void WebManager::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            IPAddress ip = ws_.remoteIP(num);
            Serial.printf("[WS] client %u connected from %s\n", num, ip.toString().c_str());
            break;
        }
        case WStype_DISCONNECTED:
            Serial.printf("[WS] client %u disconnected\n", num);
            break;
        case WStype_TEXT:
            // Future bidirectional commands go here (calibration, etc.)
            Serial.printf("[WS] msg from %u: %.*s\n", num, (int)length, (const char*)payload);
            break;
        default:
            break;
    }
}
