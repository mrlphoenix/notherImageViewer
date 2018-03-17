#include "slideshowview.h"
#include <QUrl>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <random>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <QClipboard>
SlideShowView::SlideShowView(QObject *parent) : QObject(parent)
{
    isFullScreen = 0;
    view = new QQuickView();
    view->setSource(QUrl("qrc:/slideshow.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    loadJson();
    if (cfg.move())
        view->setPosition(cfg.left, cfg.top);
    view->resize(cfg.width, cfg.height);
    view->setIcon(QIcon(":/icon.png"));
    view->show();
    if (cfg.fullscreen)
        view->showFullScreen();
    QObject *object = dynamic_cast<QObject*>(view->rootObject());
    QTimer::singleShot(300, [this, object](){
        qDebug() << "binding ";
        QObject::connect(object,SIGNAL(setDir(QString)), this, SLOT(findAllItems(QString)));
        QObject::connect(object,SIGNAL(nextItem()), this, SLOT(nextItem()));
        QObject::connect(object,SIGNAL(prevItem()), this, SLOT(prevItem()));
        QObject::connect(object,SIGNAL(randomItem()), this, SLOT(randomItem()));

        QObject::connect(object,SIGNAL(nextRItem()), this, SLOT(nextRItem()));
        QObject::connect(object,SIGNAL(prevRItem()), this, SLOT(prevRItem()));
        QObject::connect(object, SIGNAL(toggleFullScreen()), this, SLOT(toggleFullScreen()));
        QObject::connect(object, SIGNAL(copy()), this, SLOT(copyToClipboard()));
    });
    qsrand(QDateTime::currentDateTime().time().msecsSinceStartOfDay());
    int c = qrand()%120000;
    for (int i = 0; i < c; i++)
        randInt();

    QTimer *t = new QTimer();
    connect(t, &QTimer::timeout, this, &SlideShowView::saveData);
    t->start(5000);
}

int SlideShowView::randInt()
{
    static std::random_device rd;  //Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    static std::uniform_int_distribution<> dis(0, 1000000000);

    return dis(gen);
}

void SlideShowView::findAllItems(QString filename)
{
    qDebug() << "FindAllItems" << filename;
    currentItems.clear();
    randomItems.clear();
    nameToIndex.clear();
    currentItemIndex = -1;
    randomItemIndex = -1;
    QString absPath = QFileInfo(filename.replace("file:///", "").replace("\\","/")).absolutePath();
    qDebug() << absPath;
    QDirIterator it(absPath, QStringList() << "*.jpg"<<  "*.png" << "*.jpeg" << "*.gif", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        currentItems << "file:///" + it.next();
        nameToIndex[currentItems.last()] = currentItems.count() - 1;
    }
    QVector<QString> tempList(currentItems);
    while (!tempList.isEmpty())
    {
        int index = randInt()%tempList.count();
        randomItems.append(tempList[index]);
        tempList.removeAt(index);
    }
    currentItemIndex = nameToIndex["file:///"+filename];
}

void SlideShowView::showCurrentItem()
{
    if (currentItems.count() && currentItemIndex < currentItems.count())
    {
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(currentItems[currentItemIndex])));

        lastShown = currentItems[currentItemIndex];
    }
}

void SlideShowView::nextItem()
{
    if (currentItems.count())
    {
        currentItemIndex++;
        if (currentItemIndex >= currentItems.count())
            currentItemIndex = 0;
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(currentItems[currentItemIndex])));

        lastShown = currentItems[currentItemIndex];
    }
}

void SlideShowView::prevItem()
{
    if (currentItems.count())
    {
        currentItemIndex--;
        if (currentItemIndex < 0)
            currentItemIndex = currentItems.count() - 1;
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(currentItems[currentItemIndex])));

        lastShown = currentItems[currentItemIndex];
    }
}

void SlideShowView::randomItem()
{
    if (currentItems.count())
    {
        if (currentItemIndex > currentItems.count())
            currentItemIndex = 0;
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        currentItemIndex = randInt()%currentItems.count();


        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(currentItems[currentItemIndex])));

        lastShown = currentItems[currentItemIndex];
    }
}

void SlideShowView::nextRItem()
{
    if (randomItems.count())
    {
        randomItemIndex++;
        if (randomItemIndex >= randomItems.count())
            randomItemIndex = 0;
        currentItemIndex = nameToIndex[randomItems[randomItemIndex]];
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(randomItems[randomItemIndex])));

        lastShown = randomItems[randomItemIndex];
    }
}

void SlideShowView::prevRItem()
{
    if (randomItems.count())
    {
        randomItemIndex--;
        if (randomItemIndex < 0)
            randomItemIndex = randomItems.count() - 1;
        currentItemIndex = nameToIndex[randomItems[randomItemIndex]];
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(randomItems[randomItemIndex])));
        lastShown = randomItems[randomItemIndex];
    }
}

void SlideShowView::toggleFullScreen()
{
    isFullScreen = 1 - isFullScreen;
    if (isFullScreen)
        view->showFullScreen();
    else
        view->show();
}

void SlideShowView::loadJson()
{
    QFile file("config.json");
    if (file.exists() && file.open(QFile::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject o = doc.object();

        cfg.left = o["left"].toInt();
        cfg.top = o["top"].toInt();
        cfg.width = o["width"].toInt();
        cfg.height = o["height"].toInt();
        cfg.fullscreen = o["fullscreen"].toBool();

        file.close();
    }
}

void SlideShowView::saveData()
{
    qDebug() << "saveData";
    QFile file("config.json");
    file.open(QFile::WriteOnly);

    QJsonObject o;
    o["left"] = view->position().x();
    o["top"] = view->position().y();
    o["width"] = view->width();
    o["height"] = view->height();
    o["fullscreen"] = isFullScreen == 1;

    QJsonDocument doc;
    doc.setObject(o);
    file.write(doc.toJson());
    file.flush();
    file.close();
}

void SlideShowView::copyToClipboard()
{
    QImage img(lastShown.replace("file:///", ""));

    qDebug() << "COPY" << " NULL " << img.isNull();
    QPixmap pm = QPixmap::fromImage(img);
    QApplication::clipboard()->setPixmap(pm);
}










