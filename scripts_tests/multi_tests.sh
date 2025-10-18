#!/bin/bash

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo "=========================================="
echo "WebServ Complete Test Suite"
echo "=========================================="
echo ""

# Test 1: Light load with custom script
echo -e "${BLUE}Test 1: Light Load (Custom Script)${NC}"
echo "Duration: 10s, Concurrent: 10"
./stress_test.sh 10 10 http://localhost:8080/
sleep 2

# Test 2: Medium load with Apache Bench
echo ""
echo -e "${BLUE}Test 2: Medium Load (Apache Bench)${NC}"
echo "Requests: 1000, Concurrent: 50"
ab -n 1000 -c 50 http://localhost:8080/ | grep -E "Complete|Failed|Time taken|Requests per second|Failed requests"
sleep 2

# Test 3: Heavy load with custom script
echo ""
echo -e "${BLUE}Test 3: Heavy Load (Custom Script)${NC}"
echo "Duration: 30s, Concurrent: 100"
./stress_test.sh 30 100 http://localhost:8080/
sleep 2

# Test 4: Different endpoints
echo ""
echo -e "${BLUE}Test 4: Multiple Endpoints${NC}"
for endpoint in "/" "/index.html" "/test_autoindex/"; do
    echo ""
    echo "Testing: $endpoint"
    ab -n 500 -c 25 "http://localhost:8080${endpoint}" | grep -E "Complete|Failed|Requests per second"
    sleep 1
done

echo ""
echo "=========================================="
echo -e "${GREEN}All tests completed!${NC}"
echo "=========================================="
