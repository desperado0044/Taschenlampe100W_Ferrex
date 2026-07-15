# Taschenlampe100W_Ferrex

Firmware für eine selbstgebaute 100W-LED-Taschenlampe auf Basis eines STM32 "Bluepill"
(STM32F103C8), betrieben am 20V/40V-Akkusystem eines Ferrex-Werkzeugs (Aldi-Eigenmarke).

Die Hardware war bereits fertig verdrahtet, die ursprünglichen Projektdateien sind aber
verloren gegangen - dieses Repo ist der Neuaufbau der Firmware bei bereits bestehender
Verkabelung.

## Hardware

- **MCU:** STM32F103C8 "Bluepill" (Chip meldet sich mit 128 KiB Flash statt der
  nominellen 64 KiB - typisches Klon-Verhalten, kein Fehler)
- **Display:** 1,8" SPI-TFT 128x160, rotes "V1.1"-Modul (ST7735S-Klon)
- **Temperatursensor:** DS18B20 auf dem Kühlkörper
- **Lüfter:** 4-Draht-PC-Lüfter, per MOSFET PWM-gesteuert (schaltet Rot/Schwarz statt über
  den eigentlich dafür vorgesehenen Blau-Steuereingang)
- **LED:** 100W-COB-LED per MOSFET PWM-gesteuert
- **Bedienung:** Inkrementalgeber mit Taster
- **Stromversorgung:** Ferrex 20V/40V-Akkupack (im 20V/5S-Modus betrieben, ~15-21V),
  Spannungsmessung über einstellbaren Spannungsteiler an einem ADC-Pin

### Pinbelegung

Siehe [`src/config.h`](src/config.h) für die vollständige, kommentierte Zuordnung
(TFT, Lüfter-PWM, LED-PWM, Akkuspannungs-ADC, DS18B20, Inkrementalgeber - alle vermessen).

## Firmware

- **Toolchain:** [PlatformIO](https://platformio.org/), `platform = ststm32`,
  `framework = arduino` (STM32duino)
- **Display-Library:** [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI), komplett über
  `build_flags` in `platformio.ini` konfiguriert (nicht über `User_Setup.h`). Wichtig:
  `Adafruit_ST7735` initialisiert diesen Display-Klon **nicht** (bleibt weiß) -
  TFT_eSPI und `Arduino_GFX` funktionieren beide, TFT_eSPI ist aber deutlich
  flashschonender.
- **Flashen:** über ST-Link (V2-Klon) via OpenOCD, `pio run -t upload`. Der Standard-
  `upload_protocol = stlink` funktioniert mit diesem Klon nicht (`hla_swd`-Transport
  wird nicht unterstützt) - `platformio.ini` nutzt daher einen custom OpenOCD-Aufruf mit
  explizitem `swd`-Transport. Läuft das Gerät gerade im Deep-Sleep (siehe unten), kann
  ein Upload-Versuch einmalig mit "unable to connect to the target" fehlschlagen -
  einfach erneut versuchen.

## Bedienung

- **Kurzer Tasterdruck:** LED an (gespeicherte Helligkeit), oder - falls die LED bereits
  an ist - Umschalten zwischen normaler Helligkeit und Turbo (100%).
- **Langer Tasterdruck (≥600ms):** LED aus, MCU geht sofort in den STM32-STOP-Modus
  (Deep-Sleep). Auch direkt nach dem Booten schläft das Gerät sofort ein - der erste
  Tasterdruck weckt auf und schaltet die LED ein.
- **Drehen:** stellt die Helligkeit in 40 Stufen (2,5% pro Klick) ein, unabhängig vom
  Turbo-Zustand.
- Die Helligkeit wird **nicht** über einen Stromausfall hinweg gespeichert (siehe
  `LED_DEFAULT_PERCENT` in `config.h`, Standard 35%) - der Spannungswandler zwischen
  Akku und Board bricht bei Stromverlust zu abrupt weg, um einen zuverlässigen
  EEPROM-Notspeichervorgang zu garantieren.

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
  linear fallend bis 0% bei 80°C.

Beide sind bewusst als stetige Rampe statt als harter Schwellwert mit Hysterese
umgesetzt: bei einem harten Schwellwert könnte die Lastabsenkung durch hohen LED-Strom
(IR-Drop am Akku) zu einem Ein/Aus-Takten führen, sobald die Ruhespannung knapp über der
Schwelle liegt. Die Rampe ist dagegen selbstregulierend - sinkt die Helligkeit, sinkt
auch der Strom, die Spannung erholt sich, statt hart umzuschalten.

Der Spannungsteiler-Faktor in `config.h` ist gegen ein Multimeter kalibriert und muss
bei erneutem Verstellen des Trimmers neu bestimmt werden.

## Lüftersteuerung

Lineare Kurve zwischen 40°C (aus) und 65°C (100%), abhängig von der DS18B20-Messung auf
dem Kühlkörper. Läuft nur, solange die LED eingeschaltet ist (auch bei hoher
Umgebungstemperatur sonst aus) - vor dem Deep-Sleep wird die PWM zusätzlich explizit auf
0 gesetzt, da der Timer im STOP-Modus sonst mitten im PWM-Zyklus einfrieren und hängen
bleiben kann. Eine Drehzahlmessung über das Tacho-Signal des Lüfters wurde nach
ausgiebigem Debugging aufgegeben - der Lüfter liefert vermutlich kein klassisches
Puls-Tachosignal (ohne Oszilloskop nicht weiter eingrenzbar). Die Lüfteranzeige zeigt
daher den kommandierten PWM-Duty-Cycle, keine gemessene Ist-Drehzahl.

## Display

- **Akku:** Spannung, Ladezustand als Balken (0% = Cutoff-Schwelle, 100% = volle
  Ladeschlussspannung), Warnmarker bei aktiver Drosselung.
- **Temperatur:** Kühlkörpertemperatur, Warnmarker bei aktiver Drosselung.
- **LED:** Helligkeit in % sowie eine geschätzte Leistungsaufnahme in Watt
  (PWM-Duty × Nennleistung aus `LED_RATED_WATTS`) - kein echter Strommesswert, da kein
  Shunt verbaut ist, nur eine Näherung.
- **Lüfter:** kommandierter PWM-Duty-Cycle in %.
