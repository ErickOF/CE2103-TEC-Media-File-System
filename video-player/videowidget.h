#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFileDialog>
#include <QProgressBar>
#include <QSlider>
#include <QMediaPlaylist>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace Ui {
class videoWidget;
}

class videoWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit videoWidget(QWidget *parent = 0);
    ~videoWidget();

    void file();

private slots:
    void on_actionPlay_triggered();

    void on_actionStop_triggered();

    void on_actionPause_triggered();

private:
    Ui::videoWidget *ui;
    QMediaPlayer* player;
    QVideoWidget* vw;
    QProgressBar* bar;
    QSlider* slider;
    QMediaPlaylist *playlist;


};

#endif // VIDEOWIDGET_H
