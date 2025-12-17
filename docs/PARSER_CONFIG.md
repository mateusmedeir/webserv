Meeps
meeps_bot
🔔

furuno — 20/10/2025 09:15
Ah eu queria a avaliação
Meeps

 — 20/10/2025 09:16
Tipo de arquivo em anexo: acrobat
webserv-1.pdf
160.54 KB
Mandei o errado
Foi mal
Testa esse aqui
furuno — 20/10/2025 09:16
Valeuuu
furuno — 06/11/2025 09:46
Azedinho
Me tira uma duvida
O config tem vários servers{} ?
Quando vc fa o parsign foi pegando as informações de cada server
Meeps

 — 06/11/2025 09:49
O arquivo de config pode ter mais de um server
Sim
Server {}

Server {}
furuno — 06/11/2025 09:49
Top
Só queria ter crtz
vc tem um exemplo?
Meeps

 — 06/11/2025 09:49
Vou te mandar pera
Tô chegando em casa
Aliás, vou te mandar algo melhor que isso
furuno — 06/11/2025 09:50
Tranquilo, obrigada!
Tem como fazer com que o servidor rode em um ip que não seja da própria máquina? 
Por que o server pede o ip?
Meeps

 — 06/11/2025 10:26
Você quer um exemplo de .conf?
Meeps

 — 06/11/2025 10:34
server {
    listen localhost:4040;
    listen 127.0.0.1:4041;
    server_name gguedes;

    root ./;
Expandir
message.txt
2 KB

server {
    
    # Maximum allowed size for client request bodies
    client_max_body_size 10M;
Expandir
message.txt
1 KB
# Documentação Completa do Parsing de Arquivo de Configuração

## Visão Geral

O sistema de parsing de configuração é responsável por ler, validar e estruturar arquivos de configuração no formato similar ao nginx. O parser processa arquivos de texto contendo blocos de servidor e localizações, transformando-os em estruturas de dados tipadas e validadas.
Expandir
message.txt
16 KB
Meeps

 — 06/11/2025 10:38
Abre isso aqui como um arquivo .md
Aí tem tudo como o parsing do nosso projeto funciona, sem mostrar código e falando os tratamentos que a gente faz
Meeps

 — 06/11/2025 10:39
O servidor precisa do IP porque usa bind() para vincular o socket a uma interface de rede específica. Isso define em qual endereço o servidor escuta conexões.
Não é possível fazer bind em um IP que não existe na máquina
furuno — 06/11/2025 11:01
Po valeuu
Se eu tiver mais ums duvida te mando
Meeps

 — 06/11/2025 11:01
Kkkk beleza
furuno — 06/11/2025 11:01
Mas a gnt ta quase terminando a web server eu acho
Meeps

 — 06/11/2025 11:02
krl brabos
furuno — 06/11/2025 11:02
vou fazer a parte de cgi essa semana
Meeps

 — 06/11/2025 11:02
Eu to fazendo a parte de CGI agora
furuno — 06/11/2025 11:02
é nada
Meeps

 — 06/11/2025 11:02
Kkkkkkkkkkkkk
pprt
furuno — 06/11/2025 11:02
Entao já sei pra quem pedir ajuda
furuno — 06/11/2025 11:02
se tiver alguma coisa assim me manda
Meeps

 — 06/11/2025 11:03
Eu to fazendo a documentação agora
Mas ainda to arrumando umas paradas e vou fazer uns testes
Se pa semana que vem fica pronto
Aí te mando
Mas nosso cgi é feito pra pegar o bônus
Mas nao muda nada pra vc
furuno — 06/11/2025 11:04
jae
topp
valeu azedinhoo
Meeps

 — 06/11/2025 11:04
valeu tmj
Meeps

 — 10/11/2025 02:45
Um arquivo de .conf completo, caso queira
# ==============================================================================
# WebServ 42 - Arquivo de Configuração Completo e Documentado
# ==============================================================================
# Este arquivo demonstra todas as funcionalidades e diretivas suportadas
# pelo servidor WebServ 42, com exemplos práticos e comentários explicativos.
#
Expandir
message.txt
16 KB
E toda a explicação dele
O arquivo é gigante mas explica tudo
furuno — 10/11/2025 08:15
Obgg
Quando tiver o da cgi me manda pfv
Meeps

 — 10/11/2025 13:26
Mando sim
furuno — 11/11/2025 17:43
location /cgi-bin {
    root www/cgi-bin;
    methods GET POST;
    cgi .php /usr/bin/php-cgi;
    cgi .py /usr/bin/python3;
  }
Po o exemplo que tava pra mim tava assim
Meeps

 — 11/11/2025 17:47
Que exemplo?
furuno — 11/11/2025 20:04
Ah esse de cima
Era como tava o location do cgi
Ta errado?
É q o seu parece um pouco diferente
Meeps

 — 11/11/2025 20:06
location /cgi-bin {
        alias ./www/cgi-bin;
        cgi_extensions .py .php;
        allow_methods GET POST;
        cookies_enabled on;
}
Tá assim
cgi .php /usr/bin/php-cgi;
    cgi .py /usr/bin/python3;
Eu acho que essas linhas aqui
No seu
Ele interpreta o path que vc passou como sendo uma outra extensão
E da erro
Se pa
furuno — Ontem às 19:14
Consegui arrumar o cgi
Vc sabe alguma coisa que pode ser que a gnt n tenha implementado?
Meeps

 — Ontem às 19:27
Coe
Vou te mandar mais tarde o documento do CGI
Posso te mandar o tester que eu fiz tbm
Tô no aniversário da minha mãe da Carol
furuno — Ontem às 19:28
Blzz
Aproveita aii meu fi
Meeps

 — Ontem às 19:29
Devo mandar tardao mas eu mando
Amanhã eu tô na 42
furuno — Ontem às 19:29
Precisa nao
Manda quando der
Meeps

 — Ontem às 19:29
Vou ter uma reunião bolada da WebServer
Se quiser colar
furuno — Ontem às 19:29
Ah maneiro
Me manda q horas vai ser dps
Meeps

 — Ontem às 19:29
Depois do almoço
Meu grupo já era pra ter entregado o projeto
A gente tá enrolando mt
Tá foda
E quero entregar inception antes do fim do ano ainda
Pqp
Mas aí, no tester tem muita coisa, basicamente tudo
furuno — Ontem às 20:28
fi eu to fodida é pouco
eu n entreguei nada nesse rank
Meeps

 — Ontem às 20:29
Mas mano cpp é papum
O foda é ponto
Sorte que eu fiz aquele esquema ninja la
Tô com reserva agora
﻿
furuno
furunoluiza
 
# Documentação Completa do Parsing de Arquivo de Configuração

## Visão Geral

O sistema de parsing de configuração é responsável por ler, validar e estruturar arquivos de configuração no formato similar ao nginx. O parser processa arquivos de texto contendo blocos de servidor e localizações, transformando-os em estruturas de dados tipadas e validadas.

## Fluxo de Processamento

### Fase 1: Inicialização e Leitura do Arquivo

O processo inicia quando o construtor de `ConfigFile` recebe os argumentos da linha de comando. Se um arquivo de configuração for fornecido como argumento, ele será utilizado; caso contrário, um arquivo padrão será carregado.

**Tratamentos realizados:**
- Verificação de abertura do arquivo com tratamento de exceção caso o arquivo não exista ou não possa ser aberto
- Leitura completa do conteúdo do arquivo em memória

### Fase 2: Limpeza e Pré-processamento

Antes da tokenização, o conteúdo bruto passa por três etapas de limpeza sequenciais:

#### 2.1 Remoção de Comentários
- Processamento linha por linha do arquivo
- Identificação de caracteres de comentário (`#`)
- Remoção de todo conteúdo após o caractere `#` em cada linha
- Preservação da estrutura da linha antes do comentário

#### 2.2 Remoção de Espaços em Branco
- Eliminação de espaços em branco no início e fim do conteúdo completo
- Tratamento especial para arquivos completamente vazios ou contendo apenas espaços

#### 2.3 Validação Inicial
- Verificação se após a limpeza ainda existe conteúdo para processar

**Pontos fortes:**
- Processamento robusto que mantém a estrutura do arquivo mesmo com comentários
- Tratamento de casos extremos como arquivos vazios

### Fase 3: Tokenização

A tokenização é o processo de converter o conteúdo limpo em uma sequência de tokens individuais. Este é um dos componentes mais sofisticados do parser.

#### 3.1 Processamento Caractere por Caractere

O algoritmo percorre cada caractere do conteúdo, mantendo estado sobre:
- **Aspas ativas**: Identifica quando está dentro de uma string delimitada por aspas simples ou duplas
- **Contador de chaves**: Rastreia o balanceamento de chaves `{` e `}`
- **Token atual**: Acumula caracteres que formam o token sendo construído

#### 3.2 Regras de Tokenização

**Tratamento de Aspas:**
- Quando encontra aspas simples ou duplas, entra em modo de string
- Todo conteúdo dentro das aspas é preservado como um único token, incluindo espaços
- Aspas são incluídas no token final para identificação posterior

**Tratamento de Delimitadores:**
- Chaves `{` e `}` são sempre tokens individuais
- Ponto e vírgula `;` é sempre um token individual
- Espaços em branco funcionam como separadores de tokens

**Balanceamento de Chaves:**
- Incrementa contador ao encontrar `{`
- Decrementa contador ao encontrar `}`
- Ao final da tokenização, valida que o contador está em zero

**Pontos fortes:**
- Suporte completo a strings com aspas, permitindo valores com espaços
- Validação estrutural durante a tokenização (chaves balanceadas)
- Preservação de informações importantes como aspas para processamento posterior

### Fase 4: Validação de Estrutura Básica

Após a tokenização, são realizadas validações iniciais:
- Verificação se existem tokens para processar
- Validação de que o primeiro bloco é um `server`
- Verificação de que cada bloco `server` é seguido imediatamente por `{`

**Tratamentos realizados:**
- Exceções específicas para cada tipo de erro estrutural encontrado
- Mensagens de erro descritivas indicando exatamente o problema

### Fase 5: Processamento de Blocos Server

Para cada bloco `server` encontrado, é criada uma instância de `ServerBlock`. O processamento segue um padrão iterativo:

#### 5.1 Inicialização do Bloco
- Remoção dos tokens `server` e `{` da lista de tokens
- Verificação de que existem tokens restantes para processar

#### 5.2 Processamento Iterativo de Diretivas

O parser processa cada diretiva encontrada dentro do bloco `server`:

**Diretivas suportadas:**
- `listen`: Configuração de host e porta
- `server_name`: Nomes do servidor
- `client_max_body_size`: Tamanho máximo do corpo da requisição
- `error_page`: Páginas de erro personalizadas
- `location`: Blocos de localização (processados recursivamente)
- `root`: Diretório raiz do servidor

**Tratamento de Fim de Bloco:**
- Quando encontra `}`, remove o token e encerra o processamento do bloco atual
- Retorna ao processamento do nível superior

**Validação de Tokens Inválidos:**
- Qualquer token não reconhecido como diretiva válida ou delimitador gera exceção

#### 5.3 Validações Pós-processamento

Após processar todas as diretivas do bloco:
- Verificação de que `client_max_body_size` foi definido e não é zero
- Validação de integridade dos dados coletados

**Pontos fortes:**
- Processamento flexível que permite múltiplas diretivas do mesmo tipo (exceto onde não permitido)
- Validação rigorosa de valores obrigatórios
- Tratamento de erros específico para cada tipo de diretiva

### Fase 6: Processamento de Diretivas Específicas

Cada tipo de diretiva possui seu próprio método de processamento com validações específicas:

#### 6.1 Diretiva `listen`

**Processamento:**
- Suporte a formato `host:porta` ou apenas `porta`
- Conversão de `localhost` para `127.0.0.1`
- Valores padrão: `0.0.0.0:80` se não especificado

**Validações:**
- Verificação de formato IPv4 válido (4 octetos)
- Validação de que cada octeto é numérico e está entre 0 e 255
- Validação de porta entre 1 e 65535
- Detecção e rejeição de configurações `listen` duplicadas

**Pontos fortes:**
- Conversão automática de valores comuns como `localhost`
- Validação rigorosa de formato de endereço IP
- Prevenção de configurações duplicadas

#### 6.2 Diretiva `server_name`

**Processamento:**
- Suporte a múltiplos nomes de servidor em uma única diretiva
- Coleta de todos os nomes até encontrar ponto e vírgula

**Validações:**
- Verificação de que existe pelo menos um nome
- Remoção automática de duplicatas

**Pontos fortes:**
- Flexibilidade para múltiplos nomes
- Limpeza automática de duplicatas

#### 6.3 Diretiva `client_max_body_size`

**Processamento:**
- Suporte a unidades: `B`, `K`, `M`, `G`
- Conversão automática para bytes
- Validação de formato numérico seguido de unidade

**Validações:**
- Verificação de que todos os caracteres antes do último são numéricos
- Validação de unidade válida (`B`, `K`, `M`, `G`)
- Prevenção de múltiplas definições (apenas uma por bloco server)
- Validação de que o valor não é zero

**Pontos fortes:**
- Suporte intuitivo a unidades legíveis
- Prevenção de configurações conflitantes

#### 6.4 Diretiva `error_page`

**Processamento:**
- Suporte a múltiplos códigos de erro apontando para a mesma URI
- Formato: `error_page código1 código2 ... códigoN URI;`

**Validações:**
- Verificação de que cada código é numérico
- Validação de código HTTP entre 100 e 599
- Verificação de que existe pelo menos um código e uma URI

**Pontos fortes:**
- Flexibilidade para associar múltiplos códigos a uma página
- Validação de códigos HTTP válidos

#### 6.5 Diretiva `root`

**Processamento:**
- Captura de caminho do diretório raiz
- Valor padrão: `./` se não especificado

**Validações:**
- Prevenção de múltiplas definições (apenas uma por bloco server)
- Verificação de que o caminho foi fornecido

**Pontos fortes:**
- Valor padrão sensato
- Prevenção de configurações conflitantes

### Fase 7: Processamento de Blocos Location

Blocos `location` são processados recursivamente dentro de blocos `server`. O processamento segue padrão similar ao de blocos server.

#### 7.1 Inicialização do Bloco Location

**Processamento:**
- Captura da URI do location (primeiro token após `location`)
- Remoção dos tokens `location`, `URI` e `{`
- Verificação de que existem tokens para processar

**Validações:**
- Verificação de que a URI começa com `/`
- Detecção e rejeição de locations duplicados com a mesma URI

**Pontos fortes:**
- Validação de formato de URI
- Prevenção de conflitos de configuração

#### 7.2 Diretivas de Location

**Diretivas suportadas:**
- `autoindex`: Ativação/desativação de listagem de diretórios
- `can_upload`: Permissão de upload de arquivos
- `alias`: Caminho alternativo para o location
- `return`: Redirecionamento
- `upload_path`: Caminho para uploads
- `index`: Arquivos de índice
- `cgi_extensions`: Extensões de arquivos CGI
- `allow_methods`: Métodos HTTP permitidos

#### 7.3 Validações Específicas de Location

**Diretiva `autoindex` e `can_upload`:**
- Aceita apenas valores `on` ou `off`
- Conversão para boolean

**Diretiva `cgi_extensions`:**
- Suporte a múltiplas extensões
- Adição automática de ponto (`.`) se ausente
- Validação de extensões permitidas (`.php`, `.py`)
- Prevenção de extensões inválidas

**Diretiva `allow_methods`:**
- Suporte a múltiplos métodos
- Validação de métodos HTTP válidos (`GET`, `POST`, `DELETE`)
- Rejeição de métodos não suportados

**Diretiva `index`:**
- Suporte a múltiplos arquivos de índice
- Processamento sequencial até encontrar ponto e vírgula

**Pontos fortes:**
- Validação rigorosa de valores permitidos
- Normalização automática (adição de ponto em extensões)
- Suporte flexível a múltiplos valores

### Fase 8: Sistema de Validação de Tokens

O parser implementa um sistema sofisticado de validação de tokens através do método `verifyToken`, que suporta quatro tipos de validação:

#### 8.1 Tipos de Validação

**EMPTY:**
- Verifica se a lista de tokens está vazia
- Usado para garantir que existem tokens para processar

**SEMICOLON:**
- Verifica se a lista está vazia OU se o próximo token é ponto e vírgula
- Usado para detectar diretivas sem argumentos

**DIFF_SEMICOLON:**
- Verifica se a lista está vazia OU se o próximo token NÃO é ponto e vírgula
- Usado para garantir que diretivas terminam corretamente

**END_OF_FILE:**
- Verifica se chegou ao final do arquivo
- Usado para prevenir processamento além do esperado

**Pontos fortes:**
- Sistema unificado de validação
- Mensagens de erro específicas para cada tipo de validação
- Prevenção de erros de processamento

### Fase 9: Gerenciamento de Tokens

O parser mantém uma lista de tokens que é consumida progressivamente durante o processamento:

**Operações:**
- Remoção de tokens processados (`removeTokens`)
- Acesso à lista atual de tokens (`getTokens`)
- Verificação de estado da lista (`verifyToken`)

**Pontos fortes:**
- Processamento incremental eficiente
- Estado compartilhado entre diferentes níveis de parsing
- Permite processamento recursivo de blocos aninhados

## Tratamento de Erros

### Estratégia de Tratamento

O parser utiliza exceções C++ para tratamento de erros, com mensagens descritivas que incluem:
- Contexto da diretiva onde ocorreu o erro
- Tipo específico de problema encontrado
- Informações sobre o que era esperado

### Categorias de Erros Tratados

**Erros de Estrutura:**
- Arquivo não encontrado ou não pode ser aberto
- Chaves não balanceadas
- Blocos malformados
- Tokens inválidos em contexto

**Erros de Validação:**
- Valores fora de faixa permitida
- Formatos inválidos
- Valores obrigatórios ausentes
- Configurações duplicadas onde não permitido

**Erros de Formato:**
- Endereços IP inválidos
- Portas inválidas
- Códigos HTTP inválidos
- Unidades de tamanho inválidas

**Pontos fortes:**
- Mensagens de erro claras e acionáveis
- Validação em múltiplas camadas
- Prevenção de estados inválidos

## Pontos Fortes da Implementação

### 1. Arquitetura Modular

A implementação separa claramente as responsabilidades:
- **ConfigFile**: Gerencia leitura, limpeza e tokenização
- **ServerBlock**: Processa e valida configurações de servidor
- **LocationBlock**: Processa e valida configurações de localização

Esta separação facilita manutenção, testes e extensibilidade.

### 2. Tokenização Robusta

A tokenização implementa um algoritmo estado-máquina que:
- Preserva strings com aspas corretamente
- Valida estrutura durante o processamento
- Trata casos especiais como comentários e espaços

### 3. Validação em Múltiplas Camadas

Validações são realizadas em diferentes momentos:
- Durante tokenização (estrutura básica)
- Durante processamento de diretivas (valores específicos)
- Após processamento completo (integridade geral)

### 4. Flexibilidade e Rigidez Balanceadas

O parser é flexível onde apropriado:
- Múltiplas diretivas do mesmo tipo (quando permitido)
- Múltiplos valores em uma diretiva
- Valores padrão sensatos

E rigoroso onde necessário:
- Prevenção de duplicatas onde não permitido
- Validação de formatos específicos
- Valores obrigatórios

### 5. Processamento Recursivo

A arquitetura permite processamento recursivo natural de blocos aninhados:
- Blocos `server` podem conter múltiplos blocos `location`
- Cada nível gerencia seu próprio escopo de tokens
- Estado compartilhado permite comunicação entre níveis

### 6. Sistema de Validação Unificado

O sistema `verifyToken` com tipos de validação padronizados:
- Reduz duplicação de código
- Garante consistência nas validações
- Facilita adição de novos tipos de validação

### 7. Tratamento de Casos Extremos

A implementação trata adequadamente:
- Arquivos vazios
- Comentários em qualquer posição
- Espaços em branco excessivos
- Valores padrão quando não especificados
- Conversões automáticas (localhost, unidades)

### 8. Prevenção de Erros Comuns

O parser previne problemas comuns:
- Configurações duplicadas
- Valores fora de faixa
- Formatos inválidos
- Estados inconsistentes

### 9. Mensagens de Erro Informativas

Cada erro inclui contexto suficiente para:
- Identificar onde ocorreu o problema
- Entender o que era esperado
- Corrigir a configuração rapidamente

### 10. Eficiência de Processamento

O processamento incremental:
- Consome tokens conforme processa
- Não requer múltiplas passadas sobre o arquivo
- Mantém estado mínimo necessário

## Conclusão

O sistema de parsing implementa uma solução robusta e completa para processamento de arquivos de configuração. A arquitetura modular, validações rigorosas, tratamento de erros abrangente e flexibilidade controlada tornam o sistema confiável e fácil de manter. A implementação demonstra atenção aos detalhes e consideração de casos extremos, resultando em um parser que pode lidar com configurações complexas enquanto mantém alta confiabilidade.