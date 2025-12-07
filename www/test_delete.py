#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><head><title>CGI DELETE Test</title></head><body>")
print("<h1>CGI DELETE Request</h1>")
print("<p><strong>REQUEST_METHOD:</strong> " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p><strong>SCRIPT_NAME:</strong> " + os.environ.get('SCRIPT_NAME', 'N/A') + "</p>")
print("<p><strong>REQUEST_URI:</strong> " + os.environ.get('REQUEST_URI', 'N/A') + "</p>")

if os.environ.get('REQUEST_METHOD') == 'DELETE':
    print("<p><strong>Status:</strong> DELETE request received successfully!</p>")
else:
    print("<p><strong>Status:</strong> Not a DELETE request</p>")

print("</body></html>")
sys.stdout.flush()

