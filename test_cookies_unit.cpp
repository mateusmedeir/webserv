#include "includes/CookieHandler.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "🧪 TESTE UNITÁRIO - CookieHandler\n";
    std::cout << "=====================================\n\n";
    
    // Teste 1: generateSessionID
    std::cout << "Teste 1: generateSessionID()\n";
    std::string sessionId = CookieHandler::generateSessionID();
    std::cout << "  Session ID gerado: " << sessionId << "\n";
    assert(sessionId.find("sess_") == 0);
    assert(sessionId.length() > 15);
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 2: parseCookieHeader - simples
    std::cout << "Teste 2: parseCookieHeader() - simples\n";
    std::string cookieHeader = "session_id=abc123";
    std::map<std::string, std::string> cookies = CookieHandler::parseCookieHeader(cookieHeader);
    std::cout << "  Input: " << cookieHeader << "\n";
    std::cout << "  Output: session_id = " << cookies["session_id"] << "\n";
    assert(cookies["session_id"] == "abc123");
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 3: parseCookieHeader - múltiplos
    std::cout << "Teste 3: parseCookieHeader() - múltiplos\n";
    cookieHeader = "session_id=abc123; theme=dark; lang=pt";
    cookies = CookieHandler::parseCookieHeader(cookieHeader);
    std::cout << "  Input: " << cookieHeader << "\n";
    std::cout << "  Cookies parseados: " << cookies.size() << "\n";
    assert(cookies.size() == 3);
    assert(cookies["session_id"] == "abc123");
    assert(cookies["theme"] == "dark");
    assert(cookies["lang"] == "pt");
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 4: buildSetCookieHeader - básico
    std::cout << "Teste 4: buildSetCookieHeader() - básico\n";
    std::string setCookie = CookieHandler::buildSetCookieHeader("test", "value");
    std::cout << "  Output: " << setCookie << "\n";
    assert(setCookie.find("test=value") != std::string::npos);
    assert(setCookie.find("Path=/") != std::string::npos);
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 5: buildSetCookieHeader - com Max-Age
    std::cout << "Teste 5: buildSetCookieHeader() - com Max-Age\n";
    setCookie = CookieHandler::buildSetCookieHeader("test", "value", "/", 3600);
    std::cout << "  Output: " << setCookie << "\n";
    assert(setCookie.find("Max-Age=3600") != std::string::npos);
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 6: buildSetCookieHeader - deletar cookie
    std::cout << "Teste 6: buildSetCookieHeader() - deletar (Max-Age=0)\n";
    setCookie = CookieHandler::buildSetCookieHeader("test", "", "/", 0);
    std::cout << "  Output: " << setCookie << "\n";
    assert(setCookie.find("Max-Age=0") != std::string::npos);
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 7: parseCookieHeader - edge cases
    std::cout << "Teste 7: parseCookieHeader() - edge cases\n";
    
    // Cookie vazio
    cookies = CookieHandler::parseCookieHeader("");
    assert(cookies.size() == 0);
    std::cout << "  - Cookie vazio: ✅\n";
    
    // Cookie com espaços
    cookies = CookieHandler::parseCookieHeader("  name=value  ");
    assert(cookies["name"] == "value");
    std::cout << "  - Cookie com espaços: ✅\n";
    
    // Cookie com = no valor
    cookies = CookieHandler::parseCookieHeader("data=key=value");
    assert(cookies["data"] == "key=value");
    std::cout << "  - Cookie com = no valor: ✅\n";
    
    std::cout << "  ✅ TODOS PASSARAM\n\n";
    
    std::cout << "=====================================\n";
    std::cout << "✅ TODOS OS TESTES PASSARAM!\n";
    std::cout << "Implementação de cookies está correta!\n";
    
    return 0;
}

