#ifndef SLIDESHOWVIEW_H
#define SLIDESHOWVIEW_H

#include <QObject>
#include <QQuickItem>
#include <QQuickView>
#include <QWidget>
#include <QVector>

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

signals:

public slots:
    void findAllItems(QString filename);
    void showCurrentItem();
    void nextItem();
    void prevItem();
    void randomItem();
    void nextRItem();
    void prevRItem();
    void toggleFullScreen();
    void loadJson();
    void saveData();
    void copyToClipboard();
private:
    QQuickView *view;
    QVector<QString> currentItems;
    QVector<QString> randomItems;
    QHash<QString, int> nameToIndex;
    int currentItemIndex, randomItemIndex;
    Config cfg;
    int isFullScreen;
    QString lastShown;
};

#endif // SLIDESHOWVIEW_H
