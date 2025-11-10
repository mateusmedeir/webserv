# Implementação de Cookies - Documentação

## Índice

1. [O que são Cookies?](#o-que-são-cookies)
2. [Como Funcionam os Cookies HTTP?](#como-funcionam-os-cookies-http)
3. [Implementação no Projeto](#implementação-no-projeto)
4. [Decisões de Design](#decisões-de-design)
5. [Uso e Configuração](#uso-e-configuração)
6. [Fluxo de Execução](#fluxo-de-execução)
7. [Testes](#testes)

---

## O que são Cookies?

Cookies são pequenos fragmentos de dados que um servidor web envia para o navegador do cliente e que são armazenados localmente. O navegador automaticamente envia esses dados de volta ao servidor em requisições subsequentes para o mesmo domínio.

### Propósito Principal

Cookies resolvem um problema fundamental do protocolo HTTP: **a ausência de estado** (stateless). Por padrão, cada requisição HTTP é independente e o servidor não mantém informações sobre requisições anteriores do mesmo cliente. Os cookies permitem que o servidor:

- **Identifique usuários** entre múltiplas requisições
- **Mantenha sessões** de usuários autenticados
- **Armazene preferências** do usuário (tema, idioma, etc.)
- **Rastreie comportamento** para análises

### Exemplo Prático

Imagine um usuário fazendo login em um site:

1. O usuário envia credenciais e o servidor valida
2. O servidor gera um **ID de sessão único** (ex: `session_id=abc123def456`)
3. O servidor envia esse ID como cookie para o navegador
4. Nas requisições seguintes, o navegador automaticamente envia `Cookie: session_id=abc123def456`
5. O servidor reconhece o usuário pelo ID e mantém a sessão ativa

---

## Como Funcionam os Cookies HTTP?

O protocolo HTTP define dois headers principais para cookies:

### 1. Header `Set-Cookie` (Servidor → Cliente)

O servidor envia cookies ao cliente através do header `Set-Cookie` na resposta HTTP.

**Formato:**
```
Set-Cookie: nome=valor; atributo1=valor1; atributo2
```

**Exemplo:**
```
Set-Cookie: session_id=abcdefghij; Path=/; HttpOnly
```

**Atributos comuns:**
- **`Path=/`**: Define em quais caminhos o cookie é enviado. `/` significa que é enviado em todas as requisições para o domínio.
- **`HttpOnly`**: Impede que JavaScript acesse o cookie, aumentando a segurança contra ataques XSS (Cross-Site Scripting).
- **`Secure`**: Cookie só é enviado em conexões HTTPS.
- **`Max-Age=<segundos>`**: Define quando o cookie expira (em segundos).
- **`Expires=<data>`**: Define uma data específica de expiração.

### 2. Header `Cookie` (Cliente → Servidor)

O navegador envia cookies de volta ao servidor através do header `Cookie` nas requisições HTTP.

**Formato:**
```
Cookie: nome1=valor1; nome2=valor2; nome3=valor3
```

**Exemplo:**
```
Cookie: session_id=abcdefghij; theme=dark; lang=pt-BR
```

### Fluxo Básico

```
┌─────────┐                                    ┌─────────┐
│ Cliente │                                    │ Servidor│
└────┬────┘                                    └────┬────┘
     │                                              │
     │ 1. GET /login                               │
     │────────────────────────────────────────────>│
     │                                              │
     │                   2. HTTP/1.1 200 OK        │
     │                   Set-Cookie: session_id=abc│
     │<────────────────────────────────────────────│
     │    [Navegador armazena cookie]              │
     │                                              │
     │ 3. GET /dashboard                           │
     │    Cookie: session_id=abc                   │
     │────────────────────────────────────────────>│
     │    [Servidor identifica usuário]            │
     │                   4. HTTP/1.1 200 OK        │
     │<────────────────────────────────────────────│
```

---

## Implementação no Projeto

A implementação de cookies no webserv segue uma abordagem simples e eficiente, focada em **geração automática de cookies de sessão**.

### Componentes Principais

#### 1. Classe `CookieHandler`

A classe `CookieHandler` é responsável por toda a lógica de gerenciamento de cookies.

**Localização:** `includes/CookieHandler.hpp` e `source/CookieHandler.cpp`

**Características:**
- Classe com métodos estáticos (não instanciável)
- Não permite cópia ou atribuição (padrão singleton-like)
- Responsável por gerar IDs de sessão e criar headers Set-Cookie

**Métodos:**
```cpp
// Método principal: verifica se deve gerar cookie e o adiciona à resposta
static void handleCookie(HttpResponse &response, const HttpRequest &request);

// Método privado: gera string aleatória para o session_id
static std::string generateRandomString(size_t size);
```

**Lógica Principal (`handleCookie`):**
```cpp
void CookieHandler::handleCookie(HttpResponse &response, const HttpRequest &request)
{
    // Se o cliente já possui um cookie, não faz nada
    if (request.hasHeader("cookie") && !request.getHeaderValue("cookie").empty())
        return;

    // Gera um novo session_id aleatório
    std::string randomString = generateRandomString(10);

    // Cria o header Set-Cookie com atributos seguros
    std::string cookie = "session_id=" + randomString + "; Path=/; HttpOnly";

    // Adiciona à resposta HTTP
    response.setHeader("Set-Cookie", cookie);
}
```

**Por que essa lógica?**
1. **Verificação de cookie existente**: Se o cliente já enviou um cookie, o servidor não sobrescreve. Isso preserva a sessão entre requisições.
2. **Geração automática**: Se não há cookie, um novo `session_id` é gerado automaticamente.
3. **Atributos de segurança**: O cookie sempre inclui `Path=/` (disponível em todo o site) e `HttpOnly` (proteção contra XSS).

**Geração de Session ID:**
```cpp
std::string CookieHandler::generateRandomString(size_t size)
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(std::time(NULL));
        seeded = true;
    }
    std::string result;
    for (size_t i = 0; i < size; ++i) {
        char letter = 'a' + std::rand() % 26;
        result += letter;
    }
    return result;
}
```

**Características do ID gerado:**
- **10 caracteres** de comprimento
- **Apenas letras minúsculas** (a-z)
- **Aleatório** e único por sessão
- **Seed baseado em tempo** para maior aleatoriedade

#### 2. Integração com `LocationBlock`

O suporte a cookies é configurável por location block através da diretiva `cookies_enabled`.

**Localização:** `includes/LocationBlock.hpp` e `source/LocationBlock.cpp`

**Membro adicionado:**
```cpp
bool _cookiesEnabled;  // Controla se cookies estão habilitados neste location
```

**Diretiva de configuração:**
```conf
location / {
    cookies_enabled on;  # ou 'off' para desabilitar
}
```

**Parser implementado:**
```cpp
void LocationBlock::addCookiesEnabled()
{
    this->_config.removeTokens(1); // Remove o token 'cookies_enabled'
    this->_config.verifyToken(SEMICOLON, "...");
    
    std::vector<std::string> tokens = this->_config.getTokens();
    if (tokens[0] == "on")
        this->_cookiesEnabled = true;
    else if (tokens[0] == "off")
        this->_cookiesEnabled = false;
    else
        throw std::runtime_error("...");
    
    // ... validação e limpeza de tokens
}
```

**Por que por location?**
Permite flexibilidade: alguns caminhos podem precisar de cookies (ex: área autenticada), enquanto outros não (ex: arquivos estáticos públicos). Isso oferece controle granular e otimização de performance.

#### 3. Integração com `HttpResponse`

O método `processCookies` foi adicionado à classe `HttpResponse` para coordenar o processamento de cookies.

**Localização:** `includes/HttpResponse.hpp` e `source/HttpResponse.cpp`

**Método:**
```cpp
void HttpResponse::processCookies(const HttpRequest &req, const LocationBlock &location)
{
    if (location.getCookiesEnabled()) {
        CookieHandler::handleCookie(*this, req);
    }
}
```

**Quando é chamado?**

O método é chamado em `Client.cpp` após o processamento da requisição e antes de enviar a resposta:

```cpp
// Após processar a requisição
this->response.dispatchRequest(this->request);

// Process cookies if enabled for this location
std::string uri = this->request.getUri();
ServerBlock serverBlock = this->_serverListen.getServerBlock();
std::map<std::string, LocationBlock> locations = serverBlock.getLocations();

// Find best matching location (prefix match, longest wins)
std::string bestMatch = "";
for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin();
     it != locations.end(); ++it) {
    const std::string &path = it->first;
    if (uri.compare(0, path.size(), path) == 0) {
        if (path.size() > bestMatch.size()) {
            bestMatch = path;
        }
    }
}

if (!bestMatch.empty()) {
    LocationBlock location = locations.find(bestMatch)->second;
    this->response.processCookies(this->request, location);
}

// Gerar resposta final
std::string responseStr = this->response.toString();
```

**Por que nesse ponto do fluxo?**
- A requisição já foi parseada (podemos verificar se há cookie)
- A resposta já foi construída (podemos adicionar headers)
- O location já foi identificado (sabemos se cookies estão habilitados)
- Antes de serializar a resposta (headers podem ser adicionados)

#### 4. Suporte em `HttpRequest`

A classe `HttpRequest` foi estendida com métodos para verificar headers de forma case-insensitive:

**Localização:** `includes/HttpRequest.hpp` e `source/HttpRequest.cpp`

**Métodos implementados:**
```cpp
bool hasHeader(const std::string &key) const;
std::string getHeaderValue(const std::string &key) const;
```

**Implementação case-insensitive:**
Ambos os métodos realizam busca case-insensitive nos headers, seguindo a especificação HTTP onde headers são case-insensitive:

```cpp
bool HttpRequest::hasHeader(const std::string &key) const {
    std::string lowerKey = key;
    for (size_t i = 0; i < lowerKey.size(); ++i) {
        lowerKey[i] = std::tolower(lowerKey[i]);
    }
    
    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        std::string lowerHeader = it->first;
        for (size_t i = 0; i < lowerHeader.size(); ++i) {
            lowerHeader[i] = std::tolower(lowerHeader[i]);
        }
        if (lowerHeader == lowerKey) {
            return true;
        }
    }
    return false;
}
```

Esses métodos são essenciais para verificar se o cliente já possui cookies, independentemente de como o header foi escrito (Cookie, COOKIE, cookie, etc.).

---

## Decisões de Design

### 1. Por que geração automática apenas?

**Decisão**: O servidor gera automaticamente cookies de sessão apenas quando o cliente não possui nenhum.

**Razões:**
- **Simplicidade**: Não requer lógica complexa de parsing ou validação de cookies existentes
- **Segurança**: Sempre gera IDs aleatórios, reduzindo risco de sessões previsíveis
- **Eficiência**: Verificação simples (presença do header) é rápida
- **Adequado para o bônus**: Atende aos requisitos básicos de gerenciamento de sessão

### 2. Por que não fazer parsing completo dos cookies?

**Decisão**: O servidor verifica apenas a presença do header `Cookie`, não extrai valores individuais.

**Razões:**
- **Suficiente para o caso de uso**: Para geração automática de sessões, não é necessário saber qual é o `session_id` específico
- **Evita complexidade**: Parsing completo requer lidar com edge cases (cookies malformados, encoding, etc.)
- **Foco no essencial**: A implementação resolve o problema de identificar usuários entre requisições

### 3. Por que `Path=/` e `HttpOnly` fixos?

**Decisão**: Todos os cookies gerados incluem `Path=/` e `HttpOnly`.

**Razões:**
- **Path=/**: Cookies de sessão geralmente devem estar disponíveis em todo o site, permitindo navegação entre páginas sem perder a sessão
- **HttpOnly**: Segurança fundamental contra ataques XSS. Cookies de sessão não devem ser acessíveis via JavaScript
- **Simplicidade**: Para uma implementação básica, valores fixos são mais simples e seguros

### 4. Por que session_id de 10 caracteres?

**Decisão**: O `session_id` tem exatamente 10 caracteres, apenas letras minúsculas.

**Razões:**
- **Balanço**: Longo o suficiente para ser único, curto o suficiente para eficiência
- **Aleatoriedade**: 26^10 = ~141 trilhões de possibilidades, suficiente para evitar colisões práticas
- **Simplicidade**: Não requer bibliotecas externas ou algoritmos complexos
- **Legibilidade**: Fácil de debuggar e visualizar em logs

**Alternativa considerada**: IDs com timestamp + hash (mais únicos, mas mais complexos).

### 5. Por que verificação case-insensitive?

**Decisão**: Os métodos `hasHeader` e `getHeaderValue` verificam headers de forma case-insensitive.

**Razões:**
- **Padrão HTTP**: Headers HTTP são case-insensitive por especificação
- **Compatibilidade**: Clientes podem enviar `Cookie`, `COOKIE`, `cookie`, etc.
- **Robustez**: Funciona com qualquer implementação de cliente HTTP

### 6. Por que processar cookies após `dispatchRequest`?

**Decisão**: Cookies são processados após construir a resposta, mas antes de serializá-la.

**Razões:**
- **Ordem lógica**: Primeiro processa a requisição, depois adiciona metadados (cookies)
- **Headers disponíveis**: A resposta já foi construída, mas ainda pode ser modificada
- **Location conhecido**: O location correspondente já foi identificado

### 7. Por que matching de location por prefixo (longest match)?

**Decisão**: O sistema encontra o location correspondente usando prefix matching, escolhendo o match mais longo.

**Razões:**
- **Comportamento padrão**: Segue o comportamento típico de servidores web (nginx, Apache)
- **Flexibilidade**: Permite locations mais específicos sobreporem locations mais gerais
- **Precisão**: Garante que configurações específicas de cookies sejam respeitadas corretamente

---

## Uso e Configuração

### Habilitando Cookies em um Location

Para habilitar cookies em um location block, adicione a diretiva `cookies_enabled on`:

```conf
server {
    listen 8080;
    server_name localhost;
    
    root ./www;
    
    location / {
        allow_methods GET POST;
        cookies_enabled on;  # Cookies habilitados
    }
    
    location /static {
        allow_methods GET;
        cookies_enabled off;  # Cookies desabilitados
    }
    
    location /api {
        allow_methods GET POST;
        cookies_enabled on;  # Cookies habilitados para API
    }
}
```

### Comportamento

**Com `cookies_enabled on`:**
- Primeira requisição sem cookie → Servidor gera e envia `Set-Cookie`
- Requisições subsequentes com cookie → Servidor não envia `Set-Cookie` (preserva sessão)
- Cookie vazio ou header `Cookie:` sem valor → Servidor gera novo cookie

**Com `cookies_enabled off`:**
- Nenhum cookie é gerado ou enviado
- Requisições são processadas normalmente
- Header `Cookie` recebido é ignorado (não processado)

### Exemplo de Resposta HTTP

**Primeira requisição:**
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Set-Cookie: session_id=abcdefghij; Path=/; HttpOnly

[Body da resposta]
```

**Requisição subsequente (com cookie):**
```
GET /dashboard HTTP/1.1
Host: localhost:8080
Cookie: session_id=abcdefghij

HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1567

[Body da resposta]
(Note: Sem header Set-Cookie)
```

---

## Fluxo de Execução

### Fluxo Completo de uma Requisição com Cookies

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Cliente envia requisição HTTP                            │
│    GET /dashboard HTTP/1.1                                  │
│    Host: localhost:8080                                     │
│    Cookie: session_id=abcdefghij  [se existir]             │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 2. HttpRequest parseia a requisição                         │
│    - Extrai método, URI, headers                            │
│    - Detecta header "Cookie" se presente                    │
│    - Armazena headers (case-sensitive no map, busca         │
│      case-insensitive via métodos)                          │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 3. HttpResponse processa a requisição                       │
│    response.dispatchRequest(request)                        │
│    - Constrói resposta HTTP (status, headers, body)        │
│    - Processa método (GET, POST, DELETE)                   │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 4. Client identifica Location correspondente                │
│    - Obtém ServerBlock do ServerListen                      │
│    - Itera sobre locations                                  │
│    - Faz prefix matching (longest match)                    │
│    - Obtém LocationBlock correspondente                     │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 5. Verifica se cookies estão habilitados                    │
│    location.getCookiesEnabled() → true/false                │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 6. Processa cookies (se habilitado)                         │
│    response.processCookies(request, location)               │
│    ├─ CookieHandler::handleCookie(response, request)       │
│    │  ├─ Verifica: request.hasHeader("cookie")?            │
│    │  ├─ Verifica: getHeaderValue("cookie") não vazio?     │
│    │  ├─ Se SIM → return (preserva cookie existente)      │
│    │  └─ Se NÃO → gera novo session_id e adiciona          │
│    │              response.setHeader("Set-Cookie", ...)    │
│    └─ Cookies processados                                   │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 7. Serializa resposta HTTP                                  │
│    response.toString()                                      │
│    - Inclui header Set-Cookie se gerado                     │
│    - Formata status line, headers e body                    │
└───────────────────────┬─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│ 8. Cliente recebe resposta                                  │
│    HTTP/1.1 200 OK                                          │
│    Set-Cookie: session_id=xyz789klmn  [se gerado]          │
│    Content-Type: text/html                                  │
│    Content-Length: 1234                                     │
│    ...                                                      │
│                                                             │
│    [Navegador armazena cookie automaticamente]             │
└─────────────────────────────────────────────────────────────┘
```

### Fluxo Específico: Primeira Requisição

1. **Cliente**: `GET / HTTP/1.1` (sem header Cookie)
2. **Servidor**: `Client::handleEpollIn()` recebe dados
3. **Servidor**: `HttpRequest` parseia requisição
4. **Servidor**: `HttpResponse::dispatchRequest()` processa requisição
5. **Servidor**: `Client` encontra location correspondente (`/`)
6. **Servidor**: `location.getCookiesEnabled()` → `true`
7. **Servidor**: `response.processCookies()` é chamado
8. **Servidor**: `CookieHandler::handleCookie()` verifica `hasHeader("cookie")` → `false`
9. **Servidor**: Gera `session_id` aleatório (ex: `"abcdefghij"`)
10. **Servidor**: Adiciona `Set-Cookie: session_id=abcdefghij; Path=/; HttpOnly`
11. **Servidor**: Serializa e envia resposta
12. **Cliente**: Armazena cookie localmente
13. **Resultado**: Cliente possui sessão identificada

### Fluxo Específico: Requisição com Cookie Existente

1. **Cliente**: `GET / HTTP/1.1` com `Cookie: session_id=abcdefghij`
2. **Servidor**: `HttpRequest` parseia requisição e armazena header `Cookie`
3. **Servidor**: `HttpResponse::dispatchRequest()` processa requisição
4. **Servidor**: `Client` encontra location correspondente
5. **Servidor**: `location.getCookiesEnabled()` → `true`
6. **Servidor**: `response.processCookies()` é chamado
7. **Servidor**: `CookieHandler::handleCookie()` verifica `hasHeader("cookie")` → `true`
8. **Servidor**: Verifica `getHeaderValue("cookie")` → não está vazio
9. **Servidor**: `return` (não gera novo cookie)
10. **Servidor**: Serializa e envia resposta (sem `Set-Cookie`)
11. **Cliente**: Mantém cookie existente
12. **Resultado**: Sessão preservada entre requisições

---

## Testes

A implementação de cookies foi validada através de testes automatizados:

### Testes Básicos (`test_cookies_complete.sh`)

O script `test_cookies_complete.sh` realiza uma bateria completa de testes:

1. **Primeira requisição sem cookie**: Verifica se `Set-Cookie` é enviado
2. **Segunda requisição com cookie**: Verifica que cookie é preservado
3. **Atributos do cookie**: Verifica `Path=/` e `HttpOnly`
4. **Formato do session_id**: Verifica 10 caracteres, apenas letras minúsculas
5. **Persistência**: Múltiplas requisições mantêm o mesmo cookie
6. **Cookie vazio**: Verifica que novo cookie é gerado quando header está vazio
7. **Status HTTP**: Verifica que requisições retornam status 200

### Testes Avançados

- **Locations com `cookies_enabled off`**: Verifica que cookies não são enviados
- **Diferentes locations**: Verifica comportamento em múltiplos locations
- **Unicidade**: Verifica que diferentes requisições recebem IDs únicos

### Executando os Testes

```bash
# Compilar o projeto
make

# Executar testes de cookies
./test_cookies_complete.sh

# Executar testes de eficiência (inclui verificação de cookies)
./test_efficiency.sh
```

### Resultados Esperados

Todos os testes críticos devem passar:
- ✅ Primeira requisição recebe `Set-Cookie`
- ✅ Requisições subsequentes preservam cookie
- ✅ Cookie contém atributos de segurança
- ✅ Session ID tem formato correto
- ✅ Cookies respeitam configuração por location

---

## Conclusão

A implementação de cookies no webserv fornece uma solução simples e eficiente para gerenciamento de sessões, atendendo aos requisitos do bônus do projeto. A abordagem focada em geração automática de cookies de sessão:

- ✅ **Funciona**: Mantém identificação de usuários entre requisições
- ✅ **Segura**: Usa atributos `HttpOnly` e `Path=/` por padrão
- ✅ **Flexível**: Configurável por location block
- ✅ **Simples**: Implementação direta e fácil de manter
- ✅ **Adequada**: Resolve o problema de estado no protocolo HTTP
- ✅ **Testada**: Validada através de testes automatizados completos

Esta implementação serve como base sólida para funcionalidades mais avançadas de gerenciamento de sessão, autenticação e personalização de usuários.

---

## Arquivos Modificados/Criados

### Novos Arquivos
- `includes/CookieHandler.hpp` - Definição da classe CookieHandler
- `source/CookieHandler.cpp` - Implementação da classe CookieHandler

### Arquivos Modificados
- `includes/HttpRequest.hpp` - Adicionado método `hasHeader()`
- `source/HttpRequest.cpp` - Implementação de `hasHeader()` e `getHeaderValue()` case-insensitive
- `includes/LocationBlock.hpp` - Adicionado membro `_cookiesEnabled` e método `getCookiesEnabled()`
- `source/LocationBlock.cpp` - Implementação de parser para `cookies_enabled` e método `addCookiesEnabled()`
- `includes/HttpResponse.hpp` - Adicionado método `processCookies()`
- `source/HttpResponse.cpp` - Implementação de `processCookies()`
- `source/Client.cpp` - Integração do processamento de cookies após `dispatchRequest()`
- `includes/WebservHeader.hpp` - Adicionado include de `CookieHandler.hpp`
- `Makefile` - Adicionado `source/CookieHandler.cpp` aos arquivos fonte
