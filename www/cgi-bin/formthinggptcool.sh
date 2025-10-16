#!/bin/bash

echo "Content-Type: text/html"
echo ""

# Read from stdin (POST body)
if [ "$REQUEST_METHOD" = "POST" ]; then
    read -n "$CONTENT_LENGTH" FORM_DATA
else
    FORM_DATA="$QUERY_STRING"
fi

# Function to decode URL encoding
urldecode() {
    local data=${1//+/ }
    printf '%b' "${data//%/\\x}"
}

# Parse fields
IFS='&' read -ra pairs <<< "$FORM_DATA"

declare -A form
for pair in "${pairs[@]}"; do
    IFS='=' read -r key val <<< "$pair"
    key=$(urldecode "$key")
    val=$(urldecode "$val")
    form["$key"]="$val"
done

# Generate fun output
echo "<html><body>"
echo "<h1>Parsed Form Fields</h1>"
echo "<ul>"
for key in "${!form[@]}"; do
    value="${form[$key]}"
    reversed=$(echo "$value" | rev)
    upper=$(echo "$value" | tr '[:lower:]' '[:upper:]')
    echo "<li><b>$key</b>: \"$value\"<br>"
    echo "&nbsp;&nbsp;Reversed: \"$reversed\"<br>"
    echo "&nbsp;&nbsp;Uppercase: \"$upper\"</li>"
done
echo "</ul>"

echo "<hr><p>Have a nice CGI day!</p>"
echo "</body></html>"
