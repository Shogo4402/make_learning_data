#include <ros/ros.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
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
	ros::Subscriber sub1,sub2;
	void callback_init(const std_msgs::Float64MultiArray &init_value);
	void callback_getting_angular(const std_msgs::Float64MultiArray &angular_data);
	double pre_data;
	SubPub(){
		SubPub::sub1 = n.subscribe("init_info_data",10,&SubPub::callback_init,this);
		SubPub::pub = n.advertise<std_msgs::Float64>("/pre_pose_topic",10);
	};
};

void SubPub::callback_getting_angular(const std_msgs::Float64MultiArray &angular_data){
	pre_data = angular_data.data[2];
}

void SubPub::callback_init(const std_msgs::Float64MultiArray &init_value){
	//declear 
	int i;
	wheel_speed_freqs freqs;

	//(1)input data favorite pose(start_theta,delta_theta,x,y)
	//motor on
	set_power(1);
	std_msgs::Float64 msg;
	msg.data = init_value.data[0];
	
	//(2)decide omega t  of motor
	int TIME = 200;
	double Vrot = init_value.data[1]*pi*1000/(180*double(TIME));
	freqs.left_hz = -int(400*Vrot/pi);
	freqs.right_hz = -freqs.left_hz;
	//(3)camera on
		
	for(i=0;i<int(init_value.data[2]);i++){
		//(4)imu_offset off
		pub.publish(msg);
		//(5)motor start & stop
		input_raw_freqs(freqs,TIME);
		
		//(6)get imu data
		sub2 = n.subscribe("/imu/vel_pos_est",10,&SubPub::callback_getting_angular,this); 
		
		//(7)get camera data 
		//(8)pre_pose start->(5)
		msg.data = pre_data;

		//(9)save_data

		//(10)break time
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	//(11)finish motor off and camera off
	set_power(0);
	
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	//ros::NodeHandle n;
	//ros::Subscriber switch_sub = n.subscribe("switch_topic",10,callback);
	SubPub subpub;
	ros::spin();
	return 0;
}
