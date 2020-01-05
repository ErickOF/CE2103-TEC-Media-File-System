#include "videowidget.h"
#include "ui_videowidget.h"
#include <iostream>
#include <string>


videoWidget::videoWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::videoWidget)
{
    ui->setupUi(this);
    player = new QMediaPlayer(this);
    vw= new QVideoWidget(this);
    player->setVideoOutput(vw);
    this->setCentralWidget(vw);

    this->setGeometry(100,100,1000,600);
    //playlist= new QMediaPlaylist(player);

    //slider = new QSlider(this);

    //slider->setOrientation(Qt::Horizontal);
    //ui->statusbar->addPermanentWidget(slider);

    //connect(player,&QMediaPlayer::durationChanged,slider,&QSlider::setMaximum);
    //connect(player,&QMediaPlayer::positionChanged,slider,&QSlider::setValue);
    //connect(slider,&QSlider::sliderMoved,player,&QMediaPlayer::setPosition);


}

videoWidget::~videoWidget()
{
    delete ui;
}



void videoWidget::file()
{
    std::cout << "Llego aqui"<< std::endl;
    playlist = new QMediaPlaylist;
    QDir directory("/home/laptopt/Desktop/recibidos");
    QStringList videos = directory.entryList(QStringList() << "*.mp4" << "*.MP4",QDir::Files);
    int counter = 0;
    foreach(QString filename, videos) {
        QString temp;
        temp.append("/home/laptopt/Desktop/recibidos/");
        QString name = QString(std::to_string(counter).data());
        name.append(".mp4");
        temp.append(name);
        playlist->addMedia(QUrl::fromLocalFile(temp));
        counter++;
        qDebug() << temp;
    }
    playlist->setCurrentIndex(1);

    //player = new QMediaPlayer;
    player->setPlaylist(playlist);

    //vw = new QVideoWidget;
    player->setVideoOutput(vw);
    //vw->show();

    player->play();


    //player->setMedia(QUrl::fromLocalFile("/Users/Christopher/clienteTCMFS/Videos/fire.mp4"));

    on_actionPlay_triggered();
}


void videoWidget::on_actionPlay_triggered()
{
    player->play();
    ui->statusbar->showMessage("Playing...");

}

void videoWidget::on_actionStop_triggered()
{
    player->stop();
    ui->statusbar->showMessage("Stopped...");
}

void videoWidget::on_actionPause_triggered()
{
    player->pause();
    ui->statusbar->showMessage("Paused...");
}
