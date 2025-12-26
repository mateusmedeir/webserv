# 🌐 WebServ

<div align="center">

![C++](https://img.shields.io/badge/C++-98-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![HTTP](https://img.shields.io/badge/HTTP-1.1-005C97?style=for-the-badge)
![Grade](https://img.shields.io/badge/Grade-125%25-brightgreen?style=for-the-badge)
![42](https://img.shields.io/badge/42-Project-000000?style=for-the-badge)

**Um servidor HTTP de alto desempenho escrito em C++98**

*"This is when you finally understand why URLs start with HTTP"*

[Sobre](#-sobre) •
[Funcionalidades](#-funcionalidades) •
[Arquitetura](#-arquitetura) •
[Instalação](#-instalação) •
[Uso](#-uso) •
[Testes](#-testes) •
[Aprendizados](#-aprendizados)

</div>

---

## 📖 Sobre

O **WebServ** é um projeto da 42 que desafia os estudantes a implementar um servidor HTTP do zero, seguindo o padrão HTTP/1.1. Este projeto oferece uma compreensão profunda de como a web funciona nos bastidores, desde o parsing de requisições até o gerenciamento de conexões simultâneas.

### O Desafio

Construir um servidor web capaz de:
- Servir páginas estáticas e dinâmicas
- Processar requisições GET, POST e DELETE
- Executar scripts CGI (Python, Perl, Shell)
- Gerenciar múltiplas conexões simultâneas de forma não-bloqueante
- Permanecer **resiliente** e **estável** sob qualquer circunstância

---

## ✨ Funcionalidades

### Parte Mandatória (100%)

| Funcionalidade | Descrição | Status |
|----------------|-----------|--------|
| **Métodos HTTP** | GET, POST, DELETE | ✅ |
| **Arquivos Estáticos** | HTML, CSS, JS, imagens, etc. | ✅ |
| **Upload de Arquivos** | Via POST multipart/form-data | ✅ |
| **CGI** | Execução de scripts dinâmicos | ✅ |
| **Autoindex** | Listagem automática de diretórios | ✅ |
| **Redirecionamentos** | HTTP 301, 302, 307, 308 | ✅ |
| **Páginas de Erro** | Personalizáveis por código | ✅ |
| **Múltiplas Portas** | Servidores virtuais | ✅ |
| **Configuração** | Estilo NGINX | ✅ |
| **I/O Multiplexing** | Único epoll para todas operações | ✅ |
| **Non-blocking** | Todas as operações são assíncronas | ✅ |

### Parte Bônus (25%)

| Funcionalidade | Descrição | Status |
|----------------|-----------|--------|
| **Cookies & Sessions** | Gerenciamento de sessão com session_id | ✅ |
| **Múltiplos CGI** | Python, Perl, Shell | ✅ |

---

## 🏗️ Arquitetura

### Visão Geral

```
┌─────────────────────────────────────────────────────────────────┐
│                         WebServ                                  │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│    ┌──────────────┐     ┌──────────────┐     ┌──────────────┐   │
│    │  ConfigFile  │────▶│  ServerBlock │────▶│LocationBlock │   │
│    └──────────────┘     └──────────────┘     └──────────────┘   │
│           │                                                      │
│           ▼                                                      │
│    ┌──────────────────────────────────────────────────────┐     │
│    │                   EpollInstance                       │     │
│    │              (Singleton - I/O Multiplexing)           │     │
│    └──────────────────────────────────────────────────────┘     │
│           │                                                      │
│           ▼                                                      │
│    ┌──────────────┐     ┌──────────────┐     ┌──────────────┐   │
│    │ ServerListen │────▶│    Client    │────▶│  CgiHandler  │   │
│    │   (accept)   │     │  (request)   │     │   (fork)     │   │
│    └──────────────┘     └──────────────┘     └──────────────┘   │
│           │                    │                    │            │
│           └────────────────────┴────────────────────┘            │
│                          │                                       │
│                          ▼                                       │
│    ┌──────────────┐     ┌──────────────┐     ┌──────────────┐   │
│    │  HttpRequest │────▶│ HttpResponse │────▶│CookieHandler │   │
│    │   (parse)    │     │  (generate)  │     │  (session)   │   │
│    └──────────────┘     └──────────────┘     └──────────────┘   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Componentes Principais

#### 🔄 EpollInstance (Singleton)
O coração do servidor. Implementa I/O multiplexing usando `epoll`, permitindo monitorar milhares de conexões com um único file descriptor. Todas as operações de I/O passam por aqui.

```cpp
// Loop principal - único epoll_wait para todo o servidor
while (true) {
    int ready = EpollInstance::manipEpollWait();
    epollReadyListLoop(ready);
    epollValidationLoop();
    EpollInstance::deletePendingRemovals();
}
```

#### 🔌 EpollHandler (Base Abstrata)
Classe base que define a interface para todos os handlers de eventos. Permite polimorfismo elegante para diferentes tipos de conexões.

```
EpollHandler (abstract)
    ├── ServerListen  → Aceita novas conexões
    ├── Client        → Processa requisições HTTP
    └── CgiHandler    → Gerencia processos CGI
```

#### 📨 HttpRequest / HttpResponse
Parsing e geração de mensagens HTTP, seguindo rigorosamente o RFC 2616.

#### ⚙️ ConfigFile / ServerBlock / LocationBlock
Sistema de configuração hierárquico inspirado no NGINX:

```
ConfigFile
    └── ServerBlock (múltiplos)
            └── LocationBlock (múltiplos)
```

---

## 🚀 Instalação

### Pré-requisitos

- Compilador C++ com suporte a C++98
- Make
- Sistema Linux (epoll)
- Python3, Perl (para CGI)

### Compilação

```bash
# Clone o repositório
git clone <repository-url>
cd ama_webserv

# Compile o projeto
make

# Ou com flags de debug
make debug
```

### Regras do Makefile

| Comando | Descrição |
|---------|-----------|
| `make` | Compila o projeto |
| `make clean` | Remove objetos |
| `make fclean` | Remove objetos e executável |
| `make re` | Recompila tudo |
| `make debug` | Compila com símbolos de debug |

---

## 💻 Uso

### Execução Básica

```bash
# Com arquivo de configuração padrão
./webserv

# Com arquivo de configuração específico
./webserv configs/default.conf
```

### Exemplo de Configuração

```nginx
server {
    # Porta e interface
    listen 127.0.0.1:8080;
    server_name localhost;
    
    # Diretório raiz
    root ./www;
    
    # Limite de body
    client_max_body_size 10M;
    
    # Páginas de erro personalizadas
    error_page 404 /error/404.html;
    error_page 500 /error/500.html;
    
    # Location principal
    location / {
        index index.html;
        autoindex on;
        allow_methods GET POST DELETE;
        cookies_enabled on;
    }
    
    # Upload de arquivos
    location /upload {
        can_upload on;
        upload_path ./www/upload;
        allow_methods GET POST DELETE;
    }
    
    # Scripts CGI
    location /cgi-bin/ {
        cgi_extensions .py .pl .sh;
        allow_methods GET POST;
    }
    
    # Redirecionamento
    location /google {
        return 302 https://www.google.com;
    }
}
```

### Diretivas Suportadas

#### Server Block
| Diretiva | Descrição | Exemplo |
|----------|-----------|---------|
| `listen` | IP:porta | `listen 0.0.0.0:8080;` |
| `server_name` | Nome do servidor | `server_name localhost;` |
| `root` | Diretório raiz | `root ./www;` |
| `client_max_body_size` | Tamanho máx. body | `client_max_body_size 10M;` |
| `error_page` | Página de erro | `error_page 404 /error/404.html;` |

#### Location Block
| Diretiva | Descrição | Valores |
|----------|-----------|---------|
| `index` | Arquivos padrão | `index index.html index.php;` |
| `autoindex` | Listagem de dir. | `on` / `off` |
| `allow_methods` | Métodos permitidos | `GET POST DELETE` |
| `alias` | Mapeamento de dir. | `alias ./www/files;` |
| `return` | Redirecionamento | `return 302 /new-path;` |
| `can_upload` | Permite upload | `on` / `off` |
| `upload_path` | Caminho upload | `upload_path ./uploads;` |
| `cgi_extensions` | Extensões CGI | `.py .pl .sh` |
| `cookies_enabled` | Habilita cookies | `on` / `off` |

---

## 🧪 Testes

### Scripts de Teste Incluídos

```bash
# Teste de stress (availability 99.5%+)
./ok_test_stress.sh

# Teste de cookies e sessões
./ok_test_cookies.sh

# Teste de edge cases
./ok_test_edge_cases.sh

# Teste de eficiência
./ok_test_efficiency.sh

# Teste de páginas de erro
./ok_test_error_pages.sh
```

### Testes Manuais

```bash
# GET simples
curl http://localhost:8080/

# POST com upload
curl -X POST -F "file=@image.png" http://localhost:8080/upload

# DELETE
curl -X DELETE http://localhost:8080/upload/image.png

# CGI Python
curl http://localhost:8080/cgi-bin/test.py

# Com cookies
curl -v http://localhost:8080/  # Primeira requisição recebe Set-Cookie
curl -H "Cookie: session_id=abc123" http://localhost:8080/

# Teste de stress com siege
siege -b -c 50 -t 30s http://localhost:8080/
```

### Verificação de Memory Leaks

```bash
valgrind --leak-check=full --show-leak-kinds=all ./webserv configs/default.conf
```

---

## 🎯 Pontos Diferenciais

### 1. Arquitetura Event-Driven Eficiente

O servidor utiliza **epoll** com **edge-triggered events** para máxima eficiência:

```cpp
// Eventos são processados apenas quando há mudança de estado
EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP
```

**Benefícios:**
- ✅ Um único syscall monitora todas as conexões
- ✅ Baixo consumo de CPU em idle
- ✅ Escala para milhares de conexões simultâneas

### 2. Design Pattern: Handler Polimórfico

Todos os handlers de I/O herdam de `EpollHandler`, permitindo tratamento uniforme:

```cpp
class EpollHandler {
    virtual void handleEpollIn() = 0;
    virtual void handleEpollOut() = 0;
};
```

**Resultado:** Código limpo, extensível e fácil de manter.

### 3. CGI Não-Bloqueante

Diferente de implementações simples que bloqueiam durante CGI, nosso servidor:

```cpp
// CGI executa em processo filho
pid_t pid = fork();
if (pid == 0) {
    execve(interpreter, argv, envp);
}
// Processo pai continua atendendo outras requisições
// Output do CGI é lido via epoll quando disponível
```

**Resultado:** Servidor nunca trava esperando CGI.

### 4. Gerenciamento de Timeout

Sistema de timeout integrado ao loop principal:

```cpp
void epollValidationLoop() {
    for (auto& handler : handlers) {
        handler->checkTimeout();  // Remove conexões inativas
    }
}
```

**Resultado:** Recursos são liberados automaticamente.

### 5. Sistema de Logging Composável

Arquitetura de logging flexível com múltiplos handlers:

```cpp
CompositeLogHandler* composite = new CompositeLogHandler();
composite->addHandler(new StdLogHandler());      // Console
composite->addHandler(new FileLogHandler());     // Arquivo
Logger::initLogger(DEBUG, composite);
```

### 6. Cookies com Segurança

Implementação de cookies seguindo boas práticas:

```cpp
"session_id=abc123; Path=/; HttpOnly"
```

- **Path=/**: Cookie válido para todo o site
- **HttpOnly**: Proteção contra XSS

---

## 📚 Aprendizados

### Conceitos de Redes

| Conceito | O que aprendemos |
|----------|------------------|
| **Sockets** | Criação, binding, listening, accepting |
| **TCP/IP** | Handshake, buffers, estados de conexão |
| **HTTP** | Parsing de headers, status codes, métodos |
| **I/O Multiplexing** | select, poll, epoll - trade-offs |

### Sistemas Operacionais

| Conceito | O que aprendemos |
|----------|------------------|
| **Processos** | fork, execve, waitpid, signals |
| **File Descriptors** | Non-blocking I/O, pipes, duplication |
| **Memory Management** | Alocação dinâmica, prevenção de leaks |

### Engenharia de Software

| Conceito | O que aprendemos |
|----------|------------------|
| **Design Patterns** | Singleton, Observer, Strategy |
| **SOLID** | Princípios aplicados na arquitetura |
| **Parsing** | Máquinas de estado, tokenização |
| **Testing** | Testes automatizados, stress testing |

### Protocolo HTTP em Profundidade

```
Request Line: GET /index.html HTTP/1.1
Headers:      Host: localhost
              Connection: keep-alive
              Cookie: session_id=xyz
Body:         (para POST/PUT)

Response:     HTTP/1.1 200 OK
              Content-Type: text/html
              Set-Cookie: session_id=abc
              
              <html>...</html>
```

---

## 📁 Estrutura do Projeto

```
ama_webserv/
├── 📄 Makefile
├── 📄 main.cpp
├── 📂 includes/
│   ├── WebservHeader.hpp      # Header principal
│   ├── EpollHandler.hpp       # Base abstrata
│   ├── EpollInstance.hpp      # Singleton epoll
│   ├── ServerListen.hpp       # Aceita conexões
│   ├── Client.hpp             # Processa requisições
│   ├── HttpRequest.hpp        # Parsing HTTP
│   ├── HttpResponse.hpp       # Geração HTTP
│   ├── CgiHandler.hpp         # Execução CGI
│   ├── CookieHandler.hpp      # Gerenciamento de sessão
│   ├── ConfigFile.hpp         # Parser de config
│   ├── ServerBlock.hpp        # Bloco server
│   ├── LocationBlock.hpp      # Bloco location
│   └── Logger.hpp             # Sistema de logging
├── 📂 source/
│   └── [implementações .cpp]
├── 📂 configs/
│   ├── default.conf           # Configuração padrão
│   └── [outras configs]
├── 📂 www/
│   ├── index.html
│   ├── 📂 cgi-bin/            # Scripts CGI
│   ├── 📂 upload/             # Diretório de uploads
│   └── 📂 error/              # Páginas de erro
├── 📂 error_pages/            # Páginas de erro padrão
└── 📂 docs/
    ├── WEBSERV_SUBJECT.md
    └── WEBSERV_REGUA.md
```

---

## 🔧 Referências Técnicas

- [RFC 2616 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [RFC 3875 - CGI](https://datatracker.ietf.org/doc/html/rfc3875)
- [Linux epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [NGINX Configuration](https://nginx.org/en/docs/)

---

## 👥 Autores

<div align="center">

| ![Avatar](https://github.com/jAzzvdou.png?size=100) | ![Avatar](https://github.com/mateusmedeir.png?size=100) | ![Avatar](https://github.com/PradoAllan.png?size=100) |
|:---:|:---:|:---:|
| **jazevedo** | **matlopes** | **aprado** |
| [@42login](https://github.com/jAzzvdou) | [@42login](https://github.com/mateusmedeir) | [@42login](https://github.com/PradoAllan) |

</div>

---

## 📜 Licença

Este projeto foi desenvolvido como parte do currículo da [42](https://42.fr/).

---

<div align="center">

**⭐ Se este projeto foi útil, considere dar uma estrela!**

*Made with ❤️ and lots of ☕ at 42*

</div>

