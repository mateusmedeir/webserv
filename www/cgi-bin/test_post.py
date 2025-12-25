#!/usr/bin/env python3
import os
import sys

# Forçar linha buffered output (mais compatível que unbuffered)
sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><head><title>CGI Test - POST</title></head><body>")
print("<h1>CGI Test - POST Request</h1>")
print("<p><strong>REQUEST_METHOD:</strong> " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p><strong>CONTENT_TYPE:</strong> " + os.environ.get('CONTENT_TYPE', 'N/A') + "</p>")
print("<p><strong>CONTENT_LENGTH:</strong> " + os.environ.get('CONTENT_LENGTH', 'N/A') + "</p>")

# Ler body se POST
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        body = sys.stdin.read(content_length)
        print("<h2>Body received:</h2>")
        print("<pre>" + body[:1000] + "</pre>")  # Limitar a 1000 caracteres
        print("<p><strong>Body length:</strong> " + str(len(body)) + " bytes</p>")
    else:
        print("<p><em>No body received</em></p>")
else:
    print("<p><em>Not a POST request</em></p>")

print("<hr>")
print("<p><a href='/test.py'>Test GET Request</a></p>")
print("</body></html>")
sys.stdout.flush()

