#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <string>
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::send(QString video){
    if(video.isEmpty()){
        return 1;
    }

    std::string nombreVideoFullPath= video.toStdString();
    qDebug()<<"in send: "<<nombreVideoFullPath.data();
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

    std::string req;
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
    ClientSocket clientSocket(IP, PORT,  DEFAULT_BUFFER_SIZE);
    clientSocket.push(sizeInfoArr,DEFAULT_BUFFER_SIZE);
    //envia los bytes por sockets
    clientSocket.changeBufSize(req.length());
    clientSocket.push(const_cast<char *>(req.data()), req.length());
    //recibe la respuesta del server
    clientSocket.get();
    std::cout<<"servidor: "<<clientSocket.getDataRecieved()<<std::endl;
    req.clear();

}

void MainWindow::search(QString videoName)
{


}




void MainWindow::on_ceSEARCHButton_clicked(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/Users",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    QDir directory(dir);
    QStringList videos = directory.entryList(QStringList() << "*.mp4" << "*.MP4",QDir::Files);
    foreach(QString filename, videos) {
        ui->folder->addItem(filename);
        QString temp;
        temp.append(dir);
        temp.append("/");
        temp.append(filename);
            qDebug() <<"video: "<< temp;
        send(temp);
    }
}

void MainWindow::on_ceROBOTButton_clicked()
{
    std::string uuidCliente = std::string("e5f4faee-a47a-4ae0-ae50-1d8c3be2f25c");

    ClientSocket clientSocket2(IP, PORT,  DEFAULT_BUFFER_SIZE);
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
        }
        else if(ui->searchVideoLine->text().toStdString()==strs[i]){
            ui->listResult->addItem(QString(strs[i].c_str()));
        }

        else{
            std::cout <<i+1 << " " << strs[i] << std::endl;

        }
    }




    std::cout << "* size of the vector: " << strs.size() << std::endl;
    //std::cout<<"servidor: "<<clientSocket2.getDataRecieved()<<std::endl;*/
}
void limpiarCarpeta(const boost::filesystem::path& path){

    for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr){
        boost::filesystem::remove_all(itr->path());
    }
}

void MainWindow::on_playButton_clicked()
{
    limpiarCarpeta(boost::filesystem::path("/home/laptopt/Desktop/recibidos"));

    std::string uuidCliente = std::string("e5f4faee-a47a-4ae0-ae50-1d8c3be2f25c");
    std::string nombreGet =ui->searchVideoLine->text().toStdString();

    ClientSocket clientSocket1(IP, PORT,  DEFAULT_BUFFER_SIZE);

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
            std::string nameFile = basePath + std::to_string(counter)+".mp4";
            std::ofstream output(nameFile);
            output.write(&result2[i], SIZE_CHUNKS);
            output.close();
        }
        counter++;
    }



    videoWidget *video = new videoWidget(this);
    video->show();
    video->file();
}
