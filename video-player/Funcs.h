#ifndef FUNCS_H
#define FUNCS_H

#define TAMANO_UUID (36)
#include <bitset>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

class TuplaVideo{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & nombreVideo;
        ar& bloqueInicio;
        ar& bloqueFinal;
        ar& fileCount;
    }

public:
    std::string nombreVideo;
    uint bloqueInicio=0;
    uint bloqueFinal=0;
    unsigned long fileCount=0;
};

class TuplaChunk{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & disco;
        ar& bloque;
        ar& size;
        ar& esParidad;
    }

public:
    uint disco=0;
    uint bloque=0;
    unsigned long size=0;
    bool esParidad=false;
    TuplaChunk(uint disco, uint bloque, unsigned long size, bool esParidad): disco{disco}, bloque{bloque}, size{size}, esParidad{esParidad}{}
    TuplaChunk() = default;
    ~TuplaChunk() = default;
};

class TuplaCliente{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & cliente;
        ar& videos;
    }
public:
    std::string cliente;
    std::vector<TuplaVideo> videos;
};

class Funcs{
public:
    static void agregarCharsAlFinal(char* cadenaOrig, unsigned long inicio, std::string cadenaAAgregar);
    static std::string extraerInformacion(char *cadena, unsigned long inicio, unsigned long tamanoNombre);
    static char* calcularParidad(char* bytes1, char* bytes2, char* bytes3, unsigned long size);
    static void cleanMatrix(char** matrix, unsigned long rows);
    static void writeBackwards(char* bytes, unsigned long end, std::string& toAppend);

};



#endif // FUNCS_H
