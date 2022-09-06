#include <ros/ros.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include "std_msgs/Int16.h"
#include "std_msgs/Float64.h"
#include "std_msgs/Float64MultiArray.h"
#include <stdio.h>

const double  pi = 3.141592653589793238462;
int NO = 0;

class SubPub{
public:
	ros::NodeHandle n;
	ros::Publisher pub;
	ros::Subscriber sub,sub2;
	int judge1=0,judge2=0;
	double pre_data;
	void callback_judge(const std_msgs::Int16 &number);
	void callback_getting_angular(const std_msgs::Float64MultiArray &angular_data);

	SubPub(){
		SubPub::pub = n.advertise<std_msgs::Float64>("/pre_pose_topic",1);
		SubPub::sub2 = n.subscribe("/switch_topic",1,&SubPub::callback_judge,this);
		SubPub::sub = n.subscribe("/imu/vel_pos_est",1,&SubPub::callback_getting_angular,this); 
	};
};

void SubPub::callback_judge(const std_msgs::Int16 &number){
	int a = number.data;
	if(a==0 && judge1==0){
	std_msgs::Float64 msg_theta;
	msg_theta.data = 0;
	pub.publish(msg_theta);
	judge1 = 1;
	}
	else if(judge1==1 && a==1){
		judge2 = 1;
		judge1 = 0;
	}
}
void SubPub::callback_getting_angular(const std_msgs::Float64MultiArray &angular_data){
	if(judge2 == 1){
		pre_data = angular_data.data[2];
		std::ofstream writing_file;
		std::string filename = "/home/ubuntu/text_files/angluar_file.txt";
		writing_file.open(filename, std::ios::app);
		writing_file << pre_data << std::endl;	
		writing_file.close();
		judge2 = 0;
	}
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	SubPub subpub;
	int key;
	ros::spin();
	return 0;
}
