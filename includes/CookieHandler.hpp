#pragma once

#include <string>
#include <map>
#include <sstream>
#include <cstdlib>
#include <ctime>

 //| Fornece funções estáticas para:
 //| Gerar session IDs únicos
 //| Parsear header "Cookie" (cliente -> servidor)
 //| Construir header "Set-Cookie" (servidor -> cliente)
 //| Esta classe não deve ser instanciada (apenas métodos estáticos)
 
class CookieHandler
{
    public:
        //| Formato: "sess_" + timestamp + "_" + 8 caracteres hexadecimais aleatórios
        //| Exemplo: "sess_1698674820_a3f9b2d1"
        static std::string generateSessionID();

        //| Parse do header "Cookie" enviado pelo cliente
        //| Converte: "session_id=abc123; theme=dark; lang=pt"
        //| Em: map { "session_id": "abc123", "theme": "dark", "lang": "pt" }
        static std::map<std::string, std::string> parseCookieHeader(const std::string &header);

        static std::string buildSetCookieHeader(
            const std::string &name,
            const std::string &value,
            const std::string &path = "/",
            int maxAge = -1,
            bool httpOnly = false,
            bool secure = false
        );

    private:
        CookieHandler(); //| Não instanciável
        CookieHandler(const CookieHandler &); //| Não copiável
        CookieHandler &operator=(const CookieHandler &); //| Não atribuível
};

