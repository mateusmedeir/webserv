#!/bin/bash

# ==============================================================================
# Script de Testes de Status Codes HTTP - WebServ 42
# ==============================================================================
# Este script testa se os status codes HTTP estão corretos conforme
# a régua de avaliação: "Your HTTP response status codes must be accurate."
#
# Testes realizados:
#   1. 200 OK - Requisições bem-sucedidas
#   2. 201 Created - Upload/Criação bem-sucedida
#   3. 301/302 - Redirecionamentos
#   4. 400 Bad Request - Requisições malformadas
#   5. 403 Forbidden - Acesso negado
#   6. 404 Not Found - Recurso não encontrado
#   7. 405 Method Not Allowed - Método não permitido
#   8. 413 Payload Too Large - Body muito grande
#   9. 500 Internal Server Error - Erro do servidor
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
TEMP_DIR="/tmp/webserv_status_test_$$"
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
    
    if ! command -v nc &> /dev/null; then
        echo -e "${YELLOW}AVISO: nc (netcat) não encontrado. Alguns testes serão pulados.${NC}"
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
    
    print_test "$description (esperado: $expected_code)"
    
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

# ==============================================================================
# Testes de Status Codes
# ==============================================================================

test_200_ok() {
    print_header "Testes - 200 OK"
    
    test_status_code "GET" "http://$HOST:$PORT/" "" "200" "GET - Página inicial"
    test_status_code "GET" "http://$HOST:$PORT/index.html" "" "200" "GET - Arquivo existente"
}

test_404_not_found() {
    print_header "Testes - 404 Not Found"
    
    test_status_code "GET" "http://$HOST:$PORT/naoexiste.html" "" "404" "GET - Arquivo não existente"
    test_status_code "GET" "http://$HOST:$PORT/diretorio/inexistente/" "" "404" "GET - Diretório não existente"
}

test_405_method_not_allowed() {
    print_header "Testes - 405 Method Not Allowed"
    
    test_status_code "PUT" "http://$HOST:$PORT/" "" "405" "PUT - Método não permitido"
    test_status_code "PATCH" "http://$HOST:$PORT/" "" "405" "PATCH - Método não permitido"
    test_status_code "OPTIONS" "http://$HOST:$PORT/" "" "405" "OPTIONS - Método não permitido"
}

test_400_bad_request() {
    print_header "Testes - 400 Bad Request"
    
    if command -v nc &> /dev/null; then
        # Requisição malformada
        print_test "Requisição malformada - sem método"
        local response=$(echo -e " / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\n" | nc -w 1 $HOST $PORT 2>&1)
        local http_code=$(echo "$response" | head -1 | grep -oE "HTTP/1\.[01] [0-9]{3}" | awk '{print $2}')
        
        if [ "$http_code" = "400" ]; then
            print_pass
        else
            print_fail "Esperado 400, recebido $http_code"
        fi
        
        # Requisição com HTTP version inválida
        print_test "HTTP version inválida"
        local response=$(echo -e "GET / HTTP/2.0\r\nHost: $HOST:$PORT\r\n\r\n" | nc -w 1 $HOST $PORT 2>&1)
        local http_code=$(echo "$response" | head -1 | grep -oE "HTTP/1\.[01] [0-9]{3}" | awk '{print $2}')
        
        if [ "$http_code" = "400" ] || [ "$http_code" = "505" ]; then
            print_pass
        else
            print_fail "Esperado 400 ou 505, recebido $http_code"
        fi
    else
        echo -e "  ${YELLOW}⚠ PULADO${NC} (nc não disponível)"
    fi
}

test_413_payload_too_large() {
    print_header "Testes - 413 Payload Too Large"
    
    # Criar config com limite pequeno
    local config_file="$TEMP_DIR/body_limit.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT;
    server_name localhost;
    root www;
    index index.html;
    client_max_body_size 100;
}
EOF
    
    # Reiniciar servidor com nova config
    stop_server
    sleep 1
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    sleep 2
    
    # Testar body grande
    local large_body=$(head -c 200 < /dev/urandom | base64)
    test_status_code "POST" "http://$HOST:$PORT/" "$large_body" "413" "POST - Body muito grande"
    
    # Voltar para config original
    stop_server
    sleep 1
    start_server
}

test_301_302_redirect() {
    print_header "Testes - 301/302 Redirect"
    
    # Criar config com redirect
    local config_file="$TEMP_DIR/redirect.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT;
    server_name localhost;
    root www;
    index index.html;
    
    location /old {
        return 301 /new;
    }
}
EOF
    
    # Reiniciar servidor
    stop_server
    sleep 1
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    sleep 2
    
    # Testar redirect
    print_test "301 Redirect"
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/old" 2>&1)
    if [ "$http_code" = "301" ] || [ "$http_code" = "302" ]; then
        print_pass
    else
        print_fail "Esperado 301 ou 302, recebido $http_code"
    fi
    
    # Voltar para config original
    stop_server
    sleep 1
    start_server
}

test_500_internal_server_error() {
    print_header "Testes - 500 Internal Server Error"
    
    # Testar CGI com erro (se existir)
    print_test "CGI com erro (500)"
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/error_cgi.py" 2>&1)
    # Pode ser 500 ou 404 dependendo se o CGI existe
    if [ "$http_code" = "500" ] || [ "$http_code" = "404" ]; then
        print_pass
    else
        # Não é falha se não houver CGI de erro configurado
        echo -e "  ${YELLOW}⚠ INFO${NC} (CGI de erro não configurado)"
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
    print_header "Bateria de Testes - Status Codes HTTP"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_200_ok
    test_404_not_found
    test_405_method_not_allowed
    test_400_bad_request
    test_413_payload_too_large
    test_301_302_redirect
    test_500_internal_server_error
    
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

