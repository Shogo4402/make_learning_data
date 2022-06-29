#include <ros/ros.h>
#include <iostream>
#include "std_msgs/Int16.h"

void process0(){
	ROS_INFO("switch0");
}
void process1(){
	ROS_INFO("switch1");
}
void process2(){
	ROS_INFO("switch2");
}



void callback(const std_msgs::Int16 switch_no){
	if(switch_no.data==0){
		process0();
	}
	else if(switch_no.data==1){
		process1();
	}
	else if(switch_no.data==2){
		process2();
	}
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	ros::NodeHandle n;
	ros::Subscriber switch_sub = n.subscribe("switch_topic",10,callback);
	ros::spin();
	return 0;
}
