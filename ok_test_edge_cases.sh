#!/bin/bash

# ==============================================================================
# Script de Testes de Edge Cases - WebServ 42
# ==============================================================================
# Este script testa casos extremos e situações não convencionais que podem
# causar problemas no servidor.
#
# Testes realizados:
#   1. Paths muito longos
#   2. Caracteres especiais em URLs
#   3. Requisições com headers muito grandes
#   4. Múltiplas conexões simultâneas e desconexões
#   5. Timeout de conexões
#   6. Arquivos muito grandes
#   7. Encoding especial (URL encoding, etc)
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
TEMP_DIR="/tmp/webserv_edge_test_$$"
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

test_no_crash() {
    local description=$1
    local command=$2
    
    print_test "$description"
    
    # Executar comando e verificar se servidor ainda está rodando
    eval "$command" > /dev/null 2>&1
    
    sleep 1
    
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        print_pass
        return 0
    else
        print_fail "Servidor crashou"
        return 1
    fi
}

# ==============================================================================
# Testes de Edge Cases
# ==============================================================================

test_long_paths() {
    print_header "Testes - Paths Muito Longos"
    
    # Path com 200 caracteres
    local long_path=$(printf '/%s' $(seq -s '' 1 200))
    test_no_crash "Path muito longo (200+ caracteres)" \
        "curl -s --max-time 2 'http://$HOST:$PORT$long_path' > /dev/null"
    
    # Path com caracteres especiais
    test_no_crash "Path com caracteres especiais" \
        "curl -s --max-time 2 'http://$HOST:$PORT/test%20file%2Fpath.html' > /dev/null"
}

test_special_characters() {
    print_header "Testes - Caracteres Especiais"
    
    # URL encoding
    test_no_crash "URL com encoding especial" \
        "curl -s --max-time 2 'http://$HOST:$PORT/test?param1=hello%20world&param2=test%2Fpath' > /dev/null"
    
    # Caracteres não-ASCII
    test_no_crash "URL com caracteres não-ASCII" \
        "curl -s --max-time 2 'http://$HOST:$PORT/test?text=olá%20mundo' > /dev/null"
    
    # Caracteres de controle
    test_no_crash "URL com caracteres de controle" \
        "curl -s --max-time 2 'http://$HOST:$PORT/test?data=%00%01%02' > /dev/null"
}

test_large_headers() {
    print_header "Testes - Headers Muito Grandes"
    
    # Header muito grande
    local large_header=$(printf 'X-Test: %s\r\n' $(head -c 1000 < /dev/urandom | base64 | tr -d '\n'))
    
    print_test "Requisição com headers muito grandes"
    local response=$(curl -s --max-time 5 \
        -H "X-Large-Header: $(head -c 5000 < /dev/urandom | base64 | tr -d '\n')" \
        "http://$HOST:$PORT/" 2>&1)
    
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        print_pass
    else
        print_fail "Servidor crashou com headers grandes"
    fi
}

test_rapid_connections() {
    print_header "Testes - Conexões Rápidas e Desconexões"
    
    print_test "Múltiplas conexões rápidas e desconexões"
    
    local pids=()
    for i in {1..50}; do
        (
            curl -s --max-time 1 "http://$HOST:$PORT/" > /dev/null 2>&1
        ) &
        pids+=($!)
        sleep 0.01
    done
    
    # Aguardar todos os processos com timeout
    local timeout=10
    local elapsed=0
    while [ $elapsed -lt $timeout ]; do
        local all_done=true
        for pid in "${pids[@]}"; do
            if ps -p $pid > /dev/null 2>&1; then
                all_done=false
                break
            fi
        done
        
        if [ "$all_done" = true ]; then
            break
        fi
        
        sleep 0.5
        elapsed=$((elapsed + 1))
    done
    
    # Matar processos que ainda estão rodando
    for pid in "${pids[@]}"; do
        if ps -p $pid > /dev/null 2>&1; then
            kill $pid 2>/dev/null
        fi
    done
    
    # Aguardar um pouco para garantir que processos foram finalizados
    sleep 1
    
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        print_pass
    else
        print_fail "Servidor crashou com conexões rápidas"
    fi
}

test_incomplete_requests() {
    print_header "Testes - Requisições Incompletas"
    
    if ! command -v nc &> /dev/null; then
        echo -e "  ${YELLOW}⚠ PULADO${NC} (nc não disponível)"
        return
    fi
    
    # Requisição sem \r\n final
    test_no_crash "Requisição sem CRLF final" \
        "echo -n 'GET / HTTP/1.1' | nc -w 1 $HOST $PORT > /dev/null"
    
    # Requisição cortada
    test_no_crash "Requisição cortada" \
        "echo -n 'GET / HTTP/1.1\r\nHost: ' | nc -w 1 $HOST $PORT > /dev/null"
}

test_concurrent_disconnects() {
    print_header "Testes - Desconexões Concorrentes"
    
    print_test "Múltiplas desconexões simultâneas"
    
    local pids=()
    for i in {1..20}; do
        (
            curl -s --max-time 0.5 "http://$HOST:$PORT/" > /dev/null 2>&1
        ) &
        pids+=($!)
    done
    
    # Desconectar todas rapidamente
    sleep 0.2
    for pid in "${pids[@]}"; do
        kill $pid 2>/dev/null
    done
    
    # Aguardar processos finalizarem com timeout
    local timeout=5
    local elapsed=0
    while [ $elapsed -lt $timeout ]; do
        local all_done=true
        for pid in "${pids[@]}"; do
            if ps -p $pid > /dev/null 2>&1; then
                all_done=false
                break
            fi
        done
        
        if [ "$all_done" = true ]; then
            break
        fi
        
        sleep 0.5
        elapsed=$((elapsed + 1))
    done
    
    # Matar processos que ainda estão rodando
    for pid in "${pids[@]}"; do
        if ps -p $pid > /dev/null 2>&1; then
            kill -9 $pid 2>/dev/null
        fi
    done
    
    sleep 1
    
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        print_pass
    else
        print_fail "Servidor crashou com desconexões"
    fi
}

test_very_large_file() {
    print_header "Testes - Arquivo Muito Grande"
    
    # Criar arquivo grande (10MB)
    local large_file="$TEMP_DIR/large_file.dat"
    dd if=/dev/urandom of="$large_file" bs=1M count=10 2>/dev/null
    
    print_test "Servir arquivo muito grande (10MB)"
    
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 30 \
        "http://$HOST:$PORT/large_file.dat" 2>&1)
    
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        # Pode ser 200, 404 ou 413 dependendo da configuração
        if [ "$http_code" = "200" ] || [ "$http_code" = "404" ] || [ "$http_code" = "413" ]; then
            print_pass
        else
            print_fail "Código inesperado: $http_code"
        fi
    else
        print_fail "Servidor crashou"
    fi
}

test_malformed_http() {
    print_header "Testes - HTTP Malformado"
    
    if ! command -v nc &> /dev/null; then
        echo -e "  ${YELLOW}⚠ PULADO${NC} (nc não disponível)"
        return
    fi
    
    # HTTP version inválida
    test_no_crash "HTTP version inválida" \
        "echo -e 'GET / HTTP/9.9\r\nHost: $HOST:$PORT\r\n\r\n' | nc -w 1 $HOST $PORT > /dev/null"
    
    # Método muito longo
    test_no_crash "Método HTTP muito longo" \
        "echo -e 'GET'$(printf 'X%.0s' {1..100})' / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\n' | nc -w 1 $HOST $PORT > /dev/null"
    
    # URI vazia
    test_no_crash "URI vazia" \
        "echo -e 'GET / HTTP/1.1\r\nHost: $HOST:$PORT\r\n\r\n' | nc -w 1 $HOST $PORT > /dev/null"
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
    print_header "Bateria de Testes - Edge Cases"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_long_paths
    test_special_characters
    test_large_headers
    test_rapid_connections
    test_incomplete_requests
    test_concurrent_disconnects
    test_very_large_file
    test_malformed_http
    
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

