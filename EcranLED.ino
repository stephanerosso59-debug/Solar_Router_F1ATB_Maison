#include "EcranLED.h"
LGFXoled oled;
#define PIN_BOUTON_OLED 2   // bouton secours reveil/page OLED (D2/GPIO2, pull-up interne -> bouton entre D2 et GND)
void Init_LED_OLED(void) {
  //LEDs et OLRD
  //LEDgroupe = 0;   //0:pas de LED,1à 9 LEDs, 10 et 11 écran OLED SSD1306 , 12 et  13 OLED SH1106
  if (LEDgroupe > 0 && LEDgroupe < 10) {  //Simples LEDs
    pinMode(LEDyellow[LEDgroupe], OUTPUT);
    pinMode(LEDgreen[LEDgroupe], OUTPUT);
  }
  if (LEDgroupe >= 10) {
    bool SD = true;                                                     //SSD1306
    if (LEDgroupe > 11) SD = false;                                     //SH1106
    oled.LGFXoled_init(LEDyellow[LEDgroupe], LEDgreen[LEDgroupe], SD);  //SDA , SCL et SSD1306 ou SH1106
    // put your setup code here, to run once:
    oled.begin();
    oled.setRotation(2);
    oled.setBrightness(255);
    oled.setTextFont(1);
    oled.setTextColor(TFT_WHITE, TFT_BLACK);
    PrintCentreO("F1ATB", -1, 0, 2);
    oled.setScrollRect(0, 16, oled.width(), oled.height() - 16);
    oled.setTextScroll(true);
    pinMode(PIN_BOUTON_OLED, INPUT_PULLUP);   // bouton secours (pull-up interne)
    delay(100);
  }
}

//****************
//* Gestion LEDs *
//****************
void Gestion_LEDs() {

  int retard_min = 100;
  int retardI;
  cptLEDyellow++;
  if ((WiFi.status() != WL_CONNECTED && (ESP32_Type < 10 || ESP32_Type==101)) || (EthernetBug > 0 && ESP32_Type == 10)) {  // Attente connexion au Wifi ou ethernet
    if (WiFi.getMode() == WIFI_STA) {                                                                 // en  Station mode
      cptLEDyellow = (cptLEDyellow + 6) % 10;
      cptLEDgreen = cptLEDyellow;
    } else {  //AP Mode
      cptLEDyellow = cptLEDyellow % 10;
      cptLEDgreen = (cptLEDyellow + 5) % 10;
    }
  } else {
    for (int i = 0; i < NbActions; i++) {
      retardI = Retard[i];
      retard_min = min(retard_min, retardI);
    }
    if (retard_min < 100) {
      cptLEDgreen = int((cptLEDgreen + 1 + 8 / (1 + retard_min / 10))) % 10;
    } else {
      cptLEDgreen = 10;
    }
  }


  if (LEDgroupe > 0 && LEDgroupe < 10) {
    int L = 0, H = 1;  //LED classique
    if (LEDgroupe == 2 || LEDgroupe == 4) { //Inversé pour LED sur carte graphique GPIOs 4,16,17
      L = 1;
      H = 0;
    }
    if (cptLEDyellow > 5) {
      digitalWrite(LEDyellow[LEDgroupe], L);
    } else {
      digitalWrite(LEDyellow[LEDgroupe], H);
    }
    if (cptLEDgreen > 5) {
      digitalWrite(LEDgreen[LEDgroupe], L);
    } else {
      digitalWrite(LEDgreen[LEDgroupe], H);
    }
  }
  if (LEDgroupe >= 10) {
    // --- OLED : 3 pages qui alternent + veille auto (anti burn-in, sans bouton) ---
    static unsigned long wakeUntil = 8000, lastHb = 0, pageMs = 0, lastDraw = 0;
    static int lastReseau = -99999, page = 0;
    static bool oledOn = true;
    unsigned long nowMs = millis();
    int Reseau = PuissanceS_M - PuissanceI_M;   // <0 = injection (surplus solaire)

    // Bouton secours : appui = reveil + page suivante
    static bool btnPrev = true;
    bool btn = digitalRead(PIN_BOUTON_OLED);   // HIGH au repos (pull-up), LOW appuye
    if (btnPrev && !btn) { wakeUntil = nowMs + 180000UL; page = (page + 1) % 3; pageMs = nowMs; lastDraw = 0; }
    btnPrev = btn;

    // Reveil des qu'il se passe quelque chose (variation > 100 W)
    if (abs(Reseau - lastReseau) > 100) { lastReseau = Reseau; wakeUntil = nowMs + 180000UL; }  // reste allume 3 min
    // Heartbeat : toutes les 10 min, on rallume 30 s pour jeter un oeil
    if (nowMs - lastHb > 600000UL) { lastHb = nowMs; if (wakeUntil < nowMs + 30000UL) wakeUntil = nowMs + 30000UL; }

    bool wantOn = (nowMs < wakeUntil);
    if (wantOn && !oledOn) { oled.wakeup(); oled.setBrightness(255); oledOn = true; pageMs = 0; lastDraw = 0; }
    if (!wantOn && oledOn) { oled.fillScreen(TFT_BLACK); oled.setBrightness(0); oled.sleep(); oledOn = false; }

    if (oledOn) {
      if (nowMs - pageMs > 4000) { pageMs = nowMs; page = (page + 1) % 3; lastDraw = 0; }   // change de page toutes les 4 s
      if (nowMs - lastDraw > 500) { lastDraw = nowMs; OledPageRouteur(page, Reseau, 100 - retard_min); }  // rafraichit 2x/s
    }
  }
}

// --- Dessine une des 3 pages du routeur sur l'OLED ---
void OledPageRouteur(int page, int Reseau, int routage) {
  oled.fillScreen(TFT_BLACK);
  if (page == 0) {                              // Page 1 : reseau + routage + jauge
    PrintCentreO("Reseau", -1, 0, 1);
    PrintCentreO(String(Reseau) + " W", -1, 12, 2);
    int teta = 360 * Reseau / 5000;
    if (Reseau >= 0) oled.fillArc(8, 8, 0, 7, -90, teta - 90, TFT_WHITE);
    else             oled.fillArc(8, 8, 0, 7, teta - 90, -90, TFT_WHITE);
    PrintCentreO("Routage " + String(routage) + "%", -1, 40, 2);
  } else if (page == 1) {                       // Page 2 : tension / courant / temp
    PrintCentreO("U / I", -1, 0, 1);
    PrintCentreO(String(Tension_M, 0) + "V  " + String(Intensite_M, 1) + "A", -1, 16, 2);
    if (temperature[0] > -100 && Source_Temp[0] != "tempNo")
      PrintCentreO("Temp " + String(temperature[0], 1) + " C", -1, 42, 2);
  } else {                                      // Page 3 : energie du jour + IP
    PrintCentreO("Energie du jour", -1, 0, 1);
    PrintCentreO(String(EnergieJour_M_Soutiree / 1000.0, 2) + " kWh soutire", -1, 14, 1);
    PrintCentreO(String(EnergieJour_M_Injectee / 1000.0, 2) + " kWh injecte", -1, 28, 1);
    IPAddress ip = (ESP32_Type == 10) ? Ethernet.localIP() : WiFi.localIP();
    PrintCentreO("IP " + ip.toString(), -1, 48, 1);
  }
}

void PrintScroll(String m) {
  TelnetPrintln( m);
  if (LEDgroupe >= 10) {
    oled.setTextSize(1);
    oled.setTextScroll(true);
    oled.println(Ascii(m));
    oled.setTextScroll(false);
  }
  
}
void PrintCentreO(String S, int X, int Y, float Sz) {
  if (X < 0) X = oled.width() / 2;
  oled.setTextSize(Sz);
  int W = oled.textWidth(S);
  oled.drawString(S, X - W / 2, Y);
}
void PrintDroiteO(String S, int X, int Y, float Sz) {
  oled.setTextSize(Sz);
  int W = oled.textWidth(S);
  oled.drawString(S, X - W - 4, Y);
}