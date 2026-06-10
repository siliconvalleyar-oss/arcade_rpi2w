# Donkey Kong - RPi Zero 2W + ST7789

Juego de Donkey Kong con sprites PNG, fisica AABB, 3 niveles y modo demo.

## Driver

Este proyecto usa **pigpio** (no GPIO ioctl directo). Requiere instalacion:

```bash
sudo apt-get install libpigpio-dev
```

## Controles

- **Joystick opcional**: Descomentar `#define JOYSTICK_ENABLED` en HardwareProfile.h
- **Modo demo**: IA que juega automaticamente

## Compilar

```bash
make install-deps   # Instalar pigpio
make
sudo make run
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
│   ├── Game.h
│   ├── Graphics.h (Framebuffer doble + dirty-rect)
│   ├── Renderer.h (ST7789 via pigpio)
│   ├── Player.h / Enemy.h / Level.h / Physics.h
│   ├── Sound.h
│   └── Types.h
├── src/
│   ├── main.cpp
│   ├── Game.cpp / Graphics.cpp / Renderer.cpp
│   ├── Player.cpp / Enemy.cpp / Level.cpp / Physics.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
