#!/usr/bin/env python3
import os
import sys

# Script CGI Python simples para teste GET

print("Content-Type: text/html\r")
print("\r")
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>Python CGI - Hello</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }")
print("        h1 { color: #3776ab; }")
print("        .info { background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }")
print("        .success { color: green; font-weight: bold; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>🐍 Python CGI Script</h1>")
print("    <p class='success'>✅ CGI funcionando corretamente!</p>")
print("    ")
print("    <div class='info'>")
print("        <h2>Variáveis de Ambiente:</h2>")
print("        <ul>")
print(f"            <li><strong>REQUEST_METHOD:</strong> {os.environ.get('REQUEST_METHOD', 'N/A')}</li>")
print(f"            <li><strong>QUERY_STRING:</strong> {os.environ.get('QUERY_STRING', 'N/A')}</li>")
print(f"            <li><strong>SCRIPT_FILENAME:</strong> {os.environ.get('SCRIPT_FILENAME', 'N/A')}</li>")
print(f"            <li><strong>SERVER_PROTOCOL:</strong> {os.environ.get('SERVER_PROTOCOL', 'N/A')}</li>")
print(f"            <li><strong>GATEWAY_INTERFACE:</strong> {os.environ.get('GATEWAY_INTERFACE', 'N/A')}</li>")
print("        </ul>")
print("    </div>")
print("    ")
print("    <p><a href='/'>← Voltar</a></p>")
print("</body>")
print("</html>")

