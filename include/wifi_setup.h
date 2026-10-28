#pragma once

// Verbindet mit dem stärksten bekannten WLAN. Klappt das nicht, wird der
// Access Point SETUP_AP_SSID mit Einrichtungsseite gestartet. Kehrt erst
// zurück, wenn eine WLAN-Verbindung besteht.
void wifiConnectOrSetup();

// Startet neu direkt ins Einrichtungsportal; bekannte WLANs bleiben erhalten.
void wifiRequestSetup();

// Löscht alle gespeicherten WLANs und startet neu ins Einrichtungsportal.
void wifiForgetAll();
