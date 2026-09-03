# LEARNINGS - Reglas y Aprendizajes del Proyecto Arcade (RPi)

Repositorio: `arcade_rpi2w` — Arcade de juegos para Raspberry Pi (Zero 2W)
con display ST7789 240x240.

## Estructura del proyecto: una rama por juego

Cada juego es una **rama** (y una carpeta en `main`). Los juegos son muy
similares entre sí: cambian los **assets**, el **motor** y los **controles**,
pero comparten el mismo patrón de hardware.

| Rama | Juego | Carpeta |
|------|-------|---------|
| `assets` | Sprites/PNG compartidos | `assets/` |
| `cars` | Road Racer (esquiva autos) | `cars/` |
| `dino` | Chrome Dino | `dino/` |
| `mario` | Mario Bros plataformas | `mario/` |
| `monkey` | Donkey Kong | `monkey/` |
| `pacman` | Pac-Man (referencia hardware) | `pacman/` |
| `sound_project` | Efectos de sonido | `sound_project/` |
| `space_invaders` | Space Invaders | `space_invaders/` |
| `tetris` | Tetris | `tetris/` |

Todo el trabajo de un juego se hace en su rama y se pushea con su tag.

---

## Git / Versionado

**Todo push debe llevar su tag.** No se pushea sin tag. El tag identifica la
versión.

### Flujo de versionado

1. Obtener el último tag publicado en la rama (ej: `1.0.9`).
2. El archivo `VERSION` debe coincidir con ese tag.
3. Calcular la siguiente versión con el ciclo 0-9 (ver abajo).

### Reglas

- **Los mensajes de commit siguen conventional commits**: `feat:`, `fix:`,
  `docs:`, `chore:`, `refactor:`, `test:`.
- **No eliminar tags publicados.** Si hay error, crear un tag nuevo.
- **El tag y `VERSION` siempre coinciden** (tag sin `v`).
- **Cada commit significativo debe tener su tag.** No se salta ningún número.
- **El ciclo patch 0-9 es obligatorio:** no se pasa de `1.0.9` a `1.1.1`,
  va a `1.1.0`. Cada minor tiene 10 patches.
- **Primeros 100 tags:**
  `1.0.0..1.0.9` → `1.1.0..1.1.9` → `...` → `1.9.0..1.9.9`.

---

## Compilación y ejecución REMOTA (obligatorio)

La compilación **siempre es remota**, en el Raspberry Pi, no local. Se hace via
SSH con `sshpass` usando la contraseña desde `$SSHPASS`.

### Comando base

```bash
sshpass -e ssh pi@cm5.local \
  "cd /home/pi/src/arcade_rpi2w && git pull && cd <juego> && make clean && make -j4"
```

- Host: `pi@cm5.local`
- Ruta en el Pi: `/home/pi/src/arcade_rpi2w`
- La contraseña se toma de `$SSHPASS` (nunca se muestra).

### Ejecución

```bash
# En el Pi, desde la carpeta del juego
sudo ./bin/<juego>        # o: make run
```

Requerido `sudo` para acceso a `/dev/gpiochip0` y `/dev/spidev0.0`.

### Prevenir conflictos de pines GPIO

Antes de un nuevo `make run`, **matar cualquier instancia previa** del juego
que haya quedado corriendo, si no dará:

```
GPIO_GET_LINEHANDLE_IOCTL (salidas): Device or resource busy
```

```bash
sudo pkill -9 -f Game_App   # o el binario correspondiente
```

---

## Hardware: qué funciona en CM5

El host de desarrollo/ejecución es un **Raspberry Pi Compute Module 5**
(`cm5.local`, BCM2712). El proyecto fue diseñado para Zero 2W, así que hay
que comprobar la compatibilidad:

| Librería/API | ¿Funciona en CM5? | Nota |
|--------------|-------------------|------|
| **ioctl `/dev/gpiochip0` + `/dev/spidev0.0`** | ✅ Sí | Patrón usado por pacman/cars/dino/etc. Es el recomendado. |
| **pigpio** | ❌ No | Falla con "Sorry, this system does not appear to be a raspberry pi" (rev code `c04180`). No soporta BCM2712/CM5. |
| bcm2835 | ⚠️ Solo para PWM | Para `sound_project`. |

### Lección principal

**No usar pigpio.** Migrar a ioctl del kernel (mismo patrón que pacman/cars),
que funciona tanto en CM5 como en Zero 2W y no requiere librerías externas.

### Patrón ioctl de bajo nivel (referencia: `monkey/src/Hw.cpp`)

- **GPIO salidas** (DC, RST, BL): 1 handle `GPIOHANDLE_REQUEST_OUTPUT` agrupando
  los pines, escribir con `GPIOHANDLE_SET_LINE_VALUES_IOCTL`.
- **GPIO entradas** (botones/joystick): 1 handle `GPIOHANDLE_REQUEST_INPUT`,
  leer con `GPIOHANDLE_GET_LINE_VALUES_IOCTL`.
- **SPI** `/dev/spidev0.0`: `SPI_IOC_MODE`, `SPI_IOC_BITS_PER_WORD`,
  `SPI_IOC_MAX_SPEED_HZ` y `SPI_IOC_MESSAGE` con chunks de 4096 bytes.
- **Sonido**: bit-bang en un hilo dedicado (busy-wait con
  `GPIOHANDLE_SET_LINE_VALUES_IOCTL`), nunca en el hilo del game loop.

### Bug común de `HardwareProfile.h`

`#define JOYSTICK_ENABLED` (o cualquier macro que active pines dentro de un
`#ifdef`) debe ir **antes** del `#ifdef` que define los `PIN_JOY_*`. Si va
después, los `PIN_JOY_*` nunca se definen y falla la compilación con
"`PIN_JOY_LEFT` was not declared in this scope".

---

## Credenciales / Push

El push a GitHub usa el remote `https://github.com/siliconvalleyar-oss/arcade_rpi2w.git`
con el credential helper `store` (`~/.git-credentials`). No exponer el token.

```bash
git remote -v                     # verificar remote
git push origin <rama> --tags     # push de rama + tags
```

---

## Ubicaciones importantes

| Archivo | Ubicación | Contenido |
|---------|-----------|-----------|
| Credenciales git | `~/.git-credentials` | Token GitHub en URL |
| Config git global | `~/.gitconfig` | user, email, helper |
| Versión | `./VERSION` | Número del último tag |
| Lecciones | `docs/LEARNINGS.md` | Este documento |
| Skills por juego | `docs/SKILLS.md` | Instrucciones/recomendaciones por rama |
| Skills opencode | `.opencode/skills/*.jsonc` | Comandos/hardware por juego |
