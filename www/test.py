#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><head><title>CGI Test - GET</title></head><body>")
print("<h1>CGI Test - GET Request</h1>")
print("<p><strong>REQUEST_METHOD:</strong> " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p><strong>QUERY_STRING:</strong> " + os.environ.get('QUERY_STRING', 'N/A') + "</p>")
print("<p><strong>SCRIPT_NAME:</strong> " + os.environ.get('SCRIPT_NAME', 'N/A') + "</p>")
print("<p><strong>REQUEST_URI:</strong> " + os.environ.get('REQUEST_URI', 'N/A') + "</p>")
print("<p><strong>SERVER_NAME:</strong> " + os.environ.get('SERVER_NAME', 'N/A') + "</p>")
print("<p><strong>SERVER_PORT:</strong> " + os.environ.get('SERVER_PORT', 'N/A') + "</p>")

# Mostrar headers HTTP
print("<h2>HTTP Headers:</h2>")
print("<ul>")
for key, value in os.environ.items():
    if key.startswith('HTTP_'):
        header_name = key[5:].replace('_', '-').capitalize()
        print("<li><strong>" + header_name + ":</strong> " + value + "</li>")
print("</ul>")

print("<hr>")
print("<p><a href='/test_post.py'>Test POST Request</a></p>")
print("</body></html>")

