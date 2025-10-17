#!/bin/bash
# Sleep longer than CGI_TIMEOUT (5 seconds)
sleep 10
echo "Content-Type: text/html"
echo ""
echo "<html><body><h1>This should never be seen!</h1></body></html>"
