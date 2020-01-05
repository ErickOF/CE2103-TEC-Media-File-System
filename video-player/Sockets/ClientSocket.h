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
#include <netdb.h>
#include "Socket.h"

#ifndef UNTITLED_CLIENTSOCKET_H
#define UNTITLED_CLIENTSOCKET_H

/**
 * Esta clase hereda de Socket y la especializa como un cliente
 */
class ClientSocket : public Socket  {
public:
    ClientSocket()= default;
    ClientSocket(const char*,int, ulong);

};


#endif //UNTITLED_CLIENTSOCKET_H
