#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QDebug>

#include <QSemaphore>   //信号量
#include <QMutex>       //锁
#include <QMutexLocker>
#include <QImage>
#include <QPixmap>

#include <QTime>

class workerThread : public QObject
{
    Q_OBJECT
public:
    explicit workerThread(QObject *parent = 0);

signals:
    void SigToDisplay(QImage img);

public slots:
    void doProcessReadFrame(QImage img);

private:
    QTime tt;

};

#endif // WORKERTHREAD_H
