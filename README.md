# git-MiniPy
描述：一款基于ARM + Linux + Qt5 + OpenCV 的简易拍照相机。

主要功能：
1) 视频实时显示（TCP、UDP图像传输，ffmpeg实现H.264压缩编解码）；
2）图像抓拍；
3）相机参数调节（亮度、饱和度、色调、曝光、白平衡矫正等）；
4）目标尺寸自动测量（双目测距原理，基于图像处理开源库OpenCV3.4实现）；
5）图像存储：SD卡；
     
代码实现：V4L2图像采集 + TCP/UDP图像传输 + QT显示（QT多线程、网络编程、信号槽、事件循环等）+ OpenCV图像处理（相机参数调节、双目测距算法等）。
