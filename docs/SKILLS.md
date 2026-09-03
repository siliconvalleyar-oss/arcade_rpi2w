# SKILLS - Guía y recomendaciones por juego / rama

Cada rama del repo `arcade_rpi2w` corresponde a un juego. Todos comparten el
mismo hardware (ST7789 240x240 + GPIO) pero difieren en assets y motor.

**Hardware común (ST7789):**
- SPI0 `/dev/spidev0.0` — DC=25, RST=24, BL=26, SOUND=13
- GPIO ioctl `/dev/gpiochip0` (patrón pacman/cars) — **no usar pigpio** en CM5
- Compilar SIEMPRE remoto: `sshpass -e ssh pi@cm5.local "cd /home/pi/src/arcade_rpi2w && cd <juego> && make clean && make -j4"`
- Ejecutar: `sudo ./bin/<juego>` (o `make run`). `sudo pkill -f <bin>` antes de reejecutar.

---

## assets — carpeta de recursos

- **Qué es**: PNGs/sprites compartidos (mario, space invaders, sino, etc.)
  y scripts (`script_tools/divide_png.sh` para cortar spritesheets).
- **Recomendación**: no contiene motor; se usa desde los demás juegos.
  Mantener `divide_png.sh` para generar sprites `sprite_%02d.png`.

---

## cars — Road Racer (esquiva autos)

- **Motor**: loop 30 fps, carretera con scroll (lane dashes + edge lines),
  sprites PNG con alpha (picoPNG), colisión pixel-perfect.
- **Controles**: GPIO 27 (izq) / GPIO 23 (der), demo AI por defecto.
- **Claves**:
  - `include/HardwareProfile.h` define pines, SPI 40 MHz, modos.
  - Colisión: primero AABB, luego píxeles sólidos (`Sprite::isSolid`).
  - `src/GameEngine.cpp` = lógica road/player/enemies/score.
- **Verificar**: `bin/cars` (ya compila remoto en CM5).

---

## dino — Chrome Dino

- **Motor**: runner con gravedad; sprites PNG con alpha blending (FrameBuffer),
  cactus, pterodáctilos.
- **Controles**: BTN_JUMP (GPIO 5/17), BTN_COLOR (GPIO 6/22).
- **Claves**:
  - `DinoHardware.h`, `DinoGraphics.h`, `DinoEngine.h`, `DinoSound.h`.
  - Usa FrameBuffer con dirty-rect + alpha blending (igual que space_invaders).

---

## mario — Mario Bros (plataformas)

- **Motor**: plataformas con física, enemigos, bloques, tilemap, PNGs.
- **Controles**: teclado SSH (WASD/Q) + gpio buttons + modo DEMO/JUEGO.
- **Claves**:
  - `TileSet.h`, `Physics.h`, `Player.h`, `Enemy.h`, `Level.h`.
  - Motor más completo (el más reciente en `main`).
- **Recomendación**: verificar `assets/` de mario (PNGs) presentes.

---

## monkey — Donkey Kong

- **Motor**: sprites PNG, física AABB, 3 niveles, modo demo/joystick. Motor
  orientado a objetos (clases `Renderer`, `Sound`, `Game`, `Player`, `Enemy`).
- **Controles**: joystick GPIO (UP/DOWN/LEFT/RIGHT + BTN_A) o demo AI.
- **Claves** (migrado a ioctl, NO pigpio):
  - `Hw.h`/`Hw.cpp` = capa ioctl GPIO + SPI (patrón pacman).
  - `Renderer` (ST7789 via SPI ioctl), `Sound` (bit-bang en hilo propio).
  - `HardwareProfile.h`: `JOYSTICK_ENABLED` debe ir antes de los `#ifdef PIN_JOY_*`.
  - `bin/Game_App`. Compila y corre en CM5.

---

## pacman — Pac-Man (REFERENCIA de hardware)

- **Motor**: procedimental (namespace `GameEngine`, variables globales),
  laberinto 9x9, 4 fantasmas, power pellets, niveles.
- **Controles**: botones GPIO (UP/DOWN/LEFT/RIGHT) o demo AI.
- **Claves**: este juego es el **patrón de hardware correcto**:
  - `main.cpp` monolítico: ioctl gpiochip (DC/RST/BL en 1 handle + botones en
    otro) + SPI `/dev/spidev0.0` (modo 3, 40 MHz) + sonido bit-bang.
  - `Graphics` dibuja celdas con anti-flicker (borrar bbox previo).
- **Usar como plantilla** para portar cualquier otro juego a ioctl.

---

## sound_project — Sistema de sonido arcade

- **Qué es**: generador de efectos (arcade, teléfono, modem 56k).
- **Dependencia**: usa `libbcm2835` (PWM por hardware).
- **Claves**:
  - `PiezoDriver.hpp`, `SoundGenerator.hpp`, `WaveformGenerator.hpp`.
  - Instalar vía `build.sh` (bcm2835). No es un juego de pantalla; no usa
    gpiog chip ni ST7789.

---

## space_invaders — Space Invaders

- **Motor**: 8x5 invasores, escudos, niveles; FrameBuffer doble con dirty-rect,
  sprites PNG con alpha blending.
- **Controles**: BTN_LEFT (27), BTN_RIGHT (23), BTN_FIRE (6).
- **Claves**:
  - `FrameBuffer.h` (doble buffer dirty-rect), `GameEngine.cpp`, `Graphics.cpp`.
  - GPIO via ioctl gpiochip en `main.cpp`.

---

## tetris — Tetris

- **Motor**: tablero, piezas (rotación), tetrominos; soporte dual teclado SSH
  y botones GPIO.
- **Controles**: teclado (A/D, W/I/Z/X, S/K, Space, P, Q) y GPIO
  (LEFT=22, RIGHT=17, ROT=5, DOWN=27, PAUSE=6).
- **Claves**: `TetrisEngine.h`, `TetrisGfx.h`, `TetrisSound.h`, `Hardware.h`.

---

## Recomendaciones generales para trabajar en cualquier rama

1. `git checkout <juego>` y trabajar solo en los archivos de esa carpeta.
2. Compilar remoto en el Pi (nunca local).
3. Tras cambios: `make clean && make -j4`, luego `sudo ./bin/<juego>`
   (matar instancia previa con `sudo pkill -f <bin>`).
4. Commit con conventional commits + tag siguiendo `docs/LEARNINGS.md`.
5. Pushear rama + tag (`git push origin <rama> --tags`).
