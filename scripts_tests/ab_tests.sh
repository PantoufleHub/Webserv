#!/bin/bash

# Configuration
URL="http://localhost:8080/"
REQUESTS=10000
CONCURRENT=100

echo "=========================================="
echo "Apache Bench Stress Test"
echo "=========================================="
echo "URL: $URL"
echo "Total requests: $REQUESTS"
echo "Concurrent: $CONCURRENT"
echo ""

# Run ab and save output
ab -n $REQUESTS -c $CONCURRENT "$URL" > /tmp/ab_output.txt 2>&1

# Display output
cat /tmp/ab_output.txt

# Parse results and calculate availability
COMPLETE=$(grep "Complete requests:" /tmp/ab_output.txt | awk '{print $3}')
FAILED=$(grep "Failed requests:" /tmp/ab_output.txt | awk '{print $3}')

if [ -n "$COMPLETE" ] && [ -n "$FAILED" ]; then
    SUCCESS=$((COMPLETE - FAILED))
    if [ $COMPLETE -gt 0 ]; then
        AVAILABILITY=$(awk "BEGIN {printf \"%.2f\", ($SUCCESS/$COMPLETE)*100}")
    else
        AVAILABILITY="0.00"
    fi
    
    echo ""
    echo "=========================================="
    echo "Additional Stats:"
    echo "=========================================="
    echo "Total requests:    $COMPLETE"
    echo "Successful:        $SUCCESS"
    echo "Failed:            $FAILED"
    echo "Availability:      ${AVAILABILITY}%"
    echo "=========================================="
    
    # Status
    if [ "$FAILED" -eq 0 ]; then
        echo "✅ Perfect! No failed requests"
    elif [ "$FAILED" -lt $((COMPLETE / 100)) ]; then
        echo "⚠️  Minor failures (< 1%)"
    else
        echo "❌ Significant failures detected"
    fi
fi

# Cleanup
rm -f /tmp/ab_output.txt
