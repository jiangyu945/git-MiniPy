#include "v4l2Cap.h"

struct buffer
{
    void *start;
    size_t length;
};

struct buffer *buffers; //帧缓冲区

unsigned long n_buffers;

int cam_fd;

struct epoll_event ev,events[100];
int epfd;

uchar *jpg_buf=NULL;   //用于存储临时图像
uint size_jpg;


/*获取当前时间（化为总ms数）*/
int get_time_now()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000 + now.tv_usec / 1000);
}


int usTimer(long us)
{
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = us;

    return select(0,NULL,NULL,NULL,&timeout);
}



/*********************************************
*函数：void yuyv_to_rgb888(uchar* yuyv,uchar* rgb888, uint width, uint height);
*功能：YUYV格式转RGB888格式
*参数：
*        yuyv  ： yuyv图像数据起始位置
*        rgb888:  转换后rgb888数据存储起始位置
*        width :  图像宽度
*        height:  图像高度
*返回值： 无
*********************************************/
void yuyv_to_rgb888(uchar* yuyv, uchar* rgb888, uint width, uint height)
{
        uint i;
        uchar* y0 = yuyv + 0;
        uchar* u0 = yuyv + 1;
        uchar* y1 = yuyv + 2;
        uchar* v0 = yuyv + 3;

        uchar* r0 = rgb888 + 0;
        uchar* g0 = rgb888 + 1;
        uchar* b0 = rgb888 + 2;
        uchar* r1 = rgb888 + 3;
        uchar* g1 = rgb888 + 4;
        uchar* b1 = rgb888 + 5;

        float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;

        for(i = 0; i <= (width * height) / 2 ;i++)
        {
            bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128);
            gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
            rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128);

            bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128);
            gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128);
            rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128);


            if(rt0 > 250)  	rt0 = 255;
            if(rt0< 0)  rt0 = 0;

            if(gt0 > 250) 	gt0 = 255;
            if(gt0 < 0)	gt0 = 0;

            if(bt0 > 250)	bt0 = 255;
            if(bt0 < 0)	bt0 = 0;

            if(rt1 > 250)	rt1 = 255;
            if(rt1 < 0)	rt1 = 0;

            if(gt1 > 250)	gt1 = 255;
            if(gt1 < 0)	gt1 = 0;

            if(bt1 > 250)	bt1 = 255;
            if(bt1 < 0)	bt1 = 0;

            *r0 = (uchar)rt0;
            *g0 = (uchar)gt0;
            *b0 = (uchar)bt0;

            *r1 = (uchar)rt1;
            *g1 = (uchar)gt1;
            *b1 = (uchar)bt1;

            yuyv = yuyv + 4;
            rgb888 = rgb888 + 6;
            if(yuyv == NULL)
              break;

            y0 = yuyv;
            u0 = yuyv + 1;
            y1 = yuyv + 2;
            v0 = yuyv + 3;

            r0 = rgb888 + 0;
            g0 = rgb888 + 1;
            b0 = rgb888 + 2;
            r1 = rgb888 + 3;
            g1 = rgb888 + 4;
            b1 = rgb888 + 5;
        }
}

int open_cam()
{
    struct v4l2_capability cap;
    cam_fd = open(DEV_NAME, O_RDWR | O_NONBLOCK, 0);  //非阻塞方式打开摄像头
    if (cam_fd < 0)
    {
        perror("open device failed!");
        return -1;
    }

    /*获取摄像头信息*/
    if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        perror("get info failed!");
        return -1;
    }

    printf("Driver Name:%s\n Card Name:%s\n Bus info:%s\n version:%d\n capabilities:%x\n \n ", cap.driver, cap.card, cap.bus_info,cap.version,cap.capabilities);

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE){
        printf("Device %s: supports capture.\n",DEV_NAME);
    }
    if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING){
        printf("Device %s: supports streaming.\n",DEV_NAME);
    }
    return 0;
}

int set_cap_frame()
{
    struct v4l2_format fmt;

    /*设置摄像头捕捉帧格式及分辨率*/
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = WIDTH;
    fmt.fmt.pix.height      = HEIGHT;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;   //图像采集格式设为YUYV(YUV422)

    if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        perror("set fmt failed!");
        return -1;
    }

    printf("fmt.type:%d\n",fmt.type);
    printf("pix.pixelformat:%c%c%c%c\n", \
            fmt.fmt.pix.pixelformat & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF, \
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,\
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.width:%d\n",fmt.fmt.pix.width);
    printf("pix.height:%d\n",fmt.fmt.pix.height);
    printf("pix.field:%d\n",fmt.fmt.pix.field);

    return 0;
}


//获取帧率
void get_fps()
{
    struct v4l2_streamparm *parm;
    parm = (struct v4l2_streamparm *)calloc(1, sizeof(struct v4l2_streamparm));
    memset(parm, 0, sizeof(struct v4l2_streamparm));
    parm->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int rel = ioctl(cam_fd,VIDIOC_G_PARM, parm);
    if(rel == 0)
    {
        printf("Frame rate:  %u/%u\n",parm->parm.capture.timeperframe.denominator,parm->parm.capture.timeperframe.numerator);
    }
    free(parm);
}

//初始化内存映射
void init_mmap()
{
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;

    /*申请4个图像缓冲区（位于内核空间）*/
    req.count  = 4;                             //缓存区数量，即缓存队列里保持多少张照片，一般不超过5个 (经测试3.0.15内核最多申请23个缓存区)
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;              //MMAP内存映射方式
    ioctl(cam_fd, VIDIOC_REQBUFS, &req);

    printf("req.count:%d\n",req.count);

    buffers = calloc(req.count, sizeof(struct buffer));

    /*将申请的4个缓冲区映射到用户空间*/
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = n_buffers;

        ioctl(cam_fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;

        buffers[n_buffers].start = mmap(NULL,
                                        buf.length,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        cam_fd,
                                        buf.m.offset);
    }

    /*缓冲区入队列*/
    uint i;
    for(i = 0; i < n_buffers; ++i)
    {
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;
        ioctl(cam_fd, VIDIOC_QBUF, &buf);
    }

}


/*使能视频设备输出视频流*/
void start_cap()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(cam_fd, VIDIOC_STREAMON, &type);
}

//图像数据处理
void process_img(void* addr,int length)
{
    //申请格式转换后的数据存储buffer
    jpg_buf = (uchar *)malloc(length*sizeof(uchar));

    size_jpg = length;
    memcpy(jpg_buf,addr,size_jpg);

    //YUYV转RGB888
//    yuyv_to_rgb888((uchar*)addr,rgb888_buf,WIDTH,HEIGHT);

}

//摄像头加入监听池
void epoll_cam()
{
    epfd = epoll_create(512);

    ev.data.fd = cam_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, cam_fd, &ev);
}

//获取一帧图像
void read_frame()
{
    //开始监听摄像头
    int nfds = epoll_wait(epfd,events,500,-1);
    int i;
    if(nfds > 0){
        for(i=0;i<nfds;i++){
            if(events[i].data.fd == cam_fd){
                if(events[i].events & EPOLLIN){
                    //读取一帧read_frame()
                    struct v4l2_buffer buf;

                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    ioctl(cam_fd, VIDIOC_DQBUF, &buf);  //出队，从队列中取出一个缓冲帧

                    process_img(buffers[buf.index].start,buffers[buf.index].length);  //图像处理(格式转换等)//

                    ioctl(cam_fd, VIDIOC_QBUF, &buf); //重新入队
                }
            }
        }
    }
}

//停止采集
void stop_cap()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(cam_fd,VIDIOC_STREAMOFF,&type))
    {
        perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
        exit(EXIT_FAILURE);
    }
}

//关闭摄像头
void close_cam()
{
    uint i;
    //解除映射
    for (i = 0; i < n_buffers; ++i)
    {
        munmap(buffers[i].start, buffers[i].length);
    }

    free(buffers);
    close(cam_fd);
    printf("Camera Capture Done.\n");
}
