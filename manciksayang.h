#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <evhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <math.h>

#define REAL_DISTANCEPOINT    380    //this is the distance between the side red circle
#define PORT                  6777

using namespace cv;
using namespace std;
mutex m;
int xdata, ydata;
int xfirst = 0;
int yfirst = 0;
long double magic;


struct myclass {
    bool operator() (cv::Point pt1, cv::Point pt2) { return (pt1.x < pt2.x);}
} myobject;

void notfound (struct evhttp_request *req, void *params);
void http_serving();
void  setmagic(int x1,int x2,int rl);
void setzero(int x, int y, int x1, int x2, int reals);
