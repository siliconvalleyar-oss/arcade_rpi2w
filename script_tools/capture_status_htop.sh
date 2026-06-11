#!/bin/bash

# Obtener timestamp: AñoMesDia_HoraMinuto (ej: 20250610_0043)
timestamp=$(date +%Y%m%d_%H%M)

# Nombre del archivo
filename="top_${timestamp}.log"

# Ejecutar top en modo batch una vez y guardar
top -b -n 1 > "$filename"

# Mensaje de confirmación
echo "Log guardado en: $filename"
