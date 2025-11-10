#!/bin/bash

# ==============================================================================
# Script de Testes de Stress - WebServ 42
# ==============================================================================
# Este script realiza testes de stress conforme a régua de avaliação:
# - Availability should be above 99.5%
# - No memory leaks
# - No hanging connections
# - Should be able to use siege indefinitely
#
# Testes realizados:
#   1. Teste básico com curl (múltiplas requisições)
#   2. Teste de requisições simultâneas
#   3. Teste de conexões concorrentes
#   4. Monitoramento de memória
#   5. Teste de disponibilidade
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
TEMP_DIR="/tmp/webserv_stress_test_$$"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Variáveis de stress test
REQUESTS=1000
CONCURRENT=50
DURATION=30

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
        echo -e "${YELLOW}AVISO: siege não encontrado. Alguns testes serão pulados.${NC}"
        echo -e "${YELLOW}Instale com: sudo apt-get install siege (Ubuntu) ou brew install siege (macOS)${NC}"
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
    
    echo -e "${GREEN}✓ Servidor iniciado (PID: $SERVER_PID)${NC}"
}

stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    pkill -9 webserv 2>/dev/null
}

get_memory_usage() {
    if [ -n "$SERVER_PID" ] && ps -p $SERVER_PID > /dev/null 2>&1; then
        ps -o rss= -p $SERVER_PID 2>/dev/null | awk '{print $1/1024}'
    else
        echo "0"
    fi
}

# ==============================================================================
# Testes de Stress
# ==============================================================================

test_basic_stress() {
    print_header "Testes - Stress Básico (curl)"
    
    print_test "Enviar $REQUESTS requisições sequenciais"
    
    local success=0
    local failed=0
    
    for i in $(seq 1 $REQUESTS); do
        local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 2 "http://$HOST:$PORT/" 2>&1)
        if [ "$http_code" = "200" ]; then
            ((success++))
        else
            ((failed++))
        fi
        
        # Mostrar progresso a cada 100 requisições
        if [ $((i % 100)) -eq 0 ]; then
            echo -n "."
        fi
    done
    echo ""
    
    local total=$((success + failed))
    local success_rate=$(awk "BEGIN {printf \"%.2f\", ($success/$total)*100}")
    
    echo -e "  Sucesso: $success/$total (${success_rate}%)"
    
    if awk "BEGIN {exit !($success_rate >= 99.5)}"; then
        print_pass
    else
        print_fail "Taxa de sucesso abaixo de 99.5%: ${success_rate}%"
    fi
}

test_concurrent_requests() {
    print_header "Testes - Requisições Concorrentes"
    
    print_test "Enviar $CONCURRENT requisições simultâneas"
    
    local pids=()
    local success=0
    local failed=0
    
    # Iniciar requisições em background
    for i in $(seq 1 $CONCURRENT); do
        (
            local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/" 2>&1)
            echo "$http_code" > "$TEMP_DIR/result_$i.txt"
        ) &
        pids+=($!)
    done
    
    # Aguardar todas completarem com timeout
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
    
    # Contar resultados
    for i in $(seq 1 $CONCURRENT); do
        if [ -f "$TEMP_DIR/result_$i.txt" ]; then
            local http_code=$(cat "$TEMP_DIR/result_$i.txt")
            if [ "$http_code" = "200" ]; then
                ((success++))
            else
                ((failed++))
            fi
        else
            ((failed++))
        fi
    done
    
    local total=$((success + failed))
    local success_rate=$(awk "BEGIN {printf \"%.2f\", ($success/$total)*100}")
    
    echo -e "  Sucesso: $success/$total (${success_rate}%)"
    
    if awk "BEGIN {exit !($success_rate >= 95)}"; then
        print_pass
    else
        print_fail "Taxa de sucesso abaixo de 95%: ${success_rate}%"
    fi
}

test_memory_leak() {
    print_header "Testes - Memory Leak"
    
    print_test "Monitorar uso de memória durante stress test"
    
    # Memória inicial
    local mem_initial=$(get_memory_usage)
    echo -e "  Memória inicial: ${mem_initial} MB"
    
    # Fazer muitas requisições
    local pids=()
    for i in $(seq 1 500); do
        curl -s -o /dev/null --max-time 1 "http://$HOST:$PORT/" > /dev/null 2>&1 &
        pids+=($!)
        if [ $((i % 50)) -eq 0 ]; then
            sleep 0.1
        fi
    done
    
    # Aguardar todas completarem com timeout
    local timeout=15
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
    
    sleep 2
    
    # Memória final
    local mem_final=$(get_memory_usage)
    echo -e "  Memória final: ${mem_final} MB"
    
    local mem_diff=$(awk "BEGIN {printf \"%.2f\", $mem_final - $mem_initial}")
    echo -e "  Diferença: ${mem_diff} MB"
    
    # Se a memória aumentou mais de 50MB, pode ser leak
    if awk "BEGIN {exit !($mem_diff < 50)}"; then
        print_pass
    else
        print_fail "Possível memory leak: aumento de ${mem_diff} MB"
    fi
}

test_siege_availability() {
    print_header "Testes - Disponibilidade com Siege"
    
    if ! command -v siege &> /dev/null; then
        echo -e "  ${YELLOW}⚠ PULADO${NC} (siege não disponível)"
        return
    fi
    
    print_test "Siege - $DURATION segundos, $CONCURRENT usuários"
    
    # Criar arquivo de URL para siege
    echo "http://$HOST:$PORT/" > "$TEMP_DIR/urls.txt"
    
    # Executar siege
    siege -f "$TEMP_DIR/urls.txt" -c $CONCURRENT -t ${DURATION}s -b > "$TEMP_DIR/siege.log" 2>&1
    
    # Extrair disponibilidade
    local availability=$(grep "Availability" "$TEMP_DIR/siege.log" | awk '{print $2}' | sed 's/%//')
    
    if [ -z "$availability" ]; then
        print_fail "Não foi possível extrair disponibilidade do siege"
        return
    fi
    
    echo -e "  Disponibilidade: ${availability}%"
    
    if awk "BEGIN {exit !($availability >= 99.5)}"; then
        print_pass
    else
        print_fail "Disponibilidade abaixo de 99.5%: ${availability}%"
    fi
}

test_hanging_connections() {
    print_header "Testes - Conexões Travadas"
    
    print_test "Verificar se há conexões travadas após stress"
    
    # Fazer muitas requisições
    local pids=()
    for i in $(seq 1 100); do
        curl -s -o /dev/null --max-time 2 "http://$HOST:$PORT/" > /dev/null 2>&1 &
        pids+=($!)
    done
    
    # Aguardar todas completarem (com timeout)
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
        
        sleep 1
        ((elapsed++))
    done
    
    # Matar processos que ainda estão rodando
    local hanging=0
    for pid in "${pids[@]}"; do
        if ps -p $pid > /dev/null 2>&1; then
            kill $pid 2>/dev/null
            ((hanging++))
        fi
    done
    
    if [ $hanging -eq 0 ]; then
        print_pass
    else
        print_fail "Encontradas $hanging conexões travadas"
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
    print_header "Bateria de Testes - Stress Test"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_basic_stress
    test_concurrent_requests
    test_memory_leak
    test_siege_availability
    test_hanging_connections
    
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

