#ifndef V4L2CAP_H
#define V4L2CAP_H

#include <asm/types.h>
#include <linux/videodev2.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>
#include <malloc.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/epoll.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define DEBUG         //预定义调试宏
#ifdef DEBUG          //封装打印函数
        #define PRINT (printf("%s %s %d: ",__FILE__,__FUNCTION__,__LINE__),printf)
#else
        #define PRINT
#endif


#define  DEV_NAME  "/dev/video4"

#define  G_PIX_FORMAT       V4L2_PIX_FMT_MJPEG    //采集格式：V4L2_PIX_FMT_MJPEG或V4L2_PIX_FMT_YUYV
#define  G_CAP_FPS          30                    //采集帧率
#define  G_WB_TEMPERATURE   4600                  //色温值
#define  G_BRIGHTNESS       0                     //亮度
#define  G_CONTRAST         2                     //对比度
#define  G_HUE              0                     //色度
#define  G_SATURATION       48                    //饱和度
#define  G_SHARPNESS        0                     //锐度
#define  G_BACKLIGHT_COMP   56                    //逆光对比/背光补偿
#define  G_GAMMA            100                   //伽玛
#define  G_EXPOSURE_TIME    300                   //曝光时间，即快门速度(10-2500),单位100us
#define  G_GAIN             100                   //增益（80-176）

#define  WIDTH    1920
#define  HEIGHT   1080

typedef unsigned char uchar;
typedef unsigned int  uint;


int get_time_now();  //获取当前时间
int usTimer(long);   //us定时器
void yuyv_to_rgb888(uchar*,uchar*,uint,uint);  //yuyv转rgb888
int open_cam();       //打开摄像头
int get_cap_para();   //获取摄像头参数
void set_cap_para();  //设置摄像头参数
void set_cap_wb(int); //设置白平衡
void setExposureTime(int); //设置曝光时间
void init_mmap();     //初始化内存映射
void start_cap();     //使能视频流
void epoll_cam();     //摄像头加入监听池
void read_frame();    //获取一帧图像
void process_img(void*,int); //图像处理

void stop_cap();  //失能视频流采集
void close_cam(); //关闭摄像头

#ifdef __cplusplus
}
#endif

#endif // V4L2CAP_H
