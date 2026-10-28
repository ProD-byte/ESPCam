// WLAN-Verbindung mit Einrichtungsportal
//
// Bekannte WLANs liegen im NVS (Preferences, Namespace "wifi"), das zuletzt
// eingerichtete steht vorn. Beim Start wird per WiFiMulti das stärkste davon
// gewählt. Ist keins erreichbar, öffnet die Kamera den Access Point
// SETUP_AP_SSID; das Handy landet über DNS-Umleitung automatisch auf der
// Einrichtungsseite.

#include "wifi_setup.h"
#include "config.h"
#include "json_util.h"
#include "web_portal.h"

#include <algorithm>
#include <vector>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

static const int MAX_NETWORKS = 5;
static const uint32_t CONNECT_TIMEOUT_MS = 15000;
// Sind bekannte WLANs gespeichert (z. B. Router war nur kurz aus), wird nach
// dieser Zeit ohne Eingabe neu gestartet und erneut verbunden.
static const uint32_t PORTAL_TIMEOUT_MS = 5 * 60 * 1000;

struct SavedNetwork {
  String ssid;
  String pass;
};

// --- Speicher ---

static int loadNetworks(SavedNetwork *list) {
  Preferences prefs;
  prefs.begin("wifi", false);
  int n = min((int)prefs.getInt("count", 0), MAX_NETWORKS);
  for (int i = 0; i < n; i++) {
    list[i].ssid = prefs.getString(("s" + String(i)).c_str(), "");
    list[i].pass = prefs.getString(("p" + String(i)).c_str(), "");
  }
  prefs.end();
  return n;
}

static void storeNetworks(const SavedNetwork *list, int n) {
  Preferences prefs;
  prefs.begin("wifi", false);
  prefs.clear();
  prefs.putInt("count", n);
  for (int i = 0; i < n; i++) {
    prefs.putString(("s" + String(i)).c_str(), list[i].ssid);
    prefs.putString(("p" + String(i)).c_str(), list[i].pass);
  }
  prefs.end();
}

// Neues WLAN nach vorn; gleiche SSID wird ersetzt, das älteste fällt raus.
static void saveNetwork(const String &ssid, const String &pass) {
  SavedNetwork old[MAX_NETWORKS];
  int n = loadNetworks(old);
  SavedNetwork list[MAX_NETWORKS];
  list[0] = {ssid, pass};
  int count = 1;
  for (int i = 0; i < n && count < MAX_NETWORKS; i++) {
    if (old[i].ssid != ssid) list[count++] = old[i];
  }
  storeNetworks(list, count);
  Serial.printf("WLAN \"%s\" gespeichert (%d bekannt)\n", ssid.c_str(), count);
}

static void setSetupFlag(bool on) {
  Preferences prefs;
  prefs.begin("wifi", false);
  prefs.putBool("setup", on);
  prefs.end();
}

static bool takeSetupFlag() {
  Preferences prefs;
  prefs.begin("wifi", false);
  bool on = prefs.getBool("setup", false);
  if (on) prefs.remove("setup");
  prefs.end();
  return on;
}

void wifiRequestSetup() {
  setSetupFlag(true);
  delay(300);
  ESP.restart();
}

void wifiForgetAll() {
  storeNetworks(nullptr, 0);
  delay(300);
  ESP.restart();
}

// --- Verbinden ---

static bool connectKnown(const SavedNetwork *list, int n) {
  WiFiMulti multi;
  for (int i = 0; i < n; i++) multi.addAP(list[i].ssid.c_str(), list[i].pass.c_str());
  Serial.printf("Suche %d bekannte WLANs\n", n);
  for (int attempt = 0; attempt < 3; attempt++) {
    if (multi.run(CONNECT_TIMEOUT_MS) == WL_CONNECTED) {
      Serial.printf("Verbunden mit \"%s\"\n", WiFi.SSID().c_str());
      return true;
    }
  }
  Serial.println("Kein bekanntes WLAN erreichbar");
  return false;
}

// --- Einrichtungsportal ---

enum class TryState { Idle, Connecting, Ok, Failed };

static WebServer *portal = nullptr;
static DNSServer dns;
static TryState tryState = TryState::Idle;
static String trySsid, tryPass;
static uint32_t tryStart = 0;

static void handleRoot() {
  portal->sendHeader("Cache-Control", "no-store");
  portal->send(200, "text/html; charset=utf-8", PORTAL_HTML);
}

// Netzliste als JSON, nach Signalstärke sortiert. Doppelte SSIDs (Mesh,
// Repeater) erscheinen nur einmal mit dem besten Signal.
static void handleScan() {
  int found = max(0, (int)WiFi.scanNetworks());
  std::vector<int> order;
  for (int i = 0; i < found; i++) {
    if (!WiFi.SSID(i).isEmpty()) order.push_back(i);
  }
  std::sort(order.begin(), order.end(), [](int a, int b) { return WiFi.RSSI(a) > WiFi.RSSI(b); });

  String json = "[";
  for (size_t k = 0; k < order.size(); k++) {
    int i = order[k];
    String ssid = WiFi.SSID(i);
    bool seen = false;
    for (size_t j = 0; j < k; j++) {
      if (WiFi.SSID(order[j]) == ssid) seen = true;
    }
    if (seen) continue;
    if (json.length() > 1) json += ",";
    json += "{\"ssid\":\"" + jsonEscape(ssid) + "\",\"rssi\":" + String(WiFi.RSSI(i)) +
            ",\"open\":" + (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "true" : "false") + "}";
  }
  json += "]";
  WiFi.scanDelete();
  portal->sendHeader("Cache-Control", "no-store");
  portal->send(200, "application/json", json);
}

static void handleSave() {
  trySsid = portal->arg("ssid");
  tryPass = portal->arg("pass");
  if (trySsid.isEmpty()) {
    portal->send(400, "text/plain", "ssid fehlt");
    return;
  }
  Serial.printf("Teste WLAN \"%s\"\n", trySsid.c_str());
  WiFi.disconnect();
  WiFi.begin(trySsid.c_str(), tryPass.c_str());
  tryState = TryState::Connecting;
  tryStart = millis();
  portal->send(200, "text/plain", "ok");
}

static void handleStatus() {
  const char *state = "idle";
  switch (tryState) {
    case TryState::Connecting: state = "connecting"; break;
    case TryState::Ok: state = "ok"; break;
    case TryState::Failed: state = "failed"; break;
    default: break;
  }
  String json = String("{\"state\":\"") + state + "\",\"ip\":\"" + WiFi.localIP().toString() +
                "\",\"host\":\"" HOSTNAME "\"}";
  portal->sendHeader("Cache-Control", "no-store");
  portal->send(200, "application/json", json);
}

// Alles andere (Captive-Portal-Erkennung von Android/iOS/Windows) auf die
// Einrichtungsseite umleiten, damit das Handy sie von selbst öffnet.
static void handleRedirect() {
  portal->sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
  portal->send(302);
}

static void runPortal(bool haveKnown) {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(SETUP_AP_SSID, strlen(SETUP_AP_PASS) ? SETUP_AP_PASS : nullptr);
  delay(100);
  IPAddress apIp = WiFi.softAPIP();
  dns.start(53, "*", apIp);

  portal = new WebServer(80);
  portal->on("/", HTTP_GET, handleRoot);
  portal->on("/scan", HTTP_GET, handleScan);
  portal->on("/save", HTTP_POST, handleSave);
  portal->on("/status", HTTP_GET, handleStatus);
  portal->onNotFound(handleRedirect);
  portal->begin();

  Serial.printf("Einrichtung: mit WLAN \"%s\" verbinden, dann http://%s\n",
                SETUP_AP_SSID, apIp.toString().c_str());

  uint32_t portalStart = millis();
  uint32_t okSince = 0;
  while (true) {
    dns.processNextRequest();
    portal->handleClient();

    if (tryState == TryState::Connecting) {
      if (WiFi.status() == WL_CONNECTED) {
        saveNetwork(trySsid, tryPass);
        tryState = TryState::Ok;
        okSince = millis();
      } else if (millis() - tryStart > CONNECT_TIMEOUT_MS) {
        Serial.println("Verbindung fehlgeschlagen");
        WiFi.disconnect();
        tryState = TryState::Failed;
      }
    }

    // Kurz warten, damit das Handy die Erfolgsmeldung samt IP noch abholt
    if (tryState == TryState::Ok && millis() - okSince > 8000) break;

    if (haveKnown && tryState != TryState::Connecting && tryState != TryState::Ok &&
        millis() - portalStart > PORTAL_TIMEOUT_MS) {
      Serial.println("Keine Eingabe, neuer Versuch mit bekannten WLANs");
      ESP.restart();
    }
    delay(2);
  }

  portal->stop();
  delete portal;
  portal = nullptr;
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
}

void wifiConnectOrSetup() {
  WiFi.persistent(false);  // Zugangsdaten verwalten wir selbst
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);  // sonst ruckelt der Stream

  SavedNetwork list[MAX_NETWORKS];
  int n = loadNetworks(list);
  bool forceSetup = takeSetupFlag();

  if (!forceSetup && n > 0 && connectKnown(list, n)) return;
  runPortal(n > 0);
}
