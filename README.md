# ESPCam

MJPEG-Livestream im Browser mit der **AI-Thinker ESP32-CAM** (OV2640), gebaut mit PlatformIO.

## Online-Flasher

Ohne Programmierumgebung direkt aus Chrome/Edge aufspielen:
**https://prod-byte.github.io/ESPCam/** (auch auf [daveslab.de/basteln](https://daveslab.de/basteln/))

Die Firmware wird bei jedem Push automatisch von GitHub Actions gebaut
(`.github/workflows/build.yml`) und auf die Flasher-Seite (`docs/`) gestellt.
Ein Tag wie `v1.0.0` erzeugt zusätzlich ein Release mit den `.bin`-Dateien.

## Flashen mit PlatformIO

1. ESP32-CAM per USB-Seriell-Adapter (FTDI/CH340) anschließen:

   | Adapter | ESP32-CAM |
   |---------|-----------|
   | 5V      | 5V        |
   | GND     | GND       |
   | TX      | U0R       |
   | RX      | U0T       |

   Zum Flashen **GPIO0 mit GND verbinden**, dann Reset drücken.
   (Mit dem ESP32-CAM-MB-Aufsteckboard entfällt das.)
2. In VS Code mit PlatformIO: *Upload*, oder `pio run -t upload`.
3. GPIO0-Brücke entfernen, Reset drücken. Im seriellen Monitor (`pio device monitor`) steht, was die Kamera gerade tut.

## WLAN einrichten

Beim ersten Start kennt die Kamera noch kein WLAN und öffnet den Access Point **ESPCam-Setup**.

1. Handy mit dem WLAN **ESPCam-Setup** verbinden. Die Einrichtungsseite öffnet sich von selbst
   (sonst im Browser `http://192.168.4.1` aufrufen).
2. Netz antippen, Passwort eingeben, *Verbinden*.
3. Die Seite zeigt die neue Adresse der Kamera an. Danach Handy wieder ins normale WLAN.

Die Kamera merkt sich bis zu 5 WLANs und verbindet sich beim Start mit dem stärksten davon.
Ist keins erreichbar, öffnet sie wieder **ESPCam-Setup**. Ist der Router nur kurz weg, startet
sie nach 5 Minuten ohne Eingabe neu und versucht es erneut. Fällt das WLAN im Betrieb länger
als eine Minute aus, startet sie ebenfalls neu.

Weitere WLANs hinzufügen: in der Weboberfläche *WLAN hinzufügen* (bekannte bleiben erhalten).
Alle vergessen: `http://espcam.local/wifi/forget`.

Name des Access Points, optionales Passwort dafür und Hostname stehen in `include/config.h`.

## Benutzung

| Adresse                          | Funktion                              |
|----------------------------------|---------------------------------------|
| `http://espcam.local/`           | Weboberfläche mit Stream und Bedienung |
| `http://espcam.local:81/stream`  | reiner MJPEG-Stream (z. B. für VLC, Home Assistant) |
| `http://espcam.local/capture`    | Einzelbild als JPEG                   |
| `http://espcam.local/status`     | Gerätestatus und Bildeinstellungen als JSON |
| `http://espcam.local/control?var=framesize&val=VGA` | Auflösung: QVGA, VGA, SVGA, XGA, HD, SXGA, UXGA |
| `http://espcam.local/control?var=quality&val=12`    | JPEG-Qualität (10 = besser, 63 = schlechter) |
| `/control?var=brightness\|contrast\|saturation&val=-2…2` | Helligkeit, Kontrast, Sättigung |
| `/control?var=hmirror\|vflip&val=0/1` | spiegeln, auf den Kopf drehen |
| `http://espcam.local/restart`    | Neustart                              |
| `http://espcam.local/flash?on=1` | Blitz-LED an/aus                      |
| `http://espcam.local/wifi/setup` | Neustart ins Einrichtungsportal       |
| `http://espcam.local/wifi/forget`| alle WLANs löschen, dann Portal       |

Falls `.local` im Netz nicht aufgelöst wird, die IP aus dem seriellen Monitor verwenden.

## Typische Probleme

- **„Brownout detector was triggered“**: Netzteil zu schwach – stabile 5 V mit ≥ 1 A verwenden, nicht 3,3 V.
- **Kamera-Init fehlgeschlagen (0x105/0x20004)**: Flachbandkabel der Kamera sitzt nicht richtig.
- **Upload bricht ab**: GPIO0 nicht auf GND, oder `upload_speed` in `platformio.ini` auf 115200 senken.
- **Stream nur von einem Gerät gleichzeitig**: normal, der Stream-Server bedient einen Client.
