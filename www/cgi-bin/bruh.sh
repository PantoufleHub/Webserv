#!/bin/bash

echo "Content-Type: text/plain"
echo ""

sleep 5

BYTES_LEFT="${CONTENT_LENGTH:-0}"
BODY=""

while [ "$BYTES_LEFT" -gt 0 ]; do
  CHUNK=$(dd bs=1 count="$BYTES_LEFT" 2>/dev/null)
  BODY+="$CHUNK"
  BYTES_READ=${#CHUNK}
  BYTES_LEFT=$((BYTES_LEFT - BYTES_READ))
done

echo "Received body:"
echo "$BODY"