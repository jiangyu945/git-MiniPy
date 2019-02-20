#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QDebug>

#include <QSemaphore>   //信号量
#include <QMutex>       //锁
#include <QMutexLocker>

class workerThread : public QObject
{
    Q_OBJECT
public:
    explicit workerThread(QObject *parent = 0);

signals:
    void SigToDisplay(unsigned char* buf,unsigned int len);

public slots:
    void doProcessReadFrame();
};

#endif // WORKERTHREAD_H
