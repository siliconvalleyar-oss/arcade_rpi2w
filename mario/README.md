# Mario Bros Arcade - RPi Zero 2W + ST7789

Juego de plataformas estilo Mario Bros con fisica, enemigos, bloques y sonido.

## Controles (Teclado SSH)

- **A/D**: Mover izquierda/derecha
- **W**: Saltar
- **Q**: Salir

## Compilar

```bash
make
sudo make run
```

## Estructura

```
mario/
├── assets/           - PNGs del juego
├── include/
│   ├── HardwareProfile.h
│   ├── Game.h
│   ├── Graphics.h
│   ├── Player.h
│   ├── Enemy.h
│   ├── Level.h
│   ├── Physics.h
│   ├── Renderer.h
│   ├── Sound.h
│   ├── TileSet.h
│   └── ...
├── src/
│   ├── main.cpp
│   ├── Game.cpp
│   ├── Graphics.cpp
│   ├── Player.cpp
│   ├── Enemy.cpp
│   ├── Level.cpp
│   ├── Physics.cpp
│   ├── Renderer.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
