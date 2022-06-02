#include <ros/ros.h>
#include <serial/serial.h>
#include <sensor_msgs/Range.h>
#include <iostream>
#include <cmath>
using namespace std;

int main(int argc, char** argv){
    ros::init(argc, argv, "Trisonica");
    ros::NodeHandle nh;

    ros::Publisher wind_pub = nh.advertise<sensor_msgs::Range>("trisonica_data", 10);
    sensor_msgs::Range wind_msg;


    serial::Serial sp;
    serial::Timeout to = serial::Timeout::simpleTimeout(100);
    sp.setPort("/dev/ttyUSB_ANEMOMENT");
    sp.setBaudrate(115200);
    sp.setTimeout(to);

    try{
        sp.open();
    }
    catch(serial::IOException& e){
        ROS_ERROR_STREAM("Unable to open port.");
        return -1;
    }
    if(sp.isOpen()){
        ROS_INFO_STREAM("/dev/ttyUSB_ANEMOMENT is opened.");
    }
    else{
        return -1;
    }

    bool s_get = 0;
    int s_exp = 1;
    float speed = 0.0;

    bool d_get = 0;
    int d_exp = 2;
    float direction = 0;

    ros::Rate loop_rate(10);
    while(ros::ok()){
        size_t n = sp.available();
        if(n != 0){
            uint8_t buffer[1024];
            n = sp.read(buffer, n);
            for(int i=0; i<n; i++){
                //-------------------------- Wind speed ------------------------
                if(buffer[i] == 'S')
                    s_get = 1;
                else if(s_get==1 && buffer[i] >= '0' && buffer[i] <= '9'){
                    speed += (buffer[i]-'0')*pow(10,s_exp);
                    s_exp -= 1;
                    if(s_exp < -2){ // Reset all speed data reception
                        s_exp = 1;
                        s_get = 0;
                        wind_msg.range = speed;
                        speed = 0;
                    }
                }

                //------------------------ Wind direction -----------------------
                if(i > 0){
                    if(buffer[i] == 'D' && buffer[i-1] == ' ')
                        d_get = 1;
                }
                if(d_get==1 && buffer[i] >= '0' && buffer[i] <= '9'){
                    direction += (buffer[i]-'0')*pow(10,d_exp);
                    d_exp -= 1;
                    if(d_exp < 0){ // Reset all direction data reception
                        d_exp = 2;
                        d_get = 0;
                        wind_msg.field_of_view = direction;
                        direction = 0;
                        break;
                    }
                }
            }
            wind_msg.header.stamp = ros::Time::now();
            wind_pub.publish(wind_msg);
        }
        loop_rate.sleep();
    }
    sp.close();

    return 0;
}
