#!/usr/bin/env python3
import os
import sys
import time

print("before")
time.sleep(2)
print("after")

print("Content-Type: text/html")
print()
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from CGI!</h1>")
print("<p>This is a Python CGI script running on WebSaucisse.</p>")
print("<h2>Environment Variables:</h2>")
print("<ul>")
print(f"<li>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'N/A')}</li>")
print(f"<li>QUERY_STRING: {os.environ.get('QUERY_STRING', 'N/A')}</li>")
print(f"<li>PATH_INFO: {os.environ.get('PATH_INFO', 'N/A')}</li>")
print(f"<li>CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'N/A')}</li>")
print(f"<li>SERVER_NAME: {os.environ.get('SERVER_NAME', 'N/A')}</li>")
print(f"<li>SERVER_PORT: {os.environ.get('SERVER_PORT', 'N/A')}</li>")
print("</ul>")

# if os.environ.get('REQUEST_METHOD') == 'POST':
#     content_length = int(os.environ.get('CONTENT_LENGTH', 0))
#     if content_length > 0:
#         post_data = sys.stdin.read(content_length)
#         print(f"<h2>POST Data:</h2>")
#         print(f"<pre>{post_data}</pre>")

print("</body>")
print("</html>")