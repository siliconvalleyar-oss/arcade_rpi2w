# RPi Arcade - Juegos Retro para Raspberry Pi Zero 2W

Coleccion de juegos arcade para Raspberry Pi Zero 2W con pantalla ST7789 240x240.

## Hardware

- Raspberry Pi Zero 2W
- Pantalla GMT130 v1.0 (ST7789, 240x240, SPI)
- Buzzer pasivo (opcional, GPIO 12/13)
- Botones (opcional, segun cada juego)

## Proyectos

| Juego | Descripcion | Driver |
|-------|-------------|--------|
| pacman/ | Pac-Man clasico 9x9 con fantasmas | GPIO ioctl |
| dino/ | Chrome Dino Game - esquivar cactus | GPIO ioctl |
| mario/ | Mario Bros plataformas | GPIO ioctl |
| monkey/ | Donkey Kong con sprites PNG | pigpio |
| cars/ | Road Racer - esquivar autos | GPIO ioctl |
| space_invaders/ | Space Invaders clasico | GPIO ioctl |
| tetris/ | Tetris con teclado+GPIO | GPIO ioctl |
| sound_project/ | Generador de sonidos arcade/fax/modem | bcm2835 |

## Configuracion de Pines

Cada proyecto tiene su propio `include/HardwareProfile.h` con dos configuraciones:

- **BONNET_ADAFRUIT_ST7789**: Pines para el bonnet AdaFruit ST7789 (DC=25, RST=24, BL=26)
- **#else**: Pines alternativos (DC=21, RST=16, BL=20)

Seleccionar cambiando `#define BONNET_ADAFRUIT_ST7789` en el archivo.

## Instalacion

```bash
# Instalar dependencias generales
./script_tools/install_deps.sh

# Compilar y ejecutar un juego
cd pacman
make
sudo make run
```

## Dependencias

- **g++**: Compilador
- **libpthread**: Hilos POSIX
- **libpigpio-dev**: Solo para monkey/ (Donkey Kong)
- **libbcm2835**: Solo para sound_project/
