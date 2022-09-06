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
	ros::Subscriber sub,sub2,sub3;
	int judge1=0,judge2=0,judge_camera=0;
	double pre_data;
	void callback_judge(const std_msgs::Int16 &number);
	void imageCallback(const sensor_msgs::ImageConstPtr& msg);

	SubPub(){
		SubPub::pub = n.advertise<std_msgs::Float64>("/pre_pose_topic",1);
		SubPub::sub2 = n.subscribe("/switch_topic",1,&SubPub::callback_judge,this);
		SubPub::sub3 = n.subscribe("image_raw", 1, &SubPub::imageCallback,this);
	};
};

void SubPub::callback_judge(const std_msgs::Int16 &number){
        int a = number.data;
        int i;
        wheel_speed_freqs freqs;
        std_msgs::Float64 msg_theta;
        msg_theta.data = 0;
        int TIME = 200;
        double Vrot;
        double DELTA_THETA = 30*pi/180;
        int NUM = 4;
	cv::waitKey(300);
        if(a==0 && judge1==0){
		judge1 = 1; 
                set_power(1);
                for(i=0;i<NUM;i++){
                        Vrot = (DELTA_THETA)*1000/(double(TIME));
                        freqs.left_hz = -int(400*Vrot/pi);
                        freqs.right_hz = -freqs.left_hz;
                        pub.publish(msg_theta);
                        input_raw_freqs(freqs,TIME);
                        judge_camera = 1;
                        ros::spinOnce();
			cv::waitKey(1500);
                }
                set_power(0);
        }
	else if(a==2&&judge1==0){
                judge1 = 1;
                set_power(1);
		TIME=500;
		DELTA_THETA=90*pi/180;
                for(i=0;i<NUM;i++){
                        Vrot = (DELTA_THETA)*1000/(double(TIME));
                        freqs.left_hz = -int(400*Vrot/pi);
                        freqs.right_hz = freqs.left_hz;
                        pub.publish(msg_theta);
                        input_raw_freqs(freqs,TIME);
                        judge_camera = 1;
                        ros::spinOnce();
                        cv::waitKey(1500);
                }
                set_power(0);
	}
	else{
		judge1=0;
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
                std::string file_name_image("/home/ubuntu/increase_halfdata/image");
                ss << file_name_image << std::to_string(NO) << ".png";
                cv::imwrite(ss.str(),cv_ptr->image);
                cv::waitKey(10);
                judge_camera = 0;
		NO+=1;
        }
}

int main(int argc, char** argv){
	ros::init(argc,argv,"making_data_node");
	SubPub subpub;
	int key;
	ros::spin();
	return 0;
}
