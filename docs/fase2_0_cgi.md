# 🚀 Implementação CGI - WebServ

**Suporte:** Python (.py) e PHP (.php)

---

## 📊 RESUMO

Implementamos suporte completo a CGI (Common Gateway Interface) versão 1.1 no WebServ, permitindo execução de scripts Python e PHP com:

- ✅ Fork + Execve para isolamento de processos
- ✅ Pipes não-bloqueantes para comunicação
- ✅ Variáveis de ambiente CGI/1.1 completas
- ✅ Suporte a GET (query string)
- ✅ Suporte a POST (body data)
- ✅ Timeout de 5 segundos
- ✅ Tratamento de erros robusto
- ✅ Integração com arquitetura existente

---

## 🏗️ ARQUITETURA DA IMPLEMENTAÇÃO

### Arquivos Criados:

1. **`includes/Cgi.hpp`** - Classe Cgi (herda de EpollHandler)
2. **`source/Cgi.cpp`** - Implementação completa do CGI
3. **Modificado:** `source/HttpResponse.cpp` - Detecção e execução CGI
4. **Modificado:** `includes/HttpResponse.hpp` - Métodos CGI
5. **Modificado:** `source/ServerBlock.cpp` - Validação de URI melhorada

### Scripts de Teste Criados:

1. **`www/cgi-bin/hello.py`** - Teste GET Python
2. **`www/cgi-bin/post_test.py`** - Teste POST Python
3. **`www/cgi-bin/hello.php`** - Teste GET PHP
4. **`www/cgi_test.html`** - Página de teste HTML

---

## 🎯 CLASSE CGI

### Herança e Integração:

```cpp
class Cgi : public EpollHandler
```

**Vantagens:**
- Integração natural com sistema de eventos Epoll
- Reutilização de `handleEpollIn()` e `handleEpollOut()`
- Gerenciamento consistente com outros handlers

### Estados do CGI:

```cpp
enum CgiState {
    WRITING_TO_CGI,   // Escrevendo body no stdin do script
    READING_FROM_CGI, // Lendo output do stdout do script  
    DONE              // Processamento concluído
};
```

### Pipes Não-Bloqueantes:

```cpp
int _pipeFdIn[2];   // stdin do script (write)
int _pipeFdOut[2];  // stdout do script (read)
```

**Configuração:**
- `O_NONBLOCK` em ambos os pipes
- Integrados ao Epoll para I/O assíncrono
- Fechamento automático no destrutor

---

## 🔧 VARIÁVEIS DE AMBIENTE CGI/1.1

Implementamos todas as variáveis obrigatórias do padrão CGI/1.1:

| Variável | Descrição | Exemplo |
|----------|-----------|---------|
| `GATEWAY_INTERFACE` | Versão CGI | `CGI/1.1` |
| `SERVER_SOFTWARE` | Nome do servidor | `WebServ/1.0` |
| `SERVER_PROTOCOL` | Protocolo HTTP | `HTTP/1.1` |
| `REQUEST_METHOD` | Método HTTP | `GET`, `POST`, `DELETE` |
| `SCRIPT_FILENAME` | Caminho completo do script | `/path/to/script.py` |
| `QUERY_STRING` | Parâmetros GET | `name=John&age=30` |
| `PATH_INFO` | Parte do caminho após script | `/extra/path` |
| `CONTENT_TYPE` | Tipo do body (POST) | `application/x-www-form-urlencoded` |
| `CONTENT_LENGTH` | Tamanho do body (POST) | `42` |
| `HTTP_COOKIE` | Cookies (para bônus 2) | `session_id=abc123` |
| `REDIRECT_STATUS` | Status para PHP | `200` |

---

## 🔄 FLUXO DE EXECUÇÃO CGI

### 1. Detecção:

```cpp
bool HttpResponse::isCgiRequest(const std::string &uri) const {
    // Verifica extensão .py ou .php
    if (uri.ends_with(".py") || uri.ends_with(".php"))
        return true;
    return false;
}
```

### 2. Criação do Processo:

```cpp
void Cgi::execute(void) {
    // 1. Criar pipes
    setupPipes();
    
    // 2. Construir variáveis de ambiente
    buildEnvVars();
    
    // 3. Fork
    _pid = fork();
    
    if (_pid == 0) {
        // Processo filho
        executeChild();  // execve()
    }
    
    // 4. Processo pai
    setupParent();
}
```

### 3. Processo Filho (Script CGI):

```cpp
void Cgi::executeChild(void) {
    // Redirecionar stdin/stdout
    dup2(_pipeFdIn[0], STDIN_FILENO);
    dup2(_pipeFdOut[1], STDOUT_FILENO);
    
    // Executar script
    char *argv[] = {script_path, NULL};
    execve(script_path, argv, env_vars);
    
    // Se chegou aqui, erro!
    exit(EXIT_FAILURE);
}
```

### 4. Processo Pai (WebServ):

**EPOLLOUT (Escrever):**
```cpp
void Cgi::handleEpollOut(void) {
    // Escrever body no stdin do script (POST)
    write(_pipeFdIn[1], body, remaining);
    
    // Quando terminar, fechar pipe e mudar para leitura
    if (done) {
        close(_pipeFdIn[1]);
        setSocketFd(_pipeFdOut[0]);
        _state = READING_FROM_CGI;
    }
}
```

**EPOLLIN (Ler):**
```cpp
void Cgi::handleEpollIn(void) {
    // Ler output do stdout do script
    char buffer[4096];
    int bytes = read(_pipeFdOut[0], buffer, 4096);
    
    if (bytes == 0) {
        // EOF - script terminou
        waitpid(_pid, &status, 0);
        parseOutput();  // Separar headers/body
        _state = DONE;
    } else {
        _cgiOutput.append(buffer, bytes);
    }
}
```

### 5. Parse do Output:

```cpp
void Cgi::parseOutput(void) {
    // Procurar separador headers/body
    size_t end = _cgiOutput.find("\r\n\r\n");
    
    // Separar
    std::string headers = _cgiOutput.substr(0, end);
    std::string body = _cgiOutput.substr(end + 4);
    
    // Parsear headers (Content-Type, Status, etc)
    // ...
    
    // Configurar resposta HTTP
    _response.setBody(body, contentType);
    _response.setStatus(200, "OK");
}
```

---

## ⏱️ GERENCIAMENTO DE TIMEOUT

### Implementação:

```cpp
bool Cgi::isTimedOut(int maxSeconds) {
    time_t now = time(NULL);
    return (now - _startTime) > maxSeconds;
}

void Cgi::killProcess(void) {
    if (_pid > 0) {
        kill(_pid, SIGKILL);
        waitpid(_pid, NULL, 0);
        _pid = -1;
    }
}
```

**Timeout:** 5 segundos (configurável)  
**Ação:** SIGKILL + waitpid + erro 504 Gateway Timeout

---

## 🐛 TRATAMENTO DE ERROS

| Erro | Status HTTP | Descrição |
|------|-------------|-----------|
| Script não encontrado | 404 | Arquivo não existe |
| Falha no pipe() | 500 | Erro ao criar pipes |
| Falha no fork() | 500 | Erro ao criar processo |
| Falha no execve() | 500 | Script não executável |
| Timeout excedido | 504 | Script demorou > 5s |
| Output inválido | 500 | Sem separador headers/body |
| Exit code != 0 | 500 | Script terminou com erro |

---

## 📝 EXEMPLOS DE USO

### GET Request:

```bash
curl http://localhost:8080/cgi-bin/hello.py
```

**Variáveis passadas:**
- `REQUEST_METHOD=GET`
- `QUERY_STRING=`
- `SCRIPT_FILENAME=./www/cgi-bin/hello.py`

### GET com Query String:

```bash
curl "http://localhost:8080/cgi-bin/hello.py?name=John&age=30"
```

**Variáveis passadas:**
- `REQUEST_METHOD=GET`
- `QUERY_STRING=name=John&age=30`

### POST Request:

```bash
curl -X POST -d "name=John&message=Hello" \
    http://localhost:8080/cgi-bin/post_test.py
```

**Variáveis passadas:**
- `REQUEST_METHOD=POST`
- `CONTENT_TYPE=application/x-www-form-urlencoded`
- `CONTENT_LENGTH=28`

**Dados:**
- Body enviado via stdin do script

---

## 🧪 SCRIPTS DE TESTE

### 1. hello.py (GET):

```python
#!/usr/bin/env python3
import os

print("Content-Type: text/html\r")
print("\r")
print(f"<h1>Hello from Python!</h1>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING')}</p>")
```

### 2. post_test.py (POST):

```python
#!/usr/bin/env python3
import os
import sys

# Ler body
content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
body = sys.stdin.read(content_length)

print("Content-Type: text/html\r")
print("\r")
print(f"<h1>POST Received!</h1>")
print(f"<p>Data: {body}</p>")
```

### 3. hello.php (GET):

```php
#!/usr/bin/php
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<h1>Hello from PHP!</h1>";
echo "<p>Method: " . getenv('REQUEST_METHOD') . "</p>";
echo "<p>Query: " . getenv('QUERY_STRING') . "</p>";
?>
```

---

## 🔒 SEGURANÇA E BOAS PRÁTICAS

### Implementado:

- ✅ **Isolamento de processos:** Fork garante que script roda em processo separado
- ✅ **Timeout:** Previne scripts infinitos (5 segundos)
- ✅ **Pipes não-bloqueantes:** Previne deadlocks
- ✅ **Cleanup automático:** Destrutor fecha pipes e mata processos
- ✅ **waitpid():** Previne processos zumbis
- ✅ **Validação de path:** Script deve existir e ser acessível
- ✅ **Exit code:** Verifica se script terminou corretamente

### Futuras Melhorias:

- [ ] Validação de permissões de execução
- [ ] Chroot para sandbox adicional
- [ ] Limite de memória por processo CGI
- [ ] Rate limiting de requisições CGI
- [ ] Whitelist de scripts permitidos

---

## ✅ CHECKLIST DE IMPLEMENTAÇÃO

### Core:
- [x] Classe Cgi criada
- [x] Herança de EpollHandler
- [x] Fork + Execve implementado
- [x] Pipes não-bloqueantes
- [x] Variáveis de ambiente CGI/1.1
- [x] Parse de output (headers + body)

### Métodos HTTP:
- [x] GET suportado
- [x] GET com query string
- [x] POST suportado
- [x] POST com body

### Linguagens:
- [x] Python (.py)
- [x] PHP (.php)

### Tratamento de Erros:
- [x] Script não encontrado (404)
- [x] Erro no pipe (500)
- [x] Erro no fork (500)
- [x] Erro no execve (500)
- [x] Timeout (504)
- [x] Output inválido (500)

### Segurança:
- [x] Timeout de 5 segundos
- [x] Cleanup de recursos
- [x] waitpid() para zumbis
- [x] Verificação de exit code

### Testes:
- [x] Scripts de teste criados
- [x] Página HTML de teste
- [x] Compilação sem erros

---

## 🚀 PRÓXIMOS PASSOS

10. [ ] Implementar Cookies (Bônus 2)
11. [ ] Passar cookies via HTTP_COOKIE para CGI
12. [ ] Permitir CGI setar cookies via Set-Cookie
13. [ ] Testes finais completos

