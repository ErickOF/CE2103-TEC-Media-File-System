//
// Created by Allan on 09/11/17.
//

#include "ControllerNode.h"

std::mutex ControllerNode::mut;

void getIndexesForParity(const int position, int* restOfIndexes, int disks){
    int counter=0;
    for (int i =0; i < disks; i++){
        if(i!=position){
            restOfIndexes[counter++]= i - position;
        }
    }
}


void getIndexesForParityCalc(int* restOfIndexes, int inicio, int fin){
    int counter=0;
    for (int i =inicio; i < fin; i++){
        restOfIndexes[counter++]= i;
    }
}

/**
 *
 * @param comando codigo definido en protocolo segun la operacion deseada
 * @param dataRecieved bytes provenientes del socket
 * @param bytesRecibidos cantidad de bytes recibidos
 * @param pathName carpeta temporal para procesar videos
 * @return
 */
bool ControllerNode::procesar(char comando, char *dataRecieved, ulong bytesRecibidos, const char *pathName, ServerSocket *new_sock) {
    std::cout<<"comando: "<<comando<<"\n";

    auto tamanoNombreVideo = (ulong) dataRecieved[bytesRecibidos - 2];

    std::cout<<tamanoNombreVideo<<"\n";
    std::string nombreVideo = Funcs::extraerInformacion(dataRecieved, bytesRecibidos - tamanoNombreVideo - 2,
                                                        tamanoNombreVideo);
    std::cout << "recibido: " << nombreVideo << "\n";

    ulong bytesEfectivos = bytesRecibidos - 2 - tamanoNombreVideo -1;
    std::string uuidString;

    char response[DEFAULT_BUFFER_SIZE];
    bzero(response,DEFAULT_BUFFER_SIZE);

    if(comando== STORE_VIDEO) {

        char hasUuid =  dataRecieved[bytesRecibidos - 2 - tamanoNombreVideo-1];
        if (hasUuid =='0') {
            //se genera un uuid para el usuario
            std::cout << "no tiene uuid" << std::endl;
            boost::uuids::random_generator generator;//inicializa el generador de uuids
            boost::uuids::uuid uuid1 = generator();
            uuidString = boost::uuids::to_string(uuid1);

        } else {
            uuidString = Funcs::extraerInformacion(dataRecieved, bytesRecibidos -2 - tamanoNombreVideo - 1 - TAMANO_UUID,
                                                   TAMANO_UUID);
            bytesEfectivos-=TAMANO_UUID;
        }

        std::cout << "id: " << uuidString << std::endl;

        int indiceCliente=searchUUID(uuidString);
        TuplaCliente tupla;

        if(indiceCliente==-1){
            tupla.cliente=uuidString;
        }else{
            tupla = clientes[indiceCliente];
            if(searchVideo(tupla, nombreVideo)!=-1){
                //se devuelve un mensaje
                auto msg = "archivo duplicado";
                new_sock->push((char*) msg, strlen(msg));
                std::cerr<<msg<<std::endl;
                return false;
            }
        }
        boost::filesystem::path tempDir = pathName + uuidString;

        boost::filesystem::create_directory( boost::filesystem::path(pathName));
        boost::filesystem::create_directory(tempDir);

        //path dentro del folder
        std::string pathVideo = tempDir.string() + "/" + nombreVideo;

        std::ofstream out(pathVideo); //crea un nuevo archivo donde copiar la informacion recibida
        //escribe los bytes en el archivo

        out.write(dataRecieved, bytesEfectivos);
        out.close();
        std::string pathDivisiones = tempDir.string() + "/divisiones/";
        boost::filesystem::create_directory(boost::filesystem::path(pathDivisiones));

        system(("MP4Box -splits 500 " + pathVideo +
                " -out " + pathDivisiones).data());

        uint bloqueInicial= this->bloqueFinal;
        TuplaVideo tuplaVideo;
        tuplaVideo.nombreVideo= nombreVideo;
        tuplaVideo.bloqueInicio= bloqueInicial;

        ulong visitingFileCount=0;

        if(!divideFiles(pathDivisiones, visitingFileCount)){
            this->bloqueFinal= bloqueInicial;
            //se devuelve un mensaje
            auto msg = "400";
            new_sock->push((char*) msg, strlen(msg));
            return false;
        }

        tuplaVideo.fileCount = visitingFileCount;
        tuplaVideo.bloqueFinal=this->bloqueFinal;
        tupla.videos.emplace_back(tuplaVideo);

        if(indiceCliente==-1){
            clientes.emplace_back(tupla);
        }else{
            clientes[indiceCliente].videos.emplace_back(tuplaVideo);
        }
        //se devuelve un mensaje
        auto msg = "200";
        new_sock->push((char*) msg, strlen(msg));


    } else if(comando == GET_VIDEO){
        uuidString = Funcs::extraerInformacion(dataRecieved, bytesRecibidos -2 - tamanoNombreVideo - TAMANO_UUID,
                                               TAMANO_UUID);

        if(searchUUID(uuidString)==-1){
            //se devuelve un mensaje
            auto msg = "404";
            new_sock->push((char*) msg, strlen(msg));
            return false;
        }
        else{
            //se devuelve un mensaje
            int indiceCliente = searchUUID(uuidString);
            int indiceVideo = searchVideo(clientes[indiceCliente], nombreVideo);
            if(indiceVideo==-1){
                auto msg = "404";
                new_sock->push((char*) msg, strlen(msg));
                return false;
            }else{
                uint bloqueInicio = clientes[indiceCliente].videos[indiceVideo].bloqueInicio;
                uint bloqueFinal = clientes[indiceCliente].videos[indiceVideo].bloqueFinal;

                ulong fileCount=clientes[indiceCliente].videos[indiceVideo].fileCount;

                ulong clientBufferSize=SIZE_CHUNKS;
                std::vector<std::pair<ulong,ulong>> nodosBloquesFallidos;

                ulong counter=0;

                std::string fileCountStr = std::to_string(fileCount);

                char res[DEFAULT_BUFFER_SIZE];
                bzero(res, DEFAULT_BUFFER_SIZE);

                res[DEFAULT_BUFFER_SIZE-1]=((uint8_t)fileCountStr.length());
                ulong inicio = DEFAULT_BUFFER_SIZE-1-fileCountStr.length();

                memcpy(res+inicio, fileCountStr.data(),fileCountStr.length());

                new_sock->push(res,DEFAULT_BUFFER_SIZE);

                new_sock->changeBufSize(SIZE_CHUNKS);

                for (uint bloque = bloqueInicio ; bloque < bloqueFinal; bloque++) {
                    counter--;

                    for (uint nodo = 0; nodo < nodos; nodo++) {
                        counter++;
                        char video[clientBufferSize]={'\0'};
                        bool success = formatRqstGetVideo(bloque, nodo, video, clientBufferSize);

                        long indexChunk = getChunkIndex(bloque,nodo);
                        ulong size = chunks[indexChunk].size;

                        if (!success && !chunks[indexChunk].esParidad){
                            reconstructData(bloque, nodo, video, clientBufferSize, size);

                        }
                        if(!chunks[indexChunk].esParidad){
                            new_sock->push(video, clientBufferSize);
                        }
                    }
                }
                std::cout<<"\n";

                return true;
            }


        }

    }else if (comando == LISTA_VIDEOS) {
        uuidString = Funcs::extraerInformacion(dataRecieved, bytesRecibidos -1 - TAMANO_UUID,
                                               TAMANO_UUID);

        if(searchUUID(uuidString)==-1){
            //se devuelve un mensaje
            auto msg = "404";
            ulong inicio = DEFAULT_BUFFER_SIZE-1-strlen(msg);
            memcpy(response+inicio, msg,strlen(msg));

            new_sock->push(response, DEFAULT_BUFFER_SIZE);
            return false;
        }

        int indiceCliente = searchUUID(uuidString);
        std::vector<TuplaVideo> videosCliente = clientes[indiceCliente].videos;
        std::string nombreVideosStr;
        
        for (auto video : videosCliente){
            nombreVideosStr += video.nombreVideo +",";
        }

        ulong inicio = DEFAULT_BUFFER_SIZE-1-nombreVideosStr.length();
        memcpy(response+inicio, nombreVideosStr.data(), nombreVideosStr.length());
        new_sock->push(response, DEFAULT_BUFFER_SIZE);


    }else{
        std::cerr << " codigo de solicitud no soportado" << std::endl;
        return false;
    }
    std::cout<<"solicitud procesada\n";
    return true;
}

/**
 * Constructor
 * @param infoIp
 * @param configFile
 */
ControllerNode::ControllerNode(std::vector<boost::tuples::tuple<std::string, int>> infoIp, const char* configFile):
    pathConfiguracion{configFile} {
    this->infoIp= std::move(infoIp);

}


/**Compara dos elementos para ordenar la lista que los almacena*/
static bool compare(const std::string& str1, const std::string& str2){
    return str1<str2;
}

/**
 * Distribuye los videos entre los DiskNodes
 * @param path carpeta temporal con los videos recibidos
 * @return
 */
bool ControllerNode::divideFiles(std::string path, ulong& visitingFileCount) {
    boost::filesystem::path folder = boost::filesystem::path(path);

    ulong clientBufferSize=SIZE_CHUNKS;

    std::vector<std::string> fileNames;

    //itera sobre el directorio y almacena los paths
    for(boost::filesystem::directory_iterator itr(folder); itr != boost::filesystem::directory_iterator(); ++itr ){
        //boost::filesystem::path filePath = itr->path();
        fileNames.emplace_back(itr->path().string());

    }
    std::sort (fileNames.begin(), fileNames.end(), compare);


    auto bytesFiles = new char*[fileNames.size()]();
    for (auto row = 0; row< fileNames.size(); row++){
        bytesFiles[row]= new char[clientBufferSize]();
        for (auto col =0; col < clientBufferSize; col++){
            bytesFiles[row][col]='1';
        }
    }
    std::map<char*,ulong> mapaDimensiones;

    std::cout<<std::endl;
    for (auto i = 0; i < fileNames.size(); i++)
    {
        std::ifstream inputFileStream(fileNames[i], std::ios::binary|std::ios::ate);
        auto tamanoVideo= (ulong) inputFileStream.tellg();
        if(!inputFileStream.is_open() || inputFileStream.fail()){
            return false;
        }
        //se busca el inicio del archivo
        inputFileStream.seekg(0, inputFileStream.beg);
        //se almacena el archivo en el array de bytes
        inputFileStream.read(bytesFiles[i], tamanoVideo);

        bytesFiles[i][clientBufferSize-1]=STORE_VIDEO;
        mapaDimensiones[bytesFiles[i]]= tamanoVideo;
        inputFileStream.close();

    }

    int filesSentCounter=0;
    int currentParIndx=nodos-1;

    int inicioPar=0;
    int finPar=inicioPar+nodos-1;

    ulong fileCount = fileNames.size();
    for(auto file = 0; file < fileCount; file++ ){
        if(currentParIndx<0){
            currentParIndx=nodos-1;
        }
        std::string bloqueStr = std::to_string(this->bloqueFinal);
        file--;

        try {
            for (uint disco =0; disco < nodos; disco++){
                file++;
                ClientSocket cliente(infoIp[disco].get<0>().data(), infoIp[disco].get<1>(), clientBufferSize);

                if (disco == currentParIndx) {
                    int restOfIndexes[nodos-1];
                    getIndexesForParityCalc(restOfIndexes, inicioPar, finPar);

                    std::cout<<disco<<" "<<restOfIndexes[0]<<" "<<restOfIndexes[1]<<" "<<restOfIndexes[2]<<"\n";

                    char* bytes1= (restOfIndexes[0])>=fileCount? bytesFiles[fileCount-1] : bytesFiles[restOfIndexes[0]] ;
                    char* bytes2= (restOfIndexes[1])>=fileCount? bytesFiles[fileCount-1] : bytesFiles[restOfIndexes[1]] ;
                    char* bytes3= (restOfIndexes[2])>=fileCount? bytesFiles[fileCount-1] : bytesFiles[restOfIndexes[2]] ;
                    char* result= Funcs::calcularParidad(bytes1, bytes2,
                                                         bytes3,clientBufferSize);

                    result[clientBufferSize-1]=STORE_VIDEO;
                    result[clientBufferSize-2]=((uint8_t)bloqueStr.length());
                    Funcs::writeBackwards(result, clientBufferSize-3, bloqueStr);
                    ulong effective_size= clientBufferSize-2-bloqueStr.length();

                    //envia al nodo
                    cliente.push(result, clientBufferSize);

                    delete[] (result);
                    chunks.emplace_back(TuplaChunk(disco, bloqueFinal, effective_size, true));
                    inicioPar=finPar;
                    finPar +=(nodos-1);
                    file--;
                } else {

                    if(file>=fileCount){
                        bytesFiles[fileCount-1][clientBufferSize-2]=((uint8_t)bloqueStr.length());
                        Funcs::writeBackwards(bytesFiles[fileCount-1], clientBufferSize-3, bloqueStr);
                        cliente.push(bytesFiles[fileCount-1], clientBufferSize);

                        ulong effective_size= clientBufferSize-2-bloqueStr.length();
                        chunks.emplace_back(TuplaChunk(disco, bloqueFinal, effective_size, true));

                        file--;
                    }else{
                        bytesFiles[file][clientBufferSize-2]=((uint8_t)bloqueStr.length());
                        Funcs::writeBackwards(bytesFiles[file], clientBufferSize-3, bloqueStr);
                        cliente.push(bytesFiles[file], clientBufferSize);
                        ulong sizeFile = mapaDimensiones[bytesFiles[file]];
                        chunks.emplace_back(TuplaChunk(disco, bloqueFinal, sizeFile, false));
                    }
                }
                filesSentCounter++;
                //recibe la respuesta del servidor
                cliente.get();
                std::cout << "servidor: " << cliente.getDataRecieved() << std::endl;
            }
            this->bloqueFinal++;
            currentParIndx--;
        }catch (...){
            Funcs::cleanMatrix(bytesFiles, fileNames.size());
            return false;
        }

    }
    std::cout<<"bloque final "<<bloqueFinal<<"\n";
    Funcs::cleanMatrix(bytesFiles, fileNames.size());
    std::cout<<"files sent: "<<filesSentCounter<<"\n";
    std::cout<<"files en folder: "<<fileNames.size()<<"\n";
    visitingFileCount=fileCount;
    return true;
}

void ControllerNode::setIps(std::vector<boost::tuples::tuple<std::string, int>> ips){
   this->infoIp= std::move(ips);
}

void ControllerNode::saveConfig(const ControllerNode & controllerNode) {

        std::lock_guard<std::mutex> guard(ControllerNode::mut);
        std::ofstream configFileStream(controllerNode.pathConfiguracion);
        boost::archive::text_oarchive oarchive(configFileStream);
        // read class state from archive
        oarchive << controllerNode;


}

int ControllerNode::searchUUID(const std::string& uuidCliente) {
    int counter=0;
    for (auto tupla : this->clientes){
        if (tupla.cliente==uuidCliente){
            return counter;
        }
        counter++;
    }
    return -1;

}

int ControllerNode::searchVideo(TuplaCliente& cliente, std::string& nombreVideo) {
    int counter =0;
    for (auto video: cliente.videos){
        if(video.nombreVideo==nombreVideo){
            return counter;
        }
        counter++;
    }

    return -1;
}

long ControllerNode::getChunkIndex(uint bloque, uint nodo) {
    for (auto i = 0; i < chunks.size(); i++){
        if (chunks[i].bloque==bloque && chunks[i].disco==nodo){
            return i;
        }
    }
    return -1;
}



bool ControllerNode::formatRqstGetVideo(ulong bloque, ulong nodo, char* result, ulong size) {

    std::string bloqueStr = std::to_string(bloque);

    char request[SIZE_CHUNKS]={'\0'};

    try {
        ClientSocket cliente(infoIp[nodo].get<0>().data(), infoIp[nodo].get<1>(), size);
        request[SIZE_CHUNKS-1]=GET_VIDEO;
        request[SIZE_CHUNKS-2]=((uint8_t)bloqueStr.length());
        Funcs::writeBackwards(request, SIZE_CHUNKS-3, bloqueStr);

        cliente.push(request, SIZE_CHUNKS);
        cliente.get();
        memcpy(result, cliente.getDataRecieved(), cliente.getBytesRead());
    }catch(...){
        std::cerr<<"error al conectar con nodo\n";
        return false;
    }

    return true;
}

void ControllerNode::reconstructData(ulong bloque, uint nodo, char* result, ulong size, ulong realSize) {

    int restOfIndexes[nodos-1];
    getIndexesForParity(nodo, restOfIndexes, nodos);
    char bytes1[size], bytes2[size], bytes3[size];
    formatRqstGetVideo(bloque, nodo+restOfIndexes[0], bytes1, size);
    formatRqstGetVideo(bloque, nodo+restOfIndexes[1], bytes2, size);
    formatRqstGetVideo(bloque, nodo+restOfIndexes[2], bytes3, size);

    char* parity= Funcs::calcularParidad(bytes1, bytes2, bytes3,size);

    memcpy(result, parity, realSize);
    delete[] (parity);

}


