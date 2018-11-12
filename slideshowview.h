#ifndef SLIDESHOWVIEW_H
#define SLIDESHOWVIEW_H

#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QWidget>
#include <QVector>
#include <QMultiHash>
#include <QHash>
#include <QByteArray>
#include <QElapsedTimer>
#include <QList>

class SlideShowView : public QObject
{
    Q_OBJECT
public:
    explicit SlideShowView(QObject *parent = 0);
    int randInt();

    struct Config
    {
        Config() {left = -1; top = -1; width = 800; height = 600; fullscreen = false;}
        ~Config() {}
        bool move() {return !(left == -1 && top == -1);}
        int left, top, width, height;
        bool fullscreen;
    };


    struct PicDatabase {

        void saveToFile(QString filename);
        static PicDatabase load(QString filename);


        struct WatchSortedItem {
            double time;
            QString url;
        };
        QVector<QString> sortedByWatchTime();

        QHash<QString, QString> urlToHash;
        QHash<QString, QString> hashToUrl;
        QHash<QString, int> watchedCount;
        QHash<QString, double> watchedTime;
        QHash<QString, QByteArray> preview;
        QString outFileName;
    };

signals:

public slots:
    void findAllItems(QString filename);
    void showCurrentItem();
    void showText(QString text);
    void nextItem();
    void prevItem();
    void randomItem();
    void nextRItem();
    void prevRItem();
    void nextMW();
    void prevMW();
    void loadMW();
    void toggleFullScreen();
    void loadJson();
    void saveData();
    void copyToClipboard();
    void appendToDB(QString filename, QString url);
    void itemShown(QString item);
    void aboutToQuit();
private:
    QQuickView *view;
    QVector<QString> currentItems;
    QVector<QString> randomItems;
    QVector<QString> mwItems;
    QHash<QString, int> nameToIndex;
    int currentItemIndex, randomItemIndex, mwItemIndex;
    Config cfg;
    int isFullScreen;
    QString lastShown;
    QElapsedTimer elapsed;

    PicDatabase db;
};

#endif // SLIDESHOWVIEW_H
