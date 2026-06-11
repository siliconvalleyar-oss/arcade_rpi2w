# Road Racer - Esquiva Autos para RPi Zero 2W + ST7789

Juego arcade donde esquivas autos en una carretera. Pantalla ST7789 240x240, SPI a 40 MHz, GPIO vía /dev/gpiochip0.

## Controles

- **Botones GPIO**: GPIO 27 (Izquierda), GPIO 23 (Derecha)
- **Modo demo**: IA automática, activa por defecto cuando no hay botones

## Compilar y Ejecutar

```bash
make
sudo make run
```

Compilación remota vía SSH:
```bash
ssh joy@raspberry.local "cd /home/pi/src/arcade/cars && make -j4"
```

## Pinout

| Señal   | BCM GPIO |
|---------|----------|
| SPI MOSI| 10       |
| SPI SCLK| 11       |
| DC      | 25       |
| RST     | 24       |
| BL      | 26       |
| Buzzer  | 13       |
| BTN_LEFT| 27       |
| BTN_RIGHT| 23      |

## Estructura

```
cars/
├── include/
│   ├── HardwareProfile.h   - Pines, display, colores, constantes del juego
│   ├── Graphics.h          - Primitivas gráficas + draw_car
│   ├── GameEngine.h        - Motor del juego (player, enemies, score)
│   ├── Sound.h             - Sonido bit-bang por GPIO
│   └── fonts.h             - Fuente 5x7
├── src/
│   ├── main.cpp            - HW init, SPI, ST7789 driver, game loop
│   ├── Graphics.cpp        - fill_rect, texto, draw_car
│   ├── GameEngine.cpp      - Lógica: road, player, enemies, colisiones
│   └── Sound.cpp           - Tonos por GPIO bit-bang
├── Makefile
└── README.md
```
