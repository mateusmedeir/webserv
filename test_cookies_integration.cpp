#include "includes/WebservHeader.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "🧪 TESTE DE INTEGRAÇÃO - Cookies\n";
    std::cout << "=====================================\n\n";
    
    // Teste 1: HttpRequest - Parse de Cookie header
    std::cout << "Teste 1: HttpRequest - Parse Cookie header\n";
    std::string rawRequest = 
        "GET /test HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Cookie: session_id=abc123; theme=dark\r\n"
        "\r\n";
    
    HttpRequest request(rawRequest);
    
    std::cout << "  Cookies recebidos: " << request.getCookies().size() << "\n";
    std::cout << "  session_id = " << request.getCookie("session_id") << "\n";
    std::cout << "  theme = " << request.getCookie("theme") << "\n";
    
    assert(request.hasCookie("session_id"));
    assert(request.getCookie("session_id") == "abc123");
    assert(request.hasCookie("theme"));
    assert(request.getCookie("theme") == "dark");
    assert(!request.hasCookie("inexistente"));
    
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 2: HttpResponse - Set-Cookie único
    std::cout << "Teste 2: HttpResponse - Set-Cookie único\n";
    HttpResponse response;
    response.setStatus(200, "OK");
    response.setBody("Hello", "text/plain");
    response.setCookie("test_cookie", "test_value", "/", 3600);
    
    std::string httpResponse = response.toString();
    std::cout << "  Resposta contém Set-Cookie? " << (httpResponse.find("Set-Cookie:") != std::string::npos ? "SIM" : "NÃO") << "\n";
    std::cout << "  Contém test_cookie=test_value? " << (httpResponse.find("test_cookie=test_value") != std::string::npos ? "SIM" : "NÃO") << "\n";
    std::cout << "  Contém Path=/? " << (httpResponse.find("Path=/") != std::string::npos ? "SIM" : "NÃO") << "\n";
    std::cout << "  Contém Max-Age=3600? " << (httpResponse.find("Max-Age=3600") != std::string::npos ? "SIM" : "NÃO") << "\n";
    
    assert(httpResponse.find("Set-Cookie:") != std::string::npos);
    assert(httpResponse.find("test_cookie=test_value") != std::string::npos);
    assert(httpResponse.find("Path=/") != std::string::npos);
    assert(httpResponse.find("Max-Age=3600") != std::string::npos);
    
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 3: HttpResponse - Múltiplos Set-Cookie
    std::cout << "Teste 3: HttpResponse - Múltiplos Set-Cookie\n";
    HttpResponse response2;
    response2.setStatus(200, "OK");
    response2.setBody("Hello", "text/plain");
    response2.setCookie("cookie1", "value1", "/", 3600);
    response2.setCookie("cookie2", "value2", "/", 7200);
    response2.setCookie("cookie3", "value3", "/", 1800);
    
    std::string httpResponse2 = response2.toString();
    
    // Contar ocorrências de "Set-Cookie:"
    size_t count = 0;
    size_t pos = 0;
    while ((pos = httpResponse2.find("Set-Cookie:", pos)) != std::string::npos) {
        count++;
        pos++;
    }
    
    std::cout << "  Número de Set-Cookie headers: " << count << "\n";
    assert(count == 3);
    
    assert(httpResponse2.find("cookie1=value1") != std::string::npos);
    assert(httpResponse2.find("cookie2=value2") != std::string::npos);
    assert(httpResponse2.find("cookie3=value3") != std::string::npos);
    assert(httpResponse2.find("Max-Age=3600") != std::string::npos);
    assert(httpResponse2.find("Max-Age=7200") != std::string::npos);
    assert(httpResponse2.find("Max-Age=1800") != std::string::npos);
    
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 4: clearCookie (Max-Age=0)
    std::cout << "Teste 4: HttpResponse - clearCookie\n";
    HttpResponse response3;
    response3.setStatus(200, "OK");
    response3.setBody("Hello", "text/plain");
    response3.clearCookie("old_cookie");
    
    std::string httpResponse3 = response3.toString();
    std::cout << "  Contém old_cookie com Max-Age=0? " << (httpResponse3.find("old_cookie=") != std::string::npos && httpResponse3.find("Max-Age=0") != std::string::npos ? "SIM" : "NÃO") << "\n";
    
    assert(httpResponse3.find("old_cookie=") != std::string::npos);
    assert(httpResponse3.find("Max-Age=0") != std::string::npos);
    
    std::cout << "  ✅ PASSOU\n\n";
    
    // Teste 5: Request sem cookies
    std::cout << "Teste 5: HttpRequest - Sem cookies\n";
    std::string rawRequest2 = 
        "GET /test HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    
    HttpRequest request2(rawRequest2);
    std::cout << "  Cookies recebidos: " << request2.getCookies().size() << "\n";
    assert(request2.getCookies().size() == 0);
    assert(!request2.hasCookie("qualquer"));
    assert(request2.getCookie("qualquer") == "");
    
    std::cout << "  ✅ PASSOU\n\n";
    
    std::cout << "=====================================\n";
    std::cout << "✅ TODOS OS TESTES DE INTEGRAÇÃO PASSARAM!\n";
    std::cout << "\n📋 RESUMO:\n";
    std::cout << "  ✅ Parse de Cookie header funciona\n";
    std::cout << "  ✅ Múltiplos cookies são parseados corretamente\n";
    std::cout << "  ✅ Set-Cookie é gerado corretamente\n";
    std::cout << "  ✅ Múltiplos Set-Cookie funcionam\n";
    std::cout << "  ✅ clearCookie funciona (Max-Age=0)\n";
    std::cout << "  ✅ Request sem cookies é tratado corretamente\n";
    std::cout << "\n🎉 IMPLEMENTAÇÃO DE COOKIES 100% FUNCIONAL!\n";
    
    return 0;
}

