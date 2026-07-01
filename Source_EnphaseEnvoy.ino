//***********************************
//* Source EnPhase V7   			*
//***********************************

uint32_t ipToInt(IPAddress ip) {                                                                  //SR19
  return uint32_t(ip[0] << 24) | uint32_t(ip[1] << 16) | uint32_t(ip[2] << 8) | uint32_t(ip[3]);  //SR19
}

void Setup_Enphase() {

  //Résolution mDNS de http://envoy.local en adresse IP                                                                  //SR19
  //***************************************************                                                                  //SR19

  const char* host = "envoy";  //SR19
  IPAddress envoyIP;
  if (RMSextIPauto) {                                                                                //SR19
    if (!MDNS.begin(hostname)) {                                                                     //Init mDNS                                                                              //SR19
      TelnetPrintln("Erreur : impossible d'initialiser mDNS");                                       //SR19
      return;                                                                                        //SR19
    } else {                                                                                         //SR19
      envoyIP = MDNS.queryHost(host, 2000);                                                          //avec timeout 2s                                                             //SR19
    }                                                                                                //SR19
    if (envoyIP.toString() != "0.0.0.0") {                                                           //SR19
      StockMessage("IP Enphase : http://" + String(host) + ".local" + " -> " + envoyIP.toString());  //SR19
      RMSextIP = ipToInt(envoyIP);
      EcritureEnROM();                                              //IP -> uint32                                                                         //SR19
    } else {                                                        //SR19
      StockMessage("Échec! passerelle Enphase envoy déconnectée");  //SR19
      return;                                                       //SR19
    }
  }
  //Obtention Session ID
  //********************
  const char* server1Enphase = "enlighten.enphaseenergy.com";
  String Host = String(server1Enphase);
  String adrEnphase = "https://" + Host + "/login/login.json";
  String requestBody = "user[email]=" + EnphaseUser + "&user[password]=" + urlEncode(EnphasePwd);

  if (EnphaseUser != "" && EnphasePwd != "" && RMSextIP > 0) {  // test envoyIP si perte de connexion //SR19
    TelnetPrintln("Essai connexion  Enlighten server 1 pour obtention session_id!");
    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(server1Enphase, 443, 3000))
      StockMessage("Connection failed to Enlighten server :" + Host);
    else {
      TelnetPrintln("Connected to Enlighten server:" + Host);
      StockMessage("Connected to Enlighten server :" + Host);
      clientSecu.println("POST " + adrEnphase + "?" + requestBody + " HTTP/1.0");
      clientSecu.println("Host: " + Host);
      clientSecu.println("Connection: close");
      clientSecu.println();
      String line = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          TelnetPrintln("headers 1 Enlighten received");
          JsonToken = "";
        }

        JsonToken += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }
      clientSecu.stop();
    }
    Session_id = StringJson("session_id", JsonToken);
    TelnetPrintln("session_id :" + Session_id);
    StockMessage("session_id :" + Session_id);
  }

  //Obtention Token
  //********************
  if (Session_id != "" && EnphaseSerial != "" && EnphaseUser != "") {
    const char* server2Enphase = "entrez.enphaseenergy.com";
    Host = String(server2Enphase);
    adrEnphase = "https://" + Host + "/tokens";
    requestBody = "{\"session_id\":\"" + Session_id + "\", \"serial_num\":" + EnphaseSerial + ", \"username\":\"" + EnphaseUser + "\"}";
    TelnetPrintln("Essai connexion  Enlighten server 2 pour obtention token!");
    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(server2Enphase, 443, 3000))
      StockMessage("Connection failed to :" + Host);
    else {
      TelnetPrintln("Connected to :" + Host);
      StockMessage("Connected to :" + Host);
      clientSecu.println("POST " + adrEnphase + " HTTP/1.0");
      clientSecu.println("Host: " + Host);
      clientSecu.println("Content-Type: application/json");
      clientSecu.println("Content-Length:" + String(requestBody.length()));
      clientSecu.println("Connection: close");
      clientSecu.println();
      clientSecu.println(requestBody);
      clientSecu.println();
      TelnetPrintln("Attente user est connecté");
      String line = "";
      JsonToken = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          TelnetPrintln("headers 2 enlighten received");
          JsonToken = "";
        }

        JsonToken += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }
      clientSecu.stop();
      JsonToken.trim();
      TelnetPrintln("Token :" + JsonToken);
      //StockMessage("Token :" + JsonToken);
      if (JsonToken.length() > 50) {
        TokenEnphase = JsonToken;
        previousTimeRMSMin = 1000;
        previousTimeRMSMax = 1;
        previousTimeRMSMoy = 1;
        previousTimeRMS = millis();
        LastRMS_Millis = millis();
        PeriodeProgMillis = 1000;
      }
    }
  }
}

bool JSONReadingEnphase(NetworkClientSecure* pClient, String* pString, char cUntilChar, unsigned long nTimeout) {
  if ((pClient == 0) || (pString == 0))
    return true;

  pClient->setTimeout(nTimeout);
  unsigned long nT0 = millis();
  *pString = pClient->readStringUntil(cUntilChar);
  return ((millis() - nT0) > nTimeout);
}

void LectureEnphase() {
#define TIMEOUT_JSON_READING 100
#define TIMEOUT_WAITING_ANSWER 500
#define TIMEOUT_CONNECT 3000
  // init variable
  static unsigned long g_nLastGoodReading = millis();
  bool bJsonLoadingFinished = false;
  bool bTimeout;

  float PactReseau = 0.0f;
  float PvaReseau = 0.0f;
  long whDlvdCum = 0L;  // on perd les decimals après la virgule avec un type long
  long whRcvdCum = 0L;

  String jsonPayload;
  String host = IP2String(RMSextIP);
  String baseRequest;
  baseRequest = "/ivp/meters/readings HTTP/1.0\r\nHost: " + host + "\r\nAccept: application/json\r\nConnection: keep-alive\r\n";

  static uint32_t lastTokenUpdate = millis();  // premier passage, le token a été obtenu via setup_enphase
  constexpr uint32_t TOKEN_REFRESH_MS = 30UL * 24UL * 60UL * 60UL * 1000UL;
  if (TokenEnphase.length() > 50 && EnphaseUser != "") {
    // Connexion pour firmware V7 en https
    NetworkClientSecure client;

    if ((millis() - lastTokenUpdate) > TOKEN_REFRESH_MS) {  // Tout les 30 jours on recherche un nouveau Token
      lastTokenUpdate = millis();                           // overflow compatible!
      Setup_Enphase();
    }

    if (!client.connected()) {  // établi la connexion
      client.setInsecure();     // skip verification
      client.setTimeout(TIMEOUT_CONNECT);
      if (!client.connect(host.c_str(), 443)) {
        StockMessage("Connection failed to Envoy-S server! : https://" + String(host));
        return;  // on sort, pas de comm avec le server enphase
      }
      //StockMessage("Connected to Envoy-S server HTTPS!");
    }

    client.println("GET " + baseRequest + "Authorization: Bearer " + TokenEnphase + "\r\n\r\n");

    bTimeout = JSONReadingEnphase(&client, &jsonPayload, '\n', TIMEOUT_WAITING_ANSWER);
    jsonPayload.trim();
    //TelnetPrintln("HTTP: " + statusLine);
    if (bTimeout || (jsonPayload.indexOf("200") < 0)) {
      StockMessage("Envoy refused request");
      client.stop();
      return;
    }

    int nGlobalIndex = 0;
    int nPhaseIndex = 0;
    bool bMonoPhase = true;

    //TelnetPrintln("Waiting JSON data ...");

    // Saute L'entete d'ouverture de la trame JSON.
    bTimeout = JSONReadingEnphase(&client, &jsonPayload, '[', TIMEOUT_JSON_READING);
    if (bTimeout) {
      StockMessage("JSON Reading Timeout 1");
      return;
    }

    for (nGlobalIndex = 0; (nGlobalIndex < 8) && !bJsonLoadingFinished && !bTimeout; nGlobalIndex++) {
      // Read Global Topic
      bTimeout = JSONReadingEnphase(&client, &jsonPayload, '[', TIMEOUT_JSON_READING);
      if (bTimeout) {
        StockMessage("JSON Reading Timeout 2 / nGlobalIndex=" + String(nGlobalIndex) + " nPhaseIndex=" + String(nPhaseIndex));
        return;
      }
      delay(1);

      if (nGlobalIndex == 0) {
        //StockMessage(jsonPayload);
        float tension = ValJson("voltage", jsonPayload);
        long eid = LongJson("eid", jsonPayload);

        //StockMessage("Tension Global0 ="+String(tension));

        if (tension > 280.0f)
          bMonoPhase = false;

        //StockMessage("bMonoPhase ="+String(bMonoPhase));

        if (!bMonoPhase) {
          PactProd = ValJson("activePower", jsonPayload);
          Tension_M = ValJson("voltage", jsonPayload);
          Intensite_M = ValJson("current", jsonPayload);
        }
      } else if (nGlobalIndex == 1) {
        if (!bMonoPhase) {
          PactReseau = ValJson("activePower", jsonPayload);
          PactConso_M = PactReseau + PactProd;  // dans l'hypothese qu'il n'y a pas de l'énergie fournit par une batterie !
          PvaReseau = ValJson("apparentPower", jsonPayload);
          whDlvdCum = ValJson("actEnergyDlvd", jsonPayload);
          whRcvdCum = ValJson("actEnergyRcvd", jsonPayload);
          Frequence = ValJson("freq", jsonPayload);
        }
      }

      for (nPhaseIndex = 0; (nPhaseIndex < 3) && !bJsonLoadingFinished && !bTimeout; nPhaseIndex++) {
        // Read Phase
        bTimeout = JSONReadingEnphase(&client, &jsonPayload, '}', TIMEOUT_JSON_READING);
        if (bTimeout) {
          StockMessage("JSON Reading Timeout 3 / nGlobalIndex=" + String(nGlobalIndex) + " nPhaseIndex=" + String(nPhaseIndex));
          return;
        }
        jsonPayload += "}";
        delay(1);

        if ((nGlobalIndex == 0) && (nPhaseIndex == 0)) {
          if (bMonoPhase) {
            PactProd = ValJson("activePower", jsonPayload);
          }
        } else if ((nGlobalIndex == 1) && (nPhaseIndex == 0)) {
          Tension_M1 = ValJson("voltage", jsonPayload);
          Intensite_M1 = ValJson("current", jsonPayload);

          if (bMonoPhase) {
            //StockMessage(jsonPayload);
            PactReseau = ValJson("activePower", jsonPayload);
            PactConso_M = PactReseau + PactProd;  // dans l'hypothese qu'il n'y a pas de l'énergie fournit par une batterie !
            PvaReseau = ValJson("apparentPower", jsonPayload);
            whDlvdCum = ValJson("actEnergyDlvd", jsonPayload);
            whRcvdCum = ValJson("actEnergyRcvd", jsonPayload);
            Frequence = ValJson("freq", jsonPayload);

            Tension_M = Tension_M1;
            Intensite_M = Intensite_M1;
            //StockMessage("activePower="+String(PactReseau));
          }
        } else if ((nGlobalIndex == 1) && (nPhaseIndex == 1)) {
          Tension_M2 = ValJson("voltage", jsonPayload);
          Intensite_M2 = ValJson("current", jsonPayload);
        } else if ((nGlobalIndex == 1) && (nPhaseIndex == 2)) {
          Tension_M3 = ValJson("voltage", jsonPayload);
          Intensite_M3 = ValJson("current", jsonPayload);

          bJsonLoadingFinished = true;

          g_nLastGoodReading = millis();
        }
      }
    }
  }

  if (!bJsonLoadingFinished) {
    //Protection contre les mauvaises lectures qui perdureraient plus de 10s !!!
    if ((millis() - g_nLastGoodReading) > 10000) {
      PactProd = 0.0f;
      PactConso_M = 0;
      PactReseau = 0.0f;
      PactConso_M = 0.0f;
      Tension_M = 0.0f;
      Intensite_M = 0.0f;
      Frequence = 0.0f;
      Tension_M1 = 0.0f;
      Tension_M2 = 0.0f;
      Tension_M3 = 0.0f;
      Intensite_M1 = 0.0f;
      Intensite_M2 = 0.0f;
      Intensite_M3 = 0.0f;
    }
    //TelnetPrintln("JSON Loading failed");
    StockMessage("JSON Loading failed");
    return;
  }

  PactReseau = PfloatMax(PactReseau);
  if (PactReseau < 0) {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = int(-PactReseau);
  } else {
    PuissanceI_M_inst = 0;
    PuissanceS_M_inst = int(PactReseau);
  }
  PvaReseau = PfloatMax(PvaReseau);
  if (PactReseau < 0) {
    PVAS_M_inst = 0;
    PVAI_M_inst = int(PvaReseau);
  } else {
    PVAI_M_inst = 0;
    PVAS_M_inst = int(PvaReseau);
  }
  Pva_valide = true;
  filtre_puissance();
  float PowerFactor = 0.0f;
  if ((PVA_M_moy) != 0) {
    PowerFactor = floor(100.0f * fabsf(Puissance_M_moy) / PVA_M_moy) / 100.0f;
    PowerFactor = min(PowerFactor, 1.0f);
  }
  PowerFactor_M = PowerFactor;

  if (whDlvdCum != 0) {
    if (LastwhDlvdCum == 0)
      LastwhDlvdCum = whDlvdCum;
    long DeltaWhSoutire = whDlvdCum - LastwhDlvdCum;
    LastwhDlvdCum = whDlvdCum;
    if (DeltaWhSoutire > 0) {
      Energie_M_Soutiree += DeltaWhSoutire;
    }
  }

  if (whRcvdCum != 0) {
    if (LastwhRcvdCum == 0)
      LastwhRcvdCum = whRcvdCum;
    long DeltaWhInjecte = whRcvdCum - LastwhRcvdCum;
    LastwhRcvdCum = whRcvdCum;
    if (DeltaWhInjecte > 0) {
      Energie_M_Injectee += DeltaWhInjecte;
    }
  }

  EnergieActiveValide = true;
  if (PactReseau != 0 || PvaReseau != 0) PuissanceRecue = true;  // Reset du Watchdog à chaque trame reçue de la passerelle Envoy-S metered
  if (cptLEDyellow > 30) cptLEDyellow = 4;
}

String PrefiltreJson(String F1, String F2, String Json) {
  int p = Json.indexOf(F1);
  Json = Json.substring(p);
  p = Json.indexOf(F2);
  Json = Json.substring(p);
  return Json;
}
String SubJson(String F1, String F2, String Json) {
  int p = Json.indexOf(F1);
  Json = Json.substring(p);
  p = Json.indexOf(F2);
  Json = Json.substring(0, p + 1);
  return Json;
}

float ValJson(String nom, String Json) {
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  float val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toFloat();
  }
  return val;
}
long LongJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(".");
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}

long myLongJson(String nom, String Json) {  // Alternative a LongJson au dessus pour extraire chez RTE nb jour Tempo  https://particulier.RTE.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");       //<==== Recherche d'une virgule et non d'un point
  if (q == -1) q = Json.length();  //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned long ULongJson(String nom, String Json) {  // Alternative a LongJson au dessus pour extraire chez RTE nb jour Tempo  https://particulier.RTE.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");       //<==== Recherche d'une virgule et non d'un point
  if (q == -1) q = Json.length();  //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  unsigned long val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    Json = "0000" + Json;
    int L = Json.length();
    unsigned long y = (Json.substring(0, L - 5)).toInt();  //Problème des valeurs signées dans un unsigned
    unsigned long z = (Json.substring(L - 5)).toInt();
    val = (y * 100000) + z;
  }
  return val;
}
int IntJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = Json.length();
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  int val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
byte ByteJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = Json.length();
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  byte val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned short UShortJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = Json.length();
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  unsigned short val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
short ShortJson(String nom, String Json) {  // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1) q = Json.length();
  p = Json.indexOf("}");
  if (p > 0)
    p = min(p, q);
  else
    p = q;
  short val = 0;
  if (p > 0) {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}


String StringJson(String nom, String Json) {
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(0, p);
  return Json;
}
