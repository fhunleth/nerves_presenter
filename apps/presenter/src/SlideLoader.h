#ifndef SLIDELOADER_H
#define SLIDELOADER_H

#include <QObject>
#include <QMap>
#include <QImage>
#include <QStringList>

class SlideLoader : public QObject
{
    Q_OBJECT
public:
    explicit SlideLoader(QObject *parent = 0);

signals:
    void slidesFound(int count);
    void slide(int index, QImage image);
    void noSlide(int index);

public slots:
    void setSlidePath(const QString &path);
    void requestSlide(int index);

private:
    QImage loadOne(int index);
    void shuffleFiles();
    void fixCacheIndices(int index);
    void precache(int index);

private:
    QString path_;
    QStringList files_;
    typedef QMap<int,QImage> CacheMap;
    CacheMap cache_;
};

#endif // SLIDELOADER_H
