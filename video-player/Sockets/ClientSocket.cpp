//
// Created by Allan on 06/11/17.
//


#include "ClientSocket.h"
#include <arpa/inet.h>

ClientSocket::ClientSocket(const char* host, int port, ulong bufSize) : Socket(port, bufSize) {

    struct hostent *server;
    server = gethostbyname(host);

    if (server == nullptr) {
        std::cerr<<"no existe el host"<<std::endl;
        throw std::invalid_argument("no existe el host");
    }


    bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr,  server->h_length);

    //int status = inet_pton ( AF_INET, host, &serv_addr.sin_addr );

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        std::cerr<<"Error al conectar con host"<<std::endl;
        throw std::runtime_error("Error al conectar con host");
    }

}


