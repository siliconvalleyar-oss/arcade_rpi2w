# Dino Arcade - Chrome Dino Game para RPi Zero 2W

Juego estilo arcade del dinosaurio de Chrome (cactus, pterodactilos, gravedad).

## Controles

- **BTN_JUMP (GPIO 5/17)**: Saltar / Iniciar juego
- **BTN_COLOR (GPIO 6/22)**: Alternar color/blanco-negro

## Compilar

```bash
make
sudo make run
```

## Conexion

Misma que Pac-Man: ST7789 por SPI0, GPIO ioctl.

## Estructura

```
dino/
├── include/
│   ├── DinoHardware.h
│   ├── DinoGraphics.h
│   ├── DinoEngine.h
│   ├── DinoSound.h
│   └── fonts.h
├── src/
│   ├── main_dino.cpp
│   ├── DinoGraphics.cpp
│   ├── DinoEngine.cpp
│   └── DinoSound.cpp
├── Makefile
└── README.md
```
