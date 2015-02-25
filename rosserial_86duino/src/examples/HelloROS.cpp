/*
 * rosserial Publisher Example
 * Prints "hello ROS!"
 */

#include <ros.h>
#include <std_msgs/String.h>
#include <stdio.h>
#include <unistd.h>

ros::NodeHandle  nh;

std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);

char rosSrvrIp[25] = "192.168.4.175";
char hello[13] = "Hello ROS!";

int main()
{
	//nh.initNode();
	nh.initNode(rosSrvrIp);
	nh.advertise(chatter);
	while(1) {
		  str_msg.data = hello;
		  chatter.publish( &str_msg );
		  nh.spinOnce();
		  printf("chattered\n");
		  sleep(1);
	}
}

