//
// Created by Allan on 06/11/17.
//

#include <vector>
#include <fcntl.h>
#include "Socket.h"
/**
 * Constructor
 * @param port puerto donde se recibe/envia informacion
 * @param bufSize tamaño del buffer
 */
Socket::Socket(int port, ulong bufSize) {
    this->bufSize=bufSize;
    buffer = new char[bufSize]();

    this->port=port;

    //crea un nuevo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);


    if (sockfd < 0) {
        throw std::runtime_error("Error al crear socket");
    }
    // TIME_WAIT - argh
    int on = 1;
    if(setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) <0){
        throw std::runtime_error("Error al crear socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

}
/**
 * Destructor
 */
Socket::~Socket() {
    if(sockfd!=-1) {
        close(sockfd);//cierra el socket si esta activo
    }
    //libera el buffer
    delete[] buffer;

}

/**
 * envía un mensaje
 * @param msg arrego de bytes con la informacion a enviar
 * @param size tamaño del arreglo en bytes
 * @return bytes enviados
 */
ssize_t Socket::push(char* msg, size_t size) {

    bzero(buffer,bufSize);
    bytesWritten=0;

    memcpy(buffer, msg, size);

    //se envia la informacion mediante el socket

    bytesWritten = send(sockfd,buffer,size,0);
    if (bytesWritten < 0) {
        std::cerr << "Error al escribir socket"<<errno << std::endl;
        throw std::runtime_error("Error al escribir socket");
    }
    return bytesWritten;
}

/**
 * Recibe informacion del socket y lo almacena en el buffer
 * @return bytes leidos
 */
ssize_t Socket::get() {

    //inicializa el buffer en 0
    bzero(buffer,bufSize);

    ulong len=0;
    bytesRead=0;

    //lee de MB en MB

    while ( len<bufSize) {
        bytesRead = recv(sockfd, buffer+len, 100,0);
        if (bytesRead <=0) {
            break;
        }
        len += bytesRead; // se van acumulando los bytes leidos
        //si se lee menos de lo esperado, quiere decir que ya se leyó toda la informacion del socket
    }

    bytesRead=len;

    if (bytesRead < 0) {
        std::cerr << "Error al leer socket" << std::endl;
        throw std::runtime_error("Error al leer socket");
    }

    std::cout<<"Bytes recibidos: "<<bytesRead<<std::endl;

    return bytesRead;
}

/**
 * Constructor
 */
Socket::Socket() {
    //inicializa un buffer por defecto
    buffer= new char[DEFAULT_BUFFER_SIZE]();

}
/**
 * Inicializa un buffer con un tamaño
 * @param pBufSize
 */
Socket::Socket(ulong pBufSize) {
    bufSize=pBufSize;
    buffer= new char[pBufSize]();

}

/**
 * devuelve los bytes recibidos
 * @return
 */
ulong Socket::getBytesRead() {
    return (ulong)bytesRead;
}

/**
 * devuelve el último buffer recibido/enviado
 * @return
 */
char *Socket::getDataRecieved() {
    return buffer;
}

void Socket::set_non_blocking ( const bool b )
{

    int opts;
    opts = fcntl ( sockfd,
                   F_GETFL );

    if (opts < 0){
        return;
    }

    if (b) {
        opts = (opts | O_NONBLOCK);
    }else {
        opts = (opts & ~O_NONBLOCK);
    }
    fcntl (sockfd, F_SETFL, opts );

}

ulong Socket::getBytesWritten() {
    return bytesWritten;
}

void Socket::changeBufSize(ulong newSize) {
    bufSize=newSize;
    char* old = buffer;
    buffer=new char[newSize]();
    delete[](old);

}
