# Donkey Kong - RPi Zero 2W + ST7789

Juego de Donkey Kong con sprites PNG, fisica AABB, 3 niveles y modo demo.

## Driver

Este proyecto usa **ioctl del kernel** (`/dev/gpiochip0` + `/dev/spidev0.0`),
igual que pacman/cars. No requiere librerías externas (ni pigpio, ni bcm2835).

## Controles

- **Joystick (GPIO)**: UP/DOWN/LEFT/RIGHT + BTN_A para saltar
- **Modo demo**: IA que juega automaticamente

## Compilar

```bash
make
make run        # ejecuta con sudo
```

## Conexion

| ST7789 | RPi Zero 2W |
|--------|-------------|
| VCC    | Pin 1 (3.3V) |
| GND    | Pin 6 |
| SCK    | Pin 23 (GPIO 11) |
| MOSI   | Pin 19 (GPIO 10) |
| RST    | Pin 18 (GPIO 24) |
| DC     | Pin 22 (GPIO 25) |
| CS     | Pin 24 (GPIO 8) |
| BL     | Pin 16 (GPIO 26) |

## Estructura

```
monkey/
├── include/
│   ├── HardwareProfile.h
│   ├── Hw.h (GPIO+SPI via ioctl)
│   ├── Game.h
│   ├── Graphics.h (Framebuffer doble + dirty-rect)
│   ├── Renderer.h (ST7789 via SPI ioctl)
│   ├── Player.h / Enemy.h / Level.h / Physics.h
│   ├── Sound.h (bit-bang en hilo)
│   └── Types.h
├── src/
│   ├── main.cpp
│   ├── Hw.cpp / Game.cpp / Graphics.cpp / Renderer.cpp
│   ├── Player.cpp / Enemy.cpp / Level.cpp / Physics.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
