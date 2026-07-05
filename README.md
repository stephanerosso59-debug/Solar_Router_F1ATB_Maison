# ☀️ Routeur Solaire F1ATB — édition « Maison » (PCB + W5500 SPI)

Routeur photovoltaïque basé sur le **[Solar Router de F1ATB](https://f1atb.fr/fr/)** (projet open-source),
adapté à un **PCB custom** avec un module **Ethernet W5500 (SPI)** ajouté à la place du LAN8720 natif.

> 🙏 **Crédit** : le cœur logiciel est l'œuvre de **F1ATB** — https://f1atb.fr/fr/ et https://github.com/F1ATB/Solar_Router.
> Ce dépôt n'est qu'une **adaptation matérielle** (PCB + W5500) de sa V25, avec quelques ajouts (ventilateur, pinout).

---

## 🔧 Ce que fait le routeur

Un routeur solaire mesure le **surplus de production photovoltaïque** et le **dirige vers une charge**
(typiquement un chauffe-eau) via un **triac** (train de sinus / coupure de phase), au lieu de le réinjecter
sur le réseau. Il gère aussi relais, sondes de température, MQTT/Home Assistant, affichage web, etc.

Fonctions principales (héritées de F1ATB) :
- Mesure de puissance (JSY-MK, Linky/TIC, Shelly, Enphase, HomeWizard, MQTT…)
- Pilotage **Triac** (MOC3021 + détection passage à zéro) et/ou **relais SSR**
- Sondes **DS18B20**, serveur **web** de configuration, **MQTT**, **OTA**, Telnet
- Réseau **WiFi** ou **Ethernet**

---

## 🛠️ La modification « Maison »

| Ajout | Détail |
|---|---|
| **Ethernet W5500 (SPI)** | À la place du LAN8720/WT32-ETH01 natif. Piloté par la lib **EthernetESP32** (`W5500Driver`). |
| **Ventilateur PWM** | Refroidissement (GPIO33), piloté par la température (seuils 35/40/60 °C). |
| **Pinout PCB dédié** | Centralisé dans `GpioPinConfig.h`. |
| **Sécurité température** | Alerte à 75 °C. |

### 🔌 Pinout du PCB (ESP32-D0WD 30 pins)

| GPIO | Fonction | | GPIO | Fonction |
|---|---|---|---|---|
| 4 | DS18B20 | | 21 | SSR Relay |
| **5** | **W5500 CS** | | 22 | MOC3021 (triac) |
| **13** | **W5500 MOSI** | | 23 | ZC Detect |
| **14** | **W5500 CLK** | | **25** | **W5500 INT** |
| **15** | **W5500 MISO** | | 26 | JSY TX |
| 16 | Linky RX | | 27 | JSY RX |
| 17 | OLED SDA | | 32 | OLED SCL |
| 18 | LED Verte | | 33 | Fan PWM |
| 19 | LED Rouge | | | |

---

## 📟 Affichage OLED (SSD1306 0.96") — ajout « Maison »
Écran I2C optionnel sur **SDA=GPIO17 / SCL=GPIO32** (VCC 3.3V). Activé via la config carte **« WROOM + OLED »** (`LEDgroupe = 10`, adapté à ce PCB).

**3 pages qui alternent** (~4 s chacune) :
1. **Réseau** (W) + jauge + **% routage**
2. **Tension / Courant** (UxI) + **Température**
3. **Énergie du jour** (kWh soutiré/injecté) + **IP**

**Veille auto** (anti burn-in, sans tactile) : écran éteint après 3 min sans changement, **réveil automatique** sur variation de puissance (>100 W) + heartbeat (10 min / 30 s).

**Bouton secours** (optionnel) : poussoir entre **D2 (GPIO2)** et GND → réveil + page suivante *(pull-up interne, pas de résistance)*.

*(Code : `EcranLED.ino`.)*

---

## 🔒 Sécurité / configuration

- **Aucun secret n'est versionné** : le SSID / mot de passe WiFi se saisissent **au runtime**
  (moniteur série : `ssid:` / `password:` / `restart`, ou portail captif `http://192.168.4.1`, ou WPS).
- La configuration vit dans `/parametres.json` sur la **LittleFS de l'ESP32**, pas dans le code.

### Activer le W5500 (Ethernet)
Quand le PCB est monté + câble RJ45 branché : taper **`ETH01`** puis **`restart`** en série
→ passe en `ESP32_Type = 10` (Ethernet, piloté par le `W5500Driver`).
⚠️ En mode Ethernet, pas de repli WiFi — à faire uniquement avec le W5500 présent.

---

## 🔨 Compilation

- **Carte** : ESP32 Dev Module — **Partition : Custom** (`partitions.csv` fourni)
- **Core** : `esp32:esp32@3.3.10`
- **FQBN** : `esp32:esp32:esp32:PartitionScheme=custom,FlashSize=4M,CPUFreq=240`
- **Bibliothèques** : ArduinoJson **7.x**, **EthernetESP32**, UrlEncode, PubSubClient,
  DallasTemperature, LovyanGFX ; **OneWire 2.3.8 bundlé** dans le sketch.

> ⚠️ Points spécifiques à cette version : lib **EthernetESP32** (le W5500 = `W5500Driver driver(CS, INT)`),
> **ArduinoJson v7**, et `FanControl.h` utilise la **nouvelle API LEDC du core 3.x** (`ledcAttach`).

---

## 📁 Contenu

```
Solar_Router_V17_25/
├── Solar_Router_V17_25.ino   → sketch principal (base F1ATB V25 + intégration W5500/Fan)
├── GpioPinConfig.h           → pinout du PCB Maison
├── FanControl.h              → ventilateur PWM
├── TemperatureSensors.h      → (sécurité température, optionnel)
├── partitions.csv            → table de partition custom
├── *.ino / *.h               → cœur F1ATB (sources, web, MQTT, triac…)
└── README.md
```

---

*Basé sur le travail de **F1ATB** (https://f1atb.fr/fr/). Adaptation matérielle W5500 + PCB « Maison ».*
