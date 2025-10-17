#!/bin/bash

echo "=========================================="
echo "Testing Client Timeout (408)"
echo "=========================================="
echo ""
echo "Sending incomplete HTTP request..."
echo "Server should timeout after ~5 seconds"
echo ""

# Capture start time
START=$(date +%s)

# Send incomplete request using a background process
# This avoids needing the 'timeout' command
{
    echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n"
    sleep 10
} | nc localhost 8080 > /tmp/timeout_response.txt 2>&1 &

NC_PID=$!

# Wait for response or timeout (max 10 seconds)
COUNT=0
while [ $COUNT -lt 10 ]; do
    if ! kill -0 $NC_PID 2>/dev/null; then
        # Process finished
        break
    fi
    sleep 1
    COUNT=$((COUNT + 1))
done

# Kill nc if still running
kill $NC_PID 2>/dev/null
wait $NC_PID 2>/dev/null

# Capture end time
END=$(date +%s)
ELAPSED=$((END - START))

# Read response
RESPONSE=$(cat /tmp/timeout_response.txt 2>/dev/null)

echo ""
echo "Response received after $ELAPSED seconds:"
echo "------------------------------------------"
echo "$RESPONSE"
echo "------------------------------------------"
echo ""

# Check if we got 408
if echo "$RESPONSE" | grep -q "408"; then
    echo "✅ SUCCESS: Got 408 Request Timeout!"
    echo "   Elapsed time: ${ELAPSED}s (expected ~5s)"
    rm -f /tmp/timeout_response.txt
    exit 0
else
    echo "❌ FAIL: Expected 408 Request Timeout"
    if [ -z "$RESPONSE" ]; then
        echo "   (No response received)"
    fi
    rm -f /tmp/timeout_response.txt
    exit 1
fi
