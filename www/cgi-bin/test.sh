#!/bin/bash

echo "Content-Type: text/plain"
echo ""
echo "Hello from Bash CGI!"
echo ""
echo "Current date: $(date)"
echo "Script name: $SCRIPT_NAME"
echo "Request method: $REQUEST_METHOD"
echo "Query string: $QUERY_STRING"