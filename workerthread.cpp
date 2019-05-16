#include "workerthread.h"
#include "v4l2Cap.h"

extern unsigned char *jpg_buf;
extern unsigned int size_jpg;
QSemaphore Sem(1);   //定义只含一个信号灯的二值信号量

workerThread::workerThread(QObject *parent) : QObject(parent)
{

}

//读取一帧图像数据
void workerThread::doProcessReadFrame()
{
//    tt.start();
    Sem.acquire();  //获取二值信号量

    read_frame();  //获取一帧图像
    QImage img;

    img.loadFromData(jpg_buf,size_jpg);  //加载图像数据到img
    //QImage *rgb_img = new QImage(rgb_buf,WIDTH,HEIGHT,WIDTH*3,QImage::Format::RGB888);
    free(jpg_buf);  //释放临时缓存空间

    emit SigToDisplay(img);  //发送显示信号

    //释放信号量,允许
    Sem.release();
}
