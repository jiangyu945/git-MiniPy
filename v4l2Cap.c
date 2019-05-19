#include "v4l2Cap.h"

//摄像头参数设置结构体
struct v4l2_capability cap;
struct v4l2_format Format;
struct v4l2_streamparm Stream_Parm;
struct v4l2_control ctrl;



//帧缓冲区
struct buffer
{
    void *start;
    size_t length;
};

struct buffer *buffers;

unsigned long n_buffers;

int cam_fd;

struct epoll_event ev,events[100];
int epfd;

uchar *jpg_buf=NULL;   //用于存储临时图像
uint size_jpg;


/*
 *  获取当前系统时间（化为总ms数）
 *  @return int  返回值，0成功，-1失败
 */
int get_time_now()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000 + now.tv_usec / 1000);
}

//获取RTC时间
void Greein_get_rtc_time(char* time_buf)
{
    int fd, retval;
    struct rtc_time rtc_tm;

    fd = open("/dev/rtc1", O_RDONLY);
    if (fd == -1) {
            perror("/dev/rtc1");
            exit(errno);
    }

    /* Read the RTC time/date */
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1) {
            perror("ioctl");
            exit(errno);
    }
    close(fd);

    sprintf(time_buf, "%04d/%02d/%02d_%02d:%02d:%02d\n",
            rtc_tm.tm_year + 1900, rtc_tm.tm_mon + 1,rtc_tm.tm_mday,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);
}

/*
 *  us定时器
 *  @param  us   定时时间
 *  @return int  返回值，0成功，-1失败
 */
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

/*
 *  打开摄像头
 *  @return int  返回值，0成功，-1失败
 */
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

    PRINT("Driver Name:%s\n Card Name:%s\n Bus info:%s\n version:%d\n capabilities:%x\n \n ", cap.driver, cap.card, cap.bus_info,cap.version,cap.capabilities);

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE){
        PRINT("Device %s: supports capture.\n",DEV_NAME);
    }
    if ((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING){
        PRINT("Device %s: supports streaming.\n",DEV_NAME);
    }

    return 0;
}

/*
 *  获取摄像头参数
 *  @return int  返回值，0成功，-1失败
 */
int get_cap_para(){


    //相关属性查询
//    struct v4l2_queryctrl qrl;
//    qrl.id = V4L2_CID_IRIS_ABSOLUTE;

//    ioctl(cam_fd,VIDIOC_QUERYCAP,&qrl);
//    PRINT("default_value = %d\n",qrl.default_value);
//    PRINT("minimum = %d\n",qrl.minimum);
//    PRINT("maximum = %d\n",qrl.maximum);



/*
 函数   ：int ioctl(intfd, int request, struct v4l2_fmtdesc *argp);
 struct v4l2_fmtdesc
 {
    __u32 index;               // 要查询的格式序号，应用程序设置
    enum v4l2_buf_type type;   // 帧类型，应用程序设置
    __u32 flags;               // 是否为压缩格式
    __u8 description[32];      // 格式名称
    __u32 pixelformat;         // 格式
    __u32 reserved[4];         // 保留
 };
 VIDIOC_ENUM_FMT    //指令含义：获取当前驱动支持的视频格式
 */
    PRINT("【**********************所有支持格式：*****************************】\n");
    struct v4l2_fmtdesc dis_fmtdesc;
    dis_fmtdesc.index=0;
    dis_fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    PRINT("Support format:\n");
    while(ioctl(cam_fd,VIDIOC_ENUM_FMT,&dis_fmtdesc)!=-1)
    {
        PRINT("\t%d.%s\n",dis_fmtdesc.index+1,dis_fmtdesc.description);
        dis_fmtdesc.index++;
    }
    PRINT("\n");


    PRINT("【**********************获取当前格式：*****************************】\n");
    Format.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(cam_fd,VIDIOC_G_FMT,&Format)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>:  %d * %d\n",Format.fmt.pix.width,Format.fmt.pix.height);
    PRINT("pix.pixelformat:%c%c%c%c\n", \
            Format.fmt.pix.pixelformat & 0xFF,\
            (Format.fmt.pix.pixelformat >> 8) & 0xFF, \
            (Format.fmt.pix.pixelformat >> 16) & 0xFF,\
            (Format.fmt.pix.pixelformat >> 24) & 0xFF);
    PRINT("\n");


    PRINT("【***********************获取帧率：******************************】\n");
    Stream_Parm.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(cam_fd,VIDIOC_G_PARM,&Stream_Parm)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: Frame rate: %u/%u\n",Stream_Parm.parm.capture.timeperframe.numerator,Stream_Parm.parm.capture.timeperframe.denominator);
    PRINT("\n");


    PRINT("【**********************获取白平衡模式：******************************】\n");
    ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 白平衡模式: %d \n",ctrl.value);
    PRINT("\n");


    PRINT("【*********************获取白平衡色温：*****************************】\n");
    ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;  //将白平衡设置为指定的开尔文色温
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 白平衡色温值: %d \n",ctrl.value);
    PRINT("\n");


    PRINT("【**********************获取亮度值：******************************】\n");
    ctrl.id= V4L2_CID_BRIGHTNESS;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 亮度值: %d \n",ctrl.value);
    PRINT("\n");


    PRINT("【**********************获取对比度：******************************】\n");
    ctrl.id=V4L2_CID_CONTRAST;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 对比度值: %d \n",ctrl.value);
    PRINT("\n");


    PRINT("【*********************获取颜色饱和度：*****************************】\n");
    ctrl.id = V4L2_CID_SATURATION;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 饱和度值: %d \n",ctrl.value);
    PRINT("\n");

    PRINT("【***********************获取色度：******************************】\n");
    ctrl.id = V4L2_CID_HUE;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 色度: %d \n",ctrl.value);
    PRINT("\n");


    PRINT("【***********************获取锐度：******************************】\n");
    ctrl.id = V4L2_CID_SHARPNESS;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 锐度: %d \n",ctrl.value);
    PRINT("\n");

    PRINT("【**********************获取背光补偿：*****************************】\n");
    ctrl.id = V4L2_CID_BACKLIGHT_COMPENSATION;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 背光补偿: %d \n",ctrl.value);
    PRINT("\n");

    PRINT("【**********************获取伽玛值：******************************】\n");
    ctrl.id = V4L2_CID_GAMMA;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 伽玛值: %d \n",ctrl.value);
    PRINT("\n");

    PRINT("*************************获取曝光绝对值（曝光时间）****************************\n"); //一般设置曝光是设置它
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">: 曝光绝对值: %d\n",ctrl.value);
    PRINT("\n");


//    PRINT("*************************获取光圈大小****************************\n");
//    ctrl.id = V4L2_CID_IRIS_ABSOLUTE;
//    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT(">: 光圈大小: %d\n",ctrl.value);
//    PRINT("\n");

//    PRINT("*************************获取焦距****************************\n");
//    ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;
//    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT(">: 光圈大小: %d\n",ctrl.value);
//    PRINT("\n");

    PRINT("***************************获取增益值************************\n");
    ctrl.id= V4L2_CID_GAIN;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT(">>: 增益: %d \n",ctrl.value);
    PRINT("\n");

    return 0;

}

/*
 *  设置摄像头参数
 *  @return int  返回值，0成功，-1失败
 */
void set_cap_para()
{
    PRINT("【*********************设置分辨率、格式：****************************】\n");
    memset(&Format,0,sizeof(struct v4l2_format));
    Format.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Format.fmt.pix.width =  WIDTH;
    Format.fmt.pix.height = HEIGHT;
    Format.fmt.pix.pixelformat= G_PIX_FORMAT;
    Format.fmt.pix.field = (enum v4l2_field)1;
    if(ioctl(cam_fd,VIDIOC_S_FMT,&Format)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【***********************设置帧率：******************************】\n");
    Stream_Parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    Stream_Parm.parm.capture.timeperframe.denominator = G_CAP_FPS;
    Stream_Parm.parm.capture.timeperframe.numerator =1;
    if(ioctl(cam_fd,VIDIOC_S_PARM,&Stream_Parm)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【**********************设置手动白平衡(/关闭自动白平衡)：******************************】\n");
    ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    ctrl.value =  0;
    if(ioctl(cam_fd,VIDIOC_G_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

//    PRINT("【*********************设置白平衡色温：*****************************】\n");
//    ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
//    ctrl.value = G_WB_TEMPERATURE;
//    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT("\n");

    PRINT("【***********************设置亮度：******************************】\n");
    ctrl.id= V4L2_CID_BRIGHTNESS;
    ctrl.value = G_BRIGHTNESS;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【**********************设置对比度：******************************】\n");
    ctrl.id = V4L2_CID_CONTRAST;
    ctrl.value= G_CONTRAST;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【**********************设置饱和度：******************************】\n");
    ctrl.id = V4L2_CID_SATURATION;
    ctrl.value= G_SATURATION;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【***********************设置色度：******************************】\n");
    ctrl.id = V4L2_CID_HUE;
    ctrl.value = G_HUE;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【***********************设置锐度：******************************】\n");
    ctrl.id = V4L2_CID_SHARPNESS;
    ctrl.value = G_SHARPNESS;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");


    PRINT("【**********************设置背光补偿：*****************************】\n");
    ctrl.id = V4L2_CID_BACKLIGHT_COMPENSATION;
    ctrl.value = G_BACKLIGHT_COMP;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    PRINT("【**********************设置伽玛值：******************************】\n");
    ctrl.id = V4L2_CID_GAMMA;
    ctrl.value = G_GAMMA;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");


    //曝光强度=曝光时间*光圈大小*ISO(此处保持光圈大小和ISO值不变,仅调整曝光时间来进行调整)
    /***********设置手动曝光***************************/
    PRINT("【**********设置手动曝光**************************】\n");
    ctrl.id= V4L2_CID_EXPOSURE_AUTO;
    ctrl.value = V4L2_EXPOSURE_MANUAL;   //手动曝光时间,手动光圈
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

    /***********设置曝光绝对值***************************/
    PRINT("【***********设置曝光时间************************】\n");
    ctrl.id= V4L2_CID_EXPOSURE_ABSOLUTE;
    ctrl.value= G_EXPOSURE_TIME;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");

//    /***********设置光圈大小***************************/
//    PRINT("【***********设置光圈大小************************】\n");
//    ctrl.id = V4L2_CID_IRIS_ABSOLUTE;
//    ctrl.value = ;
//    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT("\n");


//    /***********设置对焦模式***************************/
//    PRINT("【***********设置对焦模式***************************】\n");
//    PRINT("1> 关闭自动对焦\n");
//    ctrl.id= V4L2_CID_FOCUS_AUTO;
//    ctrl.value= 0;
//    if(ioctl(fd,VIDIOC_S_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT("\n");

//    PRINT("2> 设置焦点值\n");
//    ctrl.id= V4L2_CID_FOCUS_ABSOLUTE;
//    ctrl.value= val;  //val需填写
//    if(ioctl(fd,VIDIOC_S_CTRL,&ctrl)==-1)
//    {
//        perror("ioctl");
//        exit(EXIT_FAILURE);
//    }
//    PRINT("\n");

     /***************设置增益***************************/
     PRINT("【***************设置增益***************************】\n");
     ctrl.id = V4L2_CID_GAIN;
     ctrl.value = G_GAIN;   //测得范围80-176，默认值100
     if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
     {
         perror("ioctl");
         exit(EXIT_FAILURE);
     }
     PRINT("\n");


    PRINT("【******************验证设置**********************】\n");
    PRINT("【******************验证设置**********************】\n");

}


/*
 *  设置白平衡色温
 *  @param  value   色温值
 *  @return void
 */
void set_cap_wb(int value)
{
    PRINT("【*********************设置白平衡色温：*****************************】\n");
    ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    ctrl.value = value;
     if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");
}

/*
 *  设置白平衡色温
 *  @param  value   曝光值
 *  @return void
 */
void setExposureTime(int value)
{
    /***********设置曝光绝对值***************************/
    PRINT("【***********设置曝光时间************************】\n");
    ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    ctrl.value = value;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
    PRINT("\n");
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

    PRINT("req.count:%d\n",req.count);

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


/*
 *  图像数据处理
 *  @param  addr   图像数据地址
 *  @param  length 数据长度
 *  @return void
 */
void process_img(void* addr,int length)
{
    //申请格式转换后的数据存储buffer
    jpg_buf = (uchar *)malloc(length*sizeof(uchar));

    size_jpg = length;
    memcpy(jpg_buf,addr,size_jpg);


    //YUYV转RGB888
    //yuyv_to_rgb888((uchar*)addr,rgb888_buf,WIDTH,HEIGHT);


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
    PRINT("Camera has closed!\n");
}
