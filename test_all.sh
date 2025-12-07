#!/bin/bash

# ==============================================================================
# Script Master - Executa Todos os Testes - WebServ 42
# ==============================================================================
# Este script executa todos os scripts de teste em sequência e gera
# um relatório consolidado.
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
REPORT_FILE="$SCRIPT_DIR/test_report_$(date +%Y%m%d_%H%M%S).txt"

# Lista de scripts de teste
TEST_SCRIPTS=(
    "test_http_methods.sh"
    "test_configuration.sh"
    "test_file_upload.sh"
    "test_status_codes.sh"
    "test_config_validation.sh"
    "test_edge_cases.sh"
    "test_cgi_complete.sh"
    "test_cookies_complete.sh"
    "test_stress.sh"
    "test_efficiency.sh"
)

# Estatísticas
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# ==============================================================================
# Funções auxiliares
# ==============================================================================

print_header() {
    echo -e "\n${CYAN}${BOLD}========================================${NC}"
    echo -e "${CYAN}${BOLD}  $1${NC}"
    echo -e "${CYAN}${BOLD}========================================${NC}\n"
}

print_separator() {
    echo -e "${BLUE}────────────────────────────────────────${NC}"
}

# ==============================================================================
# Executar testes
# ==============================================================================

run_all_tests() {
    print_header "Executando Todos os Testes - WebServ 42"
    
    echo -e "Relatório será salvo em: ${CYAN}$REPORT_FILE${NC}\n"
    echo "Iniciando testes em: $(date)" | tee "$REPORT_FILE"
    echo "" | tee -a "$REPORT_FILE"
    
    for script in "${TEST_SCRIPTS[@]}"; do
        local script_path="$SCRIPT_DIR/$script"
        
        if [ ! -f "$script_path" ]; then
            echo -e "${YELLOW}⚠ Script não encontrado: $script${NC}"
            echo "SKIP: $script (não encontrado)" | tee -a "$REPORT_FILE"
            ((SKIPPED_TESTS++))
            continue
        fi
        
        if [ ! -x "$script_path" ]; then
            chmod +x "$script_path"
        fi
        
        print_separator
        echo -e "${BOLD}Executando: ${CYAN}$script${NC}"
        print_separator
        
        echo "" | tee -a "$REPORT_FILE"
        echo "========================================" | tee -a "$REPORT_FILE"
        echo "TESTE: $script" | tee -a "$REPORT_FILE"
        echo "Início: $(date)" | tee -a "$REPORT_FILE"
        echo "========================================" | tee -a "$REPORT_FILE"
        echo "" | tee -a "$REPORT_FILE"
        
        # Executar script e capturar saída
        if "$script_path" 2>&1 | tee -a "$REPORT_FILE"; then
            echo -e "${GREEN}✓ $script - PASSOU${NC}"
            echo "RESULTADO: PASSOU" | tee -a "$REPORT_FILE"
            ((PASSED_TESTS++))
        else
            echo -e "${RED}✗ $script - FALHOU${NC}"
            echo "RESULTADO: FALHOU" | tee -a "$REPORT_FILE"
            ((FAILED_TESTS++))
        fi
        
        echo "" | tee -a "$REPORT_FILE"
        echo "Fim: $(date)" | tee -a "$REPORT_FILE"
        echo "" | tee -a "$REPORT_FILE"
        
        # Pequena pausa entre testes
        sleep 2
    done
    
    TOTAL_TESTS=$((PASSED_TESTS + FAILED_TESTS + SKIPPED_TESTS))
}

# ==============================================================================
# Gerar relatório final
# ==============================================================================

generate_report() {
    print_header "Relatório Final"
    
    echo "" | tee -a "$REPORT_FILE"
    echo "========================================" | tee -a "$REPORT_FILE"
    echo "RELATÓRIO FINAL" | tee -a "$REPORT_FILE"
    echo "========================================" | tee -a "$REPORT_FILE"
    echo "Data: $(date)" | tee -a "$REPORT_FILE"
    echo "" | tee -a "$REPORT_FILE"
    echo "Total de testes: $TOTAL_TESTS" | tee -a "$REPORT_FILE"
    echo "  ${GREEN}✓ Passou:${NC} $PASSED_TESTS" | tee -a "$REPORT_FILE"
    echo "  ${RED}✗ Falhou:${NC} $FAILED_TESTS" | tee -a "$REPORT_FILE"
    echo "  ${YELLOW}⚠ Pulado:${NC} $SKIPPED_TESTS" | tee -a "$REPORT_FILE"
    echo "" | tee -a "$REPORT_FILE"
    
    if [ $TOTAL_TESTS -gt 0 ]; then
        local success_rate=$(awk "BEGIN {printf \"%.2f\", ($PASSED_TESTS/$TOTAL_TESTS)*100}")
        echo "Taxa de sucesso: ${success_rate}%" | tee -a "$REPORT_FILE"
        echo "" | tee -a "$REPORT_FILE"
        
        if [ $FAILED_TESTS -eq 0 ]; then
            echo -e "${GREEN}${BOLD}🎉 TODOS OS TESTES PASSARAM!${NC}" | tee -a "$REPORT_FILE"
            echo "" | tee -a "$REPORT_FILE"
            return 0
        else
            echo -e "${RED}${BOLD}⚠ ALGUNS TESTES FALHARAM${NC}" | tee -a "$REPORT_FILE"
            echo "" | tee -a "$REPORT_FILE"
            return 1
        fi
    else
        echo -e "${YELLOW}Nenhum teste foi executado${NC}" | tee -a "$REPORT_FILE"
        return 1
    fi
}

# ==============================================================================
# Execução principal
# ==============================================================================

main() {
    # Garantir que estamos no diretório correto
    cd "$SCRIPT_DIR"
    
    # Limpar processos antigos
    pkill -9 webserv 2>/dev/null
    
    # Executar todos os testes
    run_all_tests
    
    # Gerar relatório
    generate_report
    EXIT_CODE=$?
    
    echo ""
    echo -e "Relatório completo salvo em: ${CYAN}$REPORT_FILE${NC}"
    echo ""
    
    exit $EXIT_CODE
}

# Executar
main

