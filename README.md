# Taschenlampe100W_Ferrex

Firmware für eine selbstgebaute 100W-LED-Taschenlampe auf Basis eines STM32 "Bluepill"
(STM32F103C8), betrieben am 20V/40V-Akkusystem eines Ferrex-Werkzeugs (Aldi-Eigenmarke).

Die Hardware war bereits fertig verdrahtet, die ursprünglichen Projektdateien sind aber
verloren gegangen - dieses Repo ist der Neuaufbau der Firmware bei bereits bestehender
Verkabelung.

## Hardware

- **MCU:** STM32F103C8 "Bluepill"
- **Display:** 1,8" SPI-TFT 128x160, rotes "V1.1"-Modul (ST7735S-Klon)
- **Temperatursensor:** DS18B20 auf dem Kühlkörper
- **Lüfter:** 4-Draht-PC-Lüfter, per MOSFET (IRF3708, 10kΩ Pull-down am Gate gegen GND)
  PWM-gesteuert
- **LED:** 100W-COB-LED, betrieben über eine externe Konstantstromquelle, deren Strom auf
  die 100W-LED eingestellt sein muss - die Firmware misst selbst keine Leistung/keinen
  Strom, sondern moduliert per MOSFET (IRF3708, 10kΩ Pull-down am Gate gegen GND) nur den
  PWM-Duty-Cycle der Konstantstromquelle
- **Bedienung:** Inkrementalgeber mit Taster (KY-040-artiges Modul)
- **Stromversorgung:** Ferrex 20V/40V-Akkupack (im 20V/5S-Modus betrieben, ~15-21V),
  Spannungsmessung über einstellbaren 10kΩ-Spannungsteiler an einem ADC-Pin

### Pinbelegung

Siehe [`src/config.h`](src/config.h) für die vollständige, kommentierte Zuordnung
(TFT, Lüfter-PWM, LED-PWM, Akkuspannungs-ADC, DS18B20, Inkrementalgeber - alle vermessen).

## Firmware

- **Toolchain:** [PlatformIO](https://platformio.org/), `platform = ststm32`,
  `framework = arduino` (STM32duino)
- **Display-Library:** [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI), komplett über
  `build_flags` in `platformio.ini` konfiguriert (nicht über `User_Setup.h`).
- **Flashen:** über ST-Link (V2-Klon) via OpenOCD, `pio run -t upload`. Der Standard-
  `upload_protocol = stlink` funktioniert mit diesem Klon nicht (`hla_swd`-Transport
  wird nicht unterstützt) - `platformio.ini` nutzt daher einen custom OpenOCD-Aufruf mit
  explizitem `swd`-Transport.

## Bedienung

Die LED hat keinen eigenen An/Aus-Zustand - dafür ist der Netzschalter am Akku da. Sobald
das Gerät Strom hat, geht die LED sofort mit der Grundhelligkeit an.

- **Kurzer Tasterdruck:** Umschalten zwischen normaler Helligkeit und Turbo (100%).
- **Langer Tasterdruck (≥1000ms):** öffnet/verlässt den Konfigurationsbildschirm (siehe
  unten). Verlassen verwirft eine gerade unbestätigte Bearbeitung.
- **Drehen:** stellt die Helligkeit in 40 Stufen (2,5% pro Klick) ein, unabhängig vom
  Turbo-Zustand - runtergedreht auf 0% dient als manuelles "Aus".

## Konfigurationsbildschirm

Langer Tasterdruck öffnet eine Liste aller Konfigurationspunkte (aktueller mit ">"
markiert). Drehen blättert durch die Liste, kurzer Druck wählt einen Punkt zum Bearbeiten
aus. Im Bearbeiten-Modus stellt Drehen den Wert ein, kurzer Druck speichert (sofort ins
EEPROM - übersteht also einen Stromausfall), langer Druck verwirft die Änderung. Alle
Werte unten sind darüber einstellbar, die in `config.h` genannten Zahlen sind nur die
Compile-Time-Vorgaben für einen frisch geflashten Controller:

- **Grundhelligkeit** - Startwert beim Einschalten.
- **LED-Nennleistung** - für die geschätzte Watt-Anzeige (siehe Display unten).
- **Lüfter ein Temp / Lüfter 100% Temp** - Eckpunkte der Lüfterkurve (siehe unten).
- **Dimmen ab Temp / LED-Aus Temp** - Eckpunkte der Temperatur-Drosselrampe (siehe
  Schutzfunktionen). Werte über 80°C werden rot markiert - siehe Warnhinweis dort.
- **Dimmen ab Volt / LED-Aus Volt** - Eckpunkte der Akku-Drosselrampe (siehe
  Schutzfunktionen).
- **Ladeschluss Volt** - für die Ladezustands-%-Anzeige (100% bei diesem Wert), rein
  kosmetisch, keine Schutzfunktion.
- **ADC-Faktor** - Kalibrierfaktor des Spannungsteilers, mit Live-Vorschau der sich daraus
  ergebenden Akkuspannung während der Bearbeitung. **Vorsicht:** Dieser Faktor kalibriert
  nur die *Anzeige/Auswertung*, er schützt nicht den ADC-Pin selbst - bevor der
  Spannungsteiler an ein Akkupack mit höherer Maximalspannung als die Ferrex 20V/5S-Reihe
  (~21V) angeschlossen wird, muss der Teiler so dimensioniert (Widerstände!) und vorher
  mit einem Multimeter gemessen werden, dass die Pinspannung am ADC bei
  Akku-Maximalspannung sicher unter 3,3V bleibt - sonst droht eine ADC-/MCU-Überlastung.

## Schutzfunktionen

Der Ferrex-Akku hat keinen internen BMS-Zellschutz (nur ein NTC-Thermistor für den
Lader, kein Balancer/Cutoff) - die Firmware ist daher die einzige Schutzschicht gegen
Tiefentladung, nicht nur ein Backup. Sowohl die Akkuspannung als auch die
Kühlkörpertemperatur wirken als **lineare Obergrenze auf die tatsächliche PWM-Leistung**
(nicht auf die wahrnehmungskorrigierte Helligkeit) - sie deckeln die eingestellte
Helligkeit nur nach oben, heben sie nie an:

- **Akku:** ab 16,5V keine Einschränkung, linear fallend bis 0% bei 15,0V
  (3,0V/Zelle bei 5S).
- **Temperatur:** ab 65°C (Lüfter läuft dort bereits auf 100%) keine Einschränkung,
  linear fallend bis 0% bei 80°C. Der DS18B20 sitzt auf dem Kühlkörper, nicht am LED-Die -
  80°C ist eine konservative Vorgabe ohne Datenblattbezug (der reale Temperaturgradient
  zur LED-Sperrschicht ist unbekannt). Der Konfigurationsbildschirm erlaubt bis 100°C für
  alle, die das eigene LED-Datenblatt kennen oder den Gap selbst gemessen haben - der Wert
  wird dann im Display rot markiert.

Beide sind bewusst als stetige Rampe statt als harter Schwellwert mit Hysterese
umgesetzt: bei einem harten Schwellwert könnte die Lastabsenkung durch hohen LED-Strom
(IR-Drop am Akku) zu einem Ein/Aus-Takten führen, sobald die Ruhespannung knapp über der
Schwelle liegt. Die Rampe ist dagegen selbstregulierend - sinkt die Helligkeit, sinkt
auch der Strom, die Spannung erholt sich, statt hart umzuschalten.

Der Spannungsteiler-Faktor ist gegen ein Multimeter kalibriert und muss bei erneutem
Verstellen des Trimmers neu bestimmt werden (über den Konfigurationsbildschirm, siehe
oben - kein Neuflashen nötig).

## Lüftersteuerung

Lineare Kurve zwischen (standardmäßig) 40°C aus und 65°C 100%, abhängig von der
DS18B20-Messung auf dem Kühlkörper - beide Eckpunkte über den Konfigurationsbildschirm
einstellbar. Läuft nur, solange die LED tatsächlich Leistung abgibt (auch bei hoher
Umgebungstemperatur sonst aus). Die Lüfteranzeige zeigt den kommandierten
PWM-Duty-Cycle, keine gemessene Ist-Drehzahl.

## Display

- **Akku:** Spannung, Ladezustand als Balken (0% = Cutoff-Schwelle, 100% = volle
  Ladeschlussspannung), Warnmarker bei aktiver Drosselung.
- **Temperatur:** Kühlkörpertemperatur, Warnmarker bei aktiver Drosselung.
- **LED:** Helligkeit in % sowie eine geschätzte Leistungsaufnahme in Watt
  (PWM-Duty × LED-Nennleistung, siehe Konfigurationsbildschirm) - kein echter
  Strommesswert, da kein Shunt verbaut ist, nur eine Näherung.
- **Lüfter:** kommandierter PWM-Duty-Cycle in %.
