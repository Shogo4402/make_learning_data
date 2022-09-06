#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>

using namespace cv;
int main(int argc, char** argv )
{
	cv::VideoCapture cap(0,cv::CAP_V4L2);
	ROS_INFO("start");
	cap.set(cv::CAP_PROP_FOURCC,VideoWriter::fourcc('M','J','P','G'));
	cap.set(cv::CAP_PROP_FPS,30);
	cap.set(cv::CAP_PROP_BUFFERSIZE,1);
	cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT,480);
	if(!cap.isOpened()){
		ROS_INFO("camera not opened");
		return -1;
	}
	int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
	double fps = 30.0;
	int width = 640;
	int height = 480;
	cv::VideoWriter writer("test.mp4", fourcc, fps, cv::Size(width, height));

	ros::init(argc,argv,"camera_node");
	ros::NodeHandle nh;
	cv::Mat image;
	int key;
	while(ros::ok()){
		/*if(cap.read(image)){
			key = cv::waitKey(10);
			writer << image;
			cv::resize(image,image,cv::Size(),0.3,0.3);
			cv::imshow("img",image);
			if(key=='q'){
				cap.release();
				writer.release();
				break;
			}
		}
		else{
			ROS_INFO("FAIL CAMERA");
		}*/
	}
	cv::destroyAllWindows();
	return 0;
}
