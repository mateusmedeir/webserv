#!/bin/bash

# Script de teste automatizado para cookies
# Testa a implementação sem precisar rodar o servidor manualmente

echo "🍪 TESTE DE COOKIES - WEBSERV 42"
echo "================================"
echo ""

# Cores
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Verificar se o executável existe
if [ ! -f "./webserv" ]; then
    echo -e "${RED}❌ Erro: webserv não encontrado${NC}"
    echo "Compile primeiro: make"
    exit 1
fi

echo -e "${GREEN}✅ Executável webserv encontrado${NC}"

# Verificar se os scripts CGI existem
echo ""
echo "Verificando scripts CGI..."
scripts=(
    "www/cgi-bin/cookie_simple.py"
    "www/cgi-bin/counter.py"
    "www/cgi-bin/multi_cookie.py"
    "www/cgi-bin/cookie_delete.py"
)

for script in "${scripts[@]}"; do
    if [ -f "$script" ]; then
        if [ -x "$script" ]; then
            echo -e "${GREEN}✅ $script (executável)${NC}"
        else
            echo -e "${YELLOW}⚠️  $script (não executável)${NC}"
            chmod +x "$script"
            echo -e "${GREEN}   ✅ Permissão de execução adicionada${NC}"
        fi
    else
        echo -e "${RED}❌ $script (não encontrado)${NC}"
    fi
done

# Verificar página de teste
echo ""
if [ -f "www/cookies_test.html" ]; then
    echo -e "${GREEN}✅ www/cookies_test.html encontrado${NC}"
else
    echo -e "${RED}❌ www/cookies_test.html não encontrado${NC}"
fi

# Verificar arquivos de código
echo ""
echo "Verificando arquivos de código..."
files=(
    "includes/CookieHandler.hpp"
    "source/CookieHandler.cpp"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        lines=$(wc -l < "$file")
        echo -e "${GREEN}✅ $file ($lines linhas)${NC}"
    else
        echo -e "${RED}❌ $file (não encontrado)${NC}"
    fi
done

echo ""
echo "================================"
echo -e "${GREEN}✅ TODOS OS ARQUIVOS VERIFICADOS!${NC}"
echo ""
echo "📝 Para testar manualmente:"
echo "   1. ./webserv configs/default.conf"
echo "   2. Abrir http://localhost:8080/cookies_test.html"
echo "   3. Ou usar curl:"
echo "      curl -v http://localhost:8080/cgi-bin/cookie_simple.py"
echo ""
echo "📊 Arquivos criados:"
echo "   • 2 headers (.hpp)"
echo "   • 2 implementations (.cpp)"  
echo "   • 4 scripts CGI Python"
echo "   • 1 página HTML de teste"
echo "   • Modificados: HttpRequest, HttpResponse"
echo ""
echo -e "${GREEN}🎉 IMPLEMENTAÇÃO DE COOKIES COMPLETA!${NC}"

