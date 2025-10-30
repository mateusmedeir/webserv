#!/usr/bin/env python3
import os
import sys

# Script CGI Python para teste POST

# Ler body do stdin
content_length = os.environ.get('CONTENT_LENGTH', '0')
if content_length.isdigit() and int(content_length) > 0:
    body = sys.stdin.read(int(content_length))
else:
    body = ""

print("Content-Type: text/html\r")
print("\r")
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>Python CGI - POST Test</title>")
print("    <style>")
print("        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }")
print("        h1 { color: #3776ab; }")
print("        .form-section { background: #e8f4f8; padding: 20px; border-radius: 5px; margin: 20px 0; }")
print("        .result { background: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }")
print("        input, textarea { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }")
print("        button { background: #3776ab; color: white; padding: 10px 20px; border: none; cursor: pointer; }")
print("        button:hover { background: #2a5a87; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <h1>🐍 Python CGI - POST Test</h1>")

if body:
    print("    <div class='result'>")
    print("        <h2>✅ POST Recebido!</h2>")
    print(f"        <p><strong>Content-Length:</strong> {content_length} bytes</p>")
    print(f"        <p><strong>Content-Type:</strong> {os.environ.get('CONTENT_TYPE', 'N/A')}</p>")
    print(f"        <p><strong>Dados recebidos:</strong></p>")
    print(f"        <pre>{body}</pre>")
    print("    </div>")

print("    <div class='form-section'>")
print("        <h2>Formulário de Teste:</h2>")
print("        <form method='POST' action='/cgi-bin/post_test.py'>")
print("            <label>Nome:</label>")
print("            <input type='text' name='name' placeholder='Seu nome' required>")
print("            ")
print("            <label>Mensagem:</label>")
print("            <textarea name='message' rows='4' placeholder='Sua mensagem' required></textarea>")
print("            ")
print("            <button type='submit'>Enviar</button>")
print("        </form>")
print("    </div>")
print("    ")
print("    <p><a href='/'>← Voltar</a></p>")
print("</body>")
print("</html>")

