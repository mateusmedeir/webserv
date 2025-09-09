#include "WebservHeader.hpp"

int main(void) {
    RunTime runtime = RunTime();


    return (0);
}

// #include <sys/socket.h>
// #include <iostream>
// #include <netinet/in.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/epoll.h>
// #include <cerrno>

// #include "WebservHeader.hpp"
// #include "RunTime.hpp"


// int set_nonblocking(int sockfd) {
//     int flags = fcntl(sockfd, F_GETFL, 0);
//     if (flags == -1) {
//         return (std::cerr << "fcntl(F_GETFL)" << std::endl, -1);
//         // perror("fcntl(F_GETFL)");
//         // return -1;
//     }
//     if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
//         return (std::cerr << "fcntl(F_SETFL)", -1);
//         // perror("fcntl(F_SETFL)");
//         // return -1;
//     }
//     return 0;
// }


// int main(void) {
//     int serverFD;
//     int epollFd;
//     struct sockaddr_in addr;
//     struct epoll_event epoll;
//     struct epoll_event ready_list[MAX_EVENTS];

//     serverFD = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverFD == -1) {
//         return (std::cerr << "Erro ao criar o socket." << std::endl, -1);
//     }
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = htons(INADDR_ANY);
//     if (bind(serverFD, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
//         return (std::cerr << "Erro ao bindar o socket." << std::endl, -1);
//     }
//     if (set_nonblocking(serverFD) == -1) {
//         return (std::cerr << "Erro ao setar o comportamento nao bloqueante." << std::endl, -1);
//     }
//     if (listen(serverFD, MAX_EVENTS) == -1) {
//         return (std::cerr << "Erro ao por o server em modo passivo." << std::endl, -1);
//     }
//     epollFd = epoll_create(1);
//     if (epollFd == -1) {
//         return (std::cerr << "Erro ao criar a instancia de epoll." << std::endl, -1);
//     }
//     epoll.events = EPOLLIN;
//     epoll.data.fd = serverFD;
//     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFD, &epoll) == -1) {
//         return (std::cerr << "Erro ao adicionar o serverFD na interest list." << std::endl, -1);
//     }

//     while (true) {
//         int n_ready_events = epoll_wait(epollFd, ready_list, MAX_EVENTS, -1);
//         if (n_ready_events == -1) {
//             std::cerr << "Erro no epoll wait." << std::endl;
//             break;
//         }

//         for (int i = 0; i < n_ready_events; i++) {
//             //percorrer os fds que estao prontos
//             if (ready_list[i].data.fd == serverFD) {
//                 //Nova conexao realizada. Precisamos dar o accept
//                 //por em modo nao bloqueante
//                 //Jogar na interest list
//                 //Voltar ao loop principal do wait
//                 while (true) {
//                     //Esse loop e importante pois, caso o serverFD tenha ido para a ready list
//                     //Significa que existem novas conexoes prontas para serem processadas
//                     //Por nao sabermos quantas conexoes sao, precisamos ir aceitando ate dar o erro.
//                     //mas esse erro so acontece se o serverFD estiver em modo nao bloqueante
//                     struct sockaddr_in client_addr;
//                     socklen_t client_len = sizeof(client_addr);
//                     int client_fd = accept(serverFD, (struct sockaddr *)&client_addr, &client_len);

//                     if (client_fd == -1) {
//                         if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                             //nao tem mais conexoes para aceitar
//                             //sai do loop de aceitar novas conexoes
//                             break;
//                         } else {
//                             std::cerr << "Erro ao aceitar novas conexoes." << std::endl;
//                             break;
//                         }
//                     }
//                     //setar o novo client fd como nao bloqueante
//                     set_nonblocking(client_fd);
//                     //Devemos adicionar esse novo client a interest list.
//                     //dessa forma, nossa instancia de epoll vai monitorar esse novo FD tbm
//                     epoll.events = EPOLLIN | EPOLLOUT;
//                     epoll.data.fd = client_fd;
//                     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, client_fd, &epoll) == -1) {
//                         std::cerr << "Erro ao adicionar a nova conexao a interest list." << std::endl;
//                         close(client_fd);
//                     }
//                 }
//             } else {
//                 //Se cair no else, nao temos uma nova conexao para aceitar
//                 //Isso quer dizer que o evento que ocorreu, isto e, o(s) fd(s) na ready list
//                 //e de algum cliente que ja havia sido adicionado
//                 //e agora ele esta pronto para I/O
//                 int client_fd = ready_list[i].data.fd;
//                 int count = 0;
//                 char buff[1024];

//                 while ((count = read(client_fd, buff, sizeof(buff))) > 0) {
//                     //processar os dados que o client esta mandando
//                     write(1, buff, count);
//                 }
//                 if (!count) {
//                     //cliente desconectou
//                     std::cout << "Cliente desconectou do servidor." << std::endl;
//                     close(client_fd);
//                     //ao darmos close no client, ele e removido diretamente do epoll
//                 } else if (count == -1) {
//                     //se o erro for EAGAIN significa que lemos tudo que o cliente mandou.
//                     //entao estamos prontos para processar o proximo evento
//                     if (errno != EAGAIN) {
//                         std::cout << "Erro" << std::endl;
//                         close(client_fd);
//                     }
//                 }
//             }
//         }
//     }
    

//     return 0;
// }
