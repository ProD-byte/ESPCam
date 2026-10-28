// ESPCam – MJPEG-Webstream für AI-Thinker ESP32-CAM
//
// Port 80: Weboberfläche (web_ui.h), /status, /capture, /control, /flash,
//          /restart, /wifi/setup, /wifi/forget
// Port 81: /stream (MJPEG) – eigener Server, damit die Oberfläche
//          während des Streams bedienbar bleibt.
//
// WLAN wird beim ersten Start per Handy eingerichtet, siehe wifi_setup.cpp.

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "config.h"
#include "json_util.h"
#include "web_ui.h"
#include "wifi_setup.h"

// Ohne WLAN so lange wird neu gestartet (dann bekannte WLANs oder Portal)
static const uint32_t WIFI_LOST_RESTART_MS = 60000;

// --- Pinbelegung AI-Thinker ESP32-CAM ---
#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

#define FLASH_LED_PIN    4

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static httpd_handle_t web_httpd = nullptr;
static httpd_handle_t stream_httpd = nullptr;
static bool flashOn = false;

// Auflösungen werden per Name übertragen, weil sich die Enum-Werte zwischen
// Versionen der esp32-camera-Bibliothek verschieben.
struct FrameSizeName {
  framesize_t size;
  const char *name;
};
static const FrameSizeName FRAME_SIZES[] = {
    {FRAMESIZE_QVGA, "QVGA"}, {FRAMESIZE_VGA, "VGA"},   {FRAMESIZE_SVGA, "SVGA"}, {FRAMESIZE_XGA, "XGA"},
    {FRAMESIZE_HD, "HD"},     {FRAMESIZE_SXGA, "SXGA"}, {FRAMESIZE_UXGA, "UXGA"},
};

static const char *frameSizeName(framesize_t size) {
  for (const auto &f : FRAME_SIZES) {
    if (f.size == size) return f.name;
  }
  return "?";
}

static bool initCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
  } else {
    Serial.println("Kein PSRAM gefunden – nur QVGA möglich");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera-Init fehlgeschlagen: 0x%x\n", err);
    return false;
  }
  return true;
}

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t statusHandler(httpd_req_t *req) {
  sensor_t *s = esp_camera_sensor_get();
  String json = "{";
  json += "\"ssid\":\"" + jsonEscape(WiFi.SSID()) + "\"";
  json += ",\"rssi\":" + String(WiFi.RSSI());
  json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"host\":\"" + jsonEscape(HOSTNAME) + "\"";
  json += ",\"setupAp\":\"" + jsonEscape(SETUP_AP_SSID) + "\"";
  json += ",\"uptime\":" + String(millis() / 1000);
  json += ",\"heap\":" + String(ESP.getFreeHeap());
  json += ",\"psram\":" + String(ESP.getFreePsram());
  json += ",\"framesize\":\"" + String(frameSizeName(s->status.framesize)) + "\"";
  json += ",\"quality\":" + String(s->status.quality);
  json += ",\"brightness\":" + String(s->status.brightness);
  json += ",\"contrast\":" + String(s->status.contrast);
  json += ",\"saturation\":" + String(s->status.saturation);
  json += ",\"hmirror\":" + String(s->status.hmirror);
  json += ",\"vflip\":" + String(s->status.vflip);
  json += ",\"flash\":" + String(flashOn ? "true" : "false");
  json += "}";
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_send(req, json.c_str(), json.length());
}

static esp_err_t captureHandler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=espcam.jpg");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

// Liest einen Query-Parameter als Text, z. B. /control?var=framesize&val=VGA
static bool queryParam(httpd_req_t *req, const char *key, char *out, size_t len) {
  char query[96];
  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) return false;
  return httpd_query_key_value(query, key, out, len) == ESP_OK;
}

static esp_err_t controlHandler(httpd_req_t *req) {
  char var[16], val[16];
  if (!queryParam(req, "var", var, sizeof(var)) || !queryParam(req, "val", val, sizeof(val))) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "var und val erwartet");
    return ESP_FAIL;
  }
  sensor_t *s = esp_camera_sensor_get();
  int v = atoi(val);
  int res = -1;
  if (!strcmp(var, "framesize")) {
    for (const auto &f : FRAME_SIZES) {
      if (!strcmp(val, f.name)) res = s->set_framesize(s, f.size);
    }
  } else if (!strcmp(var, "quality")) {
    res = s->set_quality(s, constrain(v, 4, 63));
  } else if (!strcmp(var, "brightness")) {
    res = s->set_brightness(s, constrain(v, -2, 2));
  } else if (!strcmp(var, "contrast")) {
    res = s->set_contrast(s, constrain(v, -2, 2));
  } else if (!strcmp(var, "saturation")) {
    res = s->set_saturation(s, constrain(v, -2, 2));
  } else if (!strcmp(var, "hmirror")) {
    res = s->set_hmirror(s, v ? 1 : 0);
  } else if (!strcmp(var, "vflip")) {
    res = s->set_vflip(s, v ? 1 : 0);
  }
  if (res != 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unbekannte Einstellung");
    return ESP_FAIL;
  }
  return httpd_resp_sendstr(req, "OK");
}

static esp_err_t flashHandler(httpd_req_t *req) {
  char val[4] = "0";
  queryParam(req, "on", val, sizeof(val));
  flashOn = atoi(val) != 0;
  digitalWrite(FLASH_LED_PIN, flashOn ? HIGH : LOW);
  return httpd_resp_sendstr(req, "OK");
}

static esp_err_t restartHandler(httpd_req_t *req) {
  httpd_resp_sendstr(req, "Starte neu");
  delay(300);
  ESP.restart();
  return ESP_OK;
}

static esp_err_t wifiSetupHandler(httpd_req_t *req) {
  httpd_resp_sendstr(req, "Starte Einrichtung: mit WLAN " SETUP_AP_SSID " verbinden");
  wifiRequestSetup();
  return ESP_OK;
}

static esp_err_t wifiForgetHandler(httpd_req_t *req) {
  httpd_resp_sendstr(req, "Alle WLANs geloescht, starte Einrichtung: mit WLAN " SETUP_AP_SSID " verbinden");
  wifiForgetAll();
  return ESP_OK;
}

static esp_err_t streamHandler(httpd_req_t *req) {
  char partHeader[64];
  esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Frame konnte nicht gelesen werden");
      res = ESP_FAIL;
      break;
    }
    size_t hlen = snprintf(partHeader, sizeof(partHeader), STREAM_PART, fb->len);
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, partHeader, hlen);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    if (res != ESP_OK) break;  // Client hat die Verbindung getrennt
  }
  return res;
}

static void startServers() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 12;

  const httpd_uri_t routes[] = {
      {"/", HTTP_GET, indexHandler, nullptr},
      {"/status", HTTP_GET, statusHandler, nullptr},
      {"/capture", HTTP_GET, captureHandler, nullptr},
      {"/control", HTTP_GET, controlHandler, nullptr},
      {"/flash", HTTP_GET, flashHandler, nullptr},
      {"/restart", HTTP_GET, restartHandler, nullptr},
      {"/wifi/setup", HTTP_GET, wifiSetupHandler, nullptr},
      {"/wifi/forget", HTTP_GET, wifiForgetHandler, nullptr},
  };
  if (httpd_start(&web_httpd, &config) == ESP_OK) {
    for (const auto &route : routes) httpd_register_uri_handler(web_httpd, &route);
  }

  config.server_port = 81;
  config.ctrl_port += 1;
  httpd_uri_t streamUri = {"/stream", HTTP_GET, streamHandler, nullptr};
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &streamUri);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESPCam startet");

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  if (!initCamera()) {
    delay(5000);
    ESP.restart();
  }

  wifiConnectOrSetup();

  if (MDNS.begin(HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
  }

  startServers();
  Serial.printf("Bereit: http://%s  bzw.  http://%s.local\n",
                WiFi.localIP().toString().c_str(), HOSTNAME);
}

void loop() {
  // Nach WLAN-Abbruch erst selbst neu verbinden; klappt das länger nicht,
  // Neustart – dann wird ein anderes bekanntes WLAN oder das Portal gewählt.
  static uint32_t lostSince = 0;
  if (WiFi.status() == WL_CONNECTED) {
    lostSince = 0;
  } else {
    if (lostSince == 0) {
      lostSince = millis();
      Serial.println("WLAN getrennt");
    }
    if (millis() - lostSince > WIFI_LOST_RESTART_MS) ESP.restart();
    WiFi.reconnect();
    delay(5000);
  }
  delay(1000);
}
