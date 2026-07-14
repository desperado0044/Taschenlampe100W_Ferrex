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
- **Temperatursensor:** DS18B20 auf dem Kühlkörper (Pin noch nicht vermessen)
- **Lüfter:** per MOSFET PWM-gesteuert
- **LED:** 100W-COB-LED per MOSFET PWM-gesteuert
- **Bedienung:** Inkrementalgeber mit Taster (Pins noch nicht vermessen)
- **Stromversorgung:** Ferrex 20V/40V-Akkupack (im 20V/5S-Modus betrieben, ~15-21V),
  Spannungsmessung über einstellbaren Spannungsteiler an einem ADC-Pin

### Pinbelegung

Siehe [`src/config.h`](src/config.h) für die vollständige, kommentierte Zuordnung.
Aktuell vermessen: TFT (SPI1), Lüfter-PWM, LED-PWM, Akkuspannungs-ADC. Noch offen:
DS18B20, Inkrementalgeber.

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
  explizitem `swd`-Transport.

### Akkuspannungsmessung

Der Ferrex-Akku hat keinen internen BMS-Zellschutz (nur ein NTC-Thermistor für den
Lader, kein Balancer/Cutoff) - die Firmware-seitige Unterspannungsabschaltung ist daher
die einzige Schutzschicht gegen Tiefentladung, nicht nur ein Backup:

- Ab **16,0V**: LED wird auf 10% Helligkeit gedimmt (Vorwarnung)
- Ab **15,0V**: LED wird hart abgeschaltet (3,0V/Zelle bei 5S)
- Beide Schwellen mit Hysterese, um Flattern nahe der Grenze zu vermeiden

Der Spannungsteiler-Faktor in `config.h` ist gegen ein Multimeter kalibriert und muss
bei erneutem Verstellen des Trimmers neu bestimmt werden.

## Geplanter Funktionsumfang (Display)

- LED-Helligkeit (%)
- Akkuspannung
- Ladezustand (rechnerisch aus Spannung, kein BMS-Auslesen möglich)
- Kühlkörpertemperatur (DS18B20)
- Lüfterdrehzahl (%)
- Warnungen (Unterspannung, Übertemperatur)
