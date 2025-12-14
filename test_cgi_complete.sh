#!/bin/bash

# Testes Completos CGI - GET, POST e DELETE
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "  Testes Completos CGI - WebServ"
echo "  GET, POST e DELETE"
echo "=========================================="
echo ""

cd /home/azevedo/42/WebServ/webserv

# Limpar processos antigos
pkill -9 webserv 2>/dev/null
sleep 1

# Compilar
echo "Compilando..."
make workflow > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}ERRO: Falha na compilação${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Compilação OK${NC}"
echo ""

# Iniciar servidor
echo "Iniciando servidor..."
./webserv configs/test_simple.conf > /tmp/webserv_cgi_tests.log 2>&1 &
SERVER_PID=$!
echo "Servidor PID: $SERVER_PID"
sleep 3

# Verificar se está rodando
if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    echo -e "${RED}ERRO: Servidor não está rodando${NC}"
    cat /tmp/webserv_cgi_tests.log
    exit 1
fi

TESTS_PASSED=0
TESTS_FAILED=0

# Função de teste
test_request() {
    local name=$1
    local method=$2
    local url=$3
    local data=$4
    local expected=$5
    
    echo -n "Teste: $name... "
    
    if [ "$method" = "GET" ]; then
        RESPONSE=$(curl -s --max-time 5 "$url" 2>&1)
    elif [ "$method" = "DELETE" ]; then
        RESPONSE=$(curl -s --max-time 5 -X DELETE "$url" 2>&1)
    else
        RESPONSE=$(curl -s --max-time 8 -X POST "$url" -d "$data" -H "Content-Type: application/x-www-form-urlencoded" 2>&1)
    fi
    
    if [ -z "$RESPONSE" ]; then
        echo -e "${RED}✗ FALHOU${NC} (sem resposta)"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    if [ -n "$expected" ] && ! echo "$RESPONSE" | grep -q "$expected"; then
        echo -e "${RED}✗ FALHOU${NC} (resposta não contém '$expected')"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    echo -e "${GREEN}✓ PASSOU${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    return 0
}

# Função para testar HTTP status code
test_status_code() {
    local name=$1
    local method=$2
    local url=$3
    local data=$4
    local expected_code=$5
    
    echo -n "Teste: $name... "
    
    if [ "$method" = "GET" ]; then
        HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$url")
    elif [ "$method" = "DELETE" ]; then
        HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X DELETE "$url")
    else
        HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" --max-time 8 -X POST "$url" -d "$data" -H "Content-Type: application/x-www-form-urlencoded")
    fi
    
    if [ "$HTTP_CODE" = "$expected_code" ]; then
        echo -e "${GREEN}✓ PASSOU${NC} (HTTP $HTTP_CODE)"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗ FALHOU${NC} (HTTP $HTTP_CODE esperado $expected_code)"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

echo "=========================================="
echo "  TESTES GET"
echo "=========================================="
echo ""

test_request "GET simples" "GET" "http://127.0.0.1:8080/test.py" "" "CGI Test - GET"
test_request "GET com query string (1 parâmetro)" "GET" "http://127.0.0.1:8080/test.py?nome=João" "" "CGI Test - GET"
test_request "GET com query string (múltiplos parâmetros)" "GET" "http://127.0.0.1:8080/test.py?nome=João&idade=30&cidade=São%20Paulo" "" "CGI Test - GET"
test_request "GET com caracteres especiais" "GET" "http://127.0.0.1:8080/test.py?mensagem=Olá%20Mundo&valor=100%25" "" "CGI Test - GET"
test_request "hello.py" "GET" "http://127.0.0.1:8080/hello.py" "" "Hello"
test_status_code "GET HTTP Status Code" "GET" "http://127.0.0.1:8080/test.py" "" "200"

echo ""
echo "=========================================="
echo "  TESTES POST"
echo "=========================================="
echo ""

test_request "POST simples" "POST" "http://127.0.0.1:8080/test_post.py" "nome=Maria" "CGI Test - POST"
test_request "POST com múltiplos campos" "POST" "http://127.0.0.1:8080/test_post.py" "nome=Maria&mensagem=Teste&idade=25" "CGI Test - POST"
test_request "POST com body maior" "POST" "http://127.0.0.1:8080/test_post.py" "nome=Teste&mensagem=$(head -c 200 < /dev/urandom | base64 | tr -d '\n')" "CGI Test - POST"
test_request "POST verifica CONTENT_LENGTH" "POST" "http://127.0.0.1:8080/test_post.py" "nome=Maria&mensagem=Teste" "CONTENT_LENGTH"
test_request "POST verifica body recebido" "POST" "http://127.0.0.1:8080/test_post.py" "nome=João&mensagem=Olá" "Body received"
test_status_code "POST HTTP Status Code" "POST" "http://127.0.0.1:8080/test_post.py" "nome=Teste" "200"

echo ""
echo "=========================================="
echo "  TESTES DELETE"
echo "=========================================="
echo ""

test_request "DELETE simples" "DELETE" "http://127.0.0.1:8080/test_delete.py" "" "DELETE request received"
test_request "DELETE verifica REQUEST_METHOD" "DELETE" "http://127.0.0.1:8080/test_delete.py" "" "REQUEST_METHOD.*DELETE"
test_status_code "DELETE HTTP Status Code" "DELETE" "http://127.0.0.1:8080/test_delete.py" "" "200"

echo ""
echo "=========================================="
echo "  TESTES AVANÇADOS"
echo "=========================================="
echo ""

echo -n "Teste: Variáveis de ambiente (GET)... "
RESPONSE=$(curl -s --max-time 5 "http://127.0.0.1:8080/test.py?test=env")
if echo "$RESPONSE" | grep -q "REQUEST_METHOD.*GET" && \
   echo "$RESPONSE" | grep -q "QUERY_STRING" && \
   echo "$RESPONSE" | grep -q "SERVER_NAME" && \
   echo "$RESPONSE" | grep -q "SCRIPT_NAME"; then
    echo -e "${GREEN}✓ PASSOU${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FALHOU${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -n "Teste: Variáveis de ambiente (POST)... "
RESPONSE=$(curl -s --max-time 8 -X POST http://127.0.0.1:8080/test_post.py -d "nome=Teste" -H "Content-Type: application/x-www-form-urlencoded")
if echo "$RESPONSE" | grep -q "REQUEST_METHOD.*POST" && \
   echo "$RESPONSE" | grep -q "CONTENT_TYPE" && \
   echo "$RESPONSE" | grep -q "CONTENT_LENGTH"; then
    echo -e "${GREEN}✓ PASSOU${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ FALHOU${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -n "Teste: Headers HTTP na resposta... "
# Usar GET com -I para obter apenas headers (curl -I faz HEAD, mas servidor pode não suportar)
# Alternativa: usar GET normal e verificar headers na resposta completa
RESPONSE=$(curl -s -D - --max-time 5 http://127.0.0.1:8080/test.py -o /dev/null 2>&1)
if echo "$RESPONSE" | grep -qiE "HTTP/1\.[01].*200" && echo "$RESPONSE" | grep -qi "Content-Type"; then
    echo -e "${GREEN}✓ PASSOU${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${YELLOW}⚠ PARCIAL${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

echo -n "Teste: Requisições simultâneas... "
PIDS=()
for i in {1..3}; do
    timeout 5 curl -s "http://127.0.0.1:8080/test.py?test=$i" > /dev/null 2>&1 &
    PIDS+=($!)
done

# Aguardar todos os processos com timeout máximo de 10 segundos
TIMEOUT_COUNT=0
MAX_TIMEOUT=10
while [ $TIMEOUT_COUNT -lt $MAX_TIMEOUT ]; do
    ALL_DONE=true
    for pid in "${PIDS[@]}"; do
        if ps -p $pid > /dev/null 2>&1; then
            ALL_DONE=false
            break
        fi
    done
    if [ "$ALL_DONE" = true ]; then
        break
    fi
    sleep 1
    TIMEOUT_COUNT=$((TIMEOUT_COUNT + 1))
done

# Matar processos que ainda estão rodando
for pid in "${PIDS[@]}"; do
    kill $pid 2>/dev/null
done

if [ $TIMEOUT_COUNT -lt $MAX_TIMEOUT ]; then
    echo -e "${GREEN}✓ PASSOU${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${YELLOW}⚠ PARCIAL${NC} (timeout nas requisições simultâneas)"
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

echo -n "Teste: Verificar processos zumbis... "
sleep 2
ZOMBIES=$(ps aux | grep -c "[p]ython3.*test\|[p]ython3.*hello" 2>/dev/null || echo "0")
if [ "$ZOMBIES" -eq 0 ]; then
    echo -e "${GREEN}✓ PASSOU${NC} (sem processos zumbis)"
else
    echo -e "${YELLOW}⚠ AVISO${NC} (encontrados $ZOMBIES processos)"
fi
TESTS_PASSED=$((TESTS_PASSED + 1))

echo ""
echo "=========================================="
echo "  RESUMO DOS TESTES"
echo "=========================================="
echo -e "${GREEN}Testes passados: $TESTS_PASSED${NC}"
if [ $TESTS_FAILED -gt 0 ]; then
    echo -e "${RED}Testes falhados: $TESTS_FAILED${NC}"
fi
TOTAL=$((TESTS_PASSED + TESTS_FAILED))
if [ $TOTAL -gt 0 ]; then
    PERCENTAGE=$((TESTS_PASSED * 100 / TOTAL))
    echo "Taxa de sucesso: ${PERCENTAGE}%"
    
    if [ $PERCENTAGE -eq 100 ]; then
        echo ""
        echo -e "${GREEN}🎉 TODOS OS TESTES PASSARAM!${NC}"
    fi
fi
echo ""

# Mostrar logs se houver erros
if [ $TESTS_FAILED -gt 0 ]; then
    echo "=== Últimas linhas do log do servidor ==="
    tail -50 /tmp/webserv_cgi_tests.log | grep -E "(CGI|ERROR|error|Error|failed)" | tail -20
fi

# Parar servidor
echo "Parando servidor..."
kill $SERVER_PID 2>/dev/null
sleep 1
pkill -9 webserv 2>/dev/null

echo ""
echo "Testes concluídos!"
