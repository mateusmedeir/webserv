#!/bin/bash

# ==============================================================================
# Script de Testes de Upload de Arquivos - WebServ 42
# ==============================================================================
# Este script testa a funcionalidade de upload de arquivos, conforme
# requisito obrigatório do projeto.
#
# Testes realizados:
#   1. Upload de arquivo pequeno
#   2. Upload de arquivo grande
#   3. Upload múltiplos arquivos
#   4. Verificar arquivo após upload
#   5. Upload com diferentes tipos MIME
#   6. Upload para diretório específico
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
TEMP_DIR="/tmp/webserv_upload_test_$$"
UPLOAD_DIR="$TEMP_DIR/uploads"
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
    mkdir -p "$UPLOAD_DIR"
    
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

# ==============================================================================
# Testes de Upload
# ==============================================================================

test_simple_upload() {
    print_header "Testes - Upload Simples"
    
    # Criar arquivo de teste
    local test_file="$TEMP_DIR/test_simple.txt"
    echo "Conteúdo do arquivo de teste" > "$test_file"
    
    print_test "Upload de arquivo texto simples"
    
    # Fazer upload
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 \
        -X POST "http://$HOST:$PORT/upload" \
        -F "file=@$test_file" 2>&1)
    
    if [ "$http_code" = "200" ] || [ "$http_code" = "201" ]; then
        print_pass
    else
        print_fail "Upload falhou com código HTTP $http_code"
    fi
}

test_upload_and_retrieve() {
    print_header "Testes - Upload e Recuperação"
    
    # Criar arquivo único
    local test_content="Test content $(date +%s)"
    local test_file="$TEMP_DIR/test_retrieve.txt"
    echo "$test_content" > "$test_file"
    local filename=$(basename "$test_file")
    
    print_test "Upload e depois recuperar arquivo"
    
    # Fazer upload
    local upload_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 \
        -X POST "http://$HOST:$PORT/upload" \
        -F "file=@$test_file" 2>&1)
    
    if [ "$upload_code" != "200" ] && [ "$upload_code" != "201" ]; then
        print_fail "Upload falhou: $upload_code"
        return 1
    fi
    
    # Aguardar um pouco
    sleep 1
    
    # Tentar recuperar arquivo
    local response=$(curl -s --max-time 5 "http://$HOST:$PORT/uploads/$filename" 2>&1)
    
    if echo "$response" | grep -q "$test_content"; then
        print_pass
        return 0
    else
        print_fail "Arquivo não foi encontrado ou conteúdo diferente"
        return 1
    fi
}

test_multiple_uploads() {
    print_header "Testes - Upload Múltiplos Arquivos"
    
    # Criar múltiplos arquivos
    local files=()
    for i in {1..3}; do
        local test_file="$TEMP_DIR/test_multi_$i.txt"
        echo "Conteúdo do arquivo $i" > "$test_file"
        files+=("$test_file")
    done
    
    print_test "Upload de múltiplos arquivos"
    
    local all_success=true
    for file in "${files[@]}"; do
        local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 \
            -X POST "http://$HOST:$PORT/upload" \
            -F "file=@$file" 2>&1)
        
        if [ "$http_code" != "200" ] && [ "$http_code" != "201" ]; then
            all_success=false
            break
        fi
    done
    
    if [ "$all_success" = true ]; then
        print_pass
    else
        print_fail "Alguns uploads falharam"
    fi
}

test_large_file_upload() {
    print_header "Testes - Upload de Arquivo Grande"
    
    # Criar arquivo grande (1MB)
    local large_file="$TEMP_DIR/test_large.dat"
    dd if=/dev/urandom of="$large_file" bs=1024 count=1024 2>/dev/null
    
    print_test "Upload de arquivo grande (1MB)"
    
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 30 \
        -X POST "http://$HOST:$PORT/upload" \
        -F "file=@$large_file" 2>&1)
    
    if [ "$http_code" = "200" ] || [ "$http_code" = "201" ]; then
        print_pass
    else
        print_fail "Upload de arquivo grande falhou: $http_code"
    fi
}

test_different_mime_types() {
    print_header "Testes - Upload de Diferentes Tipos MIME"
    
    # Criar arquivos de diferentes tipos
    local test_files=(
        "$TEMP_DIR/test.txt:text/plain"
        "$TEMP_DIR/test.html:text/html"
        "$TEMP_DIR/test.json:application/json"
    )
    
    echo "Plain text" > "$TEMP_DIR/test.txt"
    echo "<html><body>Test</body></html>" > "$TEMP_DIR/test.html"
    echo '{"test": "data"}' > "$TEMP_DIR/test.json"
    
    print_test "Upload de diferentes tipos MIME"
    
    local all_success=true
    for file_mime in "${test_files[@]}"; do
        IFS=':' read -r file mime <<< "$file_mime"
        local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 \
            -X POST "http://$HOST:$PORT/upload" \
            -F "file=@$file" \
            -H "Content-Type: $mime" 2>&1)
        
        if [ "$http_code" != "200" ] && [ "$http_code" != "201" ]; then
            all_success=false
            break
        fi
    done
    
    if [ "$all_success" = true ]; then
        print_pass
    else
        print_fail "Alguns tipos MIME falharam"
    fi
}

test_upload_directory() {
    print_header "Testes - Upload para Diretório Específico"
    
    local test_file="$TEMP_DIR/test_dir.txt"
    echo "Test directory upload" > "$test_file"
    
    print_test "Upload para diretório específico"
    
    # Tentar upload para diretório específico (ajustar conforme config)
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 10 \
        -X POST "http://$HOST:$PORT/uploads/" \
        -F "file=@$test_file" 2>&1)
    
    if [ "$http_code" = "200" ] || [ "$http_code" = "201" ] || [ "$http_code" = "405" ]; then
        print_pass
    else
        print_fail "Upload para diretório falhou: $http_code"
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
    print_header "Bateria de Testes - Upload de Arquivos"
    
    # Setup
    check_dependencies
    compile_project
    start_server
    
    # Executar testes
    test_simple_upload
    test_upload_and_retrieve
    test_multiple_uploads
    test_large_file_upload
    test_different_mime_types
    test_upload_directory
    
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

