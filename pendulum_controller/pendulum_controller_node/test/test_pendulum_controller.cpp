// Copyright 2019 Carlos San Vicente
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

#include <iostream>
#include <memory>
#include <utility>

#include "rttest/rttest.h"
#include "rttest/utils.h"

#include "pendulum_controller_node/pendulum_controller_node.hpp"
#include "pendulum_controller/pendulum_controller.hpp"
#include "pendulum_controller/pid_controller.hpp"

int main(int argc, char * argv[])
{
  // use a dummy period to initialize rttest
  struct timespec dummy_period;
  dummy_period.tv_sec = 0;
  dummy_period.tv_nsec = 1000000;
  rttest_init(1, dummy_period, SCHED_FIFO, 80, 0, NULL);

  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor exec;

  pendulum::PIDProperties pid;
  pid.p = 1.5;
  pid.i = 0.0;

  std::chrono::milliseconds deadline_duration(10);
  rclcpp::QoS qos_deadline_profile(10);
  qos_deadline_profile.deadline(deadline_duration);

  std::chrono::nanoseconds update_period = std::chrono::nanoseconds(970000);
  std::unique_ptr<pendulum::PendulumController> pid_controller =
    std::make_unique<pendulum::PIDController>(update_period, pid);
  auto controller_node = std::make_shared<pendulum::PendulumControllerNode>(
    "pendulum_controller",
    std::move(pid_controller),
    update_period,
    qos_deadline_profile,
    rclcpp::QoS(1),
    false,
    rclcpp::NodeOptions().use_intra_process_comms(true));

  exec.add_node(controller_node->get_node_base_interface());

  // Set the priority of this thread to the maximum safe value, and set its scheduling policy to a
  // deterministic (real-time safe) algorithm, round robin.
  if (rttest_set_sched_priority(80, SCHED_FIFO)) {
    perror("Couldn't set scheduling priority and policy");
  }

  // Lock the currently cached virtual memory into RAM, as well as any future memory allocations,
  // and do our best to prefault the locked memory to prevent future pagefaults.
  // Will return with a non-zero error code if something went wrong (insufficient resources or
  // permissions).
  // Always do this as the last step of the initialization phase.
  // See README.md for instructions on setting permissions.
  // See rttest/rttest.cpp for more details.
  if (rttest_lock_and_prefault_dynamic() != 0) {
    fprintf(stderr, "Couldn't lock all cached virtual memory.\n");
    fprintf(stderr, "Pagefaults from reading pages not yet mapped into RAM will be recorded.\n");
  }

  exec.spin();
  rclcpp::shutdown();
  return EXIT_SUCCESS;
}
