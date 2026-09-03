# WORKFLOW - Cómo trabajar en cada rama

Flujo de trabajo operativo para `arcade_rpi2w`: una rama por juego, compilación
siempre remota en el Pi, y versionado con tags. Complementa a
`docs/LEARNINGS.md` (reglas/versionado) y `docs/SKILLS.md` (por juego).

---

## 1. Modelo: una rama por juego

Cada juego/variante es una **rama** (todas ramificadas desde `main`) y una
**carpeta** con ese nombre. El commit de cada cambio se hace en la rama del
juego que se está tocando y se pushea con su tag.

| Rama | Carpeta |
|------|---------|
| `assets` | `assets/` |
| `cars` | `cars/` |
| `dino` | `dino/` |
| `mario` | `mario/` |
| `monkey` | `monkey/` |
| `pacman` | `pacman/` |
| `sound_project` | `sound_project/` |
| `space_invaders` | `space_invaders/` |
| `tetris` | `tetris/` |

> Regla general: **no mezclar cambios de distintos juegos en un mismo commit.**
> Cada push lleva su tag (ver `docs/LEARNINGS.md`).

---

## 2. Dos repos hablando entre sí

Hay **dos copias del repo**:

- **Local** (máquina de desarrollo, donde se edita el código).
- **Remoto en el Pi** (`/home/pi/src/arcade_rpi2w`), donde se **compila y
  ejecuta**.

El Pi tiene su **propia rama activa** y frecuentemente cambios locales sin
commitear (por compilar/editar in situ). Para trabajar sobre un juego en el Pi
hay que dejar el Pi en la rama correcta **antes** de compilar.

---

## 3. Flujo estándar (local → Pi)

1. **Local:** `git checkout <rama>` y editar solo los archivos de esa carpeta.
2. **Local:** `git add <archivos>` + `git commit -m "feat/fix/docs: ..."`.
3. **Pushear la rama** (el Pi la traerá después):
   ```bash
   git push origin <rama>
   ```
   (El Pi NO se actualiza solo con commits locales/pushes; hay que
   sincronizarlo explícitamente, ver paso 5.)
4. **Compilar remoto** en el Pi (ver sección 4).
5. **Cambiar la rama del Pi** a la del juego y traer los cambios (ver sección 5).
6. Repetir hasta dejar el juego funcional; luego tag + push de rama+tags.

---

## 4. Compilar remoto (siempre en el Pi, nunca local)

```bash
# Opción A: si el Pi ya está en la rama del juego y el árbol está sincronizado
sshpass -e ssh pi@cm5.local \
  "cd /home/pi/src/arcade_rpi2w && cd <juego> && make clean && make -j4"
```

- Host: `pi@cm5.local` · Ruta: `/home/pi/src/arcade_rpi2w`
- Password vía `$SSHPASS` (nunca se muestra).
- Ejecutar: `sudo ./bin/<juego>` (o `make run`). Antes de reejecutar,
  matar instancias previas para evitar `Device or resource busy`:
  ```bash
  sudo pkill -9 -f <bin>
  ```

---

## 5. Cambiar la rama del Pi (importante)

El Pi vive en su propia copia con su propia rama activa. Para trabajar un
juego ahí hay que sincronizarlo. **Antes de cambiar de rama, preserva el
trabajo sin commitear del Pi** (normalmente hay muchos archivos modificados
de sesiones anteriores de compilación/edición).

### 5a. Preservar el trabajo sin commitear del Pi

Git prohíbe (o mociona) cambiar de rama con el árbol sucio. Guarda todo de
forma segura y recuperable:

```bash
# En el Pi
git stash push -u -m "wip antes de cambiar a <rama>"
git stash list        # → stash@{0}, recuperable cuando haga falta
```

El stash permanece aunque cambies de rama; se puede recuperar con
`git stash pop`/`git stash apply stash@{0}`.

### 5b. Traer la rama y cambiarse

```bash
# En el Pi
git fetch origin
git checkout -b <rama> origin/<rama>   # si la rama local no existe aún
#   ó, si ya existe la rama local:  git checkout <rama> && git pull
git branch --show-current   # confirmar
```

> **Ojo con la ambigüedad:** si existe una **carpeta** con el mismo nombre que
> la rama (ej. `sound_project/`), `git checkout sound_project` falla con
> `fatal: 'sound_project' could be both a local file and a tracking branch`.
> La solución es crear la rama explícitamente desde la remota:
> `git checkout -b sound_project origin/sound_project`.

### 5c. Verificar

```bash
git status -s            # árbol limpio (lo esperado tras checkout)
git log --oneline -1     # el último commit de la rama
make clean && make -j4   # confirmar que compila desde el estado limpio
```

---

## 6. Resumen rápido de comandos en el Pi

```bash
cd /home/pi/src/arcade_rpi2w
git branch --show-current                 # ¿en qué rama estoy?
git status -s                             # ¿hay trabajo sin commitear?
git stash push -u -m "msg"                # guardar trabajo sucio
git fetch origin && git checkout -b <r> origin/<r>   # cambiar de rama
cd <juego> && make clean && make -j4      # compilar
sudo pkill -9 -f <bin>; sudo ./bin/<bin>  # ejecutar (matar antes)
```

---

## 7. Recordatorios de CM5 (learned en la práctica)

- **No usar `pigpio`** ni `libbcm2835` (mmap `/dev/mem`): ambos **tumban /
  reinician el CM5** (BCM2712). Solo ioctl `/dev/gpiochip0` + `/dev/spidev0.0`.
- **Tampoco** hacer bit-bang PCM a **alta frecuencia (22050 Hz+)**: la
  avalancha de syscalls `ioctl`+`nanosleep` por muestra provoca crash/panic
  del kernel (la conexión ssh se corta con "closed by remote host" y el Pi
  reinicia). Para sonido, generar **tonos simples** (ondas cuadradas a baja
  frecuencia), patrón de `pacman/src/Sound.cpp`.
- Instalar dependencias de sistema con sudo; tras configurar `sudoers`
  (NOPASSWD) el `sudo` remoto no vuelve a pedir password.
