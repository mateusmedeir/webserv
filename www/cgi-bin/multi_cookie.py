#!/usr/bin/env python3
"""
Teste de múltiplos cookies
Seta 3 cookies diferentes e mostra todos
"""

import os
from datetime import datetime

# Ler cookies existentes
cookies_raw = os.environ.get('HTTP_COOKIE', '')

# Parse cookies recebidos
received_cookies = {}
if cookies_raw:
    for cookie in cookies_raw.split(';'):
        cookie = cookie.strip()
        if '=' in cookie:
            name, value = cookie.split('=', 1)
            received_cookies[name] = value

# Headers HTTP
print("Content-Type: text/html")

# Setar múltiplos cookies
print("Set-Cookie: user_name=John_Doe; Path=/; Max-Age=3600")
print("Set-Cookie: theme=dark; Path=/; Max-Age=3600")
print("Set-Cookie: language=pt-BR; Path=/; Max-Age=3600")
print(f"Set-Cookie: timestamp={int(datetime.now().timestamp())}; Path=/; Max-Age=3600")

print()  # Linha vazia

# Body HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("    <title>Múltiplos Cookies</title>")
print("    <style>")
print("        body { font-family: 'Segoe UI', Arial; margin: 0; padding: 40px; background: #1a1a2e; color: #eee; }")
print("        .container { max-width: 800px; margin: 0 auto; background: #16213e; padding: 30px; border-radius: 10px; box-shadow: 0 8px 16px rgba(0,0,0,0.3); }")
print("        h1 { color: #0f4c75; border-bottom: 3px solid #3282b8; padding-bottom: 10px; }")
print("        h2 { color: #3282b8; }")
print("        .cookie-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 15px; margin: 20px 0; }")
print("        .cookie-card { background: #0f3460; padding: 15px; border-radius: 8px; border-left: 4px solid #3282b8; }")
print("        .cookie-card h3 { margin: 0 0 10px 0; color: #3282b8; font-size: 16px; }")
print("        .cookie-card p { margin: 5px 0; font-family: 'Courier New', monospace; }")
print("        .cookie-name { color: #bbbbbb; }")
print("        .cookie-value { color: #00d9ff; font-weight: bold; }")
print("        .info-box { background: #0f3460; padding: 15px; border-radius: 5px; margin: 20px 0; border: 1px solid #3282b8; }")
print("        .status { text-align: center; padding: 20px; background: #1a472a; border-radius: 5px; margin: 20px 0; }")
print("        .status.first-visit { background: #472a1a; }")
print("        code { background: #0a0e27; padding: 3px 8px; border-radius: 3px; color: #00d9ff; }")
print("        a { color: #3282b8; text-decoration: none; }")
print("        a:hover { color: #00d9ff; }")
print("    </style>")
print("</head>")
print("<body>")
print("    <div class='container'>")
print("        <h1>🍪 Teste de Múltiplos Cookies</h1>")
print("        ")

# Status
if not received_cookies:
    print("        <div class='status first-visit'>")
    print("            <h2>🎉 Primeira Visita!</h2>")
    print("            <p>Nenhum cookie recebido. Cookies foram setados agora.</p>")
    print("            <p><strong>Recarregue a página (F5)</strong> para ver os cookies em ação!</p>")
    print("        </div>")
else:
    print("        <div class='status'>")
    print("            <h2>✅ Cookies Ativos!</h2>")
    print(f"            <p>Recebidos <strong>{len(received_cookies)}</strong> cookie(s)</p>")
    print("        </div>")

# Cookies setados nesta resposta
print("        <h2>📤 Cookies Enviados (Set-Cookie):</h2>")
print("        <div class='cookie-grid'>")
print("            <div class='cookie-card'>")
print("                <h3>👤 Usuário</h3>")
print("                <p class='cookie-name'>Nome: <span class='cookie-value'>user_name</span></p>")
print("                <p class='cookie-name'>Valor: <span class='cookie-value'>John_Doe</span></p>")
print("                <p class='cookie-name'>Max-Age: <span class='cookie-value'>3600s</span></p>")
print("            </div>")
print("            <div class='cookie-card'>")
print("                <h3>🎨 Tema</h3>")
print("                <p class='cookie-name'>Nome: <span class='cookie-value'>theme</span></p>")
print("                <p class='cookie-name'>Valor: <span class='cookie-value'>dark</span></p>")
print("                <p class='cookie-name'>Max-Age: <span class='cookie-value'>3600s</span></p>")
print("            </div>")
print("            <div class='cookie-card'>")
print("                <h3>🌍 Idioma</h3>")
print("                <p class='cookie-name'>Nome: <span class='cookie-value'>language</span></p>")
print("                <p class='cookie-name'>Valor: <span class='cookie-value'>pt-BR</span></p>")
print("                <p class='cookie-name'>Max-Age: <span class='cookie-value'>3600s</span></p>")
print("            </div>")
print("            <div class='cookie-card'>")
print("                <h3>⏰ Timestamp</h3>")
print("                <p class='cookie-name'>Nome: <span class='cookie-value'>timestamp</span></p>")
print(f"                <p class='cookie-name'>Valor: <span class='cookie-value'>{int(datetime.now().timestamp())}</span></p>")
print("                <p class='cookie-name'>Max-Age: <span class='cookie-value'>3600s</span></p>")
print("            </div>")
print("        </div>")

# Cookies recebidos
print("        <h2>📥 Cookies Recebidos (Cookie header):</h2>")

if received_cookies:
    print("        <div class='info-box'>")
    print(f"            <p><strong>Header bruto:</strong></p>")
    print(f"            <code>{cookies_raw}</code>")
    print("        </div>")
    print("        ")
    print("        <div class='cookie-grid'>")
    
    for name, value in received_cookies.items():
        emoji = "🍪"
        if name == "user_name": emoji = "👤"
        elif name == "theme": emoji = "🎨"
        elif name == "language": emoji = "🌍"
        elif name == "timestamp": emoji = "⏰"
        
        print(f"            <div class='cookie-card'>")
        print(f"                <h3>{emoji} {name}</h3>")
        print(f"                <p class='cookie-value'>{value}</p>")
        print(f"            </div>")
    
    print("        </div>")
else:
    print("        <div class='info-box'>")
    print("            <p><em>Nenhum cookie recebido ainda.</em></p>")
    print("            <p>Recarregue a página para ver os cookies em ação!</p>")
    print("        </div>")

# Informações técnicas
print("        <h2>ℹ️ Informações Técnicas:</h2>")
print("        <div class='info-box'>")
print("            <ul>")
print("                <li><strong>Path:</strong> <code>/</code> (válido em todo o site)</li>")
print("                <li><strong>Max-Age:</strong> <code>3600</code> segundos (1 hora)</li>")
print("                <li><strong>Variável de ambiente CGI:</strong> <code>HTTP_COOKIE</code></li>")
print(f"                <li><strong>Cookies ativos:</strong> {len(received_cookies)}</li>")
print("            </ul>")
print("        </div>")

# Botões de ação
print("        <div style='text-align: center; margin-top: 30px;'>")
print("            <a href='/cgi-bin/multi_cookie.py' style='display: inline-block; padding: 12px 24px; background: #3282b8; color: white; border-radius: 5px; margin: 5px; text-decoration: none;'>🔄 Recarregar</a>")
print("            <a href='/cgi-bin/cookie_delete.py' style='display: inline-block; padding: 12px 24px; background: #e74c3c; color: white; border-radius: 5px; margin: 5px; text-decoration: none;'>🗑️ Limpar Cookies</a>")
print("            <br><br>")
print("            <a href='/cgi_test.html'>← Voltar para testes CGI</a>")
print("        </div>")
print("    </div>")
print("</body>")
print("</html>")

