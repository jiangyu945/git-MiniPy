#ifndef OPENCV_MEASURE_H
#define OPENCV_MEASURE_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <vector>

using namespace cv;
using namespace std;

void PaintText(Mat& , char* , Point);
Mat fineMinAreaRect(Mat&, Mat&,int);
Mat opencv_measure(Mat&,int);



#endif // OPENCV_MEASURE_H
