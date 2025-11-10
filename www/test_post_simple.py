#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")

print("<html><head><title>CGI POST Simple</title></head><body>")
print("<h1>POST Test</h1>")

# Forçar flush imediato
sys.stdout.flush()

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    print(f"<p>Content-Length: {content_length}</p>")
    sys.stdout.flush()
    
    if content_length > 0:
        # Ler com timeout ou de forma mais simples
        body = sys.stdin.read(content_length)
        print(f"<p>Body read: {len(body)} bytes</p>")
        print(f"<pre>{body[:100]}</pre>")
    else:
        print("<p>No content length</p>")
else:
    print("<p>Not POST</p>")

print("</body></html>")
sys.stdout.flush()

