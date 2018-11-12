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
#include <QMultiHash>
#include <QDataStream>
#include <QBuffer>
#include <QCryptographicHash>

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
    if (cfg.fullscreen)
        isFullScreen = 1;
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
        QObject::connect(object,SIGNAL(prevMW()), this, SLOT(prevMW()));
        QObject::connect(object,SIGNAL(nextMW()), this, SLOT(nextMW()));
        QObject::connect(object,SIGNAL(loadMW()), this, SLOT(loadMW()));

        QObject::connect(object,SIGNAL(nextRItem()), this, SLOT(nextRItem()));
        QObject::connect(object,SIGNAL(prevRItem()), this, SLOT(prevRItem()));
        QObject::connect(object, SIGNAL(toggleFullScreen()), this, SLOT(toggleFullScreen()));
        QObject::connect(object, SIGNAL(copy()), this, SLOT(copyToClipboard()));
        QObject::connect(object, SIGNAL(itemShown(QString)), this, SLOT(itemShown(QString)));
    });
    qsrand(QDateTime::currentDateTime().time().msecsSinceStartOfDay());
    int c = qrand()%120000;
    for (int i = 0; i < c; i++)
        randInt();

    QTimer *t = new QTimer();
    connect(t, &QTimer::timeout, this, &SlideShowView::saveData);
    t->start(5000);

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
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
    QString dbName = QFileInfo(QString(filename).replace("file:///","")).canonicalPath() + "/niv.db";
    currentItems.clear();
    randomItems.clear();
    nameToIndex.clear();
    currentItemIndex = -1;
    randomItemIndex = -1;
    QString absPath = QFileInfo(filename.replace("file:///", "").replace("\\","/")).absolutePath();
    qDebug() << absPath;
    if (absPath.isEmpty())
        return;
    bool hasDb = false;
    if (QFileInfo::exists(dbName)) {
        db = PicDatabase::load(dbName);
        qDebug() << "HAS DB";
        hasDb = true;
        qDebug() << db.urlToHash.count();
        db.outFileName = dbName;
    }

    QDirIterator it(absPath, QStringList() << "*.jpg"<<  "*.png" << "*.jpeg" << "*.gif", QDir::Files, QDirIterator::Subdirectories);
    int i = 0;
    while (it.hasNext())
    {
        QString itNext = it.next();
        currentItems << "file:///" + itNext;
        QString item = currentItems.last();
        nameToIndex[item] = currentItems.count() - 1;
        if (!hasDb || !db.urlToHash.contains(item))
            appendToDB(itNext, item);
        i++;
        if (i % 50 == 0) {
            showText(item);
            qApp->processEvents();
        }
    }
    QVector<QString> tempList(currentItems);
    while (!tempList.isEmpty())
    {
        int index = randInt()%tempList.count();
        randomItems.append(tempList[index]);
        tempList.removeAt(index);
        i++;
        if (i % 50 == 0) {
        showText(currentItems.last());
        qApp->processEvents();
        }
    }
    currentItemIndex = nameToIndex["file:///"+filename];
    if (!hasDb)
    {
        db.outFileName = dbName;
        db.saveToFile(dbName);
    }
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

void SlideShowView::showText(QString text)
{
    QObject *object = dynamic_cast<QObject*>(view->rootObject());
    QMetaObject::invokeMethod(object, "makeText", Q_ARG(QVariant, QVariant(text)));
}

void SlideShowView::nextItem()
{
    qDebug() << "ni";
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

    qDebug() << "pi";
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

    qDebug() << "ri";
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

    qDebug() << "pri";
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

void SlideShowView::nextMW()
{
    qDebug() << "nw";
    if (mwItems.count())
    {
        mwItemIndex++;
        if (mwItemIndex >= mwItems.count())
            mwItemIndex = 0;
        currentItemIndex = nameToIndex[mwItems[mwItemIndex]];
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(mwItems[mwItemIndex])));
        lastShown = mwItems[mwItemIndex];
    }
}

void SlideShowView::prevMW()
{

    qDebug() << "pw";
    if (mwItems.count())
    {
        mwItemIndex--;
        if (mwItemIndex < 0)
            mwItemIndex = mwItems.count() - 1;
        currentItemIndex = nameToIndex[mwItems[mwItemIndex]];
        QObject *object = dynamic_cast<QObject*>(view->rootObject());
        QMetaObject::invokeMethod(object,"showItem", Q_ARG(QVariant, QVariant(mwItems[mwItemIndex])));
        lastShown = mwItems[mwItemIndex];
    }
}

void SlideShowView::loadMW()
{
    mwItems = db.sortedByWatchTime();
    mwItemIndex = 0;
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

void SlideShowView::appendToDB(QString filename, QString url)
{
    QFile f(filename);
    QByteArray data;
    QByteArray preview;
    QString hashText;
    if (f.open(QFile::ReadOnly)) {
        data = f.readAll();
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(data);
        hashText = hash.result().toHex();
        f.close();
    }
    if (QFileInfo(filename).suffix().toLower() == "png")
    {
        QImage img = QImage::fromData(data,"PNG");
        QImage small = img.scaled(128, 128 ,Qt::KeepAspectRatio);
        QBuffer buffer(&preview);
        buffer.open(QIODevice::WriteOnly);
        small.save(&buffer, "PNG");
    }
    db.urlToHash[url] = hashText;
    db.hashToUrl[hashText] = url;
    db.watchedCount[hashText] = 0;
    db.watchedTime[hashText] = 0.0;
    db.preview[hashText] = preview;
}

void SlideShowView::itemShown(QString item)
{
    static QString prevItem;
    if (!prevItem.isEmpty()) {
        db.watchedTime[db.urlToHash[prevItem]] = db.watchedTime[db.urlToHash[prevItem]] + double(elapsed.elapsed())/1000.;
        qDebug() << "item watched time " << prevItem <<db.watchedTime[db.urlToHash[prevItem]];
    }
    elapsed.restart();
    prevItem = item;
    QString hash = db.urlToHash[item];
    db.watchedCount[hash] = db.watchedCount[hash] + 1;
    qDebug() << "item watched count " << item << db.watchedCount[hash];
}

void SlideShowView::aboutToQuit()
{
    if (!db.outFileName.isEmpty()) {
        db.saveToFile(db.outFileName);
    }
}

void SlideShowView::PicDatabase::saveToFile(QString filename)
{
    QByteArray out;
    QDataStream ds(&out, QIODevice::WriteOnly);
    ds << urlToHash << hashToUrl << watchedCount << watchedTime << preview;
    out = qCompress(out);
    QFile f(filename);
    if (f.open(QFile::WriteOnly)){
        f.write(out);
        f.flush();
        f.close();
    }
}

SlideShowView::PicDatabase SlideShowView::PicDatabase::load(QString filename)
{
    QFile f(filename);
    PicDatabase r;
    if (f.open(QFile::ReadOnly)){
        QByteArray in = qUncompress(f.readAll());
        QDataStream ds(&in, QIODevice::ReadOnly);
        ds >> r.urlToHash >> r.hashToUrl >> r.watchedCount >> r.watchedTime >> r.preview;
    }
    return r;
}

QVector<QString> SlideShowView::PicDatabase::sortedByWatchTime()
{
    QList<WatchSortedItem> result;
    foreach (const QString &k, watchedTime.keys()) {
        WatchSortedItem item;
        item.url = hashToUrl[k];
        item.time = watchedTime[k];
        result.append(item);
    }

    qSort(result.begin(), result.end(), [](const SlideShowView::PicDatabase::WatchSortedItem &a,
          const SlideShowView::PicDatabase::WatchSortedItem &b) -> bool { return a.time > b.time;});
    QVector<QString> res;
    foreach (const auto &i, result)
        res.append(i.url);

    qDebug() << "MW ITEM: " << result.first().url << " " << result.first().time;
    return res;
}
