#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/exceptions.h"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include <tf2/LinearMath/Quaternion.h>
using namespace std::chrono_literals;

class GoAndCheck : public rclcpp::Node
{
public:
  GoAndCheck() : Node("dshape_go_rotate")
  {
    direction_ = this->declare_parameter<std::string>("direction", "right");
    
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/diffbot_base_controller/cmd_vel_unstamped", 20);
    
    timer_ = this->create_wall_timer(50ms, std::bind(&GoAndCheck::on_timer, this));
  }

private:
  void on_timer()
  {
    std::string fromFrameRel = "odom";
    std::string toFrameRel = "base_link";
    std::string rotate = direction_.c_str();

    geometry_msgs::msg::TransformStamped t;
    
        try 
        {
          t = tf_buffer_->lookupTransform(toFrameRel, fromFrameRel, tf2::TimePointZero);
        } 
        catch (const tf2::TransformException & ex) 
        {
          RCLCPP_INFO(this->get_logger(), "ROM ROBOTICS --> Could not get transform : %s", ex.what());
          return;
        }

        tf2::Quaternion q(t.transform.rotation.x,t.transform.rotation.y,t.transform.rotation.z,t.transform.rotation.w);
        //q.x = t.transform.rotation.x;

        tf2::Matrix3x3 m(q);
        double roll, pitch, yaw;
        m.getRPY(roll, pitch, yaw);

        geometry_msgs::msg::Twist msg;

        msg.linear.x = 0.00;
        msg.angular.z = 0.00;

        if(rotate == "right")
        {
          if( std::abs(yaw) < 3.135 ) // may be error because 2pi = - 2pi
          { 
            msg.angular.z = -0.08;
            publisher_->publish(msg); 
            RCLCPP_INFO(this->get_logger(), "rotate right: , %3f",std::abs(yaw));
            //RCLCPP_INFO(this->get_logger(), "rotate right: , %3f",std::abs(-3.543));
          }
          else 
          {
            msg.angular.z = 0.00;
            publisher_->publish(msg); 
            rclcpp::shutdown();
          }
        } 
        
        else if ( rotate == "left")
        {
          if( yaw > -3.14 ) 
          { 
            msg.angular.z = 0.08;
            publisher_->publish(msg); 
            RCLCPP_INFO(this->get_logger(), "rotate left: , %3f", yaw);
          }
          else 
          {
            msg.angular.z = 0.00;
            publisher_->publish(msg); 
            rclcpp::shutdown();
          }
        } 

        
  }    
  rclcpp::TimerBase::SharedPtr timer_{nullptr};
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_{nullptr};
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_{nullptr};
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::string direction_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<GoAndCheck>());
  rclcpp::shutdown();
  return 0;
}
