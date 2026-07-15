#pragma once

// Initialisiert CLK/DT (Interrupt-Eingänge, Pullup) und SW (Taster, Pullup).
void encoderInit();

// Gibt die seit dem letzten Aufruf akkumulierte Drehung zurück (positiv = im Uhrzeigersinn)
// und setzt den internen Zähler zurück - liefert ein Delta, keinen Dauerzustand.
int32_t encoderReadDelta();

// True, während der Taster gedrückt gehalten wird (entprellt). Muss einmal pro Loop-Durchlauf
// aufgerufen werden, auch wenn der Rückgabewert nicht direkt gebraucht wird - hält die
// Kurz-/Lang-Druck-Erkennung am Laufen.
bool encoderIsButtonPressed();

// True für genau einen Aufruf, wenn ein kurzer Tastendruck beim Loslassen erkannt wurde.
bool encoderConsumeShortPress();

// True für genau einen Aufruf, sobald ein langer Tastendruck die Schwelle erreicht (sofort,
// nicht erst beim Loslassen).
bool encoderConsumeLongPress();

// Nach dem Aufwachen aus dem Deep-Sleep aufrufen: referenziert Encoder-Zählerbasis und
// Taster-Entprellung neu, statt gegen einen durch die Schlafphase verzerrten/veralteten
// Stand zu vergleichen.
void encoderResync();
