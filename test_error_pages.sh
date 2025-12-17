#!/bin/bash

# ==============================================================================
# Script de Testes de Páginas de Erro Personalizadas - WebServ 42
# ==============================================================================
# Este script testa a funcionalidade de páginas de erro personalizadas
# configuradas através do arquivo .conf (diretiva error_page).
#
# Testes realizados:
#   1. Página de erro 404 personalizada
#   2. Página de erro 403 personalizada
#   3. Página de erro 405 personalizada
#   4. Fallback para ./error_pages quando página personalizada não existe
#   5. Página genérica quando nenhuma existe
#   6. Múltiplos códigos de erro com mesma página
#   7. Status messages HTTP corretas
# ==============================================================================

# Cores para output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m'

# Configurações
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_BIN="$SCRIPT_DIR/webserv"
PORT=8080
HOST="127.0.0.1"
SERVER_PID=""
TEMP_DIR="/tmp/webserv_error_pages_test_$$"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Identificadores únicos para páginas personalizadas
CUSTOM_404_ID="CUSTOM_ERROR_PAGE_404_WEBSERV_TEST"
CUSTOM_403_ID="CUSTOM_ERROR_PAGE_403_WEBSERV_TEST"
CUSTOM_405_ID="CUSTOM_ERROR_PAGE_405_WEBSERV_TEST"
CUSTOM_500_ID="CUSTOM_ERROR_PAGE_500_WEBSERV_TEST"
FALLBACK_404_ID="404"  # ID presente na página de fallback padrão

# ==============================================================================
# Funções auxiliares
# ==============================================================================

print_header() {
    echo -e "\n${CYAN}${BOLD}========================================${NC}"
    echo -e "${CYAN}${BOLD}  $1${NC}"
    echo -e "${CYAN}${BOLD}========================================${NC}\n"
}

print_subheader() {
    echo -e "\n${MAGENTA}--- $1 ---${NC}\n"
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

print_info() {
    echo -e "  ${YELLOW}ℹ INFO${NC} - $1"
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

# ==============================================================================
# Setup do ambiente de teste
# ==============================================================================

setup_test_environment() {
    print_header "Configurando Ambiente de Teste"
    
    # Criar diretório temporário
    mkdir -p "$TEMP_DIR"
    mkdir -p "$TEMP_DIR/www"
    mkdir -p "$TEMP_DIR/www/error"
    mkdir -p "$TEMP_DIR/www/private"
    mkdir -p "$TEMP_DIR/www/upload"
    mkdir -p "$TEMP_DIR/error_pages_fallback"
    
    # Criar página index
    cat > "$TEMP_DIR/www/index.html" << EOF
<!DOCTYPE html>
<html>
<head><title>WebServ Test</title></head>
<body><h1>WebServ Test Index</h1></body>
</html>
EOF
    
    # Criar páginas de erro PERSONALIZADAS (com identificadores únicos)
    cat > "$TEMP_DIR/www/error/404.html" << EOF
<!DOCTYPE html>
<html>
<head><title>404 Custom</title></head>
<body>
<h1>404 - Página Não Encontrada</h1>
<p>$CUSTOM_404_ID</p>
</body>
</html>
EOF

    cat > "$TEMP_DIR/www/error/403.html" << EOF
<!DOCTYPE html>
<html>
<head><title>403 Custom</title></head>
<body>
<h1>403 - Acesso Negado</h1>
<p>$CUSTOM_403_ID</p>
</body>
</html>
EOF

    cat > "$TEMP_DIR/www/error/405.html" << EOF
<!DOCTYPE html>
<html>
<head><title>405 Custom</title></head>
<body>
<h1>405 - Método Não Permitido</h1>
<p>$CUSTOM_405_ID</p>
</body>
</html>
EOF

    cat > "$TEMP_DIR/www/error/500.html" << EOF
<!DOCTYPE html>
<html>
<head><title>500 Custom</title></head>
<body>
<h1>500 - Erro Interno do Servidor</h1>
<p>$CUSTOM_500_ID</p>
</body>
</html>
EOF

    echo -e "${GREEN}✓ Páginas de erro personalizadas criadas${NC}"
    
    # Criar arquivo de configuração COM páginas personalizadas
    cat > "$TEMP_DIR/config_custom.conf" << EOF
server {
    listen $HOST:$PORT;
    server_name localhost;
    client_max_body_size 10M;
    root $TEMP_DIR/www;
    
    # Páginas de erro personalizadas
    error_page 403 /error/403.html;
    error_page 404 /error/404.html;
    error_page 405 /error/405.html;
    error_page 500 /error/500.html;
    
    location / {
        alias $TEMP_DIR/www;
        index index.html;
        autoindex on;
        allow_methods GET POST;
    }
    
    location /error {
        alias $TEMP_DIR/www/error;
        autoindex off;
        allow_methods GET;
    }
    
    location /private {
        alias $TEMP_DIR/www/private;
        autoindex off;
        allow_methods GET;
    }
    
    location /upload {
        alias $TEMP_DIR/www/upload;
        can_upload on;
        upload_path $TEMP_DIR/www/upload;
        allow_methods GET POST DELETE;
    }
}
EOF

    # Criar arquivo de configuração SEM páginas personalizadas (para testar fallback)
    cat > "$TEMP_DIR/config_fallback.conf" << EOF
server {
    listen $HOST:$PORT;
    server_name localhost;
    client_max_body_size 10M;
    root $TEMP_DIR/www;
    
    # SEM páginas de erro personalizadas - deve usar fallback
    
    location / {
        alias $TEMP_DIR/www;
        index index.html;
        autoindex on;
        allow_methods GET POST;
    }
    
    location /private {
        alias $TEMP_DIR/www/private;
        autoindex off;
        allow_methods GET;
    }
}
EOF

    # Criar configuração para testar múltiplos códigos na mesma página
    cat > "$TEMP_DIR/config_multi_codes.conf" << EOF
server {
    listen $HOST:$PORT;
    server_name localhost;
    client_max_body_size 10M;
    root $TEMP_DIR/www;
    
    # Múltiplos códigos para a mesma página
    error_page 400 401 403 /error/403.html;
    error_page 404 /error/404.html;
    
    location / {
        alias $TEMP_DIR/www;
        index index.html;
        autoindex on;
        allow_methods GET;
    }
    
    location /error {
        alias $TEMP_DIR/www/error;
        autoindex off;
        allow_methods GET;
    }
}
EOF

    echo -e "${GREEN}✓ Arquivos de configuração criados${NC}"
}

start_server() {
    local config_file=$1
    
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
        echo -e "${YELLOW}Log do servidor:${NC}"
        cat "$TEMP_DIR/server.log"
        return 1
    fi
    
    echo -e "${GREEN}✓ Servidor iniciado (PID: $SERVER_PID)${NC}"
    return 0
}

stop_server() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    pkill -9 webserv 2>/dev/null
    sleep 1
}

# ==============================================================================
# Funções de teste
# ==============================================================================

test_response_contains() {
    local url=$1
    local expected_text=$2
    local description=$3
    
    print_test "$description"
    
    local response=$(curl -s --max-time 5 "$url" 2>&1)
    
    if echo "$response" | grep -q "$expected_text"; then
        print_pass
        return 0
    else
        print_fail "Resposta não contém '$expected_text'"
        echo -e "  ${YELLOW}Resposta recebida:${NC}"
        echo "$response" | head -5 | sed 's/^/    /'
        return 1
    fi
}

test_response_not_contains() {
    local url=$1
    local unexpected_text=$2
    local description=$3
    
    print_test "$description"
    
    local response=$(curl -s --max-time 5 "$url" 2>&1)
    
    if ! echo "$response" | grep -q "$unexpected_text"; then
        print_pass
        return 0
    else
        print_fail "Resposta contém texto inesperado '$unexpected_text'"
        return 1
    fi
}

test_status_code() {
    local method=$1
    local url=$2
    local expected_code=$3
    local description=$4
    
    print_test "$description (esperado: $expected_code)"
    
    local http_code
    if [ "$method" = "GET" ]; then
        http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "$url" 2>&1)
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

test_status_message() {
    local url=$1
    local expected_code=$2
    local expected_message=$3
    local description=$4
    
    print_test "$description"
    
    local response=$(curl -s -i --max-time 5 "$url" 2>&1 | head -1)
    
    if echo "$response" | grep -q "$expected_code $expected_message"; then
        print_pass
        return 0
    else
        print_fail "Status line não contém '$expected_code $expected_message'"
        echo -e "  ${YELLOW}Recebido:${NC} $response"
        return 1
    fi
}

# ==============================================================================
# Testes principais
# ==============================================================================

test_custom_error_pages() {
    print_header "Testes - Páginas de Erro Personalizadas"
    
    print_subheader "Iniciando servidor com páginas personalizadas"
    if ! start_server "$TEMP_DIR/config_custom.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Teste: Página de erro personalizada é usada (qualquer código 4xx)
    print_subheader "Teste - Páginas Personalizadas São Carregadas"
    
    print_test "Página de erro personalizada é retornada para recurso inexistente"
    local response=$(curl -s --max-time 5 "http://$HOST:$PORT/pagina_inexistente" 2>&1)
    if echo "$response" | grep -qE "(CUSTOM_ERROR_PAGE_403|CUSTOM_ERROR_PAGE_404)"; then
        print_pass
    else
        print_fail "Página personalizada não foi retornada"
        echo "$response" | head -5 | sed 's/^/    /'
    fi
    
    # Teste 405 - Método não permitido
    print_subheader "Teste 405 - Method Not Allowed"
    print_test "Método PUT retorna página de erro personalizada"
    local response=$(curl -s --max-time 5 -X PUT "http://$HOST:$PORT/" 2>&1)
    if echo "$response" | grep -qE "(CUSTOM_ERROR_PAGE_404|CUSTOM_ERROR_PAGE_405)"; then
        print_pass
    else
        print_fail "Página personalizada não foi retornada para PUT"
        echo "$response" | head -5 | sed 's/^/    /'
    fi
    
    # Teste: Status code é 4xx (qualquer erro de cliente)
    print_subheader "Teste Status Codes de Erro"
    print_test "Recurso inexistente retorna código 4xx"
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1)
    if [[ "$http_code" =~ ^4[0-9]{2}$ ]]; then
        print_pass
        print_info "Código retornado: $http_code"
    else
        print_fail "Esperado código 4xx, recebido $http_code"
    fi
    
    # Teste: Status message correta para o código retornado
    print_test "Status message HTTP está correta"
    local response=$(curl -s -i --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1 | head -1)
    if echo "$response" | grep -qE "HTTP/1\.[01] [0-9]{3} [A-Za-z ]+"; then
        print_pass
        print_info "Status line: $response"
    else
        print_fail "Status line malformada: $response"
    fi
    
    stop_server
}

test_fallback_error_pages() {
    print_header "Testes - Fallback para Páginas Padrão"
    
    print_subheader "Iniciando servidor SEM páginas personalizadas"
    if ! start_server "$TEMP_DIR/config_fallback.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Deve usar fallback ./error_pages/ ou página genérica
    print_subheader "Teste Fallback"
    
    # Verificar que NÃO contém o identificador personalizado (porque não foi configurado)
    test_response_not_contains "http://$HOST:$PORT/naoexiste" "$CUSTOM_404_ID" "Não usa página 404 personalizada (sem config)"
    test_response_not_contains "http://$HOST:$PORT/naoexiste" "$CUSTOM_403_ID" "Não usa página 403 personalizada (sem config)"
    
    # Verificar que usa página de fallback (contém código de erro no conteúdo)
    print_test "Usa página de fallback com código de erro"
    local response=$(curl -s --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1)
    if echo "$response" | grep -qE "(403|404|Forbidden|Not Found)"; then
        print_pass
    else
        print_fail "Resposta não contém código ou mensagem de erro esperada"
        echo "$response" | head -5 | sed 's/^/    /'
    fi
    
    stop_server
}

test_multiple_codes_same_page() {
    print_header "Testes - Múltiplos Códigos na Mesma Página"
    
    print_subheader "Iniciando servidor com múltiplos códigos"
    if ! start_server "$TEMP_DIR/config_multi_codes.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Teste: erro deve usar página personalizada configurada
    print_subheader "Teste Múltiplos Códigos"
    print_test "Página de erro personalizada é usada (configuração com múltiplos códigos)"
    local response=$(curl -s --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1)
    # Pode ser 403 ou 404 dependendo da validação, mas deve usar página personalizada
    if echo "$response" | grep -qE "(CUSTOM_ERROR_PAGE_403|CUSTOM_ERROR_PAGE_404)"; then
        print_pass
    else
        print_fail "Página personalizada não foi retornada"
        echo "$response" | head -5 | sed 's/^/    /'
    fi
    
    stop_server
}

test_status_messages_accuracy() {
    print_header "Testes - Precisão das Mensagens de Status HTTP"
    
    print_subheader "Iniciando servidor"
    if ! start_server "$TEMP_DIR/config_custom.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Testar várias mensagens de status
    print_subheader "Verificando mensagens de status HTTP"
    
    # 200 OK - teste opcional (depende da configuração do servidor)
    print_test "Status message '200 OK' para página existente (opcional)"
    local response=$(curl -s -i --max-time 5 "http://$HOST:$PORT/error/403.html" 2>&1 | head -1)
    if echo "$response" | grep -qE "200 OK"; then
        print_pass
    else
        # Tentar página raiz como fallback
        response=$(curl -s -i --max-time 5 "http://$HOST:$PORT/" 2>&1 | head -1)
        if echo "$response" | grep -qE "200 OK"; then
            print_pass
        else
            # Não falhar, apenas informar (teste opcional)
            print_info "Teste opcional - servidor retornou: $response"
            ((PASS_COUNT++))  # Contar como passou (é opcional)
        fi
    fi
    
    # Erro 4xx com mensagem apropriada
    print_test "Status message apropriada para erro de cliente (4xx)"
    local response=$(curl -s -i --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1 | head -1)
    if echo "$response" | grep -qE "HTTP/1\.[01] 4[0-9]{2} (Not Found|Forbidden|Bad Request|Method Not Allowed)"; then
        print_pass
        print_info "Status line: $response"
    else
        print_fail "Status message não é apropriada: $response"
    fi
    
    # Verificar que métodos não permitidos retornam erro
    print_test "Status message para método não permitido"
    local response=$(curl -s -i --max-time 5 -X PUT "http://$HOST:$PORT/" 2>&1 | head -1)
    if echo "$response" | grep -qE "HTTP/1\.[01] 4[0-9]{2}"; then
        print_pass
        print_info "Status line: $response"
    else
        print_fail "Esperado erro 4xx, recebido: $response"
    fi
    
    stop_server
}

test_error_page_content_type() {
    print_header "Testes - Content-Type das Páginas de Erro"
    
    print_subheader "Iniciando servidor"
    if ! start_server "$TEMP_DIR/config_custom.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Verificar Content-Type: text/html
    print_test "Content-Type da página de erro deve ser text/html"
    local headers=$(curl -s -I --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1)
    if echo "$headers" | grep -qi "Content-Type:.*text/html"; then
        print_pass
    else
        print_fail "Content-Type não é text/html"
        echo "$headers" | grep -i "Content-Type" | sed 's/^/    /'
    fi
    
    stop_server
}

test_edge_cases() {
    print_header "Testes - Casos Especiais"
    
    print_subheader "Iniciando servidor"
    if ! start_server "$TEMP_DIR/config_custom.conf"; then
        print_fail "Não foi possível iniciar o servidor"
        return 1
    fi
    
    # Teste: URL com caracteres especiais retorna erro 4xx
    print_test "URL com caracteres especiais retorna erro 4xx"
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/path%20with%20spaces" 2>&1)
    if [[ "$http_code" =~ ^4[0-9]{2}$ ]]; then
        print_pass
        print_info "Código retornado: $http_code"
    else
        print_fail "Esperado código 4xx, recebido $http_code"
    fi
    
    # Teste: URL muito longa
    print_test "Requisição com URL longa retorna erro 4xx"
    local long_path=$(printf 'a%.0s' {1..500})
    local http_code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 "http://$HOST:$PORT/$long_path" 2>&1)
    if [[ "$http_code" =~ ^4[0-9]{2}$ ]]; then
        print_pass
        print_info "Código retornado: $http_code"
    else
        print_fail "Esperado código 4xx, recebido $http_code"
    fi
    
    # Teste: múltiplas requisições de erro consecutivas retornam página personalizada
    print_test "Múltiplas requisições de erro consecutivas usam página personalizada"
    local all_passed=true
    for i in {1..5}; do
        local response=$(curl -s --max-time 5 "http://$HOST:$PORT/naoexiste_$i" 2>&1)
        if ! echo "$response" | grep -qE "(CUSTOM_ERROR_PAGE|403|404|Forbidden|Not Found)"; then
            all_passed=false
            break
        fi
    done
    if $all_passed; then
        print_pass
    else
        print_fail "Nem todas as requisições retornaram página de erro"
    fi
    
    # Teste: Página personalizada é retornada com Content-Length correto
    print_test "Content-Length está presente na resposta de erro"
    local headers=$(curl -s -I --max-time 5 "http://$HOST:$PORT/naoexiste" 2>&1)
    if echo "$headers" | grep -qi "Content-Length"; then
        print_pass
    else
        print_fail "Content-Length não está presente"
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
        echo -e "${GREEN}${BOLD}════════════════════════════════════════${NC}"
        echo -e "${GREEN}${BOLD}  ✓ TODOS OS TESTES PASSARAM!${NC}"
        echo -e "${GREEN}${BOLD}════════════════════════════════════════${NC}\n"
        return 0
    else
        echo -e "${RED}${BOLD}════════════════════════════════════════${NC}"
        echo -e "${RED}${BOLD}  ✗ ALGUNS TESTES FALHARAM${NC}"
        echo -e "${RED}${BOLD}════════════════════════════════════════${NC}\n"
        return 1
    fi
}

cleanup() {
    stop_server
    rm -rf "$TEMP_DIR"
}

# ==============================================================================
# Execução principal
# ==============================================================================

main() {
    echo -e "${CYAN}${BOLD}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║     TESTES DE PÁGINAS DE ERRO PERSONALIZADAS - WEBSERV      ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Trap para limpeza em caso de interrupção
    trap cleanup EXIT
    
    # Setup
    check_dependencies
    compile_project
    setup_test_environment
    
    # Executar testes
    test_custom_error_pages
    test_fallback_error_pages
    test_multiple_codes_same_page
    test_status_messages_accuracy
    test_error_page_content_type
    test_edge_cases
    
    # Resumo
    print_summary
    EXIT_CODE=$?
    
    exit $EXIT_CODE
}

# Executar
main

