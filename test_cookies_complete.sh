#!/bin/bash

# ==============================================================================
# Script Completo de Testes para Implementação de Cookies
# ==============================================================================
# Este script realiza uma bateria completa de testes para validar que a
# implementação de cookies está funcionando corretamente.
#
# Testes realizados:
#   1. Primeira requisição sem cookie (deve receber Set-Cookie)
#   2. Segunda requisição com cookie (não deve receber Set-Cookie)
#   3. Verificação de atributos (Path=/, HttpOnly)
#   4. Formato do session_id (10 caracteres, apenas letras)
#   5. Persistência entre múltiplas requisições
#   6. Locations com cookies_enabled on/off
#   7. Edge cases (cookie vazio, formato inválido)
# ==============================================================================

# Não usar 'set -e' para permitir que todos os testes sejam executados
# mesmo se alguns falharem

# Cores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Configurações
CONFIG_FILE_BASIC="configs/test_cookies_simple.conf"
CONFIG_FILE_MIXED="configs/test_cookies_mixed.conf"
PORT=8080
HOST="127.0.0.1"
SERVER_PID=""
TEMP_DIR="/tmp/webserv_test_$$"
COOKIE_FILE="$TEMP_DIR/cookies.txt"
SESSION_ID=""
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0
WARN_COUNT=0

# ==============================================================================
# Funções auxiliares
# ==============================================================================

print_header() {
    echo -e "\n${CYAN}${BOLD}========================================${NC}"
    echo -e "${CYAN}${BOLD}  $1${NC}"
    echo -e "${CYAN}${BOLD}========================================${NC}\n"
}

print_test() {
    echo -e "${BLUE}${BOLD}[TEST $TEST_COUNT]${NC} ${BLUE}$1${NC}"
    ((TEST_COUNT++))
}

print_pass() {
    echo -e "${GREEN}✓ PASS${NC} - $1"
    ((PASS_COUNT++))
}

print_fail() {
    echo -e "${RED}✗ FAIL${NC} - $1"
    ((FAIL_COUNT++))
}

print_warn() {
    echo -e "${YELLOW}⚠ WARN${NC} - $1"
    ((WARN_COUNT++))
}

print_info() {
    echo -e "${MAGENTA}ℹ INFO${NC} - $1"
}

# Função para limpar recursos
cleanup() {
    echo -e "\n${YELLOW}Limpando recursos...${NC}"
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
        echo -e "${GREEN}✓ Servidor finalizado${NC}"
    fi
    rm -rf "$TEMP_DIR" 2>/dev/null || true
    echo -e "${GREEN}✓ Limpeza concluída${NC}"
}

# Trap para garantir limpeza ao sair
trap cleanup EXIT INT TERM

# Verificar dependências
check_dependencies() {
    print_header "Verificando Dependências"
    
    if ! command -v curl &> /dev/null; then
        print_fail "curl não encontrado. Por favor, instale curl."
        exit 1
    fi
    print_pass "curl encontrado"
    
    if ! command -v make &> /dev/null; then
        print_fail "make não encontrado. Por favor, instale make."
        exit 1
    fi
    print_pass "make encontrado"
}

# Compilar o projeto
compile_project() {
    print_header "Compilando Projeto"
    
    print_info "Limpando build anterior..."
    make clean > /dev/null 2>&1 || true
    
    print_info "Compilando webserv..."
    if ! make workflow > "$TEMP_DIR/compile.log" 2>&1; then
        print_fail "Erro na compilação!"
        echo -e "${RED}Log de compilação:${NC}"
        cat "$TEMP_DIR/compile.log"
        exit 1
    fi
    
    if [ ! -f "./webserv" ]; then
        print_fail "Executável webserv não foi criado!"
        exit 1
    fi
    
    print_pass "Compilação concluída"
}

# Iniciar servidor
start_server() {
    local config_file=$1
    print_header "Iniciando Servidor"
    
    if [ ! -f "$config_file" ]; then
        print_fail "Arquivo de configuração não encontrado: $config_file"
        exit 1
    fi
    
    print_info "Usando configuração: $config_file"
    print_info "Iniciando servidor na porta $PORT..."
    
    ./webserv "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    
    # Aguardar servidor iniciar
    sleep 2
    
    # Verificar se o servidor está rodando
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        print_fail "Servidor não iniciou corretamente!"
        echo -e "${RED}Log do servidor:${NC}"
        cat "$TEMP_DIR/server.log"
        exit 1
    fi
    
    # Testar se o servidor está respondendo
    if ! curl -s "http://$HOST:$PORT/" > /dev/null 2>&1; then
        print_warn "Servidor iniciado mas não está respondendo. Aguardando mais 2 segundos..."
        sleep 2
    fi
    
    print_pass "Servidor iniciado (PID: $SERVER_PID)"
}

# ==============================================================================
# Testes de Cookies
# ==============================================================================

test_1_first_request_gets_cookie() {
    print_test "Primeira requisição sem cookie (deve receber Set-Cookie)"
    
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" 2>&1)
    
    if echo "$RESPONSE" | grep -qi "Set-Cookie:"; then
        COOKIE_HEADER=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | head -1)
        SESSION_ID=$(echo "$COOKIE_HEADER" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
        
        if [ ! -z "$SESSION_ID" ]; then
            print_pass "Cookie recebido: session_id=$SESSION_ID"
            echo "$COOKIE_HEADER" > "$COOKIE_FILE"
            return 0
        else
            print_fail "Set-Cookie encontrado mas session_id não extraído"
            return 1
        fi
    else
        print_fail "Set-Cookie header não encontrado na primeira requisição"
        echo -e "${YELLOW}Resposta recebida:${NC}"
        echo "$RESPONSE" | head -15
        return 1
    fi
}

test_2_second_request_preserves_cookie() {
    print_test "Segunda requisição com cookie (não deve receber Set-Cookie)"
    
    if [ -z "$SESSION_ID" ]; then
        print_warn "Teste pulado (session_id não disponível)"
        return 2
    fi
    
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" \
        -H "Cookie: session_id=$SESSION_ID" 2>&1)
    
    if echo "$RESPONSE" | grep -qi "Set-Cookie:"; then
        NEW_SESSION_ID=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
        if [ "$NEW_SESSION_ID" = "$SESSION_ID" ]; then
            print_warn "Set-Cookie foi enviado novamente com o mesmo session_id"
            return 2
        else
            print_fail "Novo cookie foi gerado (session_id mudou: $SESSION_ID -> $NEW_SESSION_ID)"
            return 1
        fi
    else
        print_pass "Set-Cookie não enviado (cookie preservado corretamente)"
        return 0
    fi
}

test_3_cookie_attributes() {
    print_test "Verificação de atributos do cookie (Path=/ e HttpOnly)"
    
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" 2>&1)
    COOKIE_HEADER=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | head -1)
    
    if [ -z "$COOKIE_HEADER" ]; then
        print_fail "Cookie não encontrado"
        return 1
    fi
    
    HAS_PATH=false
    HAS_HTTPONLY=false
    
    if echo "$COOKIE_HEADER" | grep -qi "Path=/"; then
        HAS_PATH=true
    fi
    if echo "$COOKIE_HEADER" | grep -qi "HttpOnly"; then
        HAS_HTTPONLY=true
    fi
    
    if [ "$HAS_PATH" = true ] && [ "$HAS_HTTPONLY" = true ]; then
        print_pass "Cookie contém Path=/ e HttpOnly"
        print_info "Cookie completo: $COOKIE_HEADER"
        return 0
    else
        print_fail "Cookie não contém todos os atributos necessários"
        echo -e "  ${YELLOW}Path=/:${NC} $HAS_PATH"
        echo -e "  ${YELLOW}HttpOnly:${NC} $HAS_HTTPONLY"
        echo -e "  ${YELLOW}Cookie:${NC} $COOKIE_HEADER"
        return 1
    fi
}

test_4_session_id_format() {
    print_test "Verificação do formato do session_id (10 caracteres, apenas letras minúsculas)"
    
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" 2>&1)
    COOKIE_HEADER=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | head -1)
    SESSION_ID=$(echo "$COOKIE_HEADER" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
    
    if [ -z "$SESSION_ID" ]; then
        print_fail "session_id não encontrado"
        return 1
    fi
    
    LENGTH=${#SESSION_ID}
    
    # Verificar comprimento
    if [ $LENGTH -ne 10 ]; then
        print_fail "session_id tem tamanho incorreto (esperado: 10, encontrado: $LENGTH)"
        return 1
    fi
    
    # Verificar se contém apenas letras minúsculas
    if [[ ! "$SESSION_ID" =~ ^[a-z]+$ ]]; then
        print_fail "session_id contém caracteres inválidos (deve conter apenas letras minúsculas a-z)"
        print_info "session_id encontrado: '$SESSION_ID'"
        return 1
    fi
    
    print_pass "session_id válido: '$SESSION_ID' ($LENGTH caracteres, apenas letras minúsculas)"
    return 0
}

test_5_multiple_requests_persistence() {
    print_test "Múltiplas requisições com mesmo cookie (persistência)"
    
    if [ -z "$SESSION_ID" ]; then
        print_warn "Teste pulado (session_id não disponível)"
        return 2
    fi
    
    # Fazer 5 requisições com o mesmo cookie
    ALL_SUCCESS=true
    for i in {1..5}; do
        RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" \
            -H "Cookie: session_id=$SESSION_ID" 2>&1)
        
        if [ $? -ne 0 ]; then
            print_fail "Requisição $i falhou"
            ALL_SUCCESS=false
        fi
        
        # Verificar que não recebemos novo cookie
        if echo "$RESPONSE" | grep -qi "Set-Cookie:"; then
            NEW_ID=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
            if [ "$NEW_ID" != "$SESSION_ID" ]; then
                print_fail "Novo cookie gerado na requisição $i"
                ALL_SUCCESS=false
                break
            fi
        fi
    done
    
    if [ "$ALL_SUCCESS" = true ]; then
        print_pass "5 requisições processadas com cookie persistente"
        return 0
    else
        print_fail "Erro na persistência do cookie"
        return 1
    fi
}

test_6_cookies_enabled_off() {
    print_test "Location com cookies_enabled off (não deve receber cookie)"
    
    # Verificar se temos um location sem cookies
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/public" 2>&1)
    
    if echo "$RESPONSE" | grep -qi "Set-Cookie:"; then
        print_fail "Cookie foi enviado em location com cookies_enabled off"
        return 1
    else
        print_pass "Nenhum cookie enviado em location sem cookies habilitados"
        return 0
    fi
}

test_7_empty_cookie_header() {
    print_test "Requisição com cookie vazio (deve gerar novo cookie)"
    
    RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" \
        -H "Cookie: " 2>&1)
    
    if echo "$RESPONSE" | grep -qi "Set-Cookie:"; then
        NEW_SESSION_ID=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
        if [ ! -z "$NEW_SESSION_ID" ]; then
            print_pass "Novo cookie gerado quando cookie header está vazio"
            return 0
        else
            print_fail "Set-Cookie encontrado mas session_id não extraído"
            return 1
        fi
    else
        print_fail "Novo cookie não foi gerado quando cookie header está vazio"
        return 1
    fi
}

test_8_different_locations() {
    print_test "Cookies em diferentes locations"
    
    # Testar location com cookies
    RESPONSE1=$(curl -s -i -X GET "http://$HOST:$PORT/" 2>&1)
    HAS_COOKIE1=false
    if echo "$RESPONSE1" | grep -qi "Set-Cookie:"; then
        HAS_COOKIE1=true
    fi
    
    # Testar location sem cookies (se disponível)
    RESPONSE2=$(curl -s -i -X GET "http://$HOST:$PORT/public" 2>&1)
    HAS_COOKIE2=false
    if echo "$RESPONSE2" | grep -qi "Set-Cookie:"; then
        HAS_COOKIE2=true
    fi
    
    if [ "$HAS_COOKIE1" = true ] && [ "$HAS_COOKIE2" = false ]; then
        print_pass "Cookies respeitam configuração por location"
        return 0
    elif [ "$HAS_COOKIE1" = true ] && [ "$HAS_COOKIE2" = true ]; then
        print_warn "Cookie enviado em ambos os locations (pode ser comportamento esperado)"
        return 2
    else
        print_fail "Comportamento inconsistente entre locations"
        return 1
    fi
}

test_9_cookie_uniqueness() {
    print_test "Unicidade dos session_ids (múltiplas requisições sem cookie)"
    
    # Fazer 3 requisições sem cookie e verificar que cada uma recebe um ID único
    declare -a SESSION_IDS
    
    for i in {1..3}; do
        RESPONSE=$(curl -s -i -X GET "http://$HOST:$PORT/" 2>&1)
        SESSION_ID=$(echo "$RESPONSE" | grep -i "Set-Cookie:" | sed -n 's/.*session_id=\([^;]*\).*/\1/p')
        
        if [ ! -z "$SESSION_ID" ]; then
            SESSION_IDS+=("$SESSION_ID")
        fi
        
        # Pequeno delay para garantir timestamp diferente se usado
        sleep 0.1
    done
    
    # Verificar unicidade
    UNIQUE_COUNT=$(printf '%s\n' "${SESSION_IDS[@]}" | sort -u | wc -l)
    
    if [ ${#SESSION_IDS[@]} -eq 3 ] && [ $UNIQUE_COUNT -eq 3 ]; then
        print_pass "Cada requisição recebeu um session_id único"
        print_info "Session IDs: ${SESSION_IDS[*]}"
        return 0
    elif [ ${#SESSION_IDS[@]} -eq 3 ]; then
        print_warn "Alguns session_ids são iguais (pode ser aceitável para cookies de sessão)"
        print_info "Session IDs: ${SESSION_IDS[*]}"
        return 2
    else
        print_fail "Nem todas as requisições receberam cookies"
        return 1
    fi
}

test_10_http_status() {
    print_test "Status HTTP correto em requisições com cookies"
    
    # Primeira requisição
    STATUS1=$(curl -s -o /dev/null -w "%{http_code}" "http://$HOST:$PORT/")
    
    # Requisição com cookie
    if [ ! -z "$SESSION_ID" ]; then
        STATUS2=$(curl -s -o /dev/null -w "%{http_code}" \
            -H "Cookie: session_id=$SESSION_ID" "http://$HOST:$PORT/")
    else
        STATUS2="000"
    fi
    
    if [ "$STATUS1" = "200" ] && [ "$STATUS2" = "200" ]; then
        print_pass "Status HTTP 200 em requisições com e sem cookie"
        return 0
    else
        print_fail "Status HTTP incorreto (sem cookie: $STATUS1, com cookie: $STATUS2)"
        return 1
    fi
}

# ==============================================================================
# Função principal
# ==============================================================================

run_basic_tests() {
    print_header "Executando Testes Básicos de Cookies"
    
    start_server "$CONFIG_FILE_BASIC"
    
    test_1_first_request_gets_cookie
    test_2_second_request_preserves_cookie
    test_3_cookie_attributes
    test_4_session_id_format
    test_5_multiple_requests_persistence
    test_7_empty_cookie_header
    test_10_http_status
    
    # Finalizar servidor
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    SERVER_PID=""
}

run_advanced_tests() {
    print_header "Executando Testes Avançados"
    
    if [ ! -f "$CONFIG_FILE_MIXED" ]; then
        print_warn "Arquivo de configuração avançado não encontrado. Pulando testes avançados."
        return
    fi
    
    start_server "$CONFIG_FILE_MIXED"
    
    test_6_cookies_enabled_off
    test_8_different_locations
    test_9_cookie_uniqueness
    
    # Finalizar servidor
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    SERVER_PID=""
}

print_summary() {
    print_header "Resumo dos Testes"
    
    TOTAL=$((PASS_COUNT + FAIL_COUNT + WARN_COUNT))
    
    echo -e "${BOLD}Estatísticas:${NC}"
    echo -e "  Total de testes: ${CYAN}$TOTAL${NC}"
    echo -e "  ${GREEN}✓ Passou:${NC} $PASS_COUNT"
    echo -e "  ${RED}✗ Falhou:${NC} $FAIL_COUNT"
    echo -e "  ${YELLOW}⚠ Avisos:${NC} $WARN_COUNT"
    echo ""
    
    if [ $FAIL_COUNT -eq 0 ]; then
        if [ $WARN_COUNT -eq 0 ]; then
            echo -e "${GREEN}${BOLD}✓ Todos os testes passaram perfeitamente!${NC}\n"
            return 0
        else
            echo -e "${GREEN}${BOLD}✓ Todos os testes críticos passaram!${NC}"
            echo -e "${YELLOW}  Há $WARN_COUNT avisos não críticos.${NC}\n"
            return 0
        fi
    else
        echo -e "${RED}${BOLD}✗ Alguns testes falharam${NC}\n"
        echo -e "${YELLOW}Últimas linhas do log do servidor:${NC}"
        tail -20 "$TEMP_DIR/server.log" 2>/dev/null || echo "Log não disponível"
        echo ""
        return 1
    fi
}

# ==============================================================================
# Execução principal
# ==============================================================================

main() {
    print_header "Bateria Completa de Testes - Implementação de Cookies"
    
    # Criar diretório temporário
    mkdir -p "$TEMP_DIR"
    
    # Verificar dependências
    check_dependencies
    
    # Compilar projeto
    compile_project
    
    # Executar testes básicos
    run_basic_tests
    
    # Executar testes avançados
    run_advanced_tests
    
    # Mostrar resumo
    print_summary
    EXIT_CODE=$?
    
    exit $EXIT_CODE
}

# Executar
main

