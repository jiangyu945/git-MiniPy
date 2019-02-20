#include "workerthread.h"
#include "v4l2Cap.h"

extern unsigned char *jpg_buf;
extern unsigned int size_jpg;
QSemaphore Sem(1);   //定义只含一个信号灯的二值信号量

workerThread::workerThread(QObject *parent) : QObject(parent)
{

}

void workerThread::doProcessReadFrame()
{
    Sem.acquire();  //获取二值信号量

    qDebug() << "Now in workerThread : start to read frame...\n";
    read_frame();  //获取一帧图像
    qDebug() << "Read finished : emit SigToDisplay()\n";
    emit SigToDisplay(jpg_buf,size_jpg);  //发送显示信号

    //释放信号量,允许
    Sem.release();
}
