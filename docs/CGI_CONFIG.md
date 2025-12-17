# Documentação Completa do Sistema CGI

## Visão Geral

O sistema CGI (Common Gateway Interface) é responsável por executar scripts externos (Python, PHP, etc.) em resposta a requisições HTTP, permitindo que o servidor web processe conteúdo dinâmico. O sistema implementa comunicação bidirecional via pipes, processamento assíncrono através de epoll, proteção contra processos travados e gerenciamento adequado de recursos do sistema.

## Fluxo de Processamento

### Fase 1: Detecção de Requisição CGI

O processo inicia quando uma requisição HTTP chega ao servidor. O `HttpResponse` verifica se a URI solicitada corresponde a um script CGI baseado na configuração do location block.

**Tratamentos realizados:**
- Busca do melhor match de location block para a URI
- Verificação se o location possui extensões CGI configuradas (`cgi_extensions`)
- Extração da extensão do arquivo solicitado da URI
- Comparação da extensão com a lista de extensões permitidas no location

**Critérios de detecção:**
- A URI deve terminar com uma extensão configurada (ex: `.py`, `.php`)
- O location block correspondente deve ter `cgi_extensions` definido
- A query string é removida antes da verificação da extensão

**Pontos fortes:**
- Detecção precisa baseada em configuração flexível
- Suporte a múltiplas extensões por location
- Tratamento adequado de query strings

### Fase 2: Determinação do Caminho do Script

Após detectar que a requisição precisa de CGI, o sistema determina o caminho completo do arquivo script no sistema de arquivos.

**Processamento:**
- Remoção da query string da URI (se presente)
- Verificação se o location possui `alias` configurado
- Se `alias` existe: remoção do prefixo do location da URI e construção do caminho usando o alias
- Se `alias` não existe: uso do `root` do server block
- Normalização do caminho (adição de `/` inicial se necessário)

**Validações:**
- Verificação de que o arquivo existe no sistema de arquivos
- Verificação de que o arquivo é um arquivo regular (não diretório)
- Tratamento de erros caso o arquivo não seja encontrado

**Pontos fortes:**
- Suporte flexível a `alias` e `root`
- Normalização automática de caminhos
- Validação rigorosa de existência do arquivo

### Fase 3: Construção do Ambiente CGI

Antes de executar o script, o sistema constrói um conjunto completo de variáveis de ambiente que o script CGI pode utilizar.

**Variáveis obrigatórias:**
- `REQUEST_METHOD`: Método HTTP da requisição (GET, POST, etc.)
- `REQUEST_URI`: URI completa da requisição
- `SCRIPT_NAME`: Nome do script sendo executado
- `PATH_INFO`: Informações de caminho adicionais (se aplicável)
- `PATH_TRANSLATED`: Caminho completo do script no sistema de arquivos
- `SERVER_PROTOCOL`: Protocolo HTTP (HTTP/1.1)
- `SERVER_SOFTWARE`: Nome e versão do servidor (WebServ/1.0)
- `GATEWAY_INTERFACE`: Interface CGI (CGI/1.1)

**Variáveis condicionais:**
- `QUERY_STRING`: String de query (apenas se presente na URI)
- `CONTENT_TYPE`: Tipo de conteúdo do body (apenas se presente)
- `CONTENT_LENGTH`: Tamanho do body em bytes (apenas se presente)
- `SERVER_PORT`: Porta do servidor (apenas se configurada)
- `SERVER_NAME`: Nome do servidor (do server_name ou "localhost" como padrão)

**Variáveis de headers HTTP:**
- Todos os headers HTTP da requisição são convertidos para variáveis de ambiente
- Formato: `HTTP_<NOME_DO_HEADER>` (com hífens convertidos para underscores)
- Headers convertidos para maiúsculas
- Headers `Content-Type` e `Content-Length` são excluídos (já processados separadamente)

**Tratamentos realizados:**
- Normalização de nomes de headers (maiúsculas, underscores)
- Conversão de valores especiais (localhost, porta padrão)
- Remoção de duplicatas (Content-Type e Content-Length não aparecem como HTTP_*)

**Pontos fortes:**
- Ambiente completo e compatível com padrão CGI
- Conversão automática de headers HTTP
- Tratamento de valores padrão sensatos

### Fase 4: Criação e Inicialização do Processo CGI

O sistema cria um processo filho que executará o script CGI, configurando comunicação via pipes.

#### 4.1 Criação de Pipes

**Processamento:**
- Criação de dois pipes: um para entrada (stdin do CGI) e outro para saída (stdout do CGI)
- Configuração de pipes como non-blocking usando `fcntl`
- Armazenamento dos file descriptors em `CgiProcess`

**Estrutura dos pipes:**
- `_pipeIn[2]`: Pipe para enviar dados ao processo CGI
  - `_pipeIn[0]`: Extremidade de leitura (usada pelo processo filho)
  - `_pipeIn[1]`: Extremidade de escrita (usada pelo processo pai)
- `_pipeOut[2]`: Pipe para receber dados do processo CGI
  - `_pipeOut[0]`: Extremidade de leitura (usada pelo processo pai)
  - `_pipeOut[1]`: Extremidade de escrita (usada pelo processo filho)

**Pontos fortes:**
- Comunicação bidirecional estabelecida
- Pipes non-blocking para operação assíncrona
- Tratamento de erros na criação de pipes

#### 4.2 Fork do Processo

**Processamento:**
- Criação de processo filho usando `fork()`
- Armazenamento do PID do processo filho
- Registro do timestamp de início para controle de timeout

**Tratamentos realizados:**
- Verificação de sucesso do fork
- Limpeza de recursos em caso de falha
- Inicialização de flags de estado (_finished, _inputClosed)

**Pontos fortes:**
- Isolamento do processo CGI do servidor principal
- Rastreamento adequado do PID
- Tratamento robusto de erros

#### 4.3 Configuração do Processo Filho

**Processamento:**
- Redirecionamento de stdin para `_pipeIn[0]`
- Redirecionamento de stdout para `_pipeOut[1]`
- Fechamento de todas as extremidades de pipe não utilizadas
- Fechamento de file descriptors extras (3-1023) para segurança
- Determinação do interpretador baseado na extensão do arquivo
- Execução do script usando `execve()`

**Determinação do interpretador:**
- `.py` → `/usr/bin/python3`
- `.php` → `/usr/bin/php`
- `.pl` → `/usr/bin/perl` (suporte no código, mas não validado pelo parser)
- `.sh` → `/bin/bash` (suporte no código, mas não validado pelo parser)
- Sem extensão → tentativa de execução direta

**Tratamentos realizados:**
- Redirecionamento correto de stdin/stdout
- Limpeza de file descriptors desnecessários
- Tratamento de falha do `execve()` (exit com código de erro)

**Pontos fortes:**
- Isolamento completo do processo filho
- Redirecionamento adequado de I/O
- Segurança através de fechamento de FDs extras

#### 4.4 Configuração do Processo Pai

**Processamento:**
- Fechamento das extremidades de pipe não utilizadas pelo pai
- Fechamento de `_pipeIn[0]` (não lemos do pipe de entrada)
- Fechamento de `_pipeOut[1]` (não escrevemos no pipe de saída)
- Registro do processo em mapas de rastreamento

**Pontos fortes:**
- Limpeza imediata de recursos não utilizados
- Preparação para comunicação unidirecional adequada

### Fase 5: Escrita de Dados no Processo CGI (POST)

Para requisições POST, o sistema envia o body da requisição para o processo CGI através do pipe de entrada.

#### 5.1 Armazenamento do Body

**Processamento:**
- Verificação se a requisição possui body
- Armazenamento do body completo em `_remainingBody` do processo
- Se não houver body (GET), fechamento imediato do pipe de entrada

**Tratamentos realizados:**
- Tratamento especial para requisições GET (fechamento imediato do pipe)
- Armazenamento do body para escrita assíncrona

**Pontos fortes:**
- Otimização para requisições GET (sem espera desnecessária)
- Preparação adequada para escrita assíncrona

#### 5.2 Escrita Assíncrona

**Processamento:**
- Registro do pipe de entrada no epoll com evento `EPOLLOUT`
- Criação de `CgiPipeHandler` para gerenciar eventos do pipe
- Escrita dos dados quando o pipe estiver pronto para escrita
- Tratamento de escrita parcial (atualização de `_remainingBody`)

**Algoritmo de escrita:**
- Tentativa de escrita de todo o `_remainingBody`
- Se escrita completa: fechamento do pipe e marcação de `_inputClosed`
- Se escrita parcial: atualização de `_remainingBody` com dados restantes
- Se pipe não está pronto (EAGAIN): espera por próximo evento EPOLLOUT

**Tratamentos realizados:**
- Tratamento de escrita parcial (non-blocking)
- Fechamento adequado do pipe após escrita completa
- Propagação de EOF ao processo filho (importante para scripts que leem stdin)
- Pequeno delay após fechar pipe para garantir propagação do EOF

**Pontos fortes:**
- Escrita não-bloqueante eficiente
- Tratamento robusto de escrita parcial
- Garantia de que o processo CGI recebe EOF corretamente

### Fase 6: Leitura de Dados do Processo CGI

O sistema lê a saída do processo CGI através do pipe de saída de forma assíncrona.

#### 6.1 Registro no Epoll

**Processamento:**
- Registro do pipe de saída no epoll com evento `EPOLLIN`
- Criação de `CgiPipeHandler` para gerenciar eventos do pipe
- Preparação para leitura assíncrona

**Pontos fortes:**
- Integração com sistema de eventos assíncronos
- Preparação adequada para leitura incremental

#### 6.2 Leitura Assíncrona

**Processamento:**
- Leitura de dados quando o pipe estiver pronto (evento EPOLLIN)
- Leitura em chunks de 8192 bytes
- Acumulação dos dados lidos em `_outputBuffer`
- Detecção de EOF (bytesRead == 0)

**Algoritmo de leitura:**
- Tentativa de leitura de até 8192 bytes
- Se dados lidos: acréscimo ao buffer e continuação
- Se EOF: fechamento do pipe e marcação de `_finished`
- Se EAGAIN: espera por próximo evento EPOLLIN
- Se erro: fechamento do pipe e marcação de `_finished`

**Tratamentos realizados:**
- Leitura não-bloqueante eficiente
- Acumulação incremental de dados
- Detecção adequada de fim de saída (EOF)
- Tratamento de erros de leitura

**Pontos fortes:**
- Leitura eficiente em chunks
- Acumulação adequada de dados
- Detecção precisa de fim de processo

#### 6.3 Detecção de Término do Processo

**Processamento:**
- Verificação periódica se o processo terminou usando `waitpid()` com `WNOHANG`
- Marcação de `_finished` quando processo termina
- Coleta de status de saída do processo (se disponível)

**Tratamentos realizados:**
- Verificação não-bloqueante do status do processo
- Tratamento de processos que terminam antes de fechar stdout
- Tratamento de processos que fecham stdout antes de terminar

**Pontos fortes:**
- Detecção robusta de término do processo
- Tratamento de diferentes cenários de término

### Fase 7: Processamento da Resposta CGI

Após o processo CGI terminar e toda a saída ser lida, o sistema processa a resposta no formato CGI.

#### 7.1 Estrutura da Resposta CGI

A resposta CGI segue o formato padrão:
```
Header1: Value1\r\n
Header2: Value2\r\n
Status: 200 OK\r\n
\r\n
Body content here...
```

**Componentes:**
- **Headers**: Linhas de cabeçalho HTTP separadas por `\r\n`
- **Separador**: Linha vazia (`\r\n\r\n`) separa headers do body
- **Body**: Conteúdo da resposta após o separador

#### 7.2 Parsing da Resposta

**Processamento:**
- Busca do separador `\r\n\r\n` na saída do CGI
- Extração da seção de headers e do body
- Parsing linha por linha dos headers
- Processamento especial do header `Status`

**Parsing de headers:**
- Divisão de cada linha no caractere `:`
- Remoção de espaços em branco do início do valor
- Remoção de `\r` do final das linhas
- Tratamento case-insensitive do header `Status`

**Header Status especial:**
- Formato: `Status: <código> <mensagem>`
- Exemplo: `Status: 404 Not Found`
- Extração do código de status (primeiros 3 caracteres)
- Extração da mensagem (após o 4º caractere)

**Validações:**
- Verificação de existência do separador `\r\n\r\n`
- Tratamento de resposta malformada (erro 502)
- Uso de código 200 como padrão se não especificado

**Pontos fortes:**
- Parsing robusto do formato CGI padrão
- Tratamento adequado do header Status
- Validação de formato de resposta

#### 7.3 Construção da Resposta HTTP

**Processamento:**
- Definição do código de status HTTP (do header Status ou 200 padrão)
- Adição de todos os headers CGI à resposta HTTP
- Definição do body da resposta
- Definição do Content-Type (do header CGI ou "text/html" padrão)

**Tratamentos realizados:**
- Preservação de todos os headers CGI
- Tratamento de Content-Type ausente
- Construção completa da resposta HTTP

**Pontos fortes:**
- Conversão adequada de formato CGI para HTTP
- Preservação de metadados (headers)
- Tratamento de valores padrão

### Fase 8: Limpeza de Recursos

Após processar a resposta, o sistema realiza limpeza completa de todos os recursos associados ao processo CGI.

#### 8.1 Limpeza do Processo

**Processamento:**
- Remoção do processo dos mapas de rastreamento
- Remoção do mapeamento cliente → processo
- Fechamento de pipes ainda abertos
- Deletar instância de `CgiProcess`

**Tratamentos realizados:**
- Limpeza de todos os file descriptors
- Remoção de todas as referências ao processo
- Prevenção de vazamentos de memória

**Pontos fortes:**
- Limpeza completa e adequada
- Prevenção de vazamentos de recursos

#### 8.2 Limpeza de Handlers de Pipe

**Processamento:**
- Remoção dos handlers de pipe do epoll
- Deletar instâncias de `CgiPipeHandler`
- Remoção dos handlers dos mapas de rastreamento

**Tratamentos realizados:**
- Remoção adequada do epoll
- Limpeza de referências aos handlers

**Pontos fortes:**
- Limpeza completa de recursos de I/O
- Prevenção de handlers órfãos no epoll

### Fase 9: Gerenciamento de Timeout

O sistema implementa proteção contra processos CGI que demoram muito para executar.

#### 9.1 Verificação de Timeout

**Processamento:**
- Verificação periódica de todos os processos ativos
- Cálculo do tempo decorrido desde o início do processo
- Comparação com timeout configurado (padrão: 2 segundos)

**Tratamentos realizados:**
- Verificação não-bloqueante de timeout
- Cálculo preciso do tempo decorrido

**Pontos fortes:**
- Proteção contra processos travados
- Verificação eficiente de múltiplos processos

#### 9.2 Encerramento por Timeout

**Processamento:**
- Envio de sinal `SIGTERM` ao processo
- Espera de 100ms para término gracioso
- Se ainda não terminou: envio de `SIGKILL` para forçar término
- Envio de resposta 504 Gateway Timeout ao cliente
- Limpeza completa de recursos

**Tratamentos realizados:**
- Tentativa de término gracioso antes de forçar
- Resposta adequada ao cliente (504)
- Limpeza mesmo em caso de timeout

**Pontos fortes:**
- Proteção robusta contra processos travados
- Término gracioso quando possível
- Resposta adequada ao cliente

### Fase 10: Gerenciamento de Processos Zombies

O sistema implementa limpeza de processos filhos que terminaram mas ainda não foram coletados.

#### 10.1 Coleta de Processos Zombies

**Processamento:**
- Chamada periódica de `waitpid(-1, &status, WNOHANG)`
- Coleta de todos os processos filhos que terminaram
- Verificação se o processo coletado é um processo CGI ativo
- Limpeza de recursos do processo coletado

**Tratamentos realizados:**
- Coleta não-bloqueante de processos
- Verificação de status de saída
- Limpeza adequada mesmo para processos não rastreados

**Pontos fortes:**
- Prevenção de acúmulo de processos zombies
- Coleta eficiente de processos terminados

### Fase 11: Verificação de Processos Pendentes

O sistema verifica processos que podem ter gerado saída mas ainda não foram processados.

#### 11.1 Detecção de Processos Pendentes

**Processamento:**
- Verificação de processos com entrada fechada mas ainda não finalizados
- Tentativa de leitura adicional do pipe de saída
- Verificação se o processo terminou após leitura adicional

**Critérios de processo pendente:**
- `_inputClosed == true` (pipe de entrada fechado)
- `_finished == false` (processo ainda não marcado como terminado)
- `_pipeOut[0] != -1` (pipe de saída ainda aberto)

**Tratamentos realizados:**
- Leitura adicional de dados que possam ter sido gerados
- Verificação de término do processo após leitura
- Processamento de resposta se processo terminou

**Pontos fortes:**
- Detecção de processos que geraram saída mas não foram detectados
- Processamento adequado de respostas atrasadas

## Tratamento de Erros

### Estratégia de Tratamento

O sistema CGI utiliza múltiplas camadas de tratamento de erros, desde validações iniciais até tratamento de falhas durante execução.

### Categorias de Erros Tratados

**Erros de Detecção:**
- URI não corresponde a script CGI (extensão não configurada)
- Location block não possui `cgi_extensions` configurado
- Extensão do arquivo não está na lista permitida

**Erros de Arquivo:**
- Script não encontrado no sistema de arquivos
- Script não é um arquivo regular (é diretório)
- Permissões insuficientes para leitura/execução

**Erros de Execução:**
- Falha na criação de pipes
- Falha no fork do processo
- Falha no `execve()` (interpretador não encontrado, script inválido)
- Processo CGI retorna código de erro

**Erros de Comunicação:**
- Falha na escrita no pipe de entrada
- Falha na leitura do pipe de saída
- Pipe fechado inesperadamente
- Timeout do processo

**Erros de Resposta:**
- Resposta CGI malformada (sem separador `\r\n\r\n`)
- Resposta vazia do processo CGI
- Headers inválidos na resposta

**Tratamentos Específicos:**

**Erro 502 Bad Gateway:**
- Usado quando: script não encontrado, execução falha, resposta malformada, resposta vazia
- Resposta ao cliente: página de erro 502

**Erro 504 Gateway Timeout:**
- Usado quando: processo CGI excede timeout configurado
- Resposta ao cliente: página de erro 504
- Processo é encerrado forçadamente

**Pontos fortes:**
- Categorização clara de erros
- Respostas HTTP adequadas para cada tipo de erro
- Limpeza de recursos mesmo em caso de erro
- Mensagens de log descritivas para debugging

## Pontos Fortes da Implementação

### 1. Arquitetura Modular

A implementação separa claramente as responsabilidades:
- **CgiHandler**: Gerencia detecção, execução e limpeza de processos CGI
- **CgiProcess**: Representa um processo CGI individual e gerencia comunicação via pipes
- **CgiPipeHandler**: Gerencia eventos de I/O assíncrono via epoll

Esta separação facilita manutenção, testes e extensibilidade.

### 2. Processamento Assíncrono

O sistema utiliza epoll para processamento assíncrono:
- Não bloqueia o servidor durante execução de scripts
- Permite múltiplos processos CGI simultâneos
- Escrita e leitura não-bloqueantes eficientes
- Integração adequada com o loop de eventos do servidor

### 3. Comunicação Bidirecional Robusta

A implementação de pipes garante:
- Comunicação confiável entre servidor e processo CGI
- Tratamento adequado de escrita parcial
- Detecção precisa de fim de comunicação (EOF)
- Propagação correta de EOF ao processo filho

### 4. Gerenciamento de Recursos Completo

O sistema gerencia recursos adequadamente:
- Limpeza automática de processos e pipes
- Prevenção de vazamentos de memória
- Coleta de processos zombies
- Remoção adequada de handlers do epoll

### 5. Proteção contra Processos Travados

Mecanismos de proteção implementados:
- Timeout configurável (padrão: 2 segundos)
- Término gracioso (SIGTERM) antes de forçar (SIGKILL)
- Resposta adequada ao cliente (504 Gateway Timeout)
- Limpeza de recursos mesmo em caso de timeout

### 6. Ambiente CGI Completo

A construção do ambiente segue o padrão CGI/1.1:
- Todas as variáveis obrigatórias presentes
- Headers HTTP convertidos adequadamente
- Valores padrão sensatos quando não especificados
- Compatibilidade com scripts CGI padrão

### 7. Parsing Robusto de Resposta

O parsing da resposta CGI é robusto:
- Suporte ao formato padrão CGI
- Tratamento especial do header Status
- Validação de formato de resposta
- Tratamento de respostas malformadas

### 8. Tratamento de Casos Extremos

A implementação trata adequadamente:
- Requisições GET sem body (fechamento imediato do pipe)
- Processos que terminam antes de fechar stdout
- Processos que fecham stdout antes de terminar
- Escrita parcial de dados grandes
- Múltiplos processos CGI simultâneos

### 9. Integração com Sistema de Eventos

A integração com epoll é adequada:
- Handlers dedicados para cada pipe
- Registro correto de eventos (EPOLLIN/EPOLLOUT)
- Remoção adequada de handlers após uso
- Tratamento de eventos de forma não-bloqueante

### 10. Suporte a Múltiplos Interpretadores

O sistema suporta múltiplos interpretadores:
- Python 3 (`.py`)
- PHP (`.php`)
- Perl (`.pl`) - suporte no código
- Bash (`.sh`) - suporte no código
- Execução direta (sem extensão)

A determinação do interpretador é automática baseada na extensão do arquivo.

### 11. Rastreamento Completo de Processos

O sistema mantém rastreamento adequado:
- Mapa de processos ativos (PID → CgiProcess)
- Mapa de clientes para processos (clientFd → CgiProcess)
- Mapa de handlers de pipe (pipeFd → CgiPipeHandler)
- Facilita limpeza e gerenciamento

### 12. Logging e Debugging

O sistema fornece logging adequado:
- Mensagens descritivas para cada etapa
- Informações sobre processos (PID, status)
- Logs de erros com contexto
- Facilita debugging e monitoramento

## Conclusão

O sistema CGI implementa uma solução robusta e completa para execução de scripts externos. A arquitetura modular, processamento assíncrono, comunicação bidirecional robusta, gerenciamento adequado de recursos, proteção contra processos travados e tratamento abrangente de erros tornam o sistema confiável e eficiente. A implementação demonstra atenção aos detalhes, consideração de casos extremos e aderência aos padrões CGI, resultando em um sistema que pode lidar com requisições CGI complexas enquanto mantém alta performance e confiabilidade.

