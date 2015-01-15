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
{	ROS_INFO("yhea?");
/*	cv::Mat img = cv_bridge::toCvCopy(msg, "bgr8")->image;// picture now stored in img
	
	float scale = 0.2;//scale the picture
	float ps = 0.35;//perspective scaler TWEAK THIS TO MATCH ROBOT SETUP, value probably between 0.3 and 0.4, higher values correct greater angles between phone and ground 0.35 
	
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
	cvtColor(blur,gray,CV_BGR2GRAY);
	cv::normalize(gray, gray, 0, 255, 1, CV_8UC1);
	double tot=0;
	double alhas=0;
	for(int i=0;i<blur.rows-1;i++){
		for(int j=0;j<blur.cols-1;j++){
			float loc=gray.at<uchar>(j,i);
			float hor=gray.at<uchar>(j+1,i);
			float vert=gray.at<uchar>(j,i+1);
			float str= abs(loc)+abs(hor);
			float hD=hor-loc; //horizontal change 
			float vD=vert-loc;//vertical change
			 alhas=alhas+(atan(hD/vD))*str;
			tot=tot+str;
		}
	}
	float alpha=alhas/tot;
	printf("alpha= %f",alpha);
	
/*	cv::Mor at detected_edges, linewindow;

	cv::Canny(blur, detected_edges, 50, 200,3);//edge detection canny
	int crop = 5;
	cv::Rect roi(crop,crop,detected_edges.cols-2*crop, detected_edges.rows-2*crop);//crop
	detected_edges = detected_edges(roi);//crop
	
	
	cv::cvtColor(detected_edges, linewindow, CV_GRAY2BGR);//make window to display lines
	
	vector<cv::Vec4i> detected_lines;
	cv::HoughLinesP(detected_edges, detected_lines, 1, CV_PI/180, 30, 30, 30 );

	
	double longest_distance=0;
	cv::Vec4i longest_line;
	
	  for(int i = 0; i < detected_lines.size(); i++ ){
		
		cv::Vec4f l = detected_lines[i];
		cv::line(linewindow, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0,0,255), 3, CV_AA);		
		cout<<(l)<<"\n";

		double dist= (l[0]-l[2])*(l[0]-l[2])+(l[1]-l[3])*(l[1]-l[3]); 	
		if(dist>longest_distance){
			longest_distance=dist;
			longest_line=l;
		}
	  }
	cv::imshow("pers",linewindow);
	double theta;
	if(longest_line[1]-longest_line[3]==0){
		theta=1;
	}else{
		if(longest_line[1]>longest_line[3]){
			theta=(longest_line[0]-longest_line[2])/(longest_line[1]-longest_line[3]);
 	 	}else
			theta=(longest_line[2]-longest_line[0])/(longest_line[3]-longest_line[1]);
  	}
	if(theta>1){
		twistmsg.linear.x=1;
		twistmsg.angular.z=1;
		ROS_INFO("linksaf");
	}else{
	
		twistmsg.linear.x=1;
		twistmsg.angular.z=-1;
		ROS_INFO("rechtsaf");
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
