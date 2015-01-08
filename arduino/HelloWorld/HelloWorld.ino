
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
#include <string.h>

#define REV1 7
#define EN1 24
#define FWD1 6 

#define REV2 3 
#define EN2 25 
#define FWD2 2 
#define fuel 1

#define LED 13

#define basepower 100
void driveCallback(const geometry_msgs::Twist &msg){
  resettimer();
           digitalWrite(LED,LOW);

  float x = msg.linear.x;
  float z = msg.angular.z;
  if(x==0){
      if(z>0){
        rightTrack=1;
        leftTrack=-1;
        return;
      }

        if(z<0){
          rightTrack=-1;
          leftTrack=1;
          return;
      }
       if(z==0){
           digitalWrite(LED,HIGH);
            rightTrack=0;
            leftTrack=0;
            return;
        }
    }  
     if(z<0){
        if(x>0){
                rightTrack=0;
                leftTrack=1;
        }else{
                rightTrack=-1;
                leftTrack=0;
        }
     }
     if(z>0){
        if(x>0){
                rightTrack=1;
                leftTrack=0;
        }else{
                rightTrack=0;
                leftTrack=-1;
        }
     }
     
     if(z==0){
       if(x>0){
         rightTrack=1;
         leftTrack=1;
       }else{
         rightTrack=-1;
         leftTrack=-1;
       }
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
int timesteps =0;
ISR(TIMER1_OVF_vect) {        // timer compare interrupt service routine
timesteps ++;
  if(timesteps>16){
  leftTrack=0;
  rightTrack=0;
  }
}

void resettimer(){
  timesteps =0;
}
void setup(){
    noInterrupts();//every 2 timesteps 
TIMSK1=0x01; // enabled global and timer overflow interrupt;
TCCR1A = 0x00; // normal operation page 148 (mode0);
TCNT1=0x0000; // 16bit counter register
TCCR1B = 0x04; // start timer/ set clock
    interrupts();
    
  pinMode(FWD1 ,OUTPUT);
  pinMode(FWD2 ,OUTPUT);
  pinMode(EN1 ,OUTPUT);
  pinMode(EN2 ,OUTPUT);
  pinMode(REV1 ,OUTPUT);
  pinMode(REV2 ,OUTPUT);
  pinMode(LED,OUTPUT);
  nh.initNode();
  nh.subscribe(sub);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  range_msg.radiation_type = sensor_msgs::Range::ULTRASOUND;
  range_msg.header.frame_id =  frameid;
  range_msg.field_of_view = 0.1;  
  range_msg.min_range = 0.0;
  range_msg.max_range = 6.47;
  rightTrack=1;
  leftTrack=1;
    
}


void loop()
{
   Serial.println(timesteps);
  long range_time;
  //publish the adc value every 50 millitimesteps
  //since it takes that long for the sensor to stablize
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
   power=1+-1/(range_msg.range/30)*basepower;
   if(range_msg.range<30){
     power=0;
   }
    
    drive();

   
    range_msg.field_of_view=rightTrack;
    range_msg.min_range=leftTrack;
    nh.spinOnce();
}
