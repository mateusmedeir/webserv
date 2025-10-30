#!/usr/bin/env python3
"""
Deleta todos os cookies setando Max-Age=0
"""

import os

# Ler cookies existentes
cookies_raw = os.environ.get('HTTP_COOKIE', '')

# Parse cookies
cookie_names = []
if cookies_raw:
    for cookie in cookies_raw.split(';'):
        cookie = cookie.strip()
        if '=' in cookie:
            name = cookie.split('=', 1)[0]
            cookie_names.append(name)

# Headers HTTP
print("Content-Type: text/html")

# Deletar todos os cookies (Max-Age=0)
if cookie_names:
    for name in cookie_names:
        print(f"Set-Cookie: {name}=; Path=/; Max-Age=0")
else:
    # Deletar cookies comuns mesmo que não estejam presentes
    print("Set-Cookie: visits=; Path=/; Max-Age=0")
    print("Set-Cookie: user_name=; Path=/; Max-Age=0")
    print("Set-Cookie: theme=; Path=/; Max-Age=0")
    print("Set-Cookie: language=; Path=/; Max-Age=0")
    print("Set-Cookie: timestamp=; Path=/; Max-Age=0")
    print("Set-Cookie: test_cookie=; Path=/; Max-Age=0")

print()  # Linha vazia

# Body HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Cookies Deletados</title>")
print("    <meta http-equiv='refresh' content='3;url=/cgi_test.html'>")
print("    <style>")
print("        body { font-family: Arial; margin: 0; padding: 0; background: linear-gradient(135deg, #e74c3c 0%, #c0392b 100%); height: 100vh; display: flex; align-items: center; justify-content: center; }")
print("        .container { background: white; padding: 40px; border-radius: 10px; box-shadow: 0 8px 16px rgba(0,0,0,0.3); text-align: center; max-width: 500px; }")
print("        h1 { color: #e74c3c; margin: 0 0 20px 0; }")
print("        .icon { font-size: 80px; margin: 20px 0; }")
print("        p { color: #666; line-height: 1.6; }")
print("        .cookie-list { background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 20px 0; text-align: left; }")
print("        code { background: #e9ecef; padding: 2px 6px; border-radius: 3px; }")
print("        .timer { color: #3498db; font-weight: bold; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <div class='icon'>🗑️</div>")
print("        <h1>Cookies Deletados!</h1>")
print("        ")

if cookie_names:
    print(f"        <p><strong>{len(cookie_names)}</strong> cookie(s) foram removidos:</p>")
    print("        <div class='cookie-list'>")
    for name in cookie_names:
        print(f"            • <code>{name}</code><br>")
    print("        </div>")
else:
    print("        <p>Nenhum cookie ativo encontrado.</p>")
    print("        <p>Cookies padrão foram limpos preventivamente.</p>")

print("        ")
print("        <p class='timer'>Redirecionando em 3 segundos...</p>")
print("        <p><a href='/cgi_test.html'>← Voltar agora</a></p>")
print("    </div>")
print("</body>")
print("</html>")

