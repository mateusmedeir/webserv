#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><head><title>CGI Hello World</title></head><body>")
print("<h1>Hello from CGI!</h1>")
print("<p>Current time: " + str(os.times()) + "</p>")
print("<p>Python version: " + sys.version + "</p>")
print("<p><strong>REQUEST_METHOD:</strong> " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p><strong>QUERY_STRING:</strong> " + os.environ.get('QUERY_STRING', 'N/A') + "</p>")

# Processar query string simples
query = os.environ.get('QUERY_STRING', '')
if query:
    print("<h2>Query Parameters:</h2>")
    print("<ul>")
    for param in query.split('&'):
        if '=' in param:
            key, value = param.split('=', 1)
            print("<li><strong>" + key + ":</strong> " + value + "</li>")
    print("</ul>")

print("</body></html>")

