#include "workerthread.h"
#include "v4l2Cap.h"

extern unsigned char *jpg_buf;
extern unsigned int size_jpg;
QSemaphore Sem(1);   //定义只含一个信号灯的二值信号量

workerThread::workerThread(QObject *parent) : QObject(parent)
{

}

void workerThread::doProcessReadFrame(QImage img)
{
    tt.start();
    Sem.acquire();  //获取二值信号量
    qDebug("等待耗时: %d ms",tt.elapsed());  //等待耗时

    read_frame();  //获取一帧图像
    qDebug("读取耗时: %d ms",tt.elapsed());  //读取耗时

    img.loadFromData(jpg_buf,size_jpg);  //加载图像数据到img
    qDebug("加载耗时: %d ms",tt.elapsed());  //加载耗时

    free(jpg_buf);  //释放临时缓存空间

    emit SigToDisplay(img);  //发送显示信号

    //释放信号量,允许
    Sem.release();
}
