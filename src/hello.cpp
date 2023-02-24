#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include "sensor_msgs/Imu.h"
int c=0;
double x=0;
double y=0;
double z=0;


void cmd_callback(const sensor_msgs::Imu & imu)
{
    std::cout << imu.linear_acceleration.x<< std::endl;
    std::cout << imu.linear_acceleration.y+0.41<< std::endl;
    std::cout << imu.linear_acceleration.z+0.3<< std::endl;
    c=c+1;
    std::cout << c << std::endl;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "hello");
	ros::NodeHandle nh;
    ros::Subscriber cmd_sub = nh.subscribe("/imu0/imu/data_raw", 10, cmd_callback);
	
	while(ros::ok()){
		ros::spinOnce();
	}
    

    return 0;
}


/*
    std::cout << imu.linear_acceleration.x -0.05<< std::endl;
    std::cout << imu.linear_acceleration.y +0.3<< std::endl;
    std::cout << imu.linear_acceleration.z -0.32<< std::endl;
*/
