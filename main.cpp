#include "mainwindow.h"
#include "slideshowview.h"
#include <QApplication>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    for (int i = 0; i < argc; i++)
    {
        qDebug() << argv[i];
    }
    SlideShowView v;
    if (argc > 1)
    {
        QString file = argv[1];
        QTimer::singleShot(500, [&v, file](){
            v.findAllItems("file:///" + file);
            v.showCurrentItem();
        });
    }

    return a.exec();
}
