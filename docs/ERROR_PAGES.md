# Documentação do Sistema de Páginas de Erro Personalizadas

## Visão Geral

O sistema de páginas de erro personalizadas permite que o servidor WebServ utilize páginas HTML específicas para cada código de erro HTTP, conforme configurado no arquivo `.conf`. Este sistema implementa uma hierarquia de fallback que garante que sempre haverá uma resposta adequada para o cliente, mesmo quando páginas personalizadas não estão disponíveis.

## Arquitetura do Sistema

### Componentes Principais

1. **ServerBlock** (`ServerBlock.hpp` / `ServerBlock.cpp`)
   - Responsável pelo parsing da diretiva `error_page` no arquivo de configuração
   - Armazena o mapeamento código → URI em `std::map<int, std::string> _errorPages`

2. **HttpResponse** (`HttpResponse.hpp` / `HttpResponse.cpp`)
   - Responsável por gerar as respostas de erro
   - Implementa a lógica de seleção de páginas de erro
   - Contém as mensagens de status HTTP corretas

3. **Client** (`Client.cpp`)
   - Responsável por invocar os métodos de erro com o contexto correto do ServerBlock

---

## Alterações Realizadas

### 1. Modificação do `setErrorPage()`

#### Problema Anterior
O método `setErrorPage()` utilizava um caminho hardcoded para as páginas de erro:

```cpp
// Código ANTIGO - problemático
void HttpResponse::setErrorPage(int code) {
    std::string path = "./error_pages/" + intToString(code) + ".html";
    // ... resto do código
}
```

Este código **ignorava completamente** as configurações do arquivo `.conf`, tornando a diretiva `error_page` inútil.

#### Solução Implementada
O método foi modificado para aceitar um ponteiro para `ServerBlock` e implementar uma hierarquia de fallback:

```cpp
// Código NOVO - corrigido
void HttpResponse::setErrorPage(int code, const ServerBlock *serverBlock) {
    std::string path;
    
    // 1. Verificar se existe página personalizada no ServerBlock
    if (serverBlock) {
        std::map<int, std::string> errorPages = serverBlock->getErrorPages();
        std::map<int, std::string>::const_iterator it = errorPages.find(code);
        if (it != errorPages.end()) {
            // Combinar root com o URI da página de erro configurada
            std::string root = serverBlock->getRoot().second;
            if (root.empty())
                root = ".";
            path = root + it->second;
        }
    }
    
    // 2. Se não encontrou página personalizada, usar fallback padrão
    if (path.empty()) {
        path = "./error_pages/" + intToString(code) + ".html";
    }
    
    // ... código de leitura do arquivo e fallbacks adicionais
}
```

#### Fluxo de Seleção de Página

```
┌─────────────────────────────────────────────────────────────────┐
│                    Requisição gera erro HTTP                     │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│     1. ServerBlock tem error_page configurado para o código?     │
└─────────────────────────────────────────────────────────────────┘
                    │                           │
                   SIM                         NÃO
                    │                           │
                    ▼                           ▼
    ┌───────────────────────────┐   ┌───────────────────────────┐
    │ Usar: {root} + {uri}      │   │ Usar fallback:            │
    │ Ex: ./www/error/404.html  │   │ ./error_pages/{code}.html │
    └───────────────────────────┘   └───────────────────────────┘
                    │                           │
                    ▼                           ▼
┌─────────────────────────────────────────────────────────────────┐
│                   2. Arquivo existe e é legível?                 │
└─────────────────────────────────────────────────────────────────┘
                    │                           │
                   SIM                         NÃO
                    │                           │
                    ▼                           ▼
    ┌───────────────────────────┐   ┌───────────────────────────┐
    │ Retornar conteúdo do      │   │ 3. Tentar fallback padrão │
    │ arquivo HTML              │   │    ./error_pages/{code}   │
    └───────────────────────────┘   └───────────────────────────┘
                                                │
                                                ▼
                                    ┌───────────────────────────┐
                                    │ 4. Arquivo fallback       │
                                    │    existe?                │
                                    └───────────────────────────┘
                                        │               │
                                       SIM             NÃO
                                        │               │
                                        ▼               ▼
                        ┌─────────────────┐   ┌─────────────────┐
                        │ Usar fallback   │   │ Gerar página    │
                        │                 │   │ HTML genérica   │
                        └─────────────────┘   └─────────────────┘
```

---

### 2. Correção e Validação dos Status Codes

#### Problema Anterior
As mensagens de status HTTP eram genéricas ("Error") para todos os códigos de erro, não seguindo a especificação HTTP.

#### Solução Implementada
Foi adicionado o método `getStatusMessageForCode()` que retorna a mensagem HTTP correta para cada código:

```cpp
std::string HttpResponse::getStatusMessageForCode(int code) const {
    switch (code) {
        // 2xx Success
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        
        // 3xx Redirection
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 304: return "Not Modified";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        
        // 4xx Client Errors
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 408: return "Request Timeout";
        case 409: return "Conflict";
        case 410: return "Gone";
        case 411: return "Length Required";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";
        case 418: return "I'm a teapot";
        case 422: return "Unprocessable Entity";
        case 429: return "Too Many Requests";
        
        // 5xx Server Errors
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        case 505: return "HTTP Version Not Supported";
        
        default: return "Error";
    }
}
```

#### Resposta HTTP Correta

**Antes:**
```http
HTTP/1.0 404 Error
Content-Type: text/html
Content-Length: 25

<h1>404 Error</h1>
```

**Depois:**
```http
HTTP/1.0 404 Not Found
Content-Type: text/html
Content-Length: 1523

<!DOCTYPE html>
<html>
... (conteúdo da página de erro personalizada)
</html>
```

---

### 3. Modificação do `setResponseByStatus()`

O método `setResponseByStatus()` foi atualizado para passar o `ServerBlock` para o `setErrorPage()`:

```cpp
// Assinatura ANTIGA
void setResponseByStatus(int statusCode, 
                         const std::string &statusMessage="OK",
                         const std::string &bodyContent="",
                         const std::string &contentType="text/html");

// Assinatura NOVA
void setResponseByStatus(int statusCode,
                         const ServerBlock *serverBlock = NULL,
                         const std::string &statusMessage="OK",
                         const std::string &bodyContent="",
                         const std::string &contentType="text/html");
```

#### Implementação:

```cpp
void HttpResponse::setResponseByStatus(int statusCode, 
                                       const ServerBlock *serverBlock,
                                       const std::string &statusMessage,
                                       const std::string &bodyContent,
                                       const std::string &contentType) {
    if (statusCode >= 400) {
        setErrorPage(statusCode, serverBlock);  // Passa o ServerBlock
    } else if (statusCode == 302) {
        setStatus(302, "Found");
        setHeader("Location", bodyContent);
        setBody("<h1>302 Found</h1>", "text/html");
    } else {
        setStatus(statusCode, statusMessage);
        setBody(bodyContent, contentType);
    }
}
```

---

### 4. Atualização das Chamadas em `Client.cpp`

Todas as chamadas de `setErrorPage()` e `setResponseByStatus()` foram atualizadas para incluir o `ServerBlock`:

```cpp
// ANTES
this->response.setErrorPage(404);
this->response.setResponseByStatus(403, "Forbidden", "<h1>Forbidden</h1>");

// DEPOIS
ServerBlock serverBlock = this->_serverListen.getServerBlock();
this->response.setErrorPage(404, &serverBlock);
this->response.setResponseByStatus(403, &serverBlock, "Forbidden", "<h1>Forbidden</h1>");
```

**Nota Importante:** Como `getServerBlock()` retorna por valor (uma cópia), é necessário armazenar em uma variável local antes de passar o endereço:

```cpp
// CORRETO - armazenar em variável local primeiro
ServerBlock serverBlock = this->_serverListen.getServerBlock();
this->response.setErrorPage(404, &serverBlock);

// INCORRETO - taking address of rvalue
this->response.setErrorPage(404, &this->_serverListen.getServerBlock()); // ERRO!
```

---

## Configuração no Arquivo .conf

### Sintaxe

```nginx
error_page <código1> [código2] [código3] ... <URI>;
```

### Exemplos de Configuração

```nginx
server {
    listen 8080;
    server_name localhost;
    client_max_body_size 10M;
    root ./www;
    
    # Página de erro individual para cada código
    error_page 400 /error/400.html;
    error_page 401 /error/401.html;
    error_page 403 /error/403.html;
    error_page 404 /error/404.html;
    error_page 405 /error/405.html;
    error_page 500 /error/500.html;
    error_page 502 /error/502.html;
    error_page 503 /error/503.html;
    
    # Múltiplos códigos para a mesma página
    # error_page 400 401 403 /error/4xx.html;
    
    location / {
        alias ./www;
        index index.html;
        allow_methods GET POST DELETE;
    }
    
    location /error {
        alias ./www/error;
        allow_methods GET;
    }
}
```

### Validação dos Códigos

O parsing no `ServerBlock::addErrorPages()` valida os códigos:

```cpp
int code = std::atoi(it->c_str());
if (code < 100 || code > 599)
    throw std::runtime_error("Configuração inválida: error_page: [code] é inválido, "
                             "deve ser um número entre 100 e 599");
```

---

## Hierarquia de Fallback

O sistema implementa uma hierarquia de 4 níveis para garantir que sempre haverá uma resposta:

| Nível | Descrição | Exemplo |
|-------|-----------|---------|
| 1 | Página personalizada configurada | `./www/error/404.html` |
| 2 | Fallback padrão do servidor | `./error_pages/404.html` |
| 3 | Fallback genérico (mesma página) | `./error_pages/{code}.html` |
| 4 | HTML gerado dinamicamente | `<html>...<h1>404 Not Found</h1>...</html>` |

### Página HTML Genérica (Nível 4)

Quando nenhum arquivo é encontrado, o servidor gera uma página HTML básica:

```cpp
setStatus(code, statusMessage);
setBody("<html><head><title>" + intToString(code) + " " + statusMessage + "</title></head>"
        "<body><h1>" + intToString(code) + " " + statusMessage + "</h1></body></html>", 
        "text/html");
```

---

## Arquivos Modificados

### Headers (`.hpp`)

| Arquivo | Modificação |
|---------|-------------|
| `HttpResponse.hpp` | Nova assinatura de `setErrorPage()` e `setResponseByStatus()`, novo método `getStatusMessageForCode()` |

### Source (`.cpp`)

| Arquivo | Modificação |
|---------|-------------|
| `HttpResponse.cpp` | Implementação da lógica de páginas personalizadas, função `getStatusMessageForCode()`, atualização de todas as chamadas internas |
| `Client.cpp` | Atualização de todas as chamadas de `setErrorPage()` e `setResponseByStatus()` para passar o `ServerBlock` |

---

## Script de Testes

Foi criado o script `test_error_pages.sh` para validar a funcionalidade:

```bash
./test_error_pages.sh
```

### Testes Implementados

1. **Páginas Personalizadas** - Verifica se páginas configuradas no `.conf` são usadas
2. **Fallback** - Verifica se usa `./error_pages/` quando não há configuração
3. **Múltiplos Códigos** - Testa `error_page 400 401 403 /error/4xx.html`
4. **Status Messages** - Valida mensagens HTTP (200 OK, 404 Not Found, etc.)
5. **Content-Type** - Confirma retorno `text/html`
6. **Casos Especiais** - URLs longas, caracteres especiais, requisições consecutivas

---

## Conformidade com a Régua de Avaliação

Esta implementação atende aos requisitos da régua de avaliação do projeto WebServ:

> **"Your HTTP response status codes must be accurate."**

- ✅ Status codes corretos para cada tipo de erro
- ✅ Mensagens de status HTTP conforme RFC 7231
- ✅ Páginas de erro personalizadas via configuração
- ✅ Fallback para páginas padrão quando necessário
- ✅ Content-Type correto (`text/html`)
- ✅ Content-Length correto

---

## Referências

- [RFC 7231 - HTTP/1.1 Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [NGINX error_page Directive](https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page)
- `docs/PARSER_CONFIG.md` - Documentação do parser de configuração
- `docs/WEBSERV_SUBJECT.md` - Subject do projeto

