# 📚 Respostas às Dúvidas sobre as Melhorias Implementadas

Este documento responde detalhadamente todas as dúvidas sobre as melhorias implementadas no projeto WebServ.

---

## 6. Correção de URI to Path - URI + MIME Type

### ❓ **URI e MIME Type estão ligados?**

**Sim, mas não diretamente.** Eles trabalham em conjunto, mas são coisas diferentes:

1. **URI (Uniform Resource Identifier)**: 
   - É o caminho que o cliente solicita (ex: `/style.css`, `/script.js`, `/images/logo.png`)
   - Contém a **extensão do arquivo** no final

2. **MIME Type**:
   - É o tipo de conteúdo HTTP que o servidor precisa retornar no header `Content-Type`
   - É determinado **baseado na extensão do arquivo** que vem do URI

### Como Funciona o Fluxo:

```cpp
// 1. Cliente faz requisição: GET /style.css HTTP/1.1
std::string uri = "/style.css";

// 2. URI é convertido para caminho do sistema de arquivos
std::string path = uriToPath(uri);  // Retorna: "./www/style.css"

// 3. Arquivo é lido do disco
// ...

// 4. MIME type é determinado pela EXTENSÃO do arquivo (não pelo URI diretamente)
std::string mimeType = getMimeType(path);  // Retorna: "text/css"
//                                 ↑
//                    A função pega a extensão ".css" do path

// 5. Response é criada com o MIME type correto
response.setBody(conteudo, mimeType);  // Content-Type: text/css
```

### Código Relevante:

```cpp
// HttpResponse.cpp
std::string HttpResponse::uriToPath(const std::string &uri) const {
    // Converte URI para path do sistema
    return "./www" + uri;  // Preserva extensão original!
}

std::string HttpResponse::getMimeType(const std::string &path) const {
    // Extrai extensão do PATH (que veio do URI)
    size_t dotPos = path.rfind('.');
    std::string ext = path.substr(dotPos);  // ".css"
    
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    // ... etc
}
```

### Resumo:
- **URI** → caminho do recurso solicitado
- **MIME Type** → tipo de conteúdo (determinado pela **extensão** que vem do URI)
- Eles trabalham juntos para servir corretamente arquivos CSS, JS, imagens, etc.

---

## 7. Validação de URI e Location Corrigida

### ❓ **Agora consegue acessar arquivos de diretórios diferentes?**
**Ex: dir1/index1.html e dir2/index2.html**

**Sim!** A correção permite isso. Veja como funciona:

### Antes (❌ PROBLEMA):
```cpp
bool ServerBlock::isUriValid(const std::string uri) {
    // Só verificava correspondência EXATA
    std::map<std::string, LocationBlock>::iterator it = this->_locations.find(uri);
    if (it != this->_locations.end())
        return true;
    return false;  // ❌ /dir1/index1.html não casava com location "/"
}
```

**Problema**: Se você tinha `location /` configurado, apenas a URI exata `/` era aceita. `/dir1/index1.html` retornava 404.

### Depois (✅ CORRIGIDO):
```cpp
bool ServerBlock::isUriValid(const std::string uri) {
    // 1. Verifica correspondência exata (para locations específicos)
    std::map<std::string, LocationBlock>::iterator it = this->_locations.find(uri);
    if (it != this->_locations.end())
        return true;
    
    // 2. Verifica se URI COMEÇA com alguma location válida
    for (it = this->_locations.begin(); it != this->_locations.end(); ++it) {
        std::string locationPath = it->first;  // Ex: "/"
        if (uri.find(locationPath) == 0) {     // URI começa com "/"?
            return true;  // ✅ /dir1/index1.html casa com location "/"
        }
    }
    
    return false;
}
```

### Como Funciona na Prática:

**Configuração exemplo:**
```nginx
server {
    listen 8080;
    
    location / {
        root ./www;
        # Permite GET, POST, DELETE
    }
}
```

**Agora funciona:**
- ✅ `GET /dir1/index1.html` → Válido (começa com `/`)
- ✅ `GET /dir2/index2.html` → Válido (começa com `/`)
- ✅ `GET /style.css` → Válido (começa com `/`)
- ✅ `GET /images/logo.png` → Válido (começa com `/`)
- ✅ `GET /api/users` → Válido (começa com `/`)

**Depois:**
1. `isUriValid()` verifica se a URI começa com alguma location válida
2. Se sim, `uriToPath()` converte para o caminho real no disco
3. Arquivo é servido normalmente

### Limitações Importantes:

⚠️ **A validação ainda depende da estrutura de diretórios no disco:**
- A validação verifica se a **URI é válida** (baseado nas locations configuradas)
- Mas o arquivo **deve existir fisicamente** no diretório `./www/dir1/index1.html` ou `./www/dir2/index2.html`
- Se o arquivo não existir, você receberá 404 (não é erro de validação, é arquivo não encontrado)

**Resumo**: Sim, agora você pode acessar arquivos de qualquer diretório, desde que:
1. A URI comece com uma location válida (ex: `/`)
2. O arquivo exista fisicamente no caminho correspondente no disco

---

## 8. Timeout de Clientes Inativos (30 segundos)

### ❓ **Existe algum motivo específico para 30 segundos?**
**É um valor arbitrário, mas comum na indústria.** Algumas referências:

- **Nginx default**: 60 segundos (mas configurável)
- **Apache default**: 300 segundos (muito alto)
- **HTTP/1.1 keep-alive**: Típico entre 5-60 segundos
- **30 segundos**: Bom equilíbrio entre liberar recursos e não desconectar usuários legítimos

### Por que 30 segundos?

1. **Previene DoS por conexões idle**: Atacantes podem abrir muitas conexões e não usar, consumindo recursos
2. **Libera recursos rapidamente**: FD, memória, entrada no epoll
3. **Não muito agressivo**: 30s é suficiente para usuários normais (páginas geralmente carregam em <5s)
4. **Valor padrão comum**: Muitos servidores web usam valores similares

### ❓ **Como essa validação é feita?**

A validação é feita no **loop principal** do servidor, após cada iteração do `epoll_wait()`:

```cpp
void serverMainLoop() {
    while (true) {
        // 1. Aguardar eventos
        int numberOfReadySockets = RunTime::getEpoll().manipEpollWait();
        
        // 2. Processar eventos recebidos
        epollReadyListLoop(numberOfReadySockets);
        
        // 3. ✅ VERIFICAR TIMEOUTS (a cada iteração do loop)
        std::map<int, Client> &clients = RunTime::getClients();
        std::vector<int> fdsToDelete;
        
        for (std::map<int, Client>::iterator it = clients.begin(); 
             it != clients.end(); ++it) {
            if (it->second.isTimedOut(30)) {  // Verifica se > 30s inativo
                std::cout << "[Timeout] Client " << it->first 
                          << " inactive for >30s, closing connection" << std::endl;
                fdsToDelete.push_back(it->first);
            }
        }
        
        // 4. Remover clientes com timeout
        for (size_t i = 0; i < fdsToDelete.size(); ++i) {
            RunTime::deleteClient(fdsToDelete[i]);
        }
    }
}
```

### Como Funciona o Timestamp:

```cpp
// Client.hpp
private:
    time_t _lastActivity;  // Timestamp da última atividade

// Client.cpp
void Client::handleEpollIn(void) {
    this->updateActivity();  // ✅ Atualiza timestamp a cada leitura
    // ... resto do código
}

void Client::updateActivity(void) {
    this->_lastActivity = time(NULL);  // Atualiza para "agora"
}

bool Client::isTimedOut(int timeoutSeconds) const {
    time_t now = time(NULL);
    return (now - this->_lastActivity) > timeoutSeconds;  // Diferença > 30s?
}
```

### Performance:

✅ **É eficiente porque:**
- Verificação é **O(n)** onde n = número de clientes conectados
- Acontece apenas **depois** de processar eventos do epoll
- Usa `time(NULL)` que é rápido (apenas consulta clock do sistema)
- Não bloqueia o servidor (é no loop principal, não em thread separada)

⚠️ **Não impacta muito a performance porque:**
- Número de clientes é limitado pelo limite de FDs do sistema
- Comparação de timestamps é muito rápida
- Se houver muitos clientes, você provavelmente já tem outros problemas de escalabilidade

### ❓ **O `for(auto...)` não é permitido no C++98**

**Correto!** Você está certo. No código atual, já está usando iteradores explicitos (compatível com C++98):

```cpp
// ✅ CORRETO (C++98 compatível) - código atual
for (std::map<int, Client>::iterator it = clients.begin(); 
     it != clients.end(); ++it) {
    if (it->second.isTimedOut(30)) {
        // ...
    }
}
```

**O `for(auto...)` seria assim (C++11+):**
```cpp
// ❌ ERRADO para C++98 (mas aparece na documentação como exemplo)
for (auto it = clients.begin(); it != clients.end(); ++it) {
    // ...
}
```

**O código implementado está correto para C++98!** ✅

---

## 9. TCP_NODELAY para Reduzir Latência

### ❓ **Vale a pena desabilitar o algoritmo de Nagle?**

**Sim, geralmente vale a pena para servidores web!** Mas vamos entender o trade-off:

### O que é o Algoritmo de Nagle?

O algoritmo de Nagle **agrupa pequenos pacotes TCP** antes de enviar:
- Quando você faz `send("H")` seguido de `send("i")`, o Nagle espera um pouco
- Se mais dados chegarem, ele agrupa: `send("Hi")` em um único pacote
- **Objetivo**: Reduzir overhead de pacotes pequenos (headers TCP são 20+ bytes)

### Por que Desabilitar (TCP_NODELAY)?

Para **servidores web**, geralmente vale a pena desabilitar porque:

1. **Requisições HTTP são pequenas**: Headers HTTP geralmente cabem em 1-2 pacotes
2. **Latência é crítica**: Usuários sentem atrasos de 20-50ms
3. **Aplicações interativas**: Cada millisegundo conta
4. **Benefício do Nagle é mínimo**: Dados HTTP já são enviados em batches (request completo)

### Trade-off:

**Com Nagle (desabilitado = TCP_NODELAY):**
- ✅ **Latência menor**: 20-50% de redução em requisições pequenas
- ✅ **Melhor UX**: Páginas carregam mais rápido
- ❌ **Mais pacotes na rede**: Ligeiramente mais overhead de headers TCP

**Sem Nagle (habilitado):**
- ✅ **Menos pacotes**: Mais eficiente para muitas requisições pequenas
- ❌ **Latência maior**: 20-50ms de delay adicional por requisição
- ❌ **Pior UX**: Páginas demoram mais para carregar

### Quando Desabilitar (TCP_NODELAY = 1)?

✅ **Recomendado para:**
- Servidores web (HTTP)
- Aplicações interativas (WebSockets, APIs REST)
- Quando latência é mais importante que throughput

❌ **NÃO recomendado para:**
- Transferências de arquivos grandes (FTP, downloads)
- Quando throughput é mais importante que latência

### Implementação no Código:

```cpp
// ServerListen.cpp - após accept()
int flag = 1;
if (setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)) < 0) {
    std::cerr << "[Warning] Failed to set TCP_NODELAY" << std::endl;
}
```

### Resumo:

**Para um servidor web (WebServ 42), vale MUITO a pena desabilitar o Nagle!**
- Melhora latência percebida pelo usuário
- Requisições HTTP são pequenas, então o benefício do Nagle é mínimo
- É uma prática comum na indústria (Nginx, Apache, etc. fazem isso)

---

## 10. Handler SIGPIPE e MSG_NOSIGNAL

### ❓ **Como testamos esse caso do cliente desconectar durante o write?**

Ótima pergunta! Aqui estão algumas formas de testar:

### Teste 1: Cliente Desconecta Durante Envio (Manual)

**Usando `curl` em um terminal:**
```bash
# Terminal 1: Inicia servidor
./webserv configs/test.conf

# Terminal 2: Faz requisição e mata o processo antes de receber resposta
curl http://127.0.0.1:8080/ &
PID=$!
sleep 0.1  # Aguarda request ser enviado
kill -9 $PID  # Mata o curl (simula desconexão abrupta)
```

### Teste 2: Cliente Desconecta Durante Envio (Script)

```bash
#!/bin/bash
# test_sigpipe.sh

SERVER_URL="http://127.0.0.1:8080"

# Inicia requisição em background e mata antes de receber
(
    curl -s "$SERVER_URL/" > /dev/null &
    CURL_PID=$!
    sleep 0.05  # Tempo mínimo para request ser enviado
    kill -9 $CURL_PID 2>/dev/null  # Mata abruptamente
) &

# Repete várias vezes rapidamente
for i in {1..10}; do
    (
        curl -s "$SERVER_URL/" > /dev/null &
        sleep 0.01
        kill -9 $! 2>/dev/null
    ) &
done

wait
echo "Teste completo - verifique se o servidor não crashou"
```

### Teste 3: Usando `nc` (netcat) para Controle Manual

```bash
# Terminal 1: Servidor
./webserv configs/test.conf

# Terminal 2: Conecta e envia request
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc 127.0.0.1 8080

# Antes do servidor enviar resposta completa, pressione Ctrl+C
# Isso simula desconexão durante o write()
```

### Teste 4: Usando Python para Controle Fino

```python
#!/usr/bin/env python3
# test_disconnect.py

import socket
import time
import sys

def test_disconnect_during_send():
    # Conecta ao servidor
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 8080))
    
    # Envia request
    request = b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    sock.send(request)
    
    # Aguarda um pouco (servidor começa a processar)
    time.sleep(0.1)
    
    # Desconecta abruptamente (simula cliente fechando conexão)
    sock.close()  # Isso gera SIGPIPE no servidor se não estiver protegido
    
    print("Cliente desconectado - servidor deve continuar rodando")

if __name__ == "__main__":
    for i in range(10):
        test_disconnect_during_send()
        time.sleep(0.1)
    print("Teste completo")
```

### Teste 5: Usando `stress` para Múltiplas Conexões

```bash
# Instalar stress-ng (se não tiver)
# sudo apt install stress-ng

# Criar script que faz muitas requisições e mata rapidamente
for i in {1..100}; do
    curl -s http://127.0.0.1:8080/ > /dev/null &
    sleep 0.01
    # Mata alguns processos aleatoriamente
    if [ $((i % 5)) -eq 0 ]; then
        pkill -9 curl 2>/dev/null
    fi
done
```

### O que Verificar:

✅ **Com proteção (correto):**
- Servidor continua rodando após desconexões
- Log mostra: `[Signal] SIGPIPE received and ignored`
- Sem crashes ou segfaults

❌ **Sem proteção (erro):**
- Servidor crasha com: `Broken pipe (SIGPIPE)`
- Processo é terminado
- Servidor para de responder

### Proteção Implementada (Dupla Camada):

```cpp
// 1. Signal Handler (main.cpp)
void signalHandler(int signum) {
    if (signum == SIGPIPE) {
        std::cerr << "[Signal] SIGPIPE received and ignored" << std::endl;
        // Não faz nada - ignora o sinal
    }
}
signal(SIGPIPE, signalHandler);

// 2. MSG_NOSIGNAL no send() (Client.cpp)
ssize_t sent = send(fd, data, size, MSG_NOSIGNAL);
// MSG_NOSIGNAL previne que send() gere SIGPIPE
// Se conexão fechada, send() retorna -1 em vez de gerar sinal
```

**Resumo**: Use qualquer um dos métodos acima. O mais fácil é o Teste 2 (script bash) ou Teste 4 (Python).

---

## 11. Verificação Correta do Retorno do send()

### ❓ **Não achei nada no PDF falando sobre essa validação do send()**

**Correto!** Não está no PDF do projeto, mas está na **régua de avaliação** (subject/requirements).

A régua menciona explicitamente:
> "checking only -1 or 0 values is not enough, both should be checked"

### Por que é Importante?

O `send()` pode retornar **3 valores diferentes**:

1. **`sent > 0`**: Enviou X bytes (pode ser parcial!)
2. **`sent == 0`**: Conexão foi fechada pelo peer
3. **`sent < 0`**: Erro (pode ser EAGAIN em non-blocking ou erro real)

### Implementação Correta:

```cpp
bool Client::sendResponse(const std::string &responseStr) {
    ssize_t sent = send(this->getSocketFd(), data, remaining, MSG_NOSIGNAL);
    
    // ✅ VERIFICAÇÃO CORRETA: AMBOS os casos
    if (sent < 0) {
        // Erro (EAGAIN ou erro real)
        // Adicionar EPOLLOUT e aguardar socket ficar pronto
        events |= EPOLLOUT;
        return true;
    } 
    else if (sent == 0) {  // ✅ IMPORTANTE: Verificar == 0
        // Conexão fechada pelo peer
        // Remover cliente conforme régua
        RunTime::deleteClient(this->getSocketFd());
        return false;
    } 
    else {
        // Enviou alguns bytes (sent > 0)
        this->_responseOffset += sent;
        // Verificar se ainda há mais para enviar
        if (this->_responseOffset < this->_pendingResponse.size()) {
            events |= EPOLLOUT;  // Continuar enviando depois
        }
        return true;
    }
}
```

### Por que `sent == 0` é Especial?

**`send() == 0` significa que a conexão foi fechada pelo peer:**
- Não é um erro tradicional (por isso `errno` não é setado)
- Mas indica que não podemos mais enviar dados
- **Conforme régua**: "if an error is returned, the client is removed"
- `sent == 0` é tratado como "erro" (conexão fechada)

### Resumo:

- Não está no PDF do projeto
- **Está na régua de avaliação** (subject)
- É uma verificação **obrigatória** para passar na avaliação
- Sem essa verificação, você pode perder pontos ou até zerar a nota
- Implementação correta verifica **ambos** `sent < 0` **E** `sent == 0`

---

## 14. Suporte a Envio Parcial e EPOLLOUT

### ❓ **Não entendi**

Vou explicar de forma mais clara! É um conceito importante de I/O não-bloqueante.

### O Problema:

Quando você faz `send()` em um socket **non-blocking**, pode acontecer:

```cpp
std::string response = "HTTP/1.1 200 OK\r\n...\r\n\r\n<body muito grande>...";
ssize_t sent = send(fd, response.c_str(), response.size(), 0);
// sent pode ser MENOR que response.size()!
// Exemplo: response tem 10KB, mas send() retorna apenas 8KB
```

**Por quê?**
- O **buffer de saída do socket está cheio**
- O TCP não consegue enviar todos os dados de uma vez
- `send()` retorna quantos bytes **conseguiu enviar** (não todos!)

### Solução: Envio Parcial + EPOLLOUT

**EPOLLOUT** é um evento do epoll que diz: *"O socket está pronto para escrita novamente!"*

### Fluxo Completo:

```
1. Cliente faz request
   ↓
2. Servidor prepara resposta (ex: 10KB)
   ↓
3. send() tenta enviar 10KB
   ↓
4. send() retorna apenas 8KB (buffer cheio!)
   ↓
5. ✅ Guardamos os 2KB restantes
   ✅ Adicionamos EPOLLOUT ao epoll (monitorar quando socket ficar pronto)
   ↓
6. Epoll detecta que socket está pronto (EPOLLOUT)
   ↓
7. handleEpollOut() é chamado
   ↓
8. send() envia os 2KB restantes
   ↓
9. ✅ Tudo enviado! Remove cliente
```

### Implementação no Código:

```cpp
// Client.hpp
private:
    std::string  _pendingResponse;  // Resposta completa (ex: 10KB)
    size_t       _responseOffset;   // Quantos bytes já foram enviados (ex: 8KB)

// Client.cpp
bool Client::sendResponse(const std::string &responseStr) {
    // 1. Guarda resposta completa na primeira vez
    if (this->_pendingResponse.empty()) {
        this->_pendingResponse = responseStr;  // Guarda 10KB
        this->_responseOffset = 0;             // Começa do início
    }
    
    // 2. Envia apenas o que falta
    const char *data = this->_pendingResponse.c_str() + this->_responseOffset;
    //                              ↑
    //                    Começa do byte 8000 (já enviou 8KB)
    
    size_t remaining = this->_pendingResponse.size() - this->_responseOffset;
    //                  ↑ 10KB                    ↑ 8KB  = 2KB restantes
    
    ssize_t sent = send(this->getSocketFd(), data, remaining, MSG_NOSIGNAL);
    
    if (sent > 0) {
        this->_responseOffset += sent;  // Atualiza: 8KB + 2KB = 10KB
        
        if (this->_responseOffset < this->_pendingResponse.size()) {
            // Ainda há dados! Adiciona EPOLLOUT para continuar depois
            events |= EPOLLOUT;
            epoll_ctl(..., EPOLL_CTL_MOD, ...);  // Monitora quando socket ficar pronto
        } else {
            // Tudo enviado! ✅
            RunTime::deleteClient(this->getSocketFd());
        }
    }
}

// Quando epoll detecta que socket está pronto (EPOLLOUT):
void Client::handleEpollOut(void) {
    // Continua enviando o que falta
    if (!sendResponse(this->_pendingResponse)) {
        return;  // Erro, cliente já foi removido
    }
}
```

### Exemplo Visual:

```
Resposta: [████████░░] 10KB
          Enviado: 8KB  Falta: 2KB

send() retorna 8KB
↓
Guardamos: offset = 8KB
Adicionamos EPOLLOUT ao epoll
↓
... espera socket ficar pronto ...
↓
Epoll aciona EPOLLOUT
↓
handleEpollOut() chama sendResponse()
↓
send() envia os 2KB restantes
↓
Resposta: [████████████] 10KB
          Enviado: 10KB  Falta: 0KB ✅
```

### Por que é Importante?

1. **Sem envio parcial**: Servidor pode travar ou perder dados
2. **Com EPOLLOUT**: Servidor continua funcionando normalmente mesmo com buffers cheios
3. **Conformidade**: É um requisito da régua para I/O não-bloqueante completo

### Resumo:

- **Envio parcial**: `send()` pode não enviar tudo de uma vez
- **EPOLLOUT**: Evento que avisa quando socket está pronto para mais escrita
- **handleEpollOut()**: Continua enviando o que faltou
- **Objetivo**: Garantir que toda a resposta seja enviada, mesmo que em partes

**É como encher um balde com uma mangueira: se o balde enche, você espera ele esvaziar um pouco antes de continuar enchendo!** 🪣

---

## 📝 Resumo Geral

| Melhoria | Resposta Principal |
|----------|-------------------|
| **URI + MIME Type** | Trabalham juntos: URI tem extensão → MIME type é determinado pela extensão |
| **Validação URI/Location** | Sim, agora funciona com qualquer diretório (verifica se URI começa com location válida) |
| **Timeout 30s** | Valor comum na indústria, verificação no loop principal a cada epoll_wait() |
| **C++98 for(auto)** | Código atual já está correto (usa iteradores explicitos) |
| **TCP_NODELAY** | Vale a pena desabilitar para servidores web (reduz latência 20-50%) |
| **SIGPIPE** | Testar com scripts que matam clientes durante envio |
| **Validação send()** | Está na régua (não no PDF), verifica `sent < 0` E `sent == 0` |
| **EPOLLOUT** | Permite continuar enviando dados quando socket buffer fica cheio |