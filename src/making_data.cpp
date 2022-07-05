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
#include "std_msgs/Float64.h"
#include "std_msgs/Float64MultiArray.h"

const double  pi = 3.141592653589793238462;

struct wheel_speed_freqs {
	int left_hz;
	int right_hz;
};


void set_power(int n){
	std::ofstream writing_file; 
	std::string filename = "/dev/rtmotoren0";
       	writing_file.open(filename,std::ios::app);
	writing_file << std::to_string(n) << std::endl;
	writing_file.close();
}

void file_write(wheel_speed_freqs freqs){
	//freqs.left_hz = 0;
	//freqs.right_hz = 0;

	std::ofstream writing_file_left;
	std::ofstream writing_file_right;
	std::string filename_left = "/dev/rtmotor_raw_l0";
	std::string filename_right= "/dev/rtmotor_raw_r0";
	writing_file_left.open(filename_left,std::ios::app);
	writing_file_right.open(filename_right,std::ios::app);
	writing_file_left << std::to_string(freqs.left_hz) << std::endl;
	writing_file_right << std::to_string(freqs.right_hz) << std::endl;
	writing_file_left.close();
	writing_file_right.close();
}

void input_raw_freqs(wheel_speed_freqs freqs,int TIME){
	file_write(freqs);
	std::this_thread::sleep_for(std::chrono::milliseconds(TIME));
	wheel_speed_freqs stop;
	stop.left_hz = 0;
	stop.right_hz =0;
	file_write(stop);
}

class SubPub{
public:
	ros::NodeHandle n;
	ros::Publisher pub;
	ros::Subscriber sub1,sub2,sub3;
	int judge_angular=0,judge_camera=0,NUMBER=0;
	double pre_data,THETA,DELTA_THETA,NUM,X,Y;
	void callback_init(const std_msgs::Float64MultiArray &init_value);
	void callback_getting_angular(const std_msgs::Float64MultiArray &angular_data);
	void imageCallback(const sensor_msgs::ImageConstPtr& msg);

	SubPub(){
		SubPub::sub1 = n.subscribe("init_info_data",1,&SubPub::callback_init,this);
		SubPub::pub = n.advertise<std_msgs::Float64>("/pre_pose_topic",1);
		SubPub::sub2 = n.subscribe("/imu/vel_pos_est",1,&SubPub::callback_getting_angular,this); 
		SubPub::sub3 = n.subscribe("/usb_cam/image_raw", 1, &SubPub::imageCallback,this);
	};
};

void SubPub::callback_getting_angular(const std_msgs::Float64MultiArray &angular_data){
	if(judge_angular==1){
		pre_data = angular_data.data[2];
		//ROS_INFO("%lf",pre_data);
		judge_angular = 0;
		cv::waitKey(2000);
	}
}

void SubPub::imageCallback(const sensor_msgs::ImageConstPtr& msg_image){
	if(judge_camera==1){
		cv_bridge::CvImagePtr cv_ptr;
		try{
			cv_ptr = cv_bridge::toCvCopy(msg_image, sensor_msgs::image_encodings::BGR8);
		}
		catch (cv_bridge::Exception& ex){
			ROS_ERROR("error");
			exit(-1);
		}

		std::string file_name_image("/home/ubuntu/image_data/image");
		file_name_image += std::to_string(NUMBER);
		file_name_image += ".png";

		cv::imwrite(file_name_image,cv_ptr->image);
		cv::waitKey(10);
		judge_camera = 0;
		NUMBER+=1;
	}
}

void SubPub::callback_init(const std_msgs::Float64MultiArray &init_value){
	//declear 
	int i;
	wheel_speed_freqs freqs;

	std::ofstream writing_file;
        std::string filename = "/home/ubuntu/text_files/sample.txt";
        writing_file.open(filename, std::ios::app);
	//(1)input data favorite pose(start_theta,delta_theta,x,y)
	//motor on
	set_power(1);
	std_msgs::Float64 msg_theta;
	msg_theta.data = init_value.data[0]*pi/180;
	//pre_data = msg_theta.data;

	THETA = init_value.data[0];
	DELTA_THETA = init_value.data[1];
	NUM = init_value.data[2];
	X = init_value.data[3];
	Y = init_value.data[4];

	
	//(2)decide omega t  of motor
	int TIME = 200;
	double Vrot = init_value.data[1]*pi*1000/(180*double(TIME));
	freqs.left_hz = -int(400*Vrot/pi);
	freqs.right_hz = -freqs.left_hz;
	//(3)camera on
		
	for(i=0;i<int(NUM);i++){
		//(4)imu_offset off
		pub.publish(msg_theta);
		//(5)motor start & stop
		input_raw_freqs(freqs,TIME);
		
		//(6)get imu data
		judge_angular = 1;
		judge_camera = 1;
		ros::spinOnce();
		//ROS_INFO("%lf",pre_data);
		//(7)get camera data 

		//(8)pre_pose start->(5)
		msg_theta.data = pre_data;

		//(9)save_data
		writing_file << i << "\t" << X << "\t" << Y << "\t"<< pre_data<<std::endl;

		//(10)break time
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	//(11)finish motor off and camera off
	set_power(0);
	writing_file.close();
	
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	//ros::NodeHandle n;
	//ros::Subscriber switch_sub = n.subscribe("switch_topic",10,callback);
	SubPub subpub;
	ros::spin();
	return 0;
}
