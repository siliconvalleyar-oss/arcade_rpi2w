#!/bin/bash
# ============================================================
#  install_deps.sh — Instalar dependencias para RPi Arcade
#  Raspberry Pi Zero 2W / Raspbian / Raspberry Pi OS
# ============================================================

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}  RPi Arcade — Instalacion de dependencias${NC}"
echo -e "${GREEN}============================================${NC}"

# ---- 1. Actualizar repositorios ----
echo -e "\n${YELLOW}[1/6] Actualizando repositorios...${NC}"
sudo apt-get update

# ---- 2. Compilador y herramientas base ----
echo -e "\n${YELLOW}[2/6] Instalando compilador y herramientas base...${NC}"
sudo apt-get install -y \
    g++ \
    build-essential \
    make \
    git

# ---- 3. SPI - Habilitar en config.txt ----
echo -e "\n${YELLOW}[3/6] Verificando SPI...${NC}"
CONFIG_FILE="/boot/firmware/config.txt"
if [ ! -f "$CONFIG_FILE" ]; then
    CONFIG_FILE="/boot/config.txt"
fi

if grep -q "dtparam=spi=on" "$CONFIG_FILE" 2>/dev/null; then
    echo -e "${GREEN}  SPI ya habilitado en $CONFIG_FILE${NC}"
else
    echo -e "${YELLOW}  Habilitando SPI en $CONFIG_FILE...${NC}"
    echo "" >> "$CONFIG_FILE"
    echo "# Habilitado por install_deps.sh" >> "$CONFIG_FILE"
    echo "dtparam=spi=on" >> "$CONFIG_FILE"
    echo -e "${GREEN}  SPI habilitado. Se requiere reinicio.${NC}"
    SPI_REBOOT_NEEDED=true
fi

# ---- 4. pigpio (necesario para monkey/Donkey Kong) ----
echo -e "\n${YELLOW}[4/6] Instalando pigpio (Donkey Kong)...${NC}"
sudo apt-get install -y libpigpio-dev || echo -e "${YELLOW}  (opcional, solo para monkey/)${NC}"

# ---- 5. bcm2835 (necesario para sound_project) ----
echo -e "\n${YELLOW}[5/6] Instalando bcm2835 (sound_project)...${NC}"
if [ ! -f /usr/local/lib/libbcm2835.a ]; then
    echo -e "${YELLOW}  Descargando bcm2835 library...${NC}"
    cd /tmp
    wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
    tar zxf bcm2835-1.71.tar.gz
    cd bcm2835-1.71
    ./configure
    make
    sudo make check
    sudo make install
    cd /tmp
    rm -rf bcm2835-1.71*
    echo -e "${GREEN}  bcm2835 instalado correctamente${NC}"
else
    echo -e "${GREEN}  bcm2835 ya instalado${NC}"
fi

# ---- 6. Verificar usuario en grupos ----
echo -e "\n${YELLOW}[6/6] Agregando usuario a grupos (SPI, GPIO)...${NC}"
sudo usermod -aG spi,gpio $USER 2>/dev/null || true
echo -e "${GREEN}  Usuario agregado a grupos spi,gpio${NC}"
echo -e "${YELLOW}  NOTA: Cerrar sesion y volver a entrar para que los grupos tengan efecto${NC}"

echo -e "\n${GREEN}============================================${NC}"
echo -e "${GREEN}  Instalacion completada${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""
echo "Resumen:"
echo "  g++        : $(g++ --version | head -1)"
echo "  SPI        : $(ls /dev/spidev0.0 2>/dev/null || echo 'No disponible (requiere reinicio)')"
echo "  pigpio     : $(dpkg -l libpigpio-dev 2>/dev/null | tail -1 | awk '{print $3}' || echo 'no instalado')"
echo "  bcm2835    : $(ls /usr/local/lib/libbcm2835.a 2>/dev/null || echo 'no instalado')"
echo ""

if [ "$SPI_REBOOT_NEEDED" = true ]; then
    echo -e "${YELLOW}⚠  Se requiere REINICIAR para activar SPI.${NC}"
    echo -e "${YELLOW}   Ejecutar: sudo reboot${NC}"
fi

echo ""
echo "Para compilar todos los juegos:"
echo "  cd pacman && make"
echo "  cd cars && make"
echo "  cd dino && make"
echo "  cd mario && make"
echo "  cd monkey && make"
echo "  cd space_invaders && make"
echo "  cd tetris && make"
echo "  cd sound_project && make"
