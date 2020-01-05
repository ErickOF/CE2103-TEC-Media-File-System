//
// Created by Allan on 06/11/17.
//

#include <fcntl.h>
#include "ServerSocket.h"

ServerSocket::ServerSocket(int port, ulong bufSize) : Socket(port, bufSize)  {

    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Error al enlazar socket");
    }
    listen(sockfd, 5);
    std::cout<<"Servidor esperando conexiones en puerto: "<<port<<std::endl;

}

void ServerSocket::acceptNew(ServerSocket* serverSocketAccepted) {

    socklen_t socketLen;
    socketLen = sizeof(serv_addr);
    int newsockfd = accept(sockfd, ( sockaddr *) &serv_addr, &socketLen);

    serverSocketAccepted->sockfd= newsockfd;

    if (newsockfd  < 0){
        throw std::runtime_error("Error al aceptar socket");
    }
}

ServerSocket::ServerSocket(ulong bufSize) : Socket(bufSize){}

