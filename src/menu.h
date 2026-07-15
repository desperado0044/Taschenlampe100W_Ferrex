#pragma once

// Main: normaler Betrieb (Taster = Turbo-Toggle, Drehen = Helligkeit).
// Menu: Konfigurationsbildschirm, Drehen blättert durch die Punkte.
// Edit: ausgewählter Punkt wird bearbeitet, Drehen ändert den Wert.
enum class UiMode { Main, Menu, Edit };

void menuInit();

// Verarbeitet Encoder-Drehung/Taster passend zum aktuellen Menü-Zustand:
// Main --langer Druck--> Menu | Menu --kurzer Druck--> Edit (Punkt übernehmen) |
// Edit --kurzer Druck--> Menu (Wert speichern) | Menu/Edit --langer Druck--> Main (verwerfen).
// Gibt den resultierenden UI-Modus zurück - main.cpp reicht encoderDelta/shortPress nur im
// Main-Modus an ledUpdate() weiter und zeigt je nach Rückgabewert die normale Anzeige oder
// menuShow() an. Einmal pro Loop-Durchlauf aufrufen.
UiMode menuUpdate(int32_t encoderDelta, bool shortPress, bool longPress);

// Zeigt den aktuell ausgewählten/bearbeiteten Menüpunkt an - nur aufrufen, wenn menuUpdate()
// zuletzt Menu oder Edit zurückgegeben hat.
void menuShow();
