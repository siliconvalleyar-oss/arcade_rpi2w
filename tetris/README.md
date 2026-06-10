# Tetris Arcade - RPi Zero 2W + ST7789

Tetris clasico con soporte dual: teclado SSH y botones GPIO.

## Controles

### Teclado (SSH)
- **A/D o J/L**: Mover izquierda/derecha
- **W/I/Z/X**: Rotar pieza
- **S/K**: Bajar rapido (soft drop)
- **Espacio**: Caida instantanea (hard drop)
- **P**: Pausa
- **Q**: Salir
- **Flechas**: ↑ rotar, ←→ mover, ↓ bajar

### GPIO
- **BTN_LEFT (GPIO 22)**: Izquierda
- **BTN_RIGHT (GPIO 17)**: Derecha
- **BTN_ROT (GPIO 5)**: Rotar
- **BTN_DOWN (GPIO 27)**: Bajar
- **BTN_PAUSE (GPIO 6)**: Pausa

## Compilar

```bash
make
sudo make run
```

## Estructura

```
tetris/
├── include/
│   ├── Hardware.h
│   ├── TetrisEngine.h
│   ├── TetrisGfx.h
│   ├── TetrisSound.h
│   └── fonts.h
├── src/
│   ├── main.cpp (GPIO ioctl + teclado raw)
│   ├── TetrisEngine.cpp
│   ├── TetrisGfx.cpp
│   └── TetrisSound.cpp
├── Makefile
└── README.md
```
