# Pontos Faltantes para Completar o Projeto WebServ

Este documento lista os pontos que ainda precisam ser implementados ou corrigidos para que o projeto WebServ esteja 100% completo conforme o subject e a régua de avaliação.

---

## 🔴 Problemas Críticos (Podem Zerar a Nota)

### 1. Uso de `errno` após `read/write` (PROIBIDO)

**Localização no código:**
- `source/CgiProcess.cpp` linha 200: verificação de `errno` após `write()`
- `source/CgiProcess.cpp` linha 266: verificação de `errno` após `read()`

**Citação da Régua de Avaliação:**
> "Checking the value of errno to adjust the server behaviour is strictly forbidden after performing a read or write operation."
> 
> "If errno is checked after read/recv/write/send, the grade is 0 and the evaluation process ends now."

**Problema:**
O código atual verifica `errno` após chamadas de `read()` e `write()` nos pipes CGI:

```cpp
// CgiProcess.cpp:200
if (errno == EAGAIN || errno == EWOULDBLOCK) {
    // ...
}

// CgiProcess.cpp:266
if (errno == EAGAIN || errno == EWOULDBLOCK) {
    // ...
}
```

**Solução:**
Remover todas as verificações de `errno` após `read/write`. Em sockets non-blocking, o retorno da função já indica o estado:
- Retorno `-1`: pode ser erro ou EAGAIN (normal em non-blocking)
- Retorno `0`: EOF ou conexão fechada
- Retorno `> 0`: dados lidos/escritos

**Nota:** A verificação de `errno` em `ServerListen.cpp:36` após `accept()` é aceitável, pois a régua proíbe apenas após `read/recv/write/send`.

---

### 2. `handleGet/handlePost/handleDelete` não usam `LocationBlock`

**Localização no código:**
- `source/HttpResponse.cpp` linhas 13-56: métodos `handleGet()`, `handlePost()`, `handleDelete()`

**Citação do Subject:**
> "Specify rules or configurations on a URL/route (no regex required here), for a website, among the following:
> - List of accepted HTTP methods for the route.
> - HTTP redirection.
> - Directory where the requested file should be located (e.g., if URL /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for /tmp/www/pouic/toto/pouet).
> - Enabling or disabling directory listing.
> - Default file to serve when the requested resource is a directory.
> - Uploading files from the clients to the server is authorized, and storage location is provided."

**Problema:**
Os métodos `handleGet()`, `handlePost()` e `handleDelete()` não recebem `LocationBlock` como parâmetro e não utilizam:
- `alias` ou `root` para construir o caminho do arquivo
- `upload_path` para determinar onde salvar uploads
- `index` para procurar arquivos padrão em diretórios
- `autoindex` para listar diretórios
- `return` para redirecionamentos
- `allow_methods` para validar métodos permitidos

**Solução:**
Refatorar os métodos para receber `LocationBlock` e `ServerBlock` como parâmetros e usar todas as configurações do location.

---

## ⚠️ Funcionalidades Obrigatórias Faltando

### 3. Redirects (`return` directive)

**Localização no código:**
- `source/LocationBlock.cpp` linha 154-167: método `addReturn()` existe
- `source/LocationBlock.cpp` linha 63: método `getReturn()` existe
- **FALTA:** Uso em `dispatchRequest()`

**Citação do Subject:**
> "HTTP redirection."

**Citação da Régua de Avaliação:**
> "Try a redirected URL."

**Problema:**
O `LocationBlock` tem suporte para `return`, mas o `dispatchRequest()` não verifica nem processa redirecionamentos. Quando um location tem `return /pages/` ou `return https://www.google.com`, nada acontece.

**Solução:**
Em `dispatchRequest()`, antes de processar a requisição, verificar:
```cpp
if (!location.getReturn().empty()) {
    // Se começa com http:// ou https://, é redirecionamento externo (301)
    // Se começa com /, é redirecionamento interno (301 ou 302)
    setStatus(301, "Moved Permanently"); // ou 302 para temporário
    setHeader("Location", location.getReturn());
    return;
}
```

---

### 4. Autoindex (Listagem de Diretórios)

**Localização no código:**
- `source/LocationBlock.cpp` linha 103-119: método `addAutoIndex()` existe
- `source/LocationBlock.cpp` linha 59: método `getAutoIndex()` existe
- **FALTA:** Implementação em `handleGet()`

**Citação do Subject:**
> "Enabling or disabling directory listing."

**Citação da Régua de Avaliação:**
> "Try to list a directory."

**Problema:**
Quando `autoindex on` está configurado e a URI aponta para um diretório, o servidor deve gerar uma página HTML listando os arquivos do diretório. Atualmente, isso não está implementado.

**Solução:**
Em `handleGet()`, quando o caminho for um diretório:
1. Verificar se `location.getAutoIndex() == true`
2. Se sim, usar `opendir()`, `readdir()` e `closedir()` para listar arquivos
3. Gerar HTML com links para cada arquivo/diretório
4. Retornar o HTML gerado

**Exemplo de implementação:**
```cpp
if (isDirectory(path) && location.getAutoIndex()) {
    std::string html = generateDirectoryListing(path, req.getUri());
    setStatus(200, "OK");
    setBody(html, "text/html");
    return;
}
```

---

### 5. Index Files (Arquivos Padrão)

**Localização no código:**
- `source/LocationBlock.cpp` linha 184-202: método `addIndex()` existe
- `source/LocationBlock.cpp` linha 65: método `getIndex()` existe
- **FALTA:** Uso em `handleGet()`

**Citação do Subject:**
> "Default file to serve when the requested resource is a directory."

**Problema:**
Quando uma URI termina com `/` (diretório), o servidor deve procurar por arquivos padrão como `index.html`, `index.htm`, `index.php`, etc., conforme configurado no `index` directive. Atualmente, o código apenas adiciona `index.html` hardcoded.

**Solução:**
Em `handleGet()`, quando o caminho for um diretório:
1. Obter lista de `location.getIndex()` (ex: `["index.html", "index.htm", "index.php"]`)
2. Tentar abrir cada arquivo na ordem especificada
3. Se encontrar, servir esse arquivo
4. Se não encontrar e `autoindex` estiver desabilitado, retornar 404

**Exemplo:**
```cpp
if (isDirectory(path)) {
    std::vector<std::string> indexFiles = location.getIndex();
    for (size_t i = 0; i < indexFiles.size(); i++) {
        std::string indexPath = path + "/" + indexFiles[i];
        if (fileExists(indexPath)) {
            // Servir este arquivo
            return serveFile(indexPath);
        }
    }
    // Nenhum index encontrado
    if (!location.getAutoIndex()) {
        setStatus(404, "Not Found");
        return;
    }
}
```

---

### 6. Alias/Root (Caminho do Arquivo)

**Localização no código:**
- `source/LocationBlock.cpp` linha 139-152: método `addAlias()` existe
- `source/LocationBlock.cpp` linha 61-62: métodos `getAlias()` e `getRoot()` existem
- `source/HttpResponse.cpp` linha 117-135: método `uriToPath()` não usa alias/root

**Citação do Subject:**
> "Directory where the requested file should be located (e.g., if URL /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for /tmp/www/pouic/toto/pouet)."

**Problema:**
O método `uriToPath()` usa um caminho hardcoded `./www` e não considera:
- `location.getAlias()`: substitui o prefixo do location pelo alias
- `serverBlock.getRoot()`: diretório raiz do servidor

**Exemplo do problema:**
- Location: `/cgi-bin/` com `alias ./scripts/`
- URI: `/cgi-bin/test.py`
- Caminho esperado: `./scripts/test.py`
- Caminho atual: `./www/cgi-bin/test.py` ❌

**Solução:**
Refatorar `uriToPath()` para:
1. Encontrar o melhor location match
2. Se location tem `alias`, substituir o prefixo do location pelo alias
3. Se não tem alias, usar `root` do server block
4. Construir caminho correto: `alias + (URI - location_prefix)`

---

### 7. Upload Path (Caminho de Upload)

**Localização no código:**
- `source/LocationBlock.cpp` linha 169-182: método `addUploadPath()` existe
- `source/LocationBlock.cpp` linha 64: método `getUploadPath()` existe
- `source/HttpResponse.cpp` linha 30-44: `handlePost()` usa caminho fixo

**Citação do Subject:**
> "Uploading files from the clients to the server is authorized, and storage location is provided."

**Citação da Régua de Avaliação:**
> "Upload some file to the server and get it back."

**Problema:**
O método `handlePost()` sempre salva em `./uploads/upload.txt` hardcoded, ignorando:
- `location.getUploadPath()`: diretório configurado para uploads
- `location.getCanUpload()`: se uploads são permitidos
- Nome do arquivo da requisição (Content-Disposition header ou URI)

**Solução:**
Refatorar `handlePost()` para:
1. Verificar se `location.getCanUpload() == true`
2. Se não, retornar 403 Forbidden
3. Usar `location.getUploadPath()` como diretório base
4. Extrair nome do arquivo do header `Content-Disposition` ou da URI
5. Salvar em `uploadPath + "/" + filename`

---

## 🎁 Bonus (Múltiplos Tipos de CGI)

### 8. Parser de Configuração Restrito para CGI

**Localização no código:**
- `source/LocationBlock.cpp` linha 204-227: método `addCgiExtensions()`
- `source/CgiProcess.cpp` linha 93-115: método `getInterpreter()` suporta `.py`, `.php`, `.pl`, `.sh`

**Citação do Subject (Bonus):**
> "Handle multiple CGI types."

**Problema:**
O parser de configuração em `addCgiExtensions()` só aceita `.php` e `.py` (linha 216):
```cpp
if (tokens[0] != ".php" && tokens[0] != ".py")
    throw std::runtime_error("Configuração inválida: cgi_extensions: extensão inválida");
```

Porém, o código de execução CGI em `CgiProcess::getInterpreter()` já suporta:
- `.py` → `/usr/bin/python3`
- `.php` → `/usr/bin/php`
- `.pl` → `/usr/bin/perl`
- `.sh` → `/bin/bash`

**Solução:**
Remover a restrição no parser para permitir `.pl` e `.sh` também:
```cpp
if (tokens[0] != ".php" && tokens[0] != ".py" && tokens[0] != ".pl" && tokens[0] != ".sh")
    throw std::runtime_error("Configuração inválida: cgi_extensions: extensão inválida");
```

**Alternativa:**
Se não quiser suportar `.pl` e `.sh`, remover o suporte do `getInterpreter()` para manter consistência.

---

## 📋 Resumo das Prioridades

### 🔴 Crítico (Nota 0 se não corrigir)
1. ✅ Remover verificações de `errno` após `read/write`
2. ✅ Refatorar `handleGet/Post/Delete` para usar `LocationBlock`

### ⚠️ Obrigatório (Pontos perdidos se não implementar)
3. ✅ Implementar redirects (`return` directive)
4. ✅ Implementar autoindex (listagem de diretórios)
5. ✅ Implementar index files (arquivos padrão)
6. ✅ Corrigir uso de `alias/root` em `uriToPath()`
7. ✅ Corrigir uso de `upload_path` em `handlePost()`

### 🎁 Bonus (Pontos extras)
8. ✅ Permitir múltiplos tipos de CGI no parser (`.pl`, `.sh`)

---

## 📝 Notas Adicionais

### Verificações de `errno` Aceitáveis

A verificação de `errno` em `ServerListen.cpp:36` após `accept()` é **aceitável**, pois:
- A régua proíbe apenas após `read/recv/write/send`
- `accept()` não está na lista de funções proibidas
- É necessário para distinguir entre "não há mais conexões" (EAGAIN) e "erro real"

### Uso de `epoll` em CGI

Os pipes CGI (`CgiProcess::writeToStdin()` e `CgiProcess::readFromStdout()`) são chamados através de `CgiPipeHandler`, que é registrado no epoll. Portanto, **todos os read/write passam por epoll**, conforme exigido pela régua.

### Validação de Métodos HTTP

A validação de métodos permitidos (`allow_methods`) deve ser feita antes de chamar `handleGet/Post/Delete`. Verificar se o método da requisição está em `location.getAllowMethods()`.

