//
// Created by Allan on 06/11/17.
//
/**
 * Este programa se encarga de escuchar nuevas solicitudes por sockets y pasarlas al nodo controlador para procesarlas
 */
#include <fstream>
#include <thread>
#include <csignal>
#include "lib/Sockets/ServerSocket.h"
#include <climits>
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "lib/Sockets/Protocolo.h"   // streaming operators etc.
#include "lib/ControllerNode.h"


int port=0;
const char* pathName;
namespace fs =boost::filesystem;

/**
 * Esta tarea es llamada por un Thread para procesar un socket nuevo
 * @param new_sock socket aceptado
 */
void newSocketTask(ServerSocket *new_sock, ControllerNode& controllerNode){
    std::cerr<<"thread: "<<std::this_thread::get_id()<<"\n";

    //recibe la informacion del socket
    new_sock->get();
    char comando = new_sock->getDataRecieved()[new_sock->getBytesRead()-1];
    if( comando==STORE_VIDEO) {
        auto sizeRep = boost::lexical_cast<int>(new_sock->getDataRecieved()[new_sock->getBytesRead() - 2]);
        std::cout << "size " << sizeRep << "\n";

        std::string sizeStr;
        for (int i = new_sock->getBytesRead() - 2 - sizeRep; i < new_sock->getBytesRead() - 2; i++) {
            sizeStr.push_back(new_sock->getDataRecieved()[i]);
        }

        auto sizeVideo = boost::lexical_cast<ulong>(sizeStr);
        std::cout << "sizeVideo: " << sizeVideo << "\n";
        new_sock->changeBufSize(sizeVideo);
        new_sock->get();

    }

    char* dataRecieved=new_sock->getDataRecieved();
    ulong bytesRecibidos=new_sock->getBytesRead();

    std::cout<<"br "<<bytesRecibidos<<"\n";
    controllerNode.procesar(dataRecieved[bytesRecibidos - 1], dataRecieved, bytesRecibidos, pathName, new_sock);
    ControllerNode::saveConfig(controllerNode);
    for (auto cliente : controllerNode.clientes){
        for (auto video : cliente.videos){
            std::cout <<"inicio: " <<video.bloqueInicio << "fin "<<video.bloqueFinal<<"\n";
        }
    }
    delete(new_sock);
}

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
 * Limpia el contenido de la carpeta
 * @param path carpeta a limpiar
 */
void limpiarCarpeta(const boost::filesystem::path& path){

    for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr){
        boost::filesystem::remove_all(itr->path());
    }
}

/**
 * Entrada del programa
 * @param argc conteo de parametros
 * @param argv argumentos: puerto   ej: ./server  49000
 * @return
 */

int main(int argc, char *argv[]) {

    // manejar señales
    signal(SIGINT, signalHandler);
    signal(SIGCHLD,SIG_IGN);
    fs::path full_path;

    const char* iPNodes;

    //se lee el puerto y path de los parametros
    if ( argc > 3 ) {
        port = (int)(strtol(argv[1], nullptr, 0));
        pathName= argv[2];
        iPNodes = argv[3];
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
    limpiarCarpeta(full_path);
    std::vector<boost::tuples::tuple<std::string, int>> parIp_Puertos;
    for (int i =1; i <=4; i++){
        parIp_Puertos.emplace_back(boost::tuples::tuple<std::string, int>(iPNodes, port+i));
    }

    try {
        //se crea un socket servidor principal
        ServerSocket serverSocket(port, DEFAULT_BUFFER_SIZE);
        const char* pathConfigFile="/home/laptopt/Desktop/config.json";
        ControllerNode controllerNode(parIp_Puertos, pathConfigFile);
        if(boost::filesystem::exists(boost::filesystem::path(pathConfigFile))){
            std::ifstream ifstream(pathConfigFile);
            boost::archive::text_iarchive ia(ifstream);
            // read class state from archive
            ia >>  controllerNode;
            controllerNode.setIps(parIp_Puertos);
        }

        controllerNode.saveConfig(controllerNode);

        while (true) {//loop infinito que espera conexiones
            try {
                //nuevo socket donde se procesa una nueva conexión
                auto serverSocketAccepted = new ServerSocket(DEFAULT_BUFFER_SIZE);

                //acepta un socket
                serverSocket.acceptNew(serverSocketAccepted);

                limpiarCarpeta(full_path);
                //pasa la tarea a un nuevo thread para continuar esperando nuevas conexiones
                std::thread newSocketThread(newSocketTask, serverSocketAccepted, std::ref(controllerNode));
                newSocketThread.detach();

            }catch(std::exception &exception){
                std::cerr<<exception.what()<<std::endl;
            }

        }
    }catch(std::exception &exception1){
        std::cerr<<exception1.what()<<std::endl;
    }

    return 0;
}