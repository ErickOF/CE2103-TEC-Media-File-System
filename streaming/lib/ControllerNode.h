//
// Created by Allan on 09/11/17.
//

#ifndef STREAMING_CONTROLLERNODE_H
#define STREAMING_CONTROLLERNODE_H

#include "Sockets/Protocolo.h"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <fstream>
#include "Funcs.h"
#include "Sockets/ServerSocket.h"
#include <string>
#include <boost/tuple/tuple.hpp>
#include <utility>
#include <vector>
#include <map>
#include <string>
#include "Sockets/ClientSocket.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <mutex>

/**
 * Esta clase se encarga de procesar solicitudes, alamacenar videos, recuperarlos, borrarlos.
 * Divide el archivo en partes iguales y las envia los nodos disco, calcula la paridad
 */
class ControllerNode {
    friend class boost::serialization::access;
    const int nodos=4;

    std::vector<boost::tuples::tuple<std::string, int>> infoIp;
    // disco bloque size esParidad
    std::vector<TuplaChunk> chunks;
    std::string pathConfiguracion;
    static std::mutex mut;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & pathConfiguracion;
        ar & clientes;
        ar & bloqueFinal;
        ar& chunks;
    }

public:
    uint bloqueFinal=0;
    std::vector<TuplaCliente> clientes;
    ControllerNode()= default;
    explicit ControllerNode(std::vector<boost::tuples::tuple<std::string, int>>, const char* string);
    bool procesar(char comando, char *dataRecieved, ulong bytesRecibidos, const char *pathName, ServerSocket *pSocket);
    bool divideFiles(std::string, ulong& i);
    void setIps(std::vector<boost::tuples::tuple<std::string, int>> ips);
    static void saveConfig(const ControllerNode&);

    int searchUUID(const std::string& basic_string);

    int searchVideo(TuplaCliente& cliente, std::string& nombreVideo);

    long getChunkIndex(uint bloque, uint nodo);

    bool formatRqstGetVideo(ulong bloque, ulong nodo, char *, ulong);

    void reconstructData(ulong bloque, uint nodo, char* result, ulong size, ulong i);
};


#endif //STREAMING_CONTROLLERNODE_H
