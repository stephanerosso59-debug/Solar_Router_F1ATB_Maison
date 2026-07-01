// ============================================
// F1ATB Maison - GPIO Configuration
// ESP32-D0WD 30 pins - PCB custom W5500 SPI
// ============================================

#ifndef GPIO_PIN_CONFIG_H
#define GPIO_PIN_CONFIG_H

// ============================================
// DIMMER MODULE (Triac Control)
// ============================================
#define GPIO_ZC_DETECT    23    // Zero-Cross Detection (LTV-814S)
#define GPIO_MOC_COMMAND  22    // MOC3021S commande (Pulse triac)

// ============================================
// SOLID STATE RELAY (SSR)
// ============================================
#define GPIO_SSR_RELAY    21    // SSR Control Output

// ============================================
// LINKY METER
// ============================================
#define GPIO_LINKY_RX     16    // Serial RX from Linky (TIC)

// ============================================
// ETHERNET W5500 SPI intégré sur PCB
// ============================================
#define GPIO_W5500_CS     5     // Chip Select
#define GPIO_W5500_MOSI   13    // MOSI
#define GPIO_W5500_MISO   15    // MISO
#define GPIO_W5500_CLK    14    // Clock
#define GPIO_W5500_INT    25    // Interrupt

// ============================================
// JSY-MK-194T POWER METER
// ============================================
#define GPIO_JSY_RX       27    // Serial RX
#define GPIO_JSY_TX       26    // Serial TX

// ============================================
// STATUS LEDs
// ============================================
#define GPIO_LED_GREEN    18    // LED Verte
#define GPIO_LED_RED      19    // LED Rouge

// ============================================
// DS18B20 TEMPERATURE SENSOR (OneWire)
// ============================================
#define GPIO_TEMP_SENSOR  4     // Sonde température DS18B20

// ============================================
// 5V FAN (PWM via 2N2222A)
// ============================================
#define GPIO_FAN_PWM      33    // Ventilateur PWM

// ============================================
// ECRAN OLED SSD1306 (optionnel)
// ============================================
#define GPIO_SDA_OLED     4     // SDA SSD1306
#define GPIO_SCL_OLED     32    // SCL SSD1306

// ============================================
// NOTES GPIO
// ============================================
// GPIO 0  = BOOT (strapping pin)
// GPIO 1  = TX debug série
// GPIO 3  = RX debug série
// GPIO 6-11 = Flash SPI interne (non disponibles)
// GPIO 12 = Strapping pin (attention boot)
// GPIO 34-39 = Entrées seules ADC

// ============================================
// RESUME PCB F1ATB MAISON
// ============================================
// GPIO4  → DS18B20 OneWire / SDA OLED
// GPIO5  → W5500 CS
// GPIO13 → W5500 MOSI
// GPIO14 → W5500 CLK
// GPIO15 → W5500 MISO
// GPIO16 → Linky RX (TIC)
// GPIO18 → LED Verte
// GPIO19 → LED Rouge
// GPIO21 → SSR Relay
// GPIO22 → MOC3021S (Triac pulse)
// GPIO23 → ZC Detect
// GPIO25 → W5500 INT
// GPIO26 → JSY TX
// GPIO27 → JSY RX
// GPIO32 → SCL OLED
// GPIO33 → Fan PWM

#endif // GPIO_PIN_CONFIG_H
