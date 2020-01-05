//
// Created by Allan on 09/11/17.
//
#include <iostream>
#include <fstream>
#include <csignal>
#include <thread>
#include <atomic>
#include "lib/Sockets/ServerSocket.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "lib/Sockets/Protocolo.h"
#include <boost/lexical_cast.hpp>

namespace fs =boost::filesystem;

int port=0;
const char* pathName;
std::atomic<ulong> filesRecieved;

/**
 * maneja señales, en caso de interrupciones se cierran los sockets primero, para poder abrirlos nuevamente
 * @param signum
 */
void signalHandler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    close(port);
    exit(signum);
}

/**
 *
 * @param comando codigo definido en protocolo segun la operacion deseada
 * @param dataRecieved bytes provenientes del socket
 * @param bytesRecibidos cantidad de bytes recibidos
 * @param pathName carpeta temporal para procesar videos
 * @return
 */
bool procesar(char comando, char *dataRecieved, ulong bytesRecibidos, std::string bloque, ServerSocket *pSocket) {
    std::cout<<"comando: "<<comando<<"\n";

    char msg2[SIZE_CHUNKS];

    if(comando== STORE_VIDEO) {

        std::cout << "recibido: "  << "\n";
        fs::create_directory(pathName);

        //path dentro del folder
        std::string pathVideo = pathName + std::string("/") + bloque;

        std::ofstream out(pathVideo); //crea un nuevo archivo donde copiar la informacion recibida
        //escribe los bytes en el archivo

        out.write(dataRecieved, bytesRecibidos);
        out.close();

        //se devuelve un mensaje
        auto msg = "guardado en nodo";
        ulong inicio = SIZE_CHUNKS-strlen(msg)-1;
        memcpy(msg2+inicio,msg,strlen(msg));
        msg2[SIZE_CHUNKS-1]=(char)strlen(msg);
        pSocket->push((char *) msg2, SIZE_CHUNKS);


    } else if(comando == GET_VIDEO){
        std::string filePathName = pathName + std::string("/") + bloque;
        fs::path filePath = fs::path(filePathName);
        if(!fs::exists(filePath)){
            std::cerr<<"\n archivo: "<<filePathName<<" no existe\n";

            auto msg = "archivo no existe";
            ulong inicio = SIZE_CHUNKS-strlen(msg)-1;
            memcpy(msg2+inicio,msg,strlen(msg));
            msg2[SIZE_CHUNKS]=(char)strlen(msg);
            pSocket->push((char *) msg2, SIZE_CHUNKS);
            return false;
        }

        std::ifstream ifstream(filePathName, std::ios::binary|std::ios::ate);
        auto tamanoVideo= (ulong) ifstream.tellg();
        std::cout<<"vid: "<<tamanoVideo<<"\n";
        char result[SIZE_CHUNKS];
        bzero(result,SIZE_CHUNKS);

        //se busca el inicio del archivo
        ifstream.seekg(0, ifstream.beg);
        //se almacena el archivo en el array de bytes
        ifstream.read(result, tamanoVideo);

        //se devuelve el video
        pSocket->push(result, SIZE_CHUNKS);
        return true;

    }else if (comando == LISTA_VIDEOS) {

    }else{
        std::cerr << " codigo de solicitud no soportado" << std::endl;
        return false;
    }
    std::cout<<"solicitud procesada\n";
    return true;
}



/**
 * Esta tarea es llamada por un Thread para procesar un socket nuevo
 * @param new_sock socket aceptado
 */
void newSocketTask(ServerSocket *new_sock){

    //recibe la informacion del socket
    new_sock->get();
    char* dataRecieved=new_sock->getDataRecieved();
    ulong bytesRecibidos=new_sock->getBytesRead();

    std::cout<<"br: "<<bytesRecibidos<<"\n";

    char comando = dataRecieved[bytesRecibidos-1];
    std::cout<<"repr"<<(uint8_t)dataRecieved[bytesRecibidos-2]<<"\n";
    uint8_t sizeNodeRepresentation =(uint8_t)dataRecieved[bytesRecibidos-2];

    ulong inicio=bytesRecibidos-2-sizeNodeRepresentation;
    ulong tamanoVideo = inicio;
    
    std::string nombreBloque="";
    std::cout<<"size" <<dataRecieved[inicio]<<"\n";

    for (int i =0; i < sizeNodeRepresentation; i++){
      nombreBloque.push_back(dataRecieved[inicio++]);

    }
    std::cout<<"nodo: "<<nombreBloque<<"\n";
    
    procesar(comando, dataRecieved, tamanoVideo, nombreBloque, new_sock);


    std::cout<<"respuesta enviada\n";
    delete(new_sock);

}


int main(int argc, char *argv[]){

    // manejar señales
    signal(SIGINT, signalHandler);
    signal(SIGCHLD,SIG_IGN);
    fs::path full_path;

    //se lee el puerto y path de los parametros
    if ( argc > 2 ) {
        port = (int)(strtol(argv[1], nullptr, 0));
        pathName= argv[2];
        full_path = fs::system_complete(fs::path(argv[2]));
    }
    else {
        std::cout << std::endl << "uso:   ./programa puerto path" << std::endl;
        return 1;
    }

    try {
        fs::create_directory(full_path);
    } catch (fs::filesystem_error &error){
        std::cerr << "error " << error.what() << std::endl;
        return 1;
    }
    ulong bufSize=SIZE_CHUNKS;


    try {
        //se crea un socket servidor principal
        ServerSocket serverSocket(port, bufSize);

        while (true) {//loop infinito que espera conexiones
            try {
                //nuevo socket donde se procesa una nueva conexión
                auto serverSocketAccepted = new ServerSocket(bufSize);

                //acepta un socket
                serverSocket.acceptNew(serverSocketAccepted);

                //pasa la tarea a un nuevo thread para continuar esperando nuevas conexiones
                std::thread newSocketThread(newSocketTask, serverSocketAccepted);
                newSocketThread.detach();

            }catch(...){
                //std::cerr<<exception.what()<<std::endl;
            }

        }
    }catch(std::exception &exception1){
        std::cerr<<exception1.what()<<std::endl;
    }

}

