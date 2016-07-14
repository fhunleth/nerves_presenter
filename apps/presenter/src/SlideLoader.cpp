#include "SlideLoader.h"
#include <QDir>
#include <QDateTime>

SlideLoader::SlideLoader(QObject *parent) : QObject(parent)
{
}

void SlideLoader::setSlidePath(const QString &path)
{
    // Seed the random number generated to get a new set of slides each time.
    qsrand((uint) QDateTime::currentMSecsSinceEpoch());

    path_ = path;
    QDir dir(path);
    QStringList nameFilters;
    nameFilters << "*.jpg"
                << "*.png"
                << "*.gif"
                << "*.jpeg";
    files_ = dir.entryList(nameFilters, QDir::Files);
    shuffleFiles();

    cache_.clear();

    emit slidesFound(files_.count());
}

void SlideLoader::requestSlide(int index)
{
    CacheMap::const_iterator i = cache_.find(index);
    if (i != cache_.constEnd()) {
        emit slide(index, i.value());
    } else {
        QImage img = loadOne(index);
        if (!img.isNull())
            emit slide(index, img);
        else
            emit noSlide(index);
    }

    precache(index);
}

void SlideLoader::precache(int index)
{
    int precacheIndex = index + 1;
    if (!cache_.contains(precacheIndex))
        loadOne(precacheIndex);

    if (cache_.count() > 5) {
        QList<int> loadedIndices = cache_.keys();
        qSort(loadedIndices);
        cache_.remove(loadedIndices.at(0));
    }
}

QImage SlideLoader::loadOne(int index)
{
    if (index < 0)
        return QImage();

    while (index < files_.count()) {
        QString fullPath = path_ + "/" + files_.at(index);
        QImage img(fullPath);
        if (img.isNull()) {
            qWarning("Couldn't load %s", qPrintable(fullPath));
            files_.removeAt(index);
            fixCacheIndices(index);
        } else {
            cache_.insert(index, img);
            return img;
        }
    }
    return QImage();
}

void SlideLoader::fixCacheIndices(int index)
{
    QList<int> tofix;
    for (CacheMap::iterator i = cache_.begin();
         i != cache_.end();
         ++i) {
        if (i.key() >= index)
            tofix << i.key();
    }
    qSort(tofix);
    for (int i = 0; i < tofix.count(); i++) {
        int oldIndex = tofix.at(i);
        QImage img = cache_.value(oldIndex);
        cache_.remove(oldIndex);
        cache_.insert(oldIndex - 1, img);
    }
}

void SlideLoader::shuffleFiles()
{
    int count = files_.count();
    for (int i = 0; i < count; i++)
        files_.swap(i, qrand() % count);
}

