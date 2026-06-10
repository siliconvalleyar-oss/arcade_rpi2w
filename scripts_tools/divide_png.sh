#!/bin/bash

INPUT="spritesheet.png"
OUTPUT_PREFIX="sprite"

# tamaño de cada sprite
WIDTH=32
HEIGHT=32

# dividir la imagen
convert "$INPUT" -crop ${WIDTH}x${HEIGHT} +repage +adjoin ${OUTPUT_PREFIX}_%02d.png

echo "Sprites generados correctamente."
