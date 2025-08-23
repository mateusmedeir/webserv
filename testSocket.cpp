#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Erro ao criar o socket()." << std::endl;
        return (-1);
    }
    else {
        std::cout << "Socket criado com sucesso!" << std::endl;
        std::cout << sockfd << std::endl;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(sockfd, (const struct sockaddr *)&address, sizeof(address)) == -1) {
        std::cerr << "Erro ao bindar o socket no endereço." << std::endl;
        return (-1);
    }
    else {
        std::cout << "Socket bindado ao endereço com sucesso!" << std::endl;
    }

    if (listen(sockfd, 5) == -1){
        std::cerr << "Erro ao por o socket em modo passivo." << std::endl;
        return (-1);
    }
    else {
        std::cout << "Socket em modo passivo!" << std::endl;
    }

    while (true) {
        //int clientFd = accept(sockfd, (struct sockaddr *)&address, sizeof(address));
        int clientFd = accept(sockfd, NULL, NULL);

        if (clientFd == -1) {
            std::cerr << "Erro ao aceitar a conexão." << std::endl;
        }
        else {
            std::cout << "Conexão aceita com sucesso!" << std::endl;
            write(clientFd, "Oie!\n", 5);
            //close(clientFd);
        }

    }

    return (0);
}