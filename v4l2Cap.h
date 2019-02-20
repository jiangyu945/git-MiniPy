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

#define  DEV_NAME  "/dev/video5"

#define  WIDTH    1280
#define  HEIGHT   720

typedef unsigned char uchar;
typedef unsigned int  uint;


int get_time_now();  //获取当前时间
int usTimer(long);   //us定时器
void yuyv_to_rgb888(uchar*,uchar*,uint,uint);  //yuyv转rgb888
int open_cam();    //打开摄像头
int set_cap_frame(); //设置帧格式
void get_fps();   //获取帧率
void init_mmap(); //初始化内存映射
void start_cap(); //使能视频流
void epoll_cam(); //摄像头加入监听池
void read_frame(); //获取一帧图像
void process_img(void*,int length); //图像处理

void stop_cap();  //失能视频流采集
void close_cam(); //关闭摄像头

#ifdef __cplusplus
}
#endif

#endif // V4L2CAP_H
