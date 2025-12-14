#!/bin/bash

# ==============================================================================
# Script de Testes de Métodos HTTP - WebServ 42
# ==============================================================================
# Este script testa os métodos HTTP obrigatórios (GET, POST, DELETE) e
# métodos não suportados, conforme a régua de avaliação.
#
# Testes realizados:
#   1. GET requests (simples, com query string, arquivos, diretórios)
#   2. POST requests (simples, upload, body grande)
#   3. DELETE requests (arquivos existentes e não existentes)
#   4. Métodos não suportados (PUT, PATCH, OPTIONS, HEAD, etc)
#   5. Status codes corretos para cada caso
# ==============================================================================

# Cores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# Configurações
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_BIN="$SCRIPT_DIR/webserv"
CONFIG_FILE="$SCRIPT_DIR/configs/test_simple.conf"
PORT=8080
HOST="127.0.0.1"
SERVER_PID=""
TEMP_DIR="/tmp/webserv_http_test_$$"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# ==============================================================================
# Funções auxiliares
# ==============================================================================

print_header() {
    echo -e "\n${CYAN}${BOLD}========================================${NC}"
    echo -e "${CYAN}${BOLD}  $1${NC}"
    echo -e "${CYAN}${BOLD}========================================${NC}\n"
}

print_test() {
    echo -e "${BLUE}[TEST $TEST_COUNT]${NC} $1"
    ((TEST_COUNT++))
}

print_pass() {
    echo -e "  ${GREEN}✓ PASSOU${NC}"
    ((PASS_COUNT++))
}

print_fail() {
    echo -e "  ${RED}✗ FALHOU${NC} - $1"
    ((FAIL_COUNT++))
}

check_dependencies() {
    if ! command -v curl &> /dev/null; then
        echo -e "${RED}ERRO: curl não encontrado${NC}"
        exit 1
    fi
}

compile_project() {
    print_header "Compilando Projeto"
    cd "$SCRIPT_DIR"
    if ! make workflow > /dev/null 2>&1; then
        echo -e "${RED}ERRO: Falha na compilação${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Compilação OK${NC}"
}

start_server() {
    print_header "Iniciando Servidor"
    mkdir -p "$TEMP_DIR"
    
    # Limpar processos antigos
    pkill -9 webserv 2>/dev/null
    sleep 1
    
    # Iniciar servidor
    "$SERVER_BIN" "$CONFIG_FILE" > "$TEMP_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    
    # Aguardar servidor iniciar
    sleep 2
    
    # Verificar se está rodando
    if ! ps -p $SERVER_PID > /dev/null 2>&1; then
        echo -e "${RED}ERRO: Servidor não iniciou${NC}"
        cat "$TEMP_DIR/server.log"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Servidor iniciado (PID: $SERVER_PID)${NC}"
}

stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    pkill -9 webserv 2>/dev/null
}

test_status_code() {
    local method=$1
    local url=$2
    local data=$3
    local expected_code=$4
    local description=$5
    
    print_test "$description"
    
    local http_code
    if [ "$method" = "GET" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$url" 2>&1)
    elif [ "$method" = "DELETE" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X DELETE "$url" 2>&1)
    elif [ "$method" = "POST" ]; then
        if [ -n "$data" ]; then
            http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST "$url" -d "$data" -H "Content-Type: application/x-www-form-urlencoded" 2>&1)
        else
            http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST "$url" 2>&1)
        fi
    else
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X "$method" "$url" 2>&1)
    fi
    
    if [ "$http_code" = "$expected_code" ]; then
        print_pass
        return 0
    else
        print_fail "Esperado HTTP $expected_code, recebido HTTP $http_code"
        return 1
    fi
}

test_response_contains() {
    local method=$1
    local url=$2
    local data=$3
    local expected_text=$4
    local description=$5
    
    print_test "$description"
    
    local response
    if [ "$method" = "GET" ]; then
        response=$(curl -s --max-time 5 "$url" 2>&1)
    elif [ "$method" = "DELETE" ]; then
        response=$(curl -s --max-time 5 -X DELETE "$url" 2>&1)
    elif [ "$method" = "POST" ]; then
        if [ -n "$data" ]; then
            response=$(curl -s --max-time 5 -X POST "$url" -d "$data" -H "Content-Type: application/x-www-form-urlencoded" 2>&1)
        else
            response=$(curl -s --max-time 5 -X POST "$url" 2>&1)
        fi
    else
        response=$(curl -s --max-time 5 -X "$method" "$url" 2>&1)
    fi
    
    if echo "$response" | grep -q "$expected_text"; then
        print_pass
        return 0
    else
        print_fail "Resposta não contém '$expected_text'"
        return 1
    fi
}

# ==============================================================================
# Testes GET
# ==============================================================================

test_get_methods() {
    print_header "Testes GET"
    
    # GET simples
    test_status_code "GET" "http://$HOST:$PORT/" "" "200" "GET - Página inicial"
    
    # GET arquivo existente
    test_status_code "GET" "http://$HOST:$PORT/index.html" "" "200" "GET - Arquivo existente"
    
    # GET arquivo não existente
    test_status_code "GET" "http://$HOST:$PORT/naoexiste.html" "" "404" "GET - Arquivo não existente"
    
    # GET com query string
    test_status_code "GET" "http://$HOST:$PORT/?test=123" "" "200" "GET - Com query string"
    
    # GET diretório (deve retornar index ou 403/404)
    test_status_code "GET" "http://$HOST:$PORT/www/" "" "200" "GET - Diretório"
}

# ==============================================================================
# Testes POST
# ==============================================================================

test_post_methods() {
    print_header "Testes POST"
    
    # POST simples
    test_status_code "POST" "http://$HOST:$PORT/" "test=data" "200" "POST - Requisição simples"
    
    # POST com body
    test_status_code "POST" "http://$HOST:$PORT/" "nome=teste&mensagem=hello" "200" "POST - Com body"
    
    # POST para rota de upload (se existir)
    test_status_code "POST" "http://$HOST:$PORT/upload" "test=data" "200" "POST - Upload"
}

# ==============================================================================
# Testes DELETE
# ==============================================================================

test_delete_methods() {
    print_header "Testes DELETE"
    
    # Criar arquivo temporário para deletar
    mkdir -p "$TEMP_DIR/delete_test"
    echo "test content" > "$TEMP_DIR/delete_test/test_file.txt"
    
    # DELETE arquivo existente (se rota permitir)
    test_status_code "DELETE" "http://$HOST:$PORT/delete_test/test_file.txt" "" "200" "DELETE - Arquivo existente"
    
    # DELETE arquivo não existente
    test_status_code "DELETE" "http://$HOST:$PORT/naoexiste.txt" "" "404" "DELETE - Arquivo não existente"
}

# ==============================================================================
# Testes Métodos Não Suportados
# ==============================================================================

test_unsupported_methods() {
    print_header "Testes Métodos Não Suportados"
    
    # PUT
    test_status_code "PUT" "http://$HOST:$PORT/" "" "405" "PUT - Método não permitido"
    
    # PATCH
    test_status_code "PATCH" "http://$HOST:$PORT/" "" "405" "PATCH - Método não permitido"
    
    # OPTIONS
    test_status_code "OPTIONS" "http://$HOST:$PORT/" "" "405" "OPTIONS - Método não permitido"
    
    # HEAD (pode ser 405 ou 200 dependendo da implementação)
    local head_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X HEAD "http://$HOST:$PORT/" 2>&1)
    if [ "$head_code" = "405" ] || [ "$head_code" = "200" ]; then
        print_test "HEAD - Método"
        print_pass
    else
        print_test "HEAD - Método"
        print_fail "Código inesperado: $head_code"
    fi
    
    # TRACE
    test_status_code "TRACE" "http://$HOST:$PORT/" "" "405" "TRACE - Método não permitido"
    
    # CONNECT
    test_status_code "CONNECT" "http://$HOST:$PORT/" "" "405" "CONNECT - Método não permitido"
}

# ==============================================================================
# Testes de Requisições Inválidas
# ==============================================================================

test_invalid_requests() {
    print_header "Testes Requisições Inválidas"
    
    # Requisição sem método
    print_test "Requisição malformada - sem método"
    local response=$(echo -e " / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\n" | nc -w 1 $HOST $PORT 2>&1)
    if echo "$response" | grep -qE "HTTP/1\.[01] (400|405)"; then
        print_pass
    else
        print_fail "Não retornou 400 ou 405"
    fi
    
    # Requisição com método inválido
    print_test "Requisição com método inválido"
    local response=$(echo -e "INVALID / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\n" | nc -w 1 $HOST $PORT 2>&1)
    if echo "$response" | grep -qE "HTTP/1\.[01] (400|405)"; then
        print_pass
    else
        print_fail "Não retornou 400 ou 405"
    fi
}

# ==============================================================================
# Resumo
# ==============================================================================

print_summary() {
    print_header "Resumo dos Testes"
    
    TOTAL=$((PASS_COUNT + FAIL_COUNT))
    
    echo -e "${BOLD}Estatísticas:${NC}"
    echo -e "  Total de testes: ${CYAN}$TOTAL${NC}"
    echo -e "  ${GREEN}✓ Passou:${NC} $PASS_COUNT"
    echo -e "  ${RED}✗ Falhou:${NC} $FAIL_COUNT"
    echo ""
    
    if [ $FAIL_COUNT -eq 0 ]; then
        echo -e "${GREEN}${BOLD}✓ Todos os testes passaram!${NC}\n"
        return 0
    else
        echo -e "${RED}${BOLD}✗ Alguns testes falharam${NC}\n"
        return 1
    fi
}

# ==============================================================================
# Execução principal
# ==============================================================================

main() {
    print_header "Bateria de Testes - Métodos HTTP"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_get_methods
    test_post_methods
    test_delete_methods
    test_unsupported_methods
    test_invalid_requests
    
    # Limpeza
    stop_server
    
    # Resumo
    print_summary
    EXIT_CODE=$?
    
    # Limpar arquivos temporários
    rm -rf "$TEMP_DIR"
    
    exit $EXIT_CODE
}

# Executar
main

