/*	
	Group 25
	Stephan Neevel and Thijs Boumans
	This code gets a picture stream from a camera and detects a line in the picture. A gray-scale picture will be viewed on screen. On basis of where this line is (to the left of the center, in the middle or to the right of the center), the code posts a geometry_msgs::Twist on the topic cmd_vel, with a turning velocity and a linear velocity. 
	RUN with $rosrun line_detection linefollower_25 _image_transport:=compressed
*/
#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <image_transport/image_transport.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <iostream>

geometry_msgs::Twist twistmsg;//the message to be published

void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
	cv::Mat img = cv_bridge::toCvCopy(msg, "bgr8")->image;// picture now imported and stored in img
	
	float scale = 0.1;//scaler
	float ps = 0.0;//perspective scaler, to be tweaked to match phone angle in the robot, value probably between 0.3 and 0.4 when the robot is inserted in the robot, higher values correct greater angles between phone and ground. During the test, the phone was taped parallel to the ground, so the value of this scaler was zero. 
	cv::Point2f  pts1[4];//setting up vector for warping, scaling and turning the picture
	cv::Point2f  pts2[4]; 
	pts1[0]=cv::Point2f((float)(0),(float)((1-ps)*img.rows));
	pts1[1]=cv::Point2f((float)(0),(float)(ps*img.rows));
	pts1[2]=cv::Point2f((float)(img.cols),(float)(img.rows));
	pts1[3]=cv::Point2f((float)(img.cols),(float)(0));
	cv::Mat out=cv::Mat::zeros(scale*img.cols,scale*img.rows, img.type());//define new matrix for warped picture
	pts2[0]=cv::Point2f((float)(0),(float)(0));
	pts2[1]=cv::Point2f((float)(out.cols),(float)(0));
	pts2[2]=cv::Point2f((float)(0),(float)(out.rows));
	pts2[3]=cv::Point2f((float)(out.cols),(float)(out.rows));
	cv::Mat pers_mat = getPerspectiveTransform(pts1,pts2);//generate the matrix to deal with these vectors
	cv::warpPerspective(img, out, pers_mat, out.size());//apply to the image and make new image out, now the picture is in the right allignment and is parallel 
 
	cv::Mat out_gray;//define matrix for a gray-scale picture
	int gray_threshold= 100;//minimum grey value of line to be detected; 0 is black 255 white
	cv::cvtColor(out, out_gray, cv::COLOR_BGR2GRAY);//convert to gray
	
	cv::threshold(out_gray, out_gray, gray_threshold,255,0);//when pixels are whiter (higher value) than gray_threshold, these pixels are filtered out. Other pixels, that is the line (and some noise), will be turned black.
	cv::imshow("image",out_gray);//show this picture
	cv::resize(out_gray, out_gray,cv::Size(3,1) ,0,0, cv::INTER_LINEAR);//resize the picture down to 1 by 3 pixels, while interpolating. 
	uchar left_pixel =out_gray.at<uchar>(0,0);//get the pixel value (black value) of the left pixel
	uchar middle_pixel =out_gray.at<uchar>(0,1);
	uchar right_pixel = out_gray.at<uchar>(0,2);    
	if(middle_pixel > right_pixel && middle_pixel > left_pixel)
	{
		twistmsg.linear.x=1;
		ROS_INFO("go straight");		
	}
	if(left_pixel < right_pixel)//left pixel is black
	{
		
		twistmsg.linear.x=1;
		twistmsg.angular.z=1;
		ROS_INFO("go left");
	}
	if(left_pixel > right_pixel) //recht pixel is black
	{
		twistmsg.linear.x=1;
		twistmsg.angular.z=-1;
		ROS_INFO("go right");
	} 	
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "linefollower_25");//initialise
	ros::NodeHandle n; //nodehandle
	ros::Publisher chatter_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10); //publish geometry_msgs::Twist on topic /cmd_vel with buffer of 10 messages
	ros::Rate loop_rate(10);//the loop rate at which the program loops
	
	image_transport::ImageTransport it(n);//image transports own nodehandle 
	image_transport::Subscriber sub = it.subscribe("/camera/image",1, imageCallback);//setting subscriber for image transport
	
	cv::namedWindow("image");//set up window to show picture
	cv::startWindowThread();//start window
	
	while (ros::ok())
	{
		chatter_pub.publish(twistmsg);//publish the Twist message
		ros::spinOnce();
		loop_rate.sleep();    
  	}
	cv::destroyWindow("image"); //when ros is not ok, (ctrl-c) the window closes
return 0;
}

