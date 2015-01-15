#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <image_transport/image_transport.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <iostream>
using namespace std;
//	make sure to have the compressed transport installed by checking rosrun image_transport list_transports
//	if not, install it with sudo apt-get install ros-indigo-compressed-image-transport
//	(via rqt> plugins>visualization>image view and selectin the topic /camera/image/compressed, the pictures will be viewed)

//	look up ip via /sbin/ifconfig | less (inet adrr:)
//	$ export ROS_IP=<inetaddr>
//	$ export ROS_MASTER_URI=http://$ROS_IP:11311/	 
//	run roscore
//	set URI on phone to http://<inetaddr>:11311/ and press connect
//	new terminal $source devel/setup.bash
//	RUN with $rosrun line_detection linefollower_25 _image_transport:=compressed

geometry_msgs::Twist twistmsg;//



void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{	

	cv::Mat img = cv_bridge::toCvCopy(msg, "bgr8")->image;// picture now stored in img
	
	float scale = 0.1;//scale the picture
	float ps = 0;//erspective scaler TWEAK THIS TO MATCH ROBOT SETUP, value probably between 0.3 and 0.4, higher values correct greater angles between phone and ground 0.35 
	
	cv::Point2f  pts1[4];
	cv::Point2f  pts2[4]; 
	//setting up vector for warping, scaling and turning
	pts1[0]=cv::Point2f((float)(0),(float)((1-ps)*img.rows));
	pts1[1]=cv::Point2f((float)(0),(float)(ps*img.rows));
	pts1[2]=cv::Point2f((float)(img.cols),(float)(img.rows));
	pts1[3]=cv::Point2f((float)(img.cols),(float)(0));

	cv::Mat out=cv::Mat::zeros(scale*img.cols,scale*img.rows, img.type());
	pts2[0]=cv::Point2f((float)(0),(float)(0));
	pts2[1]=cv::Point2f((float)(out.cols),(float)(0));
	pts2[2]=cv::Point2f((float)(0),(float)(out.rows));
	pts2[3]=cv::Point2f((float)(out.cols),(float)(out.rows));
	
	cv::Mat pers_mat = getPerspectiveTransform(pts1,pts2);//generate the matrix to deal with these vectors
	cv::warpPerspective(img, out, pers_mat, out.size());//apply to the image and make new image out

  	
	
	//now we have a picture in the right allignment and cartesion perspective
	
	 //reduce noise 
	cv::Mat blur;
	cv::Mat gray;
	double sigmaY=0;
	int borderType=1;
	GaussianBlur(out,blur,cv::Size(3,3),3,sigmaY,borderType );

	cv::Mat detected_edges, linewindow;
	//float scale2 = 0.1;
	//cv::resize(detected_edges, detected_edges, cv::Size(), scale2, scale2, cv::INTER_LINEAR);
	cv::Canny(blur, detected_edges, 50, 200,3);//edge detection canny
	/*int crop = 10;
	cv::Rect roi(crop,crop,detected_edges.cols-2*crop, detected_edges.rows-2*crop);//crop
	detected_edges = detected_edges(roi);//crop
	*/

printf("type %i\n", gray.type()); 

	
cv::imshow("pers",detected_edges);

	//::Mat mini=cv::Mat::zeros(scale*detected_edges.cols,scale*detected_edges.rows,detected_edges.type());
	float highest =0; 
	float best= 0;
	
	for(int i=0;i<detected_edges.rows;i++){
		
		float loc = detected_edges.at<uchar>(i,detected_edges.cols-4);
		if(loc>highest){
			best=i;
			highest=loc;
		}
	}
	printf("best: %f",highest);
	if((float)best/(float)detected_edges.cols>0.7){
		twistmsg.linear.x=0;
		twistmsg.angular.z=1;
		ROS_INFO("linksaf");
	}else{
		if((float)best/(float)detected_edges.cols>0.4){
		twistmsg.linear.x=1;
		twistmsg.angular.z=0;
		ROS_INFO("rechtdoor");
		
		}else{
		twistmsg.linear.x=0;
		twistmsg.angular.z=-1;
		ROS_INFO("rechtsaf");
	}
}
/*        cv::Rect r(4, 100, 64, 24);
 	cv::Mat cropped = detected_edges(r);
	cv::imshow("pers",detected_edges);
	cv::Mat result;
	cv::resize(cropped, result,cv::Size(3,1) ,0,0, cv::INTER_LINEAR);

	uchar left_pixel =result.at<uchar>(0,0);
	uchar middle_pixel =result.at<uchar>(0,1);
	uchar right_pixel = result.at<uchar>(0,2);    
	if(middle_pixel > right_pixel && middle_pixel > left_pixel)
	{
		//rij rechtdoor	
		twistmsg.linear.x=1;
		ROS_INFO("rechtdoor");		
	}
	

	if(left_pixel < right_pixel)//links is zwart
	{
		//rij naar links
		twistmsg.linear.x=1;
		twistmsg.angular.z=1;
		ROS_INFO("linksaf");
	}
	if(left_pixel > right_pixel) //recht is zwart
	{
		//rij naar rechts
		twistmsg.linear.x=1;
		twistmsg.angular.z=-1;
		ROS_INFO("rechtsaf");
	} 	
	*/

}

int main(int argc, char **argv)
{
	printf("here<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	ros::init(argc, argv, "linefollower_25");//initialise
	ros::NodeHandle n; //nodehandle
	ros::Publisher chatter_pub = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10); //publish geometry_msgs::Twist on topic /cmd_vel with buffer of 10 messages
	ros::Rate loop_rate(10);//the loop rate at which the program loops
	
	//setting sub for image transport
	image_transport::ImageTransport it(n);
	image_transport::Subscriber sub = it.subscribe("/camera/image",1, imageCallback);
	
	
	
	cv::namedWindow("pers");
	cv::startWindowThread();
	
	while (ros::ok())
	{
		
		//code for message	    	
		chatter_pub.publish(twistmsg);//finally publish the message
		ros::spinOnce();
		loop_rate.sleep();    
  	}
	
	cv::destroyWindow("pers");
  return 0;
}
