#!/usr/bin/env python3
"""
Script CGI simples para testar cookies
Seta um cookie e mostra os cookies recebidos
"""

import os

# Ler cookies do ambiente
cookies_raw = os.environ.get('HTTP_COOKIE', '')

# Header HTTP
print("Content-Type: text/html")

# Setar um cookie de teste
print("Set-Cookie: test_cookie=hello_from_cgi; Path=/; Max-Age=3600")

# Linha vazia (fim dos headers)
print()

# Body HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Cookie Test - Simple</title>")
print("    <style>")
print("        body { font-family: Arial; margin: 40px; background: #f0f0f0; }")
print("        .container { background: white; padding: 20px; border-radius: 8px; }")
print("        h1 { color: #333; }")
print("        .cookie-info { background: #e8f4f8; padding: 15px; border-radius: 5px; margin: 10px 0; }")
print("        .success { color: #28a745; }")
print("        code { background: #f4f4f4; padding: 2px 6px; border-radius: 3px; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>🍪 Cookie Test - Simple</h1>")
print("        ")
print("        <div class='cookie-info'>")
print("            <h2>Cookie Setado:</h2>")
print("            <p class='success'>✅ <code>test_cookie=hello_from_cgi</code></p>")
print("            <p>Max-Age: 3600 segundos (1 hora)</p>")
print("        </div>")
print("        ")
print("        <div class='cookie-info'>")
print("            <h2>Cookies Recebidos:</h2>")

if cookies_raw:
    print(f"            <p><code>{cookies_raw}</code></p>")
    
    # Parse e mostrar cada cookie
    cookies = {}
    for cookie in cookies_raw.split(';'):
        cookie = cookie.strip()
        if '=' in cookie:
            name, value = cookie.split('=', 1)
            cookies[name] = value
    
    print("            <ul>")
    for name, value in cookies.items():
        print(f"                <li><strong>{name}</strong> = {value}</li>")
    print("            </ul>")
else:
    print("            <p><em>Nenhum cookie recebido (primeira visita)</em></p>")

print("        </div>")
print("        ")
print("        <div class='cookie-info'>")
print("            <h2>Como testar:</h2>")
print("            <ol>")
print("                <li>Primeira visita: Nenhum cookie</li>")
print("                <li>Recarregue a página (F5)</li>")
print("                <li>Você verá: <code>test_cookie=hello_from_cgi</code></li>")
print("            </ol>")
print("        </div>")
print("        ")
print("        <p><a href='/cgi_test.html'>← Voltar para testes CGI</a></p>")
print("    </div>")
print("</body>")
print("</html>")

