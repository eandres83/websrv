#!/usr/bin/env bash
# Lanza N peticiones en bloques concurrentes con curl y reporta fallos.
# Uso: ./Strest_test.sh [URL] [N] [BATCH] [SLEEP_MS]
#  - URL:     destino (por defecto http://127.0.0.1:8080/index.html)
#  - N:       total de peticiones (por defecto 1000)
#  - BATCH:   tamaño del bloque concurrente (por defecto 100)
#  - SLEEP_MS:espera entre bloques en milisegundos (por defecto 10ms)

set -euo pipefail

URL="${1:-http://127.0.0.1:8080/index.html}"
N="${2:-9000}"
BATCH="${3:-100}"
SLEEP_MS="${4:-10}"

tmp_fail_log="$(mktemp)"
trap 'rm -f "$tmp_fail_log"' EXIT

sent=0
idx=0
while [ "$sent" -lt "$N" ]; do
  to_send=$(( N - sent ))
  if [ "$to_send" -gt "$BATCH" ]; then
    to_send="$BATCH"
  fi

  for _ in $(seq 1 "$to_send"); do
    idx=$(( idx + 1 ))
    (
      if ! curl -sS -f -o /dev/null --connect-timeout 2 --max-time 10 "$URL"; then
        echo "$idx" >> "$tmp_fail_log"
      fi
    ) &
  done

  # Esperar a que termine el bloque
  wait

  sent=$(( sent + to_send ))

  # Dormir entre bloques si aún quedan más
  if [ "$sent" -lt "$N" ]; then
    # convertir ms a segundos con decimales (requiere GNU/BusyBox sleep; en macOS funciona)
    sleep "$(printf '0.%03d' "$SLEEP_MS")"
  fi
done

if [[ -s "$tmp_fail_log" ]]; then
  count=$(wc -l < "$tmp_fail_log")
  echo "Fallaron $count de $N peticiones."
  echo -n "Índices fallidos: "
  tr '\n' ' ' < "$tmp_fail_log"; echo
  exit 1
else
  echo "Todas las $N peticiones tuvieron éxito."
fi
