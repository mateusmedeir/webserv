#include "CookieHandler.hpp"

std::string CookieHandler::generateSessionID()
{
    //| Seed do random (apenas na primeira vez)
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(std::time(NULL));
        seeded = true;
    }
    
    std::stringstream ss;

    ss << "sess_" << std::time(NULL) << "_";
    
    //| Adicionar 8 caracteres hexadecimais aleatórios
    const char* hexChars = "0123456789abcdef";
    for (int i = 0; i < 8; i++)
        ss << hexChars[std::rand() % 16];
    
    return ss.str();
}

std::map<std::string, std::string> CookieHandler::parseCookieHeader(const std::string &header)
{
    std::map<std::string, std::string> cookies;
    
    if (header.empty()) //| Se header vazio, retorna map vazio
        return cookies;
    
    //| Dividir por ';'
    std::string currentPair;
    std::stringstream ss(header);
    
    while (std::getline(ss, currentPair, ';'))
    {
        //| Remover espaços no início
        size_t start = 0;
        while (start < currentPair.length() && std::isspace(currentPair[start]))
            start++;

        currentPair = currentPair.substr(start);
        
        //| Remover espaços no final
        size_t end = currentPair.length();
        while (end > 0 && std::isspace(currentPair[end - 1]))
            end--;

        currentPair = currentPair.substr(0, end);
        
        //| Se vazio após trim, pular
        if (currentPair.empty())
            continue;
        
        //| Encontrar primeiro '='
        size_t equalPos = currentPair.find('=');
        
        //| Se não tem '=', ignorar (cookie inválido)
        if (equalPos == std::string::npos)
            continue;
        
        //| Extrair name e value
        std::string name = currentPair.substr(0, equalPos);
        std::string value = "";
        
        //| Value é tudo depois do primeiro '='
        if (equalPos + 1 < currentPair.length())
            value = currentPair.substr(equalPos + 1);
        
        //| Adicionar ao map (se name não estiver vazio)
        if (!name.empty())
            cookies[name] = value;
    }
    
    return cookies;
}

std::string CookieHandler::buildSetCookieHeader(const std::string &name, const std::string &value, const std::string &path, int maxAge, bool httpOnly, bool secure)
{
    std::stringstream ss;
    
    //| Parte obrigatória: name=value
    ss << name << "=" << value;
    
    //| Path (padrão: "/")
    if (!path.empty())
        ss << "; Path=" << path;
    
    //| Max-Age (se >= 0)
    //| Max-Age=0 significa "deletar cookie"
    //| Max-Age=-1 (ou omitido) significa "session cookie"
    if (maxAge >= 0)
        ss << "; Max-Age=" << maxAge;
    
    //| HttpOnly (não acessível via JavaScript)
    if (httpOnly)
        ss << "; HttpOnly";
    
    // Secure (apenas HTTPS)
    if (secure)
        ss << "; Secure";
    
    return ss.str();
}

