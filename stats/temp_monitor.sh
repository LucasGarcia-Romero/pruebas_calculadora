#!/bin/bash
set -euo pipefail

DATA_DIR="${DATA_DIR:-/data}"
SLEEP_INTERVAL="${SLEEP_INTERVAL:-60}"
MAX_LINES="${MAX_LINES:-43200}"           # da para 1 mes a 1 grabacion por minuto (43.200 lineas * 37 bytes ≈ 1.6 MB)

TEMP_FILE="${DATA_DIR}/cpu_temp.txt"

mkdir -p "$DATA_DIR"
touch "$TEMP_FILE"

while true; do
  if [ -f /sys/class/thermal/thermal_zone0/temp ]; then
    TEMP_RAW="$(cat /sys/class/thermal/thermal_zone0/temp)"
    TEMP_C="$(awk '{printf "%.1f", $1/1000}' <<< "$TEMP_RAW")"
    TIMESTAMP="$(date '+%Y-%m-%d %T')"

    echo "${TIMESTAMP} BOARD_TEMP=${TEMP_C}C" >> "$TEMP_FILE"

    # Rotación para no crecer indefinidamente
    LINE_COUNT="$(wc -l < "$TEMP_FILE")"
    if [ "$LINE_COUNT" -gt "$((MAX_LINES + 100))" ]; then
      TMP="$(mktemp)"
      tail -n "$MAX_LINES" "$TEMP_FILE" > "$TMP"
      mv "$TMP" "$TEMP_FILE"
    fi
  fi

  sleep "$SLEEP_INTERVAL"
done


# El tiene esta forma

# 2025-07-10 14:32:05 BOARD_TEMP=42.3C
# 2025-07-10 14:32:15 BOARD_TEMP=42.5C
# ...