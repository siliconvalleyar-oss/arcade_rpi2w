# Space Invaders - RPi Zero 2W + ST7789

Juego clasico de Space Invaders con 8x5 invasores, escudos y niveles.

## Controles

- **BTN_LEFT (GPIO 27)**: Mover izquierda
- **BTN_RIGHT (GPIO 23)**: Mover derecha
- **BTN_FIRE (GPIO 6)**: Disparar

## Compilar

```bash
make
sudo make run
```

## Estructura

```
space_invaders/
├── include/
│   ├── HardwareProfile.h
│   ├── Graphics.h
│   ├── GameEngine.h
│   ├── FrameBuffer.h (doble buffer con dirty-rect)
│   ├── Sound.h
│   └── fonts.h
├── src/
│   ├── main.cpp (GPIO via ioctl gpiochip)
│   ├── Graphics.cpp
│   ├── GameEngine.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
