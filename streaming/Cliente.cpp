//
// Created by allan on 07/11/17.
//
#include <iostream>
#include <fstream>
#include <csignal>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "lib/Sockets/ClientSocket.h"
#include <string>
#include "lib/Funcs.h"

//#include "boost/filesystem/path.hpp"
#include "lib/Sockets/Protocolo.h"        // streaming operators etc.

namespace fs = boost::filesystem;

/**
 * Ejemplo para enviar un video mediante sockets
 * @param argc conteo de parametros
 * @param argv argumentos: host puerto   ej: ./cliente localhost 49000
 * @return
 */
int main(int argc, char**argv){
    signal(SIGCHLD,SIG_IGN);//evita procesos vampiro

    if(argc!=3){//si no se le pasan los parametros mediante la linea de comandos
        std::cerr<<"indicar host puerto"<<std::endl;
        return -1;
    }
    auto port = (int)(strtol(argv[2], nullptr, 0));//lee el puerto de los parametros

    const char* nombreVideoFullPath="/home/laptopt/Desktop/videos/squirrels.mp4";
    fs::path videoPath = fs::path(nombreVideoFullPath);

    if(!fs::exists(videoPath)){
        std::cerr<<"video no existe\n";
        return 1;
    }
    //nombre del video sin el path
    std::string nombreVideoLimpio = videoPath.filename().string();

    //abre el archivo de la computadora
    auto inputFileStream= new std::ifstream(nombreVideoFullPath, std::ios::binary|std::ios::ate);
    auto tamanoVideo= (ulong) inputFileStream->tellg();

    if(!inputFileStream->is_open() || inputFileStream->fail()){
        return -1;
    }

    //array donde se almacenaran los bytes del video
    //auto result = new char[tamanoVideo]();
    std::vector<char> resVector(tamanoVideo);

    //se busca el inicio del archivo
    inputFileStream->seekg(0, inputFileStream->beg);
    //se almacena el archivo en el array de bytes
    inputFileStream->read(&resVector[0], tamanoVideo);
    inputFileStream->close();
    std::string uuidCliente = std::string("e5f4faee-a47a-4ae0-ae50-1d8c3be2f25c");

    //////store video
    /*std::string req;
    for (auto i =0; i < tamanoVideo; i++){
        req.push_back(resVector[i]);
    }
    req += uuidCliente + std::to_string(true) + nombreVideoLimpio+ (char) nombreVideoLimpio.length() +'s';

    //informacion sobre el tamaño del video a enviar
    char sizeInfoArr[DEFAULT_BUFFER_SIZE];
    bzero(sizeInfoArr,DEFAULT_BUFFER_SIZE);

    std::string sizeInfo = std::to_string(req.length());
    sizeInfo+=std::to_string(sizeInfo.length())+'s';


    std::cout<<sizeInfo<<"\n";
    //se copia la informacion en el array
    for (auto i = 0; i < sizeInfo.length(); i++){
        sizeInfoArr[DEFAULT_BUFFER_SIZE-1-i]=sizeInfo[sizeInfo.length()-1-i];
    }
    std::cout<<sizeInfoArr<<"\n";
    //crea un nuevo socket
    ClientSocket clientSocket(argv[1], port,  DEFAULT_BUFFER_SIZE);
    clientSocket.push(sizeInfoArr,DEFAULT_BUFFER_SIZE);

    //envia los bytes por sockets
    clientSocket.changeBufSize(req.length());
    clientSocket.push(const_cast<char *>(req.data()), req.length());

    //recibe la respuesta del server
    clientSocket.get();
    std::cout<<"servidor: "<<clientSocket.getDataRecieved()<<std::endl;
    req.clear();*/


    ////////get video
    std::string nombreGet ="squirrels.mp4";

    ClientSocket clientSocket1(argv[1], port,  DEFAULT_BUFFER_SIZE);

    std::string request = uuidCliente + nombreGet;
    request.push_back((char) nombreGet.length());
    request.push_back(GET_VIDEO);

    const char* requestStr = request.data();

    char req[DEFAULT_BUFFER_SIZE];
    ulong inicio = DEFAULT_BUFFER_SIZE-2-uuidCliente.length()-nombreGet.length();

    memcpy(req+inicio, requestStr, strlen(requestStr));
    
    clientSocket1.push(req, DEFAULT_BUFFER_SIZE);
    clientSocket1.get();

    ulong bytesRecvd = clientSocket1.getBytesRead();

    auto bloquesSizeRep =(int)clientSocket1.getDataRecieved()[bytesRecvd-1];
    std::cout<<"sRep: "<<bloquesSizeRep<<"\n";
    std::string fileCountStr;

    int inicio1= DEFAULT_BUFFER_SIZE-1-bloquesSizeRep;

    for (int i =inicio1; i< DEFAULT_BUFFER_SIZE-1; i++){
        fileCountStr.push_back(clientSocket1.getDataRecieved()[i]);
    }
    auto fileCount = boost::lexical_cast<int>(fileCountStr);
    std::cout<<"fileCount: "<<fileCount<<"\n";
    clientSocket1.changeBufSize(SIZE_CHUNKS);

    long len=0;
    long bytesRead=0;
    //char* result1= new char[SIZE_CHUNKS*fileCount]();
    std::vector<char> result2(SIZE_CHUNKS * fileCount);

    while((bytesRead=clientSocket1.get())>0){
        memcpy(&result2[len], clientSocket1.getDataRecieved(), clientSocket1.getBytesRead());

        //se lee si se envia un msj de error
        if(clientSocket1.getBytesRead()==3 && len==0){
            std::string codigo;
            for(int i =0; i < 3; i++){
                codigo.push_back(clientSocket1.getDataRecieved()[i]);
            }

            int codigoN=boost::lexical_cast<uint>(codigo);
            if(codigoN==NO_ENCONTRADO) {
                std::cerr << "Archivo no encontrado" << "\n";
            }
            break;

        }
        len+=bytesRead;

    }
    int counter=0;
    const char* basePath = "/home/laptopt/Desktop/recibidos/";

    for (auto i =0; i < SIZE_CHUNKS*fileCount; i+=SIZE_CHUNKS){
        std::cout<<"512kB : recibidos\n";

        {
            std::string nameFile = basePath + std::to_string(counter);
            std::ofstream output(nameFile);
            output.write(&result2[i], SIZE_CHUNKS);
            output.close();
        }
        counter++;
    }

    ////get lista videos

   /* ClientSocket clientSocket2(argv[1], port,  DEFAULT_BUFFER_SIZE);

    char request[DEFAULT_BUFFER_SIZE];
    bzero(request,DEFAULT_BUFFER_SIZE);

    std::string requestLista = uuidCliente+LISTA_VIDEOS;

    ulong inicio = DEFAULT_BUFFER_SIZE-1-uuidCliente.length();
    memcpy(request+inicio, requestLista.data(), requestLista.length());

    clientSocket2.push(request, DEFAULT_BUFFER_SIZE);

    clientSocket2.get();

    std::string line;
    for (auto i =0; i < clientSocket2.getBytesRead(); i++){
        char caracter=clientSocket2.getDataRecieved()[i];
        if ( caracter!='\0'){
            line.push_back(caracter);
        }
    }
    std::vector<std::string> strs;
    boost::split(strs,line,boost::is_any_of(","));
    
    for (size_t i = 0; i < strs.size(); i++) {
        if(strs[i]==""){
            strs.erase(strs.begin()+i);
            i--;
        }else{
        std::cout <<i+1 << " " << strs[i] << std::endl;
            }
        
    }
    std::cout << "* size of the vector: " << strs.size() << std::endl;
    //std::cout<<"servidor: "<<clientSocket2.getDataRecieved()<<std::endl;*/
}

