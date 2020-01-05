//
// Created by Allan on 06/11/17.
//

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef UNTITLED_SOCKET_H
#define UNTITLED_SOCKET_H

#define DEFAULT_BUFFER_SIZE (128)

typedef unsigned long ulong;

/**
 * Esta Clase es la base para crear Sockets del lado del servidor y del lado del cliente
 */

class Socket {
protected:

    int sockfd=-1, port=0;
    //bytes leidos           escritos
    ssize_t bytesRead = 0, bytesWritten = 0;

    struct sockaddr_in serv_addr;

    //tamaño por defecto del buffer
    ulong bufSize = DEFAULT_BUFFER_SIZE;

    //buffer donde se almacena la info
    char* buffer;

public:
    Socket(int, ulong);
    Socket();
    explicit Socket(ulong);
    ~Socket();
    ssize_t push(char*, size_t);
    ssize_t get();

    ulong getBytesRead();
    char* getDataRecieved();
    ulong getBytesWritten();
    void changeBufSize(ulong newSize);


    void set_non_blocking(const bool b);
};


#endif //UNTITLED_SOCKET_H
