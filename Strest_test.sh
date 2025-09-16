#!/usr/bin/env bash
# Lanza N peticiones concurrentes con curl y reporta fallos.
URL="${1:-http://127.0.0.1:8080/index.html}"
N="${2:-1999}"

tmp_fail_log="$(mktemp)"
trap 'rm -f "$tmp_fail_log"' EXIT

for i in $(seq 1 "$N"); do
  (
    if ! curl -sS -f -o /dev/null --connect-timeout 9 --max-time 10 "$URL"; then
      echo "$i" >> "$tmp_fail_log"
    fi
  ) &
done

wait

if [[ -s "$tmp_fail_log" ]]; then
  count=$(wc -l < "$tmp_fail_log")
  echo "Fallaron $count de $N peticiones."
  echo -n "Índices fallidos: "
  tr '\n' ' ' < "$tmp_fail_log"; echo
  exit 1
else
  echo "Todas las $N peticiones tuvieron éxito."
fi
