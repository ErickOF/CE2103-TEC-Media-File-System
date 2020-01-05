#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <csignal>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "Sockets/ClientSocket.h"
#include "videowidget.h"





//---------------------------------------------------------------
#define DEFAULT_BUFFER_SIZE (128)
#define PORT (32534)
#define IP ("localhost")
//---------------------------------------------------------------






#include "Funcs.h"
#include "boost/filesystem/path.hpp"
#include "Sockets/Protocolo.h"        // streaming operators etc.

namespace fs = boost::filesystem;
typedef unsigned long ulong;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int send(QString video);
    void search(QString videoName);
private slots:
    void on_ceSEARCHButton_clicked();

    void on_ceROBOTButton_clicked();

    void on_playButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
