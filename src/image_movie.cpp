#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <ros/ros.h>

using namespace cv;
int main(int argc, char** argv )
{
	cv::VideoCapture cap(0,cv::CAP_V4L2);
	ROS_INFO("start");
	cap.set(cv::CAP_PROP_FOURCC,VideoWriter::fourcc('M','J','P','G'));
	cap.set(cv::CAP_PROP_FPS,30);
	cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT,480);
	if(!cap.isOpened()){
		ROS_INFO("camera not opened");
		return -1;
	}


	cv::Mat image;
	int key;
	while(cap.read(image)){
		key = cv::waitKey(30);
		if(key==27){
			cv::imwrite("img.png",image);
			break;
		}		
		cv::resize(image,image,cv::Size(),0.25,0.25);
		cv::imshow("img",image);
		//ROS_INFO("Successfully reading");
	}
	cv::destroyAllWindows();
	return 0;
}
