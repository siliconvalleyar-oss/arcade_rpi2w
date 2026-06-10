================================================================
  DONKEY KONG - Raspberry Pi Zero 2W + ST7789 240x240
  C++20 | pigpio | picoPNG | RGB565 | Demo + Joystick
================================================================

CONEXION DEL DISPLAY ST7789 (240x240)
--------------------------------------
ST7789 Pin    ->  Raspberry Pi (BCM)
─────────────────────────────────────
VCC           ->  Pin 1  (3.3V)    ← NUNCA 5V
GND           ->  Pin 6  (GND)
SCL / SCK     ->  Pin 23 (GPIO11 SPI0_CLK)
SDA / MOSI    ->  Pin 19 (GPIO10 SPI0_MOSI)
RES / RST     ->  Pin 18 (GPIO24)
DC            ->  Pin 22 (GPIO25)
CS            ->  Pin 24 (GPIO8  SPI0_CE0)
BLK / BL      ->  Pin 16 (GPIO26)

BUZZER PASIVO
─────────────
+  -> Pin 32 (GPIO12 / PWM0)
-  -> Pin 6  (GND)

JOYSTICK (OPCIONAL - modo demo si no está conectado)
────────────────────────────────────────────────────
UP     -> GPIO17 (Pin 11)
DOWN   -> GPIO27 (Pin 13)
LEFT   -> GPIO22 (Pin 15)
RIGHT  -> GPIO23 (Pin 16)
BTN_A  -> GPIO5  (Pin 29) [JUMP]
GND    -> GND

Para habilitar el joystick, editar include/HardwareProfile.h
y descomentar: #define JOYSTICK_ENABLED


INSTALACIÓN
-----------
1. Habilitar SPI:
   sudo raspi-config -> Interface Options -> SPI -> Enable -> Reboot

2. Instalar dependencias:
   make install-deps
   # o manualmente:
   sudo apt-get install libpigpio-dev g++ build-essential

3. Compilar:
   make

4. Ejecutar:
   sudo ./bin/Game_App


ESTRUCTURA DE ARCHIVOS
----------------------
donkeykong/
├── assets/            <- PNGs del juego (ver abajo)
├── bin/
│   └── Game_App       <- binario compilado
├── include/
│   ├── Enemy.h        <- Barrel, Flame, DonkeyKong
│   ├── Entity.h       <- Clase base con pool, animación
│   ├── fonts.h        <- Font 5x7 integrada (sin archivos externos)
│   ├── Game.h         <- Game engine principal
│   ├── Graphics.h     <- Framebuffer doble + dirty-rect
│   ├── HardwareProfile.h <- Pines GPIO, SPI, constantes
│   ├── Level.h        <- Definición de niveles y atlas
│   ├── Physics.h      <- Motor de física AABB
│   ├── Player.h       <- Jumpman (Mario)
│   ├── Renderer.h     <- Driver ST7789 parcial via pigpio
│   ├── Sound.h        <- Buzzer PWM
│   └── Types.h        <- Color16, Vec2, Rect, enums
├── obj/               <- objetos de compilación
├── src/
│   ├── Enemy.cpp
│   ├── Entity.cpp
│   ├── Game.cpp
│   ├── Graphics.cpp
│   ├── Level.cpp
│   ├── main.cpp
│   ├── Physics.cpp
│   ├── Player.cpp
│   ├── Renderer.cpp
│   └── Sound.cpp
├── Makefile
└── README.txt  <- este archivo


ASSETS PNG NECESARIOS (en carpeta assets/)
------------------------------------------
El juego usa picoPNG para cargar sprites.
Si los archivos no existen, se generan placeholders coloreados
y el juego funciona igual (modo demo con formas geométricas).

Para sprites reales, crear PNGs RGBA de estos tamaños sugeridos:

  Jugador (Jumpman/Mario):
    player_idle.png    12x16  - parado
    player_walk_a.png  12x16  - paso A
    player_walk_b.png  12x16  - paso B
    player_jump.png    12x16  - saltando
    player_climb.png   12x16  - en escalera
    player_hammer.png  14x16  - con martillo
    player_dead.png    14x10  - muerto

  Donkey Kong:
    dk_idle.png        32x32  - reposo
    dk_throw.png       32x32  - lanzando barril
    dk_roar.png        32x32  - rugiendo
    dk_stun.png        32x32  - aturdido

  Barriles:
    barrel_roll1.png   12x12  - frame 1
    barrel_roll2.png   12x12  - frame 2
    barrel_explode.png 16x16  - explosión

  Llamas:
    flame1.png         10x12  - frame 1
    flame2.png         10x12  - frame 2

  Tiles:
    tile_floor.png      8x4   - suelo/plataforma (se tilea en X)
    tile_ladder.png     8x8   - escalera (se tilea en Y)
    tile_ladder_top.png 8x4   - tope de escalera

  Bonus:
    bonus_hat.png       8x8
    bonus_umbrella.png  8x8
    bonus_handbag.png   8x8
    bonus_hammer.png    8x8

  Personaje rescatable:
    pauline.png        10x16

NOTA SOBRE TRANSPARENCIA:
  La pantalla ST7789 NO soporta canales alfa.
  El sistema usa color 0xFFFF como "color transparente" sentinel.
  Cualquier píxel con alpha < 128 en el PNG se tratará como
  transparente (no se dibuja, deja ver el fondo).
  El color 0xFFFF (blanco puro 5-6-5) NO debe usarse en sprites.


OPTIMIZACIÓN DE REFRESCO (DIRTY RECT)
--------------------------------------
El sistema NO redibuja toda la pantalla cada frame.
Solo las filas que cambiaron se envían por SPI al ST7789.

Funcionamiento:
  1. Framebuffer tiene dos buffers: back (dibujo) y front (pantalla)
  2. Cada setPixel() marca la fila como "sucia"
  3. getDirtyRanges() devuelve rangos contiguos de filas sucias
  4. Renderer envía RASET+RAMWR solo para esos rangos
  5. commit() copia back->front y limpia los flags

En una escena de plataformas típica, solo ~30-40% de filas
cambian por frame, reduciendo la transferencia SPI a 1/3.
Esto elimina el parpadeo visible en pantallas SPI lentas.

NOTA: No se llama fb_.clear() en cada frame.
Las entidades deben "borrar" su posición anterior antes de
moverse. Game.cpp gestiona esto con el fondo de nivel.


NIVELES
-------
3 niveles incluidos con geometría diferente:
  Nivel 1: Plataformas inclinadas clásicas, 5 pisos
  Nivel 2: Plataformas separadas con hueco central, más difícil
  Nivel 3: Plataformas simétricas, mayor separación vertical

Después del nivel 3 vuelve al nivel 1 con mayor dificultad.


DIFICULTAD
----------
Configurable en Game.h: difficulty_ = Difficulty::EASY/NORMAL/HARD

EASY:   Gravity x0.75, barriles lentos, DK lanza cada 3s
NORMAL: Gravity x1.0,  barriles normales, DK lanza cada 2s
HARD:   Gravity x1.2,  barriles rápidos, DK lanza cada 1.2s,
        hasta 3 llamas adicionales, multiplicador de score x1.5


MODO DEMO vs MODO JUGADOR
--------------------------
Por defecto corre en MODO DEMO: el personaje es controlado
por una IA simple que intenta llegar a Pauline evitando barriles.

Para jugar manualmente:
  1. Conectar joystick a los pines indicados
  2. Descomentar #define JOYSTICK_ENABLED en HardwareProfile.h
  3. make clean && make


HERRAMIENTAS ÚTILES
-------------------
# Ver si SPI está habilitado:
ls /dev/spidev*

# Probar pigpio:
sudo pigpiod
pigs modes 0

# Monitorear CPU (el juego debería usar < 30% en Pi Zero 2W):
top

# Captura de pantalla del framebuffer (debug):
# No aplicable directamente, usar lógica de debug en Graphics.cpp
================================================================
