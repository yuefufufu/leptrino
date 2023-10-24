cmake_minimum_required(VERSION 3.0.2)
project(leptrino)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  sensor_msgs
  std_msgs
  geometry_msgs
)

catkin_package()

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)


add_executable(axissensor src/6axissensor.cpp)
target_link_libraries(axissensor ${catkin_LIBRARIES})
add_executable(hello src/hello.cpp)
target_link_libraries(hello ${catkin_LIBRARIES})
