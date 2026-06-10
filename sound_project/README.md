# Arcade Sound System - RPi Zero 2W

Generador de sonidos arcade, telefonos y modem 56k usando **libbcm2835**.

## Dependencias

Requiere libbcm2835 (PWM por hardware):

```bash
# Via build.sh
./build.sh
```

## Compilar

```bash
make
sudo make run
```

## Sonidos incluidos

- Arcade: Beep, Blip, Explosion, Coin, Victory, Laser, Power Up, Game Over
- Telefono: Ring clasico, Nokia Tune, Motorola, SMS, DTMF
- Fax/Modem 56k: Handshake, Transmission, Error, Connect, Training

## Estructura

```
sound_project/
├── include/
│   ├── PiezoDriver.hpp
│   ├── SoundGenerator.hpp
│   └── WaveformGenerator.hpp
├── src/
│   ├── main.cpp
│   ├── SoundGenerator.cpp
│   └── WaveformGenerator.cpp
├── Makefile
├── build.sh
└── README.md
```
