// ============================================
// F1ATB Maison - Temperature Sensors
// DS18B20 OneWire - Lecture + Protection Triac
// ============================================

#ifndef TEMPERATURE_SENSORS_H
#define TEMPERATURE_SENSORS_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include "GpioPinConfig.h"

// ============================================
// CONFIGURATION
// ============================================
#define TEMP_READ_INTERVAL_MS   5000    // Lecture toutes les 5 secondes
#define TEMP_MAX_CHAUFFE_EAU    60.0    // °C - Seuil coupure chauffe-eau
#define TEMP_MAX_SECURITE       75.0    // °C - Seuil sécurité absolue (tous)
#define TEMP_ERROR_VALUE        -127.0  // Valeur Dallas si sonde déconnectée

// ============================================
// NOMS DES SONDES (par index OneWire)
// ============================================
#define SONDE_CHAUFFE_EAU       0       // Index 0 = chauffe-eau
#define SONDE_EXTERIEUR         1       // Index 1 = extérieur (si présente)
// Ajouter d'autres index si plus de sondes

// ============================================
// OBJETS
// ============================================
OneWire oneWire(GPIO_TEMP_SENSOR);   // DS18B20 sur GPIO4 (cf. GpioPinConfig.h)
DallasTemperature sensors(&oneWire);

// Températures courantes
float tempChauffEau   = 0.0;
float tempExterieur   = 0.0;
int   nbSondes        = 0;

// Flag de sécurité - mis à true si T° dépassée
bool  tempSecuriteAtteinte = false;

// Timestamp dernière lecture
unsigned long lastTempRead = 0;

// ============================================
// INIT
// ============================================
void initTemperatureSensors() {
    sensors.begin();
    nbSondes = sensors.getDeviceCount();
    Serial.print("[TEMP] Sondes DS18B20 détectées : ");
    Serial.println(nbSondes);
}

// ============================================
// LECTURE DES SONDES
// Retourne true si une température de sécurité
// est dépassée → le Triac doit être coupé
// ============================================
bool readTemperatures() {
    unsigned long now = millis();
    if (now - lastTempRead < TEMP_READ_INTERVAL_MS) {
        return tempSecuriteAtteinte; // Pas encore l'heure de lire
    }
    lastTempRead = now;

    sensors.requestTemperatures();

    // Lecture sonde chauffe-eau
    if (nbSondes > SONDE_CHAUFFE_EAU) {
        tempChauffEau = sensors.getTempCByIndex(SONDE_CHAUFFE_EAU);
        if (tempChauffEau == TEMP_ERROR_VALUE) {
            Serial.println("[TEMP] ERREUR: Sonde chauffe-eau déconnectée !");
            tempSecuriteAtteinte = true; // Sécurité : on coupe si sonde morte
        } else {
            Serial.print("[TEMP] Chauffe-eau: ");
            Serial.print(tempChauffEau);
            Serial.println(" C");
            // --- Reconstruit le 2026-07-01 apres troncature (a verifier) ---
            // Securite absolue : coupe le Triac si le seuil critique est depasse
            if (tempChauffEau >= TEMP_MAX_SECURITE) {
                tempSecuriteAtteinte = true;
                Serial.println("[TEMP] SECURITE: seuil critique depasse -> coupure Triac");
            } else if (tempChauffEau < TEMP_MAX_CHAUFFE_EAU) {
                tempSecuriteAtteinte = false; // temperature redescendue sous le seuil
            }
        }
    }

    // Lecture sonde exterieure (optionnelle)
    if (nbSondes > SONDE_EXTERIEUR) {
        tempExterieur = sensors.getTempCByIndex(SONDE_EXTERIEUR);
    }

    return tempSecuriteAtteinte;
}

#endif // TEMPERATURE_SENSORS_H