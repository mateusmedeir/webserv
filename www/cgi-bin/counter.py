#!/usr/bin/env python3
"""
Contador de visitas usando cookies
Demonstra persistência de estado entre requisições
"""

import os

# Ler cookies do ambiente
cookies_raw = os.environ.get('HTTP_COOKIE', '')

# Parse cookies
visits = 0
found_cookie = False

if cookies_raw:
    for cookie in cookies_raw.split(';'):
        cookie = cookie.strip()
        if cookie.startswith('visits='):
            try:
                visits = int(cookie.split('=', 1)[1])
                found_cookie = True
            except:
                visits = 0

# Incrementar contador
visits += 1

# Headers HTTP
print("Content-Type: text/html")
print(f"Set-Cookie: visits={visits}; Path=/; Max-Age=86400")  # 24 horas
print()

# Body HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Contador de Visitas</title>")
print("    <style>")
print("        body { font-family: Arial; margin: 40px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); }")
print("        .container { background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); max-width: 600px; margin: 0 auto; }")
print("        h1 { color: #333; text-align: center; }")
print("        .counter { font-size: 72px; text-align: center; color: #667eea; font-weight: bold; margin: 30px 0; }")
print("        .info { background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 20px 0; }")
print("        .btn { display: inline-block; padding: 10px 20px; background: #667eea; color: white; text-decoration: none; border-radius: 5px; margin: 5px; }")
print("        .btn:hover { background: #5568d3; }")
print("        .status { text-align: center; color: #666; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>🔢 Contador de Visitas</h1>")
print("        ")
print("        <div class='counter'>")
print(f"            {visits}")
print("        </div>")
print("        ")
print("        <div class='status'>")

if visits == 1:
    print("            <p>🎉 <strong>Primeira visita!</strong> Bem-vindo!</p>")
elif visits == 2:
    print("            <p>👋 <strong>Segunda visita!</strong> Obrigado por voltar!</p>")
else:
    print(f"            <p>🌟 <strong>Você já visitou {visits} vezes!</strong></p>")

print("        </div>")
print("        ")
print("        <div class='info'>")
print("            <h3>ℹ️ Como funciona:</h3>")
print("            <ul>")
print("                <li>Cookie: <code>visits</code></li>")
print("                <li>Valor atual: <code>" + str(visits) + "</code></li>")
print("                <li>Validade: 24 horas</li>")
print("                <li>Toda visita incrementa o contador</li>")
print("            </ul>")
print("        </div>")
print("        ")
print("        <div style='text-align: center;'>")
print("            <a href='/cgi-bin/counter.py' class='btn'>🔄 Recarregar</a>")
print("            <a href='/cgi-bin/cookie_delete.py' class='btn' style='background: #dc3545;'>🗑️ Resetar</a>")
print("            <br><br>")
print("            <a href='/cgi_test.html'>← Voltar para testes CGI</a>")
print("        </div>")
print("    </div>")
print("</body>")
print("</html>")

