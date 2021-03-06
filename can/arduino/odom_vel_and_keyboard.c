#include <ros.h>
#include <geometry_msgs/Vector3.h>
#include <std_msgs/Float32.h>

#include <CAN.h>

typedef union { 
    float f; 
    struct{ 
        unsigned int bite0 : 8; 
        unsigned int bite1 : 8; 
        unsigned int bite2 : 8; 
        unsigned int bite3 : 8;   
    } raw; 
} myfloat;

void messageCb(const std_msgs::Float32& msg){
  myfloat current;
  myfloat rpm;

  current.f = 0.85;
  rpm.f = msg.data;
  CAN.beginPacket(0x501);
  CAN.write(rpm.raw.bite0);
  CAN.write(rpm.raw.bite1);
  CAN.write(rpm.raw.bite2);
  CAN.write(rpm.raw.bite3);

  
  CAN.write(current.raw.bite0);
  CAN.write(current.raw.bite1);
  CAN.write(current.raw.bite2);
  CAN.write(current.raw.bite3);
  
  CAN.endPacket();
}


ros::NodeHandle  nh;

geometry_msgs::Vector3 msg;

ros::Subscriber<std_msgs::Float32> sub("tester", &messageCb);

ros::Publisher chatter("raw_odom", &msg);

void setup(){
  Serial.begin(57600);
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(chatter);
  if (!CAN.begin(1000E3)) {
    while (1);
  }
}

void loop(){
  unsigned int collector[8];
  unsigned int index = 0;
  myfloat odom;
  
  if(CAN.parsePacket()){
    
    if(CAN.packetId()==0x403){//motor
      while (CAN.available()) {
            collector[index]=CAN.read();
            index++;
      }
      
      odom.raw.bite0 = collector[4];
      odom.raw.bite1 = collector[5];
      odom.raw.bite2 = collector[6];
      odom.raw.bite3 = collector[7];
      msg.x = odom.f;
    
    }/*
    if(CAN.packetId()==0x402){//steer
    
      while (CAN.available()) {
        f.raw.bite0 = CAN.read();
        f.raw.bite1 = CAN.read();
        f.raw.bite2 = CAN.read();
        f.raw.bite3 = CAN.read();
      }
      msg.z = f.f;
    }*/
    chatter.publish(&msg);
  }
     
  nh.spinOnce();
}