#include <Arduino.h>
#include <math.h>

#include "menu.h"
#include "settings.h"
#include "battery.h"
#include "display.h"

namespace {

struct ConfigItem {
    const char* name;
    float (*getValue)();
    void (*setValue)(float);
    float min;
    float max;
    float step;
    const char* unit;
    // Optional (nullptr wenn ungenutzt) - zusätzliche Live-Vorschau während des Bearbeitens,
    // berechnet aus dem gerade editierten (noch nicht gespeicherten) Wert.
    float (*preview)(float editValue);
    // NAN = kein Warnhinweis. Sonst: roter Warnhinweis, sobald der editierte Wert diesen Wert
    // überschreitet - z.B. für die LED-Temperaturpunkte oberhalb der konservativen,
    // datenblattfreien Vorgabe (siehe config.h).
    float warnAboveValue;
};

// Live-Spannungsanzeige für den ADC-Korrekturfaktor: aus der aktuell geglätteten Spannung und
// dem AKTUELL GESPEICHERTEN Faktor lässt sich die rohe Teiler-Pinspannung zurückrechnen, damit
// beim Drehen sofort sichtbar ist, welche Akkuspannung der NEUE (noch nicht gespeicherte)
// Faktor ergeben würde - ohne dafür extra die Rohspannung in battery.cpp mitführen zu müssen.
float adcRatioPreview(float editValue) {
    float rawPinVoltage = batteryGetVoltageSmoothed() / settingsGetAdcDividerRatio();
    return rawPinVoltage * editValue;
}

const ConfigItem items[] = {
    {"Grundhelligkeit", settingsGetDefaultBrightness, settingsSetDefaultBrightness, 0.0f,
     100.0f, 5.0f, "%", nullptr, NAN},
    {"LED-Nennleistung", settingsGetLedRatedWatts, settingsSetLedRatedWatts, 1.0f, 300.0f, 5.0f,
     "W", nullptr, NAN},
    {"Luefter ein Temp", settingsGetFanOnTempC, settingsSetFanOnTempC, 0.0f, 100.0f, 1.0f, "C",
     nullptr, NAN},
    {"Luefter 100% Temp", settingsGetFanFullTempC, settingsSetFanFullTempC, 0.0f, 100.0f, 1.0f,
     "C", nullptr, NAN},
    {"Dimmen ab Temp", settingsGetLedThrottleStartC, settingsSetLedThrottleStartC, 0.0f, 100.0f,
     1.0f, "C", nullptr, 80.0f},
    {"LED-Aus Temp", settingsGetLedThrottleZeroC, settingsSetLedThrottleZeroC, 50.0f, 100.0f,
     1.0f, "C", nullptr, 80.0f},
    {"Dimmen ab Volt", settingsGetBatteryRampStartVoltage, settingsSetBatteryRampStartVoltage,
     15.0f, 21.0f, 0.1f, "V", nullptr, NAN},
    {"LED-Aus Volt", settingsGetBatteryCutoffVoltage, settingsSetBatteryCutoffVoltage, 15.0f,
     18.0f, 0.1f, "V", nullptr, NAN},
    {"Ladeschluss Volt", settingsGetBatteryFullVoltage, settingsSetBatteryFullVoltage, 15.0f,
     40.0f, 0.1f, "V", nullptr, NAN},
    {"ADC-Faktor", settingsGetAdcDividerRatio, settingsSetAdcDividerRatio, 1.0f, 20.0f, 0.01f,
     "", adcRatioPreview, NAN},
};
const uint8_t ITEM_COUNT = sizeof(items) / sizeof(items[0]);

UiMode mode = UiMode::Main;
uint8_t selectedIndex = 0;
float editValue = 0.0f;

}  // namespace

void menuInit() {
}

UiMode menuUpdate(int32_t encoderDelta, bool shortPress, bool longPress) {
    switch (mode) {
        case UiMode::Main:
            if (longPress) {
                selectedIndex = 0;
                mode = UiMode::Menu;
            }
            break;

        case UiMode::Menu:
            if (longPress) {
                mode = UiMode::Main;
            } else if (shortPress) {
                editValue = items[selectedIndex].getValue();
                mode = UiMode::Edit;
            } else if (encoderDelta != 0) {
                int32_t newIndex = (int32_t)selectedIndex + encoderDelta;
                selectedIndex =
                    (uint8_t)constrain(newIndex, (int32_t)0, (int32_t)(ITEM_COUNT - 1));
            }
            break;

        case UiMode::Edit: {
            const ConfigItem& item = items[selectedIndex];
            if (longPress) {
                mode = UiMode::Menu;  // Abbrechen ohne Speichern.
            } else if (shortPress) {
                item.setValue(editValue);
                mode = UiMode::Menu;
            } else if (encoderDelta != 0) {
                float newValue = editValue + (float)encoderDelta * item.step;
                editValue = constrain(newValue, item.min, item.max);
            }
            break;
        }
    }

    return mode;
}

void menuShow() {
    if (mode == UiMode::Menu) {
        // Scroll-Fenster, das der Auswahl folgt: minimaler Offset, der selectedIndex sichtbar
        // hält - kein eigener persistenter Scroll-Zustand nötig, ergibt sich direkt aus
        // selectedIndex.
        uint8_t scrollOffset = 0;
        if (selectedIndex >= MENU_VISIBLE_ROWS) {
            scrollOffset = selectedIndex - MENU_VISIBLE_ROWS + 1;
        }
        uint8_t visibleCount = min((uint8_t)(ITEM_COUNT - scrollOffset), MENU_VISIBLE_ROWS);

        const char* names[MENU_VISIBLE_ROWS];
        for (uint8_t i = 0; i < visibleCount; i++) {
            names[i] = items[scrollOffset + i].name;
        }

        bool hasMoreAbove = scrollOffset > 0;
        bool hasMoreBelow = (scrollOffset + visibleCount) < ITEM_COUNT;
        displayShowMenuList(names, visibleCount, selectedIndex - scrollOffset, hasMoreAbove,
                             hasMoreBelow);
        return;
    }

    // UiMode::Edit
    const ConfigItem& item = items[selectedIndex];
    float preview = (item.preview != nullptr) ? item.preview(editValue) : NAN;
    bool warning = !isnan(item.warnAboveValue) && editValue > item.warnAboveValue;
    displayShowMenuEdit(item.name, editValue, item.unit, preview, warning);
}
