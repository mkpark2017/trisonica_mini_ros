#include <ros/ros.h>
#include <serial/serial.h>
#include <iostream>

int main(int argc, char** argv)
{
    ros::init(argc, argv, "Trisonica");
    ros::NodeHandle nh;

    serial::Serial sp;
    serial::Timeout to = serial::Timeout::simpleTimeout(100);

    sp.setPort("/dev/ttyUSB_ANEMOMENT");
    sp.setBaudrate(115200);
    sp.setTimeout(to);

    try
    {
        sp.open();
    }
    catch(serial::IOException& e)
    {
        ROS_ERROR_STREAM("Unable to open port.");
        return -1;
    }

    if(sp.isOpen())
    {
        ROS_INFO_STREAM("/dev/ttyUSB_ANEMOMENT is opened.");
    }
    else
    {
        return -1;
    }

    ros::Rate loop_rate(10);
    while(ros::ok())
    {
        size_t n = sp.available();
        if(n != 0)
        {
            uint8_t buffer[1024];
            n = sp.read(buffer, n);
            for(int i=0; i<n; i++)
            {
                std::cout << std::hex << (buffer[i]);
            }
            std::cout << std::endl;
            //sp.write(buffer, n)
        }
        loop_rate.sleep();
    }
    sp.close();

    return 0;
}
