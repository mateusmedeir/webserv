#!/usr/bin/env python3
import os
import sys
import signal

# Debug: imprimir imediatamente quando receber EOF
def signal_handler(sig, frame):
    print("DEBUG: Received signal", file=sys.stderr)
    sys.exit(0)

signal.signal(signal.SIGPIPE, signal_handler)

print("Content-Type: text/html\r")
print("Status: 200 OK\r")
print("\r")
sys.stdout.flush()

print("<html><head><title>CGI POST Debug</title></head><body>")
print("<h1>POST Debug Test</h1>")
sys.stdout.flush()

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    print(f"<p>Content-Length: {content_length}</p>")
    sys.stdout.flush()
    
    if content_length > 0:
        # Tentar ler do stdin
        try:
            body = sys.stdin.read(content_length)
            print(f"<p>Body read: {len(body)} bytes</p>")
            print(f"<pre>{body[:100]}</pre>")
        except Exception as e:
            print(f"<p>Error reading: {e}</p>")
    else:
        print("<p>No content length</p>")
else:
    print("<p>Not POST</p>")

print("</body></html>")
sys.stdout.flush()

