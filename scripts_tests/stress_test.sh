#!/bin/bash

# Configuration
URL="http://localhost:8080/"
CONCURRENT=50
DURATION=30

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "WebServ Stress Test"
echo "=========================================="
echo "URL: $URL"
echo "Concurrent workers: $CONCURRENT"
echo "Duration: ${DURATION}s"
echo ""

# Temporary files
TEMP_DIR="/tmp/webserv_stress_$$"
mkdir -p "$TEMP_DIR"
SUCCESS_FILE="$TEMP_DIR/success"
FAILED_FILE="$TEMP_DIR/failed"
TOTAL_FILE="$TEMP_DIR/total"

echo "0" > "$SUCCESS_FILE"
echo "0" > "$FAILED_FILE"
echo "0" > "$TOTAL_FILE"

# Function to safely increment counter
increment_counter() {
    local file=$1
    local count=$(cat "$file")
    echo $((count + 1)) > "$file"
}

# Worker function
worker() {
    local worker_id=$1
    local end_time=$(($(date +%s) + DURATION))
    local worker_success=0
    local worker_failed=0
    local worker_total=0
    
    while [ $(date +%s) -lt $end_time ]; do
        # Make request and capture HTTP code
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$URL" 2>/dev/null)
        
        worker_total=$((worker_total + 1))
        
        # Check if request succeeded
        if [ "$http_code" = "200" ]; then
            worker_success=$((worker_success + 1))
            increment_counter "$SUCCESS_FILE"
        else
            worker_failed=$((worker_failed + 1))
            increment_counter "$FAILED_FILE"
        fi
        
        increment_counter "$TOTAL_FILE"
    done
    
    # Write worker summary to file
    echo "Worker $worker_id: $worker_total requests ($worker_success success, $worker_failed failed)" >> "$TEMP_DIR/worker_${worker_id}.log"
}

# Check if server is running
echo "Checking if server is running..."
if ! curl -s -o /dev/null -w "%{http_code}" --max-time 2 "$URL" > /dev/null 2>&1; then
    echo -e "${RED}❌ Server not responding at $URL${NC}"
    rm -rf "$TEMP_DIR"
    exit 1
fi
echo -e "${GREEN}✅ Server is running${NC}"
echo ""

# Start time
START=$(date +%s)
echo "Starting $CONCURRENT workers..."

# Launch workers in background
for i in $(seq 1 $CONCURRENT); do
    worker $i &
done

# Progress indicator
echo -n "Running test"
for i in $(seq 1 $DURATION); do
    sleep 1
    echo -n "."
done
echo ""

# Wait for all workers to finish
wait

# End time
END=$(date +%s)
ELAPSED=$((END - START))

# Get final counts
TOTAL=$(cat "$TOTAL_FILE")
SUCCESS=$(cat "$SUCCESS_FILE")
FAILED=$(cat "$FAILED_FILE")

# Calculate stats
if [ $ELAPSED -gt 0 ]; then
    RATE=$((TOTAL / ELAPSED))
else
    RATE=0
fi

if [ $TOTAL -gt 0 ]; then
    AVAILABILITY=$(awk "BEGIN {printf \"%.2f\", ($SUCCESS/$TOTAL)*100}")
else
    AVAILABILITY="0.00"
fi

# Display results
echo ""
echo "=========================================="
echo "Results:"
echo "=========================================="
echo "Transactions:          $TOTAL hits"
echo "Availability:          ${AVAILABILITY}%"
echo "Elapsed time:          ${ELAPSED}s"
echo "Response rate:         ${RATE} trans/sec"
echo "Successful:            $SUCCESS"
echo "Failed:                $FAILED"
echo "Concurrent workers:    $CONCURRENT"
echo ""

# Show worker details
echo "Worker Details:"
echo "------------------------------------------"
cat "$TEMP_DIR"/worker_*.log 2>/dev/null | head -10
if [ $(ls "$TEMP_DIR"/worker_*.log 2>/dev/null | wc -l) -gt 10 ]; then
    echo "... (showing first 10 workers)"
fi
echo ""

# Status check
echo "=========================================="
if [ "$FAILED" -eq 0 ] && [ "$SUCCESS" -gt 0 ]; then
    echo -e "${GREEN}✅ All requests successful!${NC}"
elif [ "$FAILED" -lt $((TOTAL / 10)) ]; then
    echo -e "${YELLOW}⚠️  Some requests failed (< 10%)${NC}"
else
    echo -e "${RED}❌ High failure rate!${NC}"
fi

# Check if server is still running
if curl -s -o /dev/null -w "%{http_code}" --max-time 2 "$URL" > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Server still running after stress test${NC}"
else
    echo -e "${RED}❌ Server crashed or stopped!${NC}"
fi
echo "=========================================="

# Cleanup
rm -rf "$TEMP_DIR"
