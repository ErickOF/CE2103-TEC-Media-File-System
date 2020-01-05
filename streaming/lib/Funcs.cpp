//
// Created by Allan on 09/11/17.
//
#include "Funcs.h"

void Funcs::agregarCharsAlFinal(char* cadenaOrig, ulong inicio, std::string cadenaAAgregar){
    for (auto i =0; i < cadenaAAgregar.size(); i++){
        cadenaOrig[inicio + i] = cadenaAAgregar[i];
    }
}

std::string Funcs::extraerInformacion(char *cadena, ulong inicio, ulong tamanoNombre){
    std::string nombre;
    for (auto i =inicio; i < inicio+tamanoNombre; i++){
        nombre.push_back(cadena[i]);
    }
    return nombre;
}

char* Funcs::calcularParidad(char* bytes1, char* bytes2, char* bytes3, ulong size){
    auto paridad = new char[size]();

    for (auto i =0; i < size; i++){
        paridad[i] = (char)(std::bitset<8>(bytes1[i]) ^ std::bitset<8>(bytes2[i]) ^ std::bitset<8>(bytes3[i]) ).to_ulong();
    }
    return paridad;
}

void Funcs::cleanMatrix(char** matrix, ulong rows) {
    for (auto row = 0; row < rows; row++){
        delete[] (matrix[row]);
    }
    delete[] (matrix);

}

/**
 * agrega un string de atras hacia adelante en un array
 * @param bytes
 * @param end
 * @param toAppend
 */
void Funcs::writeBackwards(char* bytes, ulong end, std::string& toAppend){
    auto counter= toAppend.length();
    while(counter>0){
        bytes[end--]=toAppend[counter-- -1];
    }
}

