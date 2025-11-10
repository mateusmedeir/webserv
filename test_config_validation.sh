#!/bin/bash

# ==============================================================================
# Script de Validação de Configuração - WebServ 42
# ==============================================================================
# Este script testa a validação de arquivos de configuração, conforme
# mencionado na régua de avaliação.
#
# Testes realizados:
#   1. Port duplicado (não deve funcionar)
#   2. Configuração inválida
#   3. Arquivo de configuração não existente
#   4. Sintaxe inválida
#   5. Valores inválidos (portas, paths, etc)
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
TEMP_DIR="/tmp/webserv_config_val_$$"
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

compile_project() {
    print_header "Compilando Projeto"
    cd "$SCRIPT_DIR"
    if ! make workflow > /dev/null 2>&1; then
        echo -e "${RED}ERRO: Falha na compilação${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Compilação OK${NC}"
}

test_config_should_fail() {
    local config_file=$1
    local description=$2
    
    print_test "$description"
    
    mkdir -p "$TEMP_DIR"
    
    # Tentar iniciar servidor com config inválida
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    local server_pid=$!
    
    sleep 2
    
    # Servidor não deve estar rodando
    if ! ps -p $server_pid > /dev/null 2>&1; then
        print_pass
        kill $server_pid 2>/dev/null
        return 0
    else
        print_fail "Servidor iniciou com configuração inválida"
        kill $server_pid 2>/dev/null
        return 1
    fi
}

test_config_should_work() {
    local config_file=$1
    local description=$2
    
    print_test "$description"
    
    mkdir -p "$TEMP_DIR"
    
    # Tentar iniciar servidor com config válida
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    local server_pid=$!
    
    sleep 2
    
    # Servidor deve estar rodando
    if ps -p $server_pid > /dev/null 2>&1; then
        print_pass
        kill $server_pid 2>/dev/null
        wait $server_pid 2>/dev/null
        return 0
    else
        print_fail "Servidor não iniciou com configuração válida"
        cat "$TEMP_DIR/server.log"
        return 1
    fi
}

# ==============================================================================
# Testes de Validação
# ==============================================================================

test_duplicate_ports() {
    print_header "Testes - Ports Duplicados"
    
    # Criar config com port duplicado
    local config_file="$TEMP_DIR/duplicate_port.conf"
    cat > "$config_file" << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root www;
}

server {
    listen 8080;
    server_name localhost;
    root www;
}
EOF
    
    # Segundo server com mesmo port não deve funcionar
    test_config_should_fail "$config_file" "Port duplicado - não deve funcionar"
}

test_nonexistent_config() {
    print_header "Testes - Arquivo de Configuração Não Existente"
    
    local fake_config="$TEMP_DIR/naoexiste.conf"
    
    print_test "Arquivo de configuração não existente"
    
    "$SERVER_BIN" "$fake_config" > "$TEMP_DIR/server.log" 2>&1 &
    local server_pid=$!
    
    sleep 2
    
    if ! ps -p $server_pid > /dev/null 2>&1; then
        print_pass
    else
        print_fail "Servidor iniciou com arquivo inexistente"
        kill $server_pid 2>/dev/null
    fi
}

test_invalid_syntax() {
    print_header "Testes - Sintaxe Inválida"
    
    # Config com sintaxe inválida
    local config_file="$TEMP_DIR/invalid_syntax.conf"
    cat > "$config_file" << 'EOF'
server {
    listen 8080
    server_name localhost
    root www
    # Faltando chaves de fechamento
EOF
    
    test_config_should_fail "$config_file" "Sintaxe inválida - faltando chaves"
    
    # Config com valores inválidos
    local config_file2="$TEMP_DIR/invalid_values.conf"
    cat > "$config_file2" << 'EOF'
server {
    listen -1;
    server_name localhost;
    root www;
}
EOF
    
    test_config_should_fail "$config_file2" "Valores inválidos - port negativo"
}

test_invalid_paths() {
    print_header "Testes - Paths Inválidos"
    
    # Root path não existente
    local config_file="$TEMP_DIR/invalid_root.conf"
    cat > "$config_file" << EOF
server {
    listen 8080;
    server_name localhost;
    root /caminho/que/nao/existe;
}
EOF
    
    # Pode ou não falhar dependendo da implementação
    print_test "Root path não existente"
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    local server_pid=$!
    sleep 2
    
    if ps -p $server_pid > /dev/null 2>&1; then
        # Servidor pode iniciar mesmo com path inválido
        echo -e "  ${YELLOW}⚠ INFO${NC} (servidor iniciou, mas pode ter problemas)"
        kill $server_pid 2>/dev/null
    else
        print_pass
    fi
}

test_valid_config() {
    print_header "Testes - Configuração Válida"
    
    # Config válida básica
    local config_file="$TEMP_DIR/valid.conf"
    cat > "$config_file" << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root www;
    index index.html;
}
EOF
    
    test_config_should_work "$config_file" "Configuração válida - deve funcionar"
}

test_multiple_valid_ports() {
    print_header "Testes - Múltiplos Ports Válidos"
    
    # Config com múltiplos ports diferentes
    local config_file="$TEMP_DIR/multiple_ports.conf"
    cat > "$config_file" << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root www;
}

server {
    listen 8081;
    server_name localhost;
    root www;
}

server {
    listen 8082;
    server_name localhost;
    root www;
}
EOF
    
    test_config_should_work "$config_file" "Múltiplos ports diferentes - deve funcionar"
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
    print_header "Bateria de Testes - Validação de Configuração"
    
    # Setup
    compile_project
    
    # Executar testes
    test_duplicate_ports
    test_nonexistent_config
    test_invalid_syntax
    test_invalid_paths
    test_valid_config
    test_multiple_valid_ports
    
    # Limpar processos
    pkill -9 webserv 2>/dev/null
    
    # Resumo
    print_summary
    EXIT_CODE=$?
    
    # Limpar arquivos temporários
    rm -rf "$TEMP_DIR"
    
    exit $EXIT_CODE
}

# Executar
main

