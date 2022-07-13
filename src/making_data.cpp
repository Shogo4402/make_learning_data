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
int NO = 0;

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
	int judge_angular=0,judge_camera=0,NUMBER=0,DATE,SET;
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
		judge_angular = 0;
		cv::waitKey(1000);
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
	
		std::stringstream ss;
		std::string file_name_image("/home/ubuntu/image_data/image");
		ss << file_name_image << "_" << std::to_string(DATE) << "_" << std::to_string(SET) << "_" << std::to_string(NO) << ".png";
		cv::imwrite(ss.str(),cv_ptr->image);
		cv::waitKey(10);
		judge_camera = 0;
		NO+=1;
	}
}

void SubPub::callback_init(const std_msgs::Float64MultiArray &init_value){
	//declear 
	int i;
	wheel_speed_freqs freqs;
	std_msgs::Float64 msg_theta;
	msg_theta.data = init_value.data[2]*pi/180;
	int TIME = 200;
	double Vrot;
	DATE = init_value.data[0]; //220705
	SET = init_value.data[1]; //01
	THETA = init_value.data[2]*pi/180;
	DELTA_THETA = init_value.data[3]*pi/180;
	NUM = init_value.data[4];
	X = init_value.data[5];
	Y = init_value.data[6];
	std::ofstream writing_file;
        std::string filename = "/home/ubuntu/text_files/sample.txt";
        writing_file.open(filename, std::ios::app);

	//motor on
	set_power(1);

	for(i=0;i<=int(NUM);i++){
		//decide motor_speed_freqs
		Vrot = (THETA+DELTA_THETA*(i)-msg_theta.data)*1000/(double(TIME));
		freqs.left_hz = -int(400*Vrot/pi);
		freqs.right_hz = -freqs.left_hz;
		//imu_offset off
		pub.publish(msg_theta);
		//motor start & stop
		input_raw_freqs(freqs,TIME);
		//get imu data and image of camera
		judge_angular = 1;
		judge_camera = 1;
		ros::spinOnce();
		//substitute pre_pose 
		msg_theta.data = pre_data;
		//save_data
		writing_file << DATE<< "\t" << SET <<"\t" << NO << "\t" << X << "\t" << Y << "\t"<< pre_data<<std::endl;

	}
	//finish motor off and file close
	set_power(0);
	writing_file.close();
	
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	SubPub subpub;
	ros::spin();
	return 0;
}
