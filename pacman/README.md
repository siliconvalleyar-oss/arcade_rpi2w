# Pac-Man - Raspberry Pi Zero 2W + ST7789

Juego de Pac-Man clasico con laberinto 9x9, 4 fantasmas, power pellets y niveles.

## Controles

- **Botones GPIO**: Arriba/Abajo/Izquierda/Derecha
- **Modo demo**: IA automatica si USE_BUTTONS no esta definido

## Conexion

| GMT130 | RPi Zero 2W |
|--------|-------------|
| VCC    | Pin 17 (3.3V) |
| GND    | Pin 20 |
| SCK    | Pin 23 (GPIO 11) |
| SDATA  | Pin 19 (GPIO 10) |
| DC     | Pin 22 (GPIO 25) |
| RST    | Pin 18 (GPIO 24) |
| BL     | Pin 16 (GPIO 23/26) |
| Buzzer | Pin 32 (GPIO 12/13) |

## Compilar y Ejecutar

```bash
make
sudo make run
```

## Estructura

```
pacman/
├── include/
│   ├── HardwareProfile.h  - Configuracion de pines
│   ├── Graphics.h         - Primitivas graficas
│   ├── GameEngine.h       - Logica del juego
│   ├── Sound.h            - Sonido bit-bang
│   └── fonts.h            - Fuente 5x7
├── src/
│   ├── main.cpp           - Hardware SPI+GPIO
│   ├── Graphics.cpp       - Sprites Pac-Man/fantasmas
│   ├── GameEngine.cpp     - Motor del juego
│   └── Sound.cpp          - Tonos por GPIO
├── Makefile
└── README.md
```
