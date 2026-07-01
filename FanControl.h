// ============================================
// F1ATB Maison - Contrôle Ventilateur
// PWM via GPIO 33 → R 2.2kΩ → Base 2N2222A
// Piloté par température DS18B20
// ============================================

#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "GpioPinConfig.h"

// ============================================
// CONFIGURATION PWM
// ============================================
#define FAN_PWM_CHANNEL     0       // Canal LEDC ESP32
#define FAN_PWM_FREQ        25000   // 25 kHz (inaudible)
#define FAN_PWM_RESOLUTION  8       // 8 bits → 0-255

// ============================================
// SEUILS DE TEMPÉRATURE → VITESSE VENTILATEUR
// ============================================
#define FAN_TEMP_OFF        35.0    // °C - En dessous : ventilateur arrêté
#define FAN_TEMP_MIN        40.0    // °C - Début vitesse minimale
#define FAN_TEMP_MAX        60.0    // °C - Vitesse maximale (255)

// ============================================
// INIT
// ============================================
void initFan() {
    // Core ESP32 3.x : nouvelle API LEDC (ledcAttach remplace ledcSetup+ledcAttachPin)
    ledcAttach(GPIO_FAN_PWM, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcWrite(GPIO_FAN_PWM, 0);  // Ventilateur arrêté au démarrage
    Serial.println("[FAN] Ventilateur PWM initialisé sur GPIO 33");
}

// ============================================
// MISE À JOUR VITESSE
// Appeler après readTemperatures()
// Passe la température la plus haute en paramètre
// ============================================
void updateFanSpeed(float temperature) {
    uint8_t pwmValue = 0;

    if (temperature <= FAN_TEMP_OFF) {
        pwmValue = 0;  // Arrêt
    } else if (temperature >= FAN_TEMP_MAX) {
        pwmValue = 255;  // Pleine vitesse
    } else if (temperature >= FAN_TEMP_MIN) {
        // Interpolation linéaire entre FAN_TEMP_MIN et FAN_TEMP_MAX
        pwmValue = (uint8_t)((temperature - FAN_TEMP_MIN) / (FAN_TEMP_MAX - FAN_TEMP_MIN) * 255);
    } else {
        // Entre FAN_TEMP_OFF et FAN_TEMP_MIN : vitesse minimale
        pwmValue = 30;  // Valeur minimale pour démarrer le moteur
    }

    ledcWrite(GPIO_FAN_PWM, pwmValue);   // core 3.x : ledcWrite prend le PIN (pas le canal)

    Serial.print("[FAN] T=");
    Serial.print(temperature);
    Serial.print("°C → PWM=");
    Serial.println(pwmValue);
}

#endif // FAN_CONTROL_H