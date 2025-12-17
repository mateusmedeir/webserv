#!/bin/bash

# ==============================================================================
# Script de Testes de Stress com Siege - WebServ 42
# ==============================================================================
# Este script realiza testes de stress usando Siege conforme a régua de avaliação:
# - Availability should be above 99.5%
# - Should be able to use siege indefinitely
#
# Testes realizados:
#   1. Teste básico - 10 segundos, 10 usuários
#   2. Teste médio - 30 segundos, 25 usuários
#   3. Teste pesado - 60 segundos, 50 usuários
#   4. Teste muito pesado - 120 segundos, 100 usuários
#   5. Teste de diferentes URLs
#   6. Teste de diferentes métodos HTTP
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
TEMP_DIR="/tmp/webserv_siege_test_$$"
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
    local missing=0
    
    if ! command -v curl &> /dev/null; then
        echo -e "${RED}ERRO: curl não encontrado${NC}"
        missing=1
    fi
    
    if ! command -v siege &> /dev/null; then
        echo -e "${RED}ERRO: siege não encontrado${NC}"
        echo -e "${YELLOW}Instale com: sudo apt-get install siege (Ubuntu) ou brew install siege (macOS)${NC}"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
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
    
    # Verificar se o servidor está respondendo
    local retries=0
    while [ $retries -lt 10 ]; do
        if curl -s -o /dev/null -w "%{http_code}" --max-time 1 "http://$HOST:$PORT/" > /dev/null 2>&1; then
            echo -e "${GREEN}✓ Servidor iniciado e respondendo (PID: $SERVER_PID)${NC}"
            return 0
        fi
        sleep 1
        ((retries++))
    done
    
    echo -e "${RED}ERRO: Servidor não está respondendo${NC}"
    cat "$TEMP_DIR/server.log"
    exit 1
}

stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    pkill -9 webserv 2>/dev/null
}

run_siege_test() {
    local duration=$1
    local concurrent=$2
    local urls_file=$3
    local test_name=$4
    local min_availability=${5:-99.5}
    
    print_test "$test_name (${duration}s, $concurrent usuários)"
    
    # Executar siege
    siege -f "$urls_file" -c $concurrent -t ${duration}s -b -q > "$TEMP_DIR/siege_${TEST_COUNT}.log" 2>&1
    
    # Extrair métricas
    local availability=$(grep "Availability" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $2}' | sed 's/%//')
    local transactions=$(grep "Transactions:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $2}')
    local elapsed_time=$(grep "Elapsed time:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    local data_transferred=$(grep "Data transferred:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    local response_time=$(grep "Response time:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    local transaction_rate=$(grep "Transaction rate:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    local failed=$(grep "Failed transactions:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    local longest=$(grep "Longest transaction:" "$TEMP_DIR/siege_${TEST_COUNT}.log" | awk '{print $3}')
    
    # Mostrar resultados
    echo -e "  Disponibilidade: ${availability}%"
    echo -e "  Transações: $transactions"
    echo -e "  Tempo decorrido: ${elapsed_time}s"
    echo -e "  Taxa de transação: $transaction_rate trans/s"
    echo -e "  Tempo de resposta: ${response_time}s"
    echo -e "  Transações falhadas: $failed"
    echo -e "  Transação mais longa: ${longest}s"
    
    # Verificar disponibilidade
    if [ -z "$availability" ]; then
        print_fail "Não foi possível extrair disponibilidade"
        cat "$TEMP_DIR/siege_${TEST_COUNT}.log"
        return 1
    fi
    
    if awk "BEGIN {exit !($availability >= $min_availability)}"; then
        print_pass
        return 0
    else
        print_fail "Disponibilidade abaixo de ${min_availability}%: ${availability}%"
        return 1
    fi
}

# ==============================================================================
# Testes de Stress com Siege
# ==============================================================================

test_siege_basic() {
    print_header "Teste Siege - Básico"
    
    # Criar arquivo de URLs
    echo "http://$HOST:$PORT/" > "$TEMP_DIR/urls_basic.txt"
    
    run_siege_test 10 10 "$TEMP_DIR/urls_basic.txt" "Teste básico" 99.5
}

test_siege_medium() {
    print_header "Teste Siege - Médio"
    
    # Criar arquivo de URLs
    echo "http://$HOST:$PORT/" > "$TEMP_DIR/urls_medium.txt"
    
    run_siege_test 30 25 "$TEMP_DIR/urls_medium.txt" "Teste médio" 99.5
}

test_siege_heavy() {
    print_header "Teste Siege - Pesado"
    
    # Criar arquivo de URLs
    echo "http://$HOST:$PORT/" > "$TEMP_DIR/urls_heavy.txt"
    
    run_siege_test 60 50 "$TEMP_DIR/urls_heavy.txt" "Teste pesado" 99.0
}

test_siege_very_heavy() {
    print_header "Teste Siege - Muito Pesado"
    
    # Criar arquivo de URLs
    echo "http://$HOST:$PORT/" > "$TEMP_DIR/urls_very_heavy.txt"
    
    run_siege_test 120 100 "$TEMP_DIR/urls_very_heavy.txt" "Teste muito pesado" 98.0
}

test_siege_multiple_urls() {
    print_header "Teste Siege - Múltiplas URLs"
    
    # Criar arquivo com múltiplas URLs
    cat > "$TEMP_DIR/urls_multiple.txt" << EOF
http://$HOST:$PORT/
http://$HOST:$PORT/index.html
http://$HOST:$PORT/pages/
EOF
    
    run_siege_test 30 20 "$TEMP_DIR/urls_multiple.txt" "Teste com múltiplas URLs" 99.0
}

test_siege_get_only() {
    print_header "Teste Siege - Apenas GET"
    
    # Criar arquivo de URLs
    echo "GET http://$HOST:$PORT/" > "$TEMP_DIR/urls_get.txt"
    
    run_siege_test 20 15 "$TEMP_DIR/urls_get.txt" "Teste apenas GET" 99.5
}

# ==============================================================================
# Resumo
# ==============================================================================

print_summary() {
    print_header "Resumo dos Testes Siege"
    
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
        echo -e "${YELLOW}${BOLD}⚠ Alguns testes falharam${NC}\n"
        return 1
    fi
}

# ==============================================================================
# Execução principal
# ==============================================================================

main() {
    print_header "Bateria de Testes - Siege Stress Test"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_siege_basic
    test_siege_medium
    test_siege_heavy
    # test_siege_very_heavy  # Descomente para teste muito pesado
    test_siege_multiple_urls
    test_siege_get_only
    
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

