/* 
 * rosserial Ultrasound Example
 * 
 * This example is for the Maxbotix Ultrasound rangers.
 */

#include <ros.h>
#include <ros/time.h>
#include <sensor_msgs/Range.h>
#include <geometry_msgs/Twist.h>
#include "HelloWorld.h"
#define sensNumber 1
#define trigPin 23
#define echoPin 22

#define REV1 7
#define EN1 24
#define FWD1 6 

#define REV2 3 
#define EN2 25 
#define FWD2 2 
#define fuel 1


void driveCallback(const geometry_msgs::Twist &msg){
  rightTrack=0;
  leftTrack=0;
  Serial.println("hello");
  if(msg.linear.x==0&&(msg.angular.z>0)){
      rightTrack=1;
      leftTrack=-1;
      Serial.println("1");
      return;
    }
    if(msg.linear.x==0&&(msg.angular.z<0)){
        rightTrack=-1;
        leftTrack=1;
        Serial.println("2");
        return;
    }
     if(msg.linear.x==0&&(msg.angular.z==0)){
      rightTrack=0;
      leftTrack=0;
      Serial.println("3");
      return;
      }
  
     if(msg.angular.z<0){
        rightTrack=-1;
        leftTrack=1;
     }
     if(msg.angular.z>0){
          rightTrack=1;
          leftTrack=-1;
     }
     if(msg.angular.z==0){
       rightTrack=1;
       leftTrack=1;
      }
     
  
  if(msg.linear.x<0){
    rightTrack=rightTrack*-1;
    leftTrack=leftTrack*-1;
  }
Serial.println("4");
  return; 
}
void drive(){
  if(fuel){

    
    if(rightTrack==1){
      digitalWrite(REV2,LOW);
      analogWrite(FWD2,power);
      digitalWrite(EN2,HIGH);

    }
    if(leftTrack==1){
      digitalWrite(REV1,LOW);
      analogWrite(FWD1,power);
      digitalWrite(EN1,HIGH);
    }
    if(rightTrack==-1){
       digitalWrite(FWD2,LOW);
      analogWrite(REV2,power);
      digitalWrite(EN2,HIGH);
    }
     if(leftTrack==-1){
        digitalWrite(FWD1,LOW);
        analogWrite(REV1,power);
        digitalWrite(EN1,HIGH);
    }
    if(rightTrack==0){
      digitalWrite(EN2,LOW);
    }
    if(leftTrack==0){
      digitalWrite(EN1,LOW);
    }
    
}
}

void setup(){
  pinMode(FWD1 ,OUTPUT);
  pinMode(FWD2 ,OUTPUT);
  pinMode(EN1 ,OUTPUT);
  pinMode(EN2 ,OUTPUT);
  pinMode(REV1 ,OUTPUT);
  pinMode(REV2 ,OUTPUT);

  nh.initNode();
  nh.subscribe(sub);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  nh.initNode();
  range_msg.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg.header.frame_id =  frameid;
  range_msg.field_of_view = 0.1;  
  range_msg.min_range = 0.0;
  range_msg.max_range = 6.47;

}



void loop()
{
  long range_time;
  //publish the adc value every 50 milliseconds
  //since it takes that long for the sensor to stablize
     long duration, distance;
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(1000); - Removed this line
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  distance = pulseIn(echoPin, HIGH,100000)/ 58.2; //change timeout value, increase for more range decrease for higher sample rate 
  
    range_msg.range = distance;
    range_msg.header.stamp = nh.now();
    range_time =  millis() + 50;
    range_msg.field_of_view = trigPin; 
    range_msg.min_range = echoPin;
 
    if(range_msg.range<20){
      rightTrack=-1;
      leftTrack=-1;
    }else{
      rightTrack=-1;
      leftTrack=-1;
    }
    drive();

   
    range_msg.field_of_view=rightTrack;
    range_msg.min_range=leftTrack;
  
  nh.spinOnce();
}
