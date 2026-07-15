#include <Arduino.h>
#include <math.h>
#include <TFT_eSPI.h>

#include "display.h"
#include "config.h"

namespace {

TFT_eSPI tft = TFT_eSPI();

const uint16_t COLOR_LABEL = TFT_DARKGREY;
const uint16_t COLOR_NEUTRAL = TFT_WHITE;

const int16_t METER_X = 0;
const int16_t METER_WIDTH = 150;
const int16_t METER_HEIGHT = 6;

// Anzeigebereich für den Temperatur-Balken (rein kosmetisch, keine Regelschwelle) - 20°C als
// "kalt"-Referenz bis zur Übertemperatur-Schwelle.
const float TEMP_METER_MIN_C = 20.0f;

// Grün -> Gelb -> Rot entlang der Balkenposition, nicht nach dem Endwert - klassischer
// Tacho-/Pegel-Look. positionPercent = wo entlang der vollen Balkenbreite (0-100).
// reversed: Grün am gefüllten (hohen) statt am leeren Ende - für Werte, bei denen "voll"
// gut bedeutet (Akku), statt "hoch" schlecht (Temperatur, LED/Fan-Intensität).
uint16_t gradientColor(uint8_t positionPercent, bool reversed) {
    uint8_t p = reversed ? (100 - positionPercent) : positionPercent;
    uint8_t r, g;
    if (p <= 50) {
        r = map(p, 0, 50, 0, 255);
        g = 255;
    } else {
        r = 255;
        g = map(p, 50, 100, 255, 0);
    }
    // RGB565 packen (5-6-5 Bit).
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3);
}

// Balken-Meter mit fließendem Grün-Gelb-Rot-Übergang im gefüllten Teil, Rest in gedimmtem
// Grau - in schmalen Spalten gezeichnet, da TFT_eSPI keinen Verlaufsfüllbefehl hat. Immer
// beide Teile neu gezeichnet (kein Flackern, kein separates Löschen nötig).
void drawMeter(int16_t y, uint8_t percent, bool reversed = false) {
    int16_t filledWidth = (int16_t)((uint32_t)METER_WIDTH * percent / 100);
    const int16_t columnWidth = 3;
    for (int16_t x = 0; x < filledWidth; x += columnWidth) {
        uint8_t positionPercent = (uint8_t)((uint32_t)x * 100 / METER_WIDTH);
        int16_t width = min((int16_t)columnWidth, (int16_t)(filledWidth - x));
        tft.fillRect(METER_X + x, y, width, METER_HEIGHT, gradientColor(positionPercent, reversed));
    }
    tft.fillRect(METER_X + filledWidth, y, METER_WIDTH - filledWidth, METER_HEIGHT,
                 COLOR_LABEL);
}

}  // namespace

void displayInit() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(1);  // Querformat, 160x128
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    // Deckender Textfarben-Hintergrund: jedes Zeichen überschreibt den alten Inhalt beim
    // Zeichnen direkt, kein separates fillScreen()+Neuzeichnen nötig -> kein Flackern.
    tft.setTextColor(COLOR_NEUTRAL, TFT_BLACK);
}

void displayShowBattery(float voltage, uint8_t chargePercent, bool throttleActive) {
    // Wert trägt dieselbe Verlaufsfarbe wie sein Meter (an der aktuellen Füllposition) -
    // Text und Balken bilden dieselbe Farbaussage.
    uint16_t color = gradientColor(chargePercent, true);

    tft.setCursor(0, 0);
    tft.setTextColor(COLOR_LABEL, TFT_BLACK);
    tft.print("Akku");
    char line[16];
    snprintf(line, sizeof(line), "%7.3fV", voltage);
    tft.setTextColor(color, TFT_BLACK);
    tft.print(line);
    // Kompakter Textmarker zusätzlich zur Farbe, damit die Drosselung nicht allein an der
    // Farbe hängt (Farbfehlsichtigkeit/Displayblickwinkel).
    tft.print(throttleActive ? "!" : " ");

    drawMeter(18, chargePercent, true);
}

void displayShowTemperature(float tempC, bool throttleActive) {
    uint8_t meterPercent = 0;
    if (!isnan(tempC)) {
        float fraction =
            (tempC - TEMP_METER_MIN_C) / (LED_THROTTLE_ZERO_C - TEMP_METER_MIN_C) * 100.0f;
        meterPercent = (uint8_t)constrain(fraction, 0.0f, 100.0f);
    }
    uint16_t color = gradientColor(meterPercent, false);

    tft.setCursor(0, 32);
    tft.setTextColor(COLOR_LABEL, TFT_BLACK);
    tft.print("Temp");
    char line[16];
    if (isnan(tempC)) {
        snprintf(line, sizeof(line), "  --.-C");
    } else {
        snprintf(line, sizeof(line), "%6.1fC", tempC);
    }
    tft.setTextColor(color, TFT_BLACK);
    tft.print(line);
    tft.print(throttleActive ? "!" : " ");

    drawMeter(50, meterPercent);
}

void displayShowLed(uint8_t percent, float watts) {
    uint16_t color = gradientColor(percent, false);

    tft.setCursor(0, 64);
    tft.setTextColor(COLOR_LABEL, TFT_BLACK);
    tft.print("LED");
    char line[16];
    snprintf(line, sizeof(line), "%3d%% %3dW", percent, (int)roundf(watts));
    tft.setTextColor(color, TFT_BLACK);
    tft.print(line);

    drawMeter(82, percent);
}

void displayShowFan(uint8_t percent) {
    uint16_t color = gradientColor(percent, false);

    tft.setCursor(0, 96);
    tft.setTextColor(COLOR_LABEL, TFT_BLACK);
    tft.print("Fan");
    char line[16];
    snprintf(line, sizeof(line), "%8d%%", percent);
    tft.setTextColor(color, TFT_BLACK);
    tft.print(line);

    drawMeter(114, percent);
}

