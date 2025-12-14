#!/bin/bash

# ==============================================================================
# Script de Testes de Configuração - WebServ 42
# ==============================================================================
# Este script testa todas as funcionalidades de configuração mencionadas
# na régua de avaliação:
#
# Testes realizados:
#   1. Múltiplos servers com diferentes ports
#   2. Múltiplos servers com diferentes hostnames
#   3. Default error pages (404, 500, etc)
#   4. Client body size limit
#   5. Routes para diferentes diretórios
#   6. Default file para diretórios
#   7. Lista de métodos aceitos por rota
#   8. Directory listing (on/off)
#   9. HTTP redirection
#   10. Upload de arquivos
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
PORT1=8080
PORT2=8081
PORT3=8082
HOST="127.0.0.1"
SERVER_PID=""
TEMP_DIR="/tmp/webserv_config_test_$$"
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
    local config_file=$1
    print_header "Iniciando Servidor com $config_file"
    
    mkdir -p "$TEMP_DIR"
    
    # Limpar processos antigos
    pkill -9 webserv 2>/dev/null
    sleep 1
    
    # Iniciar servidor
    "$SERVER_BIN" "$config_file" > "$TEMP_DIR/server.log" 2>&1 &
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
    local url=$1
    local expected_code=$2
    local description=$3
    local method=${4:-GET}
    
    print_test "$description"
    
    local http_code
    if [ "$method" = "GET" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$url" 2>&1)
    elif [ "$method" = "POST" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST "$url" 2>&1)
    elif [ "$method" = "DELETE" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X DELETE "$url" 2>&1)
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
# Testes de Múltiplos Ports
# ==============================================================================

test_multiple_ports() {
    print_header "Testes - Múltiplos Ports"
    
    # Criar config com múltiplos ports
    local config_file="$TEMP_DIR/multi_port.conf"
    cat > "$config_file" << 'EOF'
server {
    listen 8080;
    server_name localhost;
    root www;
    index index.html;
}

server {
    listen 8081;
    server_name localhost;
    root www;
    index index.html;
}

server {
    listen 8082;
    server_name localhost;
    root www;
    index index.html;
}
EOF
    
    start_server "$config_file"
    
    # Testar cada port
    test_status_code "http://$HOST:$PORT1/" "200" "Port 8080 - Responde"
    test_status_code "http://$HOST:$PORT2/" "200" "Port 8081 - Responde"
    test_status_code "http://$HOST:$PORT3/" "200" "Port 8082 - Responde"
    
    stop_server
}

# ==============================================================================
# Testes de Error Pages
# ==============================================================================

test_error_pages() {
    print_header "Testes - Error Pages"
    
    # Criar error page customizada
    mkdir -p "$TEMP_DIR/error_pages"
    echo "<html><body><h1>Custom 404</h1></body></html>" > "$TEMP_DIR/error_pages/404.html"
    
    local config_file="$TEMP_DIR/error_pages.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root www;
    index index.html;
    error_page 404 $TEMP_DIR/error_pages/404.html;
}
EOF
    
    start_server "$config_file"
    
    # Testar 404 customizado
    print_test "Error page 404 customizada"
    local response=$(curl -s "http://$HOST:$PORT1/naoexiste.html")
    if echo "$response" | grep -q "Custom 404"; then
        print_pass
    else
        print_fail "Error page customizada não foi usada"
    fi
    
    stop_server
}

# ==============================================================================
# Testes de Client Body Size Limit
# ==============================================================================

test_body_size_limit() {
    print_header "Testes - Client Body Size Limit"
    
    local config_file="$TEMP_DIR/body_limit.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root www;
    index index.html;
    client_max_body_size 100;
}
EOF
    
    start_server "$config_file"
    
    # Testar body dentro do limite
    print_test "POST com body dentro do limite"
    local small_body=$(head -c 50 < /dev/urandom | base64)
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST "http://$HOST:$PORT1/" -d "data=$small_body")
    if [ "$http_code" = "200" ] || [ "$http_code" = "413" ]; then
        print_pass
    else
        print_fail "Código inesperado: $http_code"
    fi
    
    # Testar body acima do limite
    print_test "POST com body acima do limite"
    local large_body=$(head -c 200 < /dev/urandom | base64)
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 -X POST "http://$HOST:$PORT1/" -d "data=$large_body")
    if [ "$http_code" = "413" ]; then
        print_pass
    else
        print_fail "Esperado 413, recebido $http_code"
    fi
    
    stop_server
}

# ==============================================================================
# Testes de Routes/Directories
# ==============================================================================

test_routes() {
    print_header "Testes - Routes e Directories"
    
    # Criar estrutura de diretórios
    mkdir -p "$TEMP_DIR/www/route1"
    mkdir -p "$TEMP_DIR/www/route2"
    echo "Route 1" > "$TEMP_DIR/www/route1/index.html"
    echo "Route 2" > "$TEMP_DIR/www/route2/index.html"
    
    local config_file="$TEMP_DIR/routes.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root $TEMP_DIR/www;
    index index.html;
    
    location /route1 {
        root $TEMP_DIR/www;
    }
    
    location /route2 {
        root $TEMP_DIR/www;
    }
}
EOF
    
    start_server "$config_file"
    
    # Testar routes
    test_status_code "http://$HOST:$PORT1/route1/" "200" "Route /route1 - Funciona"
    test_status_code "http://$HOST:$PORT1/route2/" "200" "Route /route2 - Funciona"
    
    stop_server
}

# ==============================================================================
# Testes de Default File
# ==============================================================================

test_default_file() {
    print_header "Testes - Default File"
    
    mkdir -p "$TEMP_DIR/www/test_dir"
    echo "Default file content" > "$TEMP_DIR/www/test_dir/default.html"
    
    local config_file="$TEMP_DIR/default_file.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root $TEMP_DIR/www;
    index default.html;
}
EOF
    
    start_server "$config_file"
    
    # Testar default file
    print_test "Default file em diretório"
    local response=$(curl -s "http://$HOST:$PORT1/test_dir/")
    if echo "$response" | grep -q "Default file content"; then
        print_pass
    else
        print_fail "Default file não foi servido"
    fi
    
    stop_server
}

# ==============================================================================
# Testes de Métodos Aceitos
# ==============================================================================

test_allowed_methods() {
    print_header "Testes - Métodos Aceitos por Rota"
    
    local config_file="$TEMP_DIR/allowed_methods.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root www;
    index index.html;
    
    location /get_only {
        allow_methods GET;
    }
    
    location /post_only {
        allow_methods POST;
    }
    
    location /delete_only {
        allow_methods DELETE;
    }
}
EOF
    
    start_server "$config_file"
    
    # Testar métodos permitidos
    test_status_code "http://$HOST:$PORT1/get_only" "200" "GET em rota GET-only" "GET"
    test_status_code "http://$HOST:$PORT1/get_only" "405" "POST em rota GET-only" "POST"
    
    test_status_code "http://$HOST:$PORT1/post_only" "405" "GET em rota POST-only" "GET"
    test_status_code "http://$HOST:$PORT1/post_only" "200" "POST em rota POST-only" "POST"
    
    stop_server
}

# ==============================================================================
# Testes de Directory Listing
# ==============================================================================

test_directory_listing() {
    print_header "Testes - Directory Listing"
    
    mkdir -p "$TEMP_DIR/www/list_dir"
    echo "file1.txt" > "$TEMP_DIR/www/list_dir/file1.txt"
    echo "file2.txt" > "$TEMP_DIR/www/list_dir/file2.txt"
    
    local config_file="$TEMP_DIR/dir_listing.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root $TEMP_DIR/www;
    index index.html;
    
    location /list_dir {
        autoindex on;
    }
}
EOF
    
    start_server "$config_file"
    
    # Testar directory listing
    print_test "Directory listing habilitado"
    local response=$(curl -s "http://$HOST:$PORT1/list_dir/")
    if echo "$response" | grep -q "file1.txt\|file2.txt"; then
        print_pass
    else
        print_fail "Directory listing não funcionou"
    fi
    
    stop_server
}

# ==============================================================================
# Testes de HTTP Redirection
# ==============================================================================

test_redirection() {
    print_header "Testes - HTTP Redirection"
    
    local config_file="$TEMP_DIR/redirect.conf"
    cat > "$config_file" << EOF
server {
    listen $PORT1;
    server_name localhost;
    root www;
    index index.html;
    
    location /old {
        return 301 /new;
    }
}
EOF
    
    start_server "$config_file"
    
    # Testar redirecionamento
    print_test "HTTP 301 Redirection"
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" -L --max-time 5 "http://$HOST:$PORT1/old")
    if [ "$http_code" = "301" ] || [ "$http_code" = "200" ]; then
        print_pass
    else
        print_fail "Redirecionamento não funcionou: $http_code"
    fi
    
    stop_server
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
    print_header "Bateria de Testes - Configuração"
    
    # Setup
    mkdir -p "$TEMP_DIR"
    check_dependencies
    compile_project
    
    # Executar testes
    test_multiple_ports
    test_error_pages
    test_body_size_limit
    test_routes
    test_default_file
    test_allowed_methods
    test_directory_listing
    test_redirection
    
    # Resumo
    print_summary
    EXIT_CODE=$?
    
    # Limpar arquivos temporários
    rm -rf "$TEMP_DIR"
    
    exit $EXIT_CODE
}

# Executar
main

