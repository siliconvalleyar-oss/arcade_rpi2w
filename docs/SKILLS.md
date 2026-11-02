# SKILLS - Guía y recomendaciones por juego / rama

Cada rama del repo `arcade_rpi2w` corresponde a un juego. Todos comparten el
mismo hardware (ST7789 240x240 + GPIO) pero difieren en assets y motor.

**Hardware común (ST7789):**
- SPI0 `/dev/spidev0.0` — DC=25, RST=24, BL=26, SOUND=13
- GPIO ioctl `/dev/gpiochip0` (patrón pacman/cars) — **no usar pigpio** ni
  `libbcm2835` en CM5
- Compilar SIEMPRE remoto: `sshpass -e ssh pi@cm5.local "cd /home/pi/src/arcade_rpi2w && cd <juego> && make clean && make -j4"`
- Ejecutar: `sudo ./bin/<juego>` (o `make run`). `sudo pkill -f <bin>` antes de reejecutar.
- Flujo por rama y sincronización local↔Pi: ver **`docs/WORKFLOW.md`**.

---

## assets — carpeta de recursos

- **Qué es**: PNGs/sprites compartidos (mario, space invaders, dino, etc.)
  y scripts (`script_tools/divide_png.sh` para cortar spritesheets).
- **Recomendación**: no contiene motor; se usa desde los demás juegos.

---

## cars — Road Racer (esquiva autos)

- **Motor**: loop 30 fps, carretera con scroll, sprites PNG con alpha
  (picoPNG), colisión pixel-perfect (AABB + píxeles sólidos `isSolid`).
- **Controles**: GPIO 27 (izq) / GPIO 23 (der), demo AI por defecto.
- **Claves**: `HardwareProfile.h` define pines, SPI 40 MHz; `GameEngine.cpp`
  = road/player/enemies/score. Compila remoto en CM5.

---

## dino — Chrome Dino (T-Rex Runner)

- **Motor**: runner infinito con gravedad; sprites PNG (2 dino + 2 cactus) con
  alpha blending contra el framebuffer; doble buffer dirty-rect (FrameBuffer);
  scroll de suelo con "pebbles"; 60 fps (16666µs).
- **Estados**: `title` ("DINO RUNNER", espera FIRE) → `gameplay` → `game_over`
  (espera FIRE para reiniciar, conserva hi-score).
- **Claves de código**:
  - `include/FrameBuffer.h` — singleton, doble buffer (buffer/prev), hasta 32
    dirty-rects, full-flush al sobrescribir.
  - `include/Sprite.h` / `src/Sprite.cpp` — PNG RGBA (picoPNG), `solid[alpha>0]`.
  - `src/Graphics.cpp` — `drawSprite` con blend: a==255 directo RGB565,
    a<255 mezcla contra el pixel previo del framebuffer.
  - `src/GameEngine.cpp` — física (gravedad=1, JUMP_VEL=-13), animación 2
    frames cada 8 ticks, cactus spawn/move, colisión AABB con margen de 4px.
  - `include/HardwareProfile.h` — pines y constantes de juego.
- **Controles reales usados**: SOLO `BTN_FIRE()` (GPIO 6) para saltar y
  empezar/reiniciar. `BTN_LEFT`/`BTN_RIGHT` y `BTN_START` están definidos pero
  **no se usan** en la lógica.
- **⚠️ Falla conocida (botones)**: el dino no responde al botón de salto
  configurado en GPIO 6. El joystick que SÍ funciona en el CM5 es el de
  pacman (UP=17, DOWN=22, LEFT=27, RIGHT=23). El salto debería mapearse a uno
  de esos pines y no a 5/6.

---

## Liberación de recursos en dino (revisión)

Recursos usados: `gpio_fd`, `gpio_out_fd`, `gpio_in_fd` (fds de
`GPIO_GET_LINEHANDLE_IOCTL`) y `spi_fd` — todos en `src/main.cpp`. Sprites y
FrameBuffer son estáticos (se liberan al terminar el proceso).

**Correcto:**
- `cleanup()` cierra los 4 fds y hace `BL_LOW()`. Se llama desde el handler de
  señal (SIGINT/SIGTERM → `cleanup()` + `_exit(0)`) y desde `hw_close()` al
  final de `main()`.

**Fallos / riesgos a NO repetir:**
1. **`game_loop()` es `while(true)` sin retorno**: el juego solo termina por
   señal; `hw_close()` final de `main()` prácticamente no se alcanza en el
   flujo normal.
2. **Fuga de fds si `gpio_init()` falla a medias**: si `gpio_out_fd` se abre
   pero el handle de entrada falla (`return -3`), `hw_init()` devuelve -1 y
   `main()` hace `return 1` **sin llamar a `cleanup()`** → quedan `gpio_fd` y
   `gpio_out_fd` abiertos. Mitigar: en el error de `hw_init()`, liberar lo ya
   abierto antes de retornar.
3. **Configuración muerta**: `BTN_START_PIN`, `BTN_LEFT`, `BTN_RIGHT` definidos
   pero sin uso real — no fuga, pero confunde y no aporta.
4. **No tocar pines del bus SPI como GPIO mientras el display está activo**:
   configurar **GPIO11 (SCLK)** como entrada externamente desestabilizó el
   sistema (sobrecarga/sobrecalentamiento del disipador y caída del SSH).
   Nunca reconfigurar pines en uso por el driver de pantalla.
5. **Sonido/PWM a alta frecuencia y libbcm2835/pigpio tumban el CM5**: solo
   usar ioctl gpiochip y tonos simples (patrón pacman). Ver WORKFLOW.

---

## monkey — Donkey Kong

- **Motor**: sprites PNG, física AABB, 3 niveles, demo/joystick. Orientado a
  objetos (`Renderer`, `Sound`, `Game`, `Player`, `Enemy`).
- **Claves** (migrado a ioctl, NO pigpio): `Hw.h`/`Hw.cpp` = capa ioctl
  GPIO+SPI; `HardwareProfile.h`: `JOYSTICK_ENABLED` antes de los `PIN_JOY_*`.

---

## pacman — Pac-Man (REFERENCIA de hardware)

- **Motor**: procedimental, laberinto 9x9, 4 fantasmas, power pellets.
- **Controles**: joystick GPIO (UP=17, DOWN=22, LEFT=27, RIGHT=23).
- **Claves**: es el **patrón de hardware correcto** — ioctl gpiochip
  (DC/RST/BL en 1 handle + botones en otro) + SPI `/dev/spidev0.0` (modo 3,
  40 MHz) + sonido bit-bang en tonos simples.
- **Usar como plantilla** para portar hardware/controles de otros juegos.

---

## sound_project — Sistema de sonido arcade

- Migrado de `libbcm2835` a **ioctl `/dev/gpiochip0`**. Compila en CM5.
- **⚠️ Limitación CM5**: el bit-bang PCM a 22050Hz (syscall por muestra)
  **tumba el kernel**. Usar solo tonos simples (`generateTone`), patrón pacman.

---

## space_invaders — Space Invaders

- **Motor**: 8x5 invasores, escudos; FrameBuffer doble dirty-rect, sprites PNG
  alpha. Controles: LEFT=27, RIGHT=23, FIRE=6. GPIO ioctl en `main.cpp`.

---

## tetris — Tetris

- **Motor**: tablero, piezas/rotación, tetrominos; teclado SSH + botones GPIO
  (LEFT=22, RIGHT=17, ROT=5, DOWN=27, PAUSE=6).

---

## Recomendaciones generales para trabajar en cualquier rama

1. `git checkout <juego>` y trabajar solo en los archivos de esa carpeta.
2. Compilar remoto en el Pi (nunca local). Cambiar la rama del Pi a la del
   juego (ver `docs/WORKFLOW.md`) y preservar trabajo sucio con `git stash`.
3. Tras cambios: `make clean && make -j4`, luego `sudo ./bin/<juego>`
   (matar instancia previa con `sudo pkill -f <bin>`).
4. Commit con conventional commits + tag siguiendo `docs/LEARNINGS.md`.
5. Pushear rama + tag (`git push origin <rama> --tags`).
6. **No reconfigurar pines SPI/GPIO en uso** por el display; verificar
   liberación de fds en todos los caminos de error; evitar bit-bang/sonido de
   alta frecuencia (tumban el CM5).
