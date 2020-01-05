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
#include <stdexcept>
#include "Socket.h"

#ifndef UNTITLED_SERVERSOCKET_H
#define UNTITLED_SERVERSOCKET_H

/**
 * Esta clase hereda de la clase Socket y la especializa como un servidor
 */

class ServerSocket : public Socket {
public:
    ServerSocket()= default;
    ServerSocket(int, ulong);
    explicit ServerSocket(ulong);
    void acceptNew(ServerSocket*);

};


#endif //UNTITLED_SERVERSOCKET_H
