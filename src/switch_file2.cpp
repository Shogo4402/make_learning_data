#include <ros/ros.h>
#include <iostream>
#include <fstream>
#include <string>
#include "std_msgs/Int16.h"
#include "fcntl.h"


char get_SW0(void){
 char buf[2];
 int SW0;
 SW0 = open("/dev/rtswitch0",O_RDONLY);
 read(SW0,buf,2);
 close(SW0);
 return buf[0];
}

char get_SW1(void){
 char buf[2];
 int SW1;
 SW1 = open("/dev/rtswitch1",O_RDONLY);
 read(SW1,buf,2);
 close(SW1);
 return buf[0];
}

char get_SW2(void){
 char buf[2];
 int SW2;
 SW2 = open("/dev/rtswitch2",O_RDONLY);
 read(SW2,buf,2);
 close(SW2);
 return buf[0];
}


int main(int argc, char** argv){

	ros::init(argc,argv,"switch_node");
	ros::NodeHandle n;
	ros::Publisher switch_pub = n.advertise<std_msgs::Int16>("switch_topic",1);

	std_msgs::Int16 msg;
	int sw0,sw1,sw2;

	while(ros::ok()){
		if(int(get_SW0()==48)){
			msg.data = 0;
			switch_pub.publish(msg);
		}
		else{
			if(int(get_SW1()==48)){
				msg.data = 2;
				switch_pub.publish(msg);
			}

			else{
				msg.data=1;
				switch_pub.publish(msg);
			}
		}
	}

	/*while(ros::ok()){
		if(int(get_SW0())==48){msg.data=0;switch_pub.publish(msg);}
		else if(int(get_SW1())==48){msg.data=1;switch_pub.publish(msg);}
		else if(int(get_SW2())==48){msg.data=2;switch_pub.publish(msg);}
	}*/


	return 0;
}
