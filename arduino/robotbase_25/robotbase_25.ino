/** 
 * Group number: 25 
 * Student 1: 
 * Thijs Boumans, 4214854
 * Student 2: 
 * Stephan Neevel, 4232623
 */

/*
 This code lets the robot listen to the ROS Topic cmd_vel, and responds to the direction given on it. 
 Also, the robot interrupts its driving when the ultrasonic sensor detects something in front of the robot.
 A timeout is implemented when the code doesn't receive any new messsages for a certain time
 */

#include <ros.h>
#include <ros/time.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Twist.h>
#include "robotbase_25.h"
#define sensNumber 1
#define trigPin 23
#define echoPin 22
#include <string.h>

#define REV1 7
#define EN1 24
#define FWD1 6 
#define REV2 3 
#define EN2 25 
#define FWD2 2 
#define fuel 1
#define basepower 55
/*
  receive a message how we are driving 
 */
void driveCallback(const geometry_msgs::Twist &msg){
  resettimer();//reset timer as we are still connected to the laptop 

  float x = msg.linear.x;
  float z = msg.angular.z;
  {
    leftTrack=x-z;  //power settings
    rightTrack=x+z;
  }

  return; 
}
void drive(){

  if(fuel){//we have given "fuel" to the robot (easy way of switching tracks on and off)
    if(rightTrack>0.1){
      digitalWrite(REV2,LOW);
      analogWrite(FWD2,power*rightTrack);
      digitalWrite(EN2,HIGH);

    }
    if(leftTrack>0.1){
      digitalWrite(REV1,LOW);
      analogWrite(FWD1,power*leftTrack);
      digitalWrite(EN1,HIGH);
    }
    if(rightTrack<-0.1){
      digitalWrite(FWD2,LOW);
      analogWrite(REV2,power*rightTrack);
      digitalWrite(EN2,HIGH);
    }
    if(leftTrack<-0.1){
      digitalWrite(FWD1,LOW);
      analogWrite(REV1,power*leftTrack);
      digitalWrite(EN1,HIGH);
    }
    if(rightTrack>-0.1&&rightTrack<0.1){
      digitalWrite(EN2,LOW);
    }

    if(leftTrack>-0.1&&leftTrack<0.1){
      digitalWrite(EN1,LOW);
    }
  }
}
int timesteps =0;
/*
 timer compare interrupt service routine, triggers are regular intervals 
*/
ISR(TIMER1_OVF_vect) {       
  timesteps ++;
  if(timesteps>2){
    leftTrack=0;//disable engine in next loop 
    rightTrack=0;
  }
}
/*
  resets interupts be wary of shared data with ISR(TIMER1_OVF_vect) for timesteps
*/
void resettimer(){  
  noInterrupts();  
  timesteps =0;
  interrupts();
}
void setupTimeoutInterrupt(){
  noInterrupts();//every 2 timesteps 
  TIMSK1=0x01; // enabled global and timer overflow interrupt;
  TCCR1A = 0x00; // normal operation page 148 (mode0);
  TCNT1=0x0000; // 16bit counter register
  TCCR1B = 0x04; // start timer/ set clock
  interrupts();
}
void setup(){
//setup the timeout interupt 
setupTimeoutInterrupt();


//pinmodes 
  pinMode(FWD1 ,OUTPUT);
  pinMode(FWD2 ,OUTPUT);
  pinMode(EN1 ,OUTPUT);
  pinMode(EN2 ,OUTPUT);
  pinMode(REV1 ,OUTPUT);
  pinMode(REV2 ,OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
//setup ros 
  nh.initNode();
  nh.subscribe(sub);
  range_msg.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg.header.frame_id =  frameid;
  range_msg.field_of_view = 0.1;  
  range_msg.min_range = 0.0;
  range_msg.max_range = 6.47;
//turn on tracks so we can see that the robot is active 
  rightTrack=1;
  leftTrack=1;

}


void loop()
{
  long range_time;
//check distance sensor 
  long duration, distance;
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  distance = pulseIn(echoPin, HIGH,100000)/ 58.2; //change timeout value, increase for more range decrease for higher sample rate 

  range_msg.range = distance;
  range_msg.header.stamp = nh.now();
  range_time =  millis() + 50;
  range_msg.field_of_view = trigPin; 
  range_msg.min_range = echoPin;
//adjust power to distance (slow down if close to object)
  power=basepower;
  if(range_msg.range>90){
    power=(1-((range_msg.range-30)/60))*basepower;
  }

  if(range_msg.range<30){
    power=0;
  }

  drive();

  range_msg.field_of_view=rightTrack;
  range_msg.min_range=leftTrack;
  nh.spinOnce();
}

