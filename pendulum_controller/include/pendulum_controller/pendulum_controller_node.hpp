// Copyright 2019
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PENDULUM__CONTROLLER_NODE_HPP_
#define PENDULUM__CONTROLLER_NODE_HPP_

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/publisher.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "rcutils/logging_macros.h"

#include "lifecycle_msgs/msg/transition_event.hpp"
#include "lifecycle_msgs/msg/transition.hpp"

#include "pendulum_msgs/msg/joint_command.hpp"
#include "pendulum_msgs/msg/joint_state.hpp"
#include <pendulum_msgs/msg/rttest_results.hpp>

#include "pendulum_controller.hpp"
#include "pendulum_controller/visibility_control.h"

using namespace std::chrono_literals;

namespace pendulum
{

class PendulumControllerNode : public rclcpp_lifecycle::LifecycleNode
{
public:
    COMPOSITION_PUBLIC
    explicit PendulumControllerNode(const rclcpp::NodeOptions & options)
    : rclcpp_lifecycle::LifecycleNode("Controller", options)
    { }

    COMPOSITION_PUBLIC
    explicit PendulumControllerNode(const std::string & node_name,
            std::unique_ptr<PendulumController> controller,
            std::chrono::nanoseconds update_period,
            const rclcpp::NodeOptions & options);
    void on_sensor_message(const pendulum_msgs::msg::JointState::SharedPtr msg);
    void on_pendulum_setpoint(
      const pendulum_msgs::msg::JointCommand::SharedPtr msg);

    void notification_callback(
      const lifecycle_msgs::msg::TransitionEvent::SharedPtr msg);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
      on_configure(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
      on_activate(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
      on_deactivate(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
      on_cleanup(const rclcpp_lifecycle::State &);
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn
      on_shutdown(const rclcpp_lifecycle::State & state);

    void control_timer_callback();

private:
    std::shared_ptr<rclcpp::Subscription<
      pendulum_msgs::msg::JointState>> sub_sensor_;
    std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<
      pendulum_msgs::msg::JointCommand>> command_pub_;
    std::shared_ptr<rclcpp::Subscription<
      pendulum_msgs::msg::JointCommand>> setpoint_sub_;
    std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<
      pendulum_msgs::msg::RttestResults>> logger_pub_;
    std::shared_ptr<rclcpp::Subscription<
      lifecycle_msgs::msg::TransitionEvent>> sub_notification_;

    rclcpp::TimerBase::SharedPtr timer_;
    pendulum_msgs::msg::JointCommand command_message_;
    std::chrono::nanoseconds update_period_ = 1000000ns;
    std::unique_ptr<PendulumController> controller_;
};

}  // namespace pendulum

#endif  // PENDULUM__CONTROLLER_NODE_HPP_
