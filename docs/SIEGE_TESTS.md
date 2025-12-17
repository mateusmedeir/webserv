# Testes de Stress com Siege - WebServ 42

## O que é o teste `test_stress.sh`?

O `test_stress.sh` é um script de testes de stress que verifica se o servidor WebServ atende aos requisitos de performance e disponibilidade da régua de avaliação:

### Testes Realizados:

1. **Requisições Sequenciais (1000 requisições)**
   - Envia 1000 requisições GET sequenciais
   - Verifica se a taxa de sucesso é >= 99.5%

2. **Requisições Concorrentes (50 simultâneas)**
   - Envia 50 requisições GET simultâneas
   - Verifica se a taxa de sucesso é >= 95%

3. **Memory Leak Detection**
   - Monitora o uso de memória durante 500 requisições
   - Verifica se não há vazamento de memória (aumento < 50MB)

4. **Disponibilidade com Siege**
   - Executa Siege por 30 segundos com 50 usuários concorrentes
   - Verifica se a disponibilidade é >= 99.5%

5. **Conexões Travadas**
   - Envia 100 requisições e verifica se todas completam
   - Garante que não há conexões travadas

---

## Instalação do Siege

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install siege
```

### macOS:
```bash
brew install siege
```

### Verificar instalação:
```bash
siege --version
```

---

## Script de Testes com Siege

Criei um script dedicado `test_siege.sh` que realiza testes mais completos com Siege:

### Testes Incluídos:

1. **Teste Básico** - 10 segundos, 10 usuários
2. **Teste Médio** - 30 segundos, 25 usuários
3. **Teste Pesado** - 60 segundos, 50 usuários
4. **Teste Muito Pesado** - 120 segundos, 100 usuários (comentado por padrão)
5. **Múltiplas URLs** - Testa diferentes endpoints simultaneamente
6. **Apenas GET** - Teste focado em requisições GET

### Executar o script:

```bash
./test_siege.sh
```

---

## Uso Manual do Siege

### Comando Básico:

```bash
# Teste básico - 10 segundos, 10 usuários concorrentes
siege -c 10 -t 10s http://127.0.0.1:8080/

# Teste com arquivo de URLs
echo "http://127.0.0.1:8080/" > urls.txt
siege -f urls.txt -c 25 -t 30s

# Teste benchmark (até completar)
siege -c 50 -t 60s -b http://127.0.0.1:8080/
```

### Parâmetros Importantes:

- `-c N` - Número de usuários concorrentes
- `-t Xs` - Duração do teste em segundos
- `-t Xm` - Duração do teste em minutos
- `-f arquivo.txt` - Arquivo com lista de URLs (uma por linha)
- `-b` - Modo benchmark (não espera entre requisições)
- `-q` - Modo quiet (menos output)
- `-v` - Modo verbose (mais detalhes)
- `-i` - Modo interativo (espera entre requisições)

### Exemplo de Arquivo de URLs:

```
http://127.0.0.1:8080/
http://127.0.0.1:8080/index.html
http://127.0.0.1:8080/pages/
GET http://127.0.0.1:8080/
POST http://127.0.0.1:8080/upload
```

---

## Métricas Importantes

O Siege retorna várias métricas importantes:

### Availability (Disponibilidade)
- **Requisito:** >= 99.5%
- Indica a porcentagem de requisições bem-sucedidas
- Calculado como: `(sucessful_transactions / total_transactions) * 100`

### Transactions (Transações)
- Número total de requisições processadas

### Transaction Rate (Taxa de Transação)
- Requisições por segundo
- Indica a capacidade de throughput do servidor

### Response Time (Tempo de Resposta)
- Tempo médio de resposta
- Deve ser baixo para boa performance

### Failed Transactions (Transações Falhadas)
- Número de requisições que falharam
- Deve ser zero ou muito próximo de zero

### Longest Transaction (Transação Mais Longa)
- Tempo da requisição mais lenta
- Útil para identificar problemas de performance

---

## Exemplo de Saída do Siege

```
** SIEGE 4.0.4
** Preparing 50 concurrent users for battle.
The server is now under siege...
Lifting the server siege...      done.

Transactions:                   5000 hits
Availability:                  99.50 %
Elapsed time:                  30.00 secs
Data transferred:               2.50 MB
Response time:                  0.15 secs
Transaction rate:             166.67 trans/sec
Throughput:                     0.08 MB/sec
Concurrency:                   25.00
Successful transactions:        4975
Failed transactions:              25
Longest transaction:            2.50
Shortest transaction:           0.01
```

---

## Testes Recomendados

### 1. Teste de Disponibilidade Básica
```bash
siege -c 10 -t 30s http://127.0.0.1:8080/
```
**Esperado:** Availability >= 99.5%

### 2. Teste de Carga Média
```bash
siege -c 25 -t 60s http://127.0.0.1:8080/
```
**Esperado:** Availability >= 99.5%, sem falhas

### 3. Teste de Carga Pesada
```bash
siege -c 50 -t 120s http://127.0.0.1:8080/
```
**Esperado:** Availability >= 99.0%, sistema estável

### 4. Teste de Stress Extremo
```bash
siege -c 100 -t 300s http://127.0.0.1:8080/
```
**Esperado:** Sistema continua funcionando, mesmo com algumas falhas

### 5. Teste Indefinido (conforme requisito)
```bash
siege -c 50 -t 0 http://127.0.0.1:8080/
```
**Nota:** `-t 0` significa executar indefinidamente. Use Ctrl+C para parar.

---

## Troubleshooting

### Siege não encontrado
```bash
# Ubuntu/Debian
sudo apt-get install siege

# macOS
brew install siege
```

### Porta já em uso
```bash
# Verificar processos na porta 8080
lsof -i :8080

# Matar processo
kill -9 <PID>
```

### Servidor não responde
- Verifique se o servidor está rodando
- Verifique o arquivo de configuração
- Verifique os logs do servidor

### Disponibilidade baixa
- Verifique se há memory leaks
- Verifique se há conexões travadas
- Aumente recursos do sistema se necessário
- Verifique se há problemas no código do servidor

---

## Comparação: test_stress.sh vs test_siege.sh

| Característica | test_stress.sh | test_siege.sh |
|----------------|----------------|---------------|
| Ferramenta principal | curl + Siege | Siege |
| Testes básicos | ✅ | ✅ |
| Testes avançados | ✅ | ✅ |
| Múltiplas URLs | ❌ | ✅ |
| Diferentes métodos | ❌ | ✅ |
| Memory leak check | ✅ | ❌ |
| Conexões travadas | ✅ | ❌ |
| Mais detalhado | ❌ | ✅ |

**Recomendação:** Use ambos os scripts para uma cobertura completa de testes.

