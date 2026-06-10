# Road Racer - Esquiva Autos para RPi Zero 2W

Juego arcade donde esquivas autos en una carretera. Basado en la estructura de Pac-Man.

## Controles

- **Botones GPIO**: Izquierda/Derecha para mover el auto
- **Modo demo**: IA automatica (por defecto)
- **Teclado**: Conectar por SSH (modo juego con terminal)

## Compilar

```bash
make
sudo make run
```

## Estructura

```
cars/
├── include/
│   ├── HardwareProfile.h
│   ├── Graphics.h
│   ├── GameEngine.h
│   ├── Sound.h
│   └── fonts.h
├── src/
│   ├── main.cpp
│   ├── Graphics.cpp
│   ├── GameEngine.cpp
│   └── Sound.cpp
├── Makefile
└── README.md
```
