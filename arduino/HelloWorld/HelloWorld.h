#include <ros.h>
#include <ros/time.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>


                int power =150; 
                class NewHardware : public ArduinoHardware {
                  public: NewHardware(): ArduinoHardware(&Serial1,57600){};
                }; ros::NodeHandle_<NewHardware> nh;
                
               
		std_msgs::String str_msg;


		sensor_msgs::Range range_msg;
		char frameid[13] = "/ultrasound";

		 int rightTrack=1;
		 int leftTrack=1;
		void driveCallback(const geometry_msgs::Twist &msg);
                ros::Subscriber<geometry_msgs::Twist> sub ("/cmd_vel",&driveCallback);


