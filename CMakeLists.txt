cmake_minimum_required(VERSION 2.8.3)
project(ros_clams)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS rosconsole)

## System dependencies are found with CMake's conventions
#find_package(Boost REQUIRED COMPONENTS system program_options filesystem thread)
find_package(Boost REQUIRED)
find_package(PCL REQUIRED) # containing eigen3
find_package(OpenCV REQUIRED)
find_package(VTK REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
link_directories(${Boost_LIBRARY_DIR})

## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

## To declare and build messages, services or actions from within this
## package, follow these steps:
## * Let MSG_DEP_SET be the set of packages whose message types you use in
##   your messages/services/actions (e.g. std_msgs, actionlib_msgs, ...).
## * In the file package.xml:
##   * add a build_depend tag for "message_generation"
##   * add a build_depend and a run_depend tag for each package in MSG_DEP_SET
##   * If MSG_DEP_SET isn't empty the following dependency has been pulled in
##     but can be declared for certainty nonetheless:
##     * add a run_depend tag for "message_runtime"
## * In this file (CMakeLists.txt):
##   * add "message_generation" and every package in MSG_DEP_SET to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * add "message_runtime" and every package in MSG_DEP_SET to
##     catkin_package(CATKIN_DEPENDS ...)
##   * uncomment the add_*_files sections below as needed
##     and list every .msg/.srv/.action file to be processed
##   * uncomment the generate_messages entry below
##   * add every package in MSG_DEP_SET to generate_messages(DEPENDENCIES ...)

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   std_msgs  # Or other packages containing msgs
# )

################################################
## Declare ROS dynamic reconfigure parameters ##
################################################

## To declare and build dynamic reconfigure parameters within this
## package, follow these steps:
## * In the file package.xml:
##   * add a build_depend and a run_depend tag for "dynamic_reconfigure"
## * In this file (CMakeLists.txt):
##   * add "dynamic_reconfigure" to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * uncomment the "generate_dynamic_reconfigure_options" section below
##     and list every .cfg file to be processed

## Generate dynamic reconfigure parameters in the 'cfg' folder
# generate_dynamic_reconfigure_options(
#   cfg/DynReconf1.cfg
#   cfg/DynReconf2.cfg
# )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ros_clams
  CATKIN_DEPENDS rosconsole
  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${Eigen_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
)

## Declare a C++ library
# from external to utility to core

# external/gzstream
add_library(gzstream src/external/gzstream/gzstream.cpp)
target_link_libraries(gzstream z)

add_executable(test_gzip src/external/gzstream/test_gzip.cpp)
target_link_libraries(test_gzip gzstream)

add_executable(test_gunzip src/external/gzstream/test_gunzip.cpp)
target_link_libraries(test_gunzip gzstream)

# utility/bag_of_tricks
add_library(bag_of_tricks
  src/utility/bag_of_tricks/high_res_timer.cpp
  src/utility/bag_of_tricks/lockable.cpp
)

# utility/eigen_extensions
add_library(eigen_extensions src/utility/eigen_extensions/random.cpp)
target_link_libraries(eigen_extensions
  ${Eigen3_LIBRARIES}
  gzstream
  timer
)

add_executable(cat src/utility/eigen_extensions/cat.cpp)
target_link_libraries(cat
  ${Boost_LIBRARIES}
  ${Eigen3_LIBRARIES}
  gzstream
)

add_executable(convert src/utility/eigen_extensions/convert.cpp)
target_link_libraries(convert
  ${Boost_LIBRARIES}
  ${Eigen3_LIBRARIES}
  gzstream
)

# utility/serializable
add_library(serializable src/utility/serializable/lib/serializable.cpp)

# utility/timer
add_library(timer src/utility/timer/lib/timer.cpp)
target_link_libraries(timer rt)

# core/stream_sequence
add_library(stream_sequence
  src/core/stream_sequence/lib/stream_sequence_base.cpp
  src/core/stream_sequence/lib/stream_sequence_pcl_wrapper.cpp
  src/core/stream_sequence/lib/stream_sequence.cpp
  src/core/stream_sequence/lib/frame_projector.cpp
)
target_link_libraries(stream_sequence
  ${catkin_LIBRARIES}
  ${Boost_LIBRARIES}
  ${PCL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  serializable
  eigen_extensions
)

#core/clams
add_library(clams
  src/core/clams/lib/discrete_depth_distortion_model.cpp
  src/core/clams/lib/discrete_depth_distortion_model_helpers.cpp
  src/core/clams/lib/slam_calibrator.cpp
  src/core/clams/lib/trajectory_visualizer.cpp
  src/core/clams/lib/trajectory.cpp
)
target_link_libraries(clams
  ${catkin_LIBRARIES}
  ${VTK_LIBRARIES}
  ${PCL_LIBRARIES}
  ${OpenCV_LIBRARIES}
  stream_sequence
  bag_of_tricks
)

add_executable(inspect src/core/clams/prog/inspect.cpp)
target_link_libraries(inspect ${Boost_LIBRARIES} clams boost_program_options) # boost not fully configured by cmake

add_executable(calibrate src/core/clams/prog/calibrate.cpp)
target_link_libraries(calibrate ${Boost_LIBRARIES} clams boost_program_options)

add_executable(visualize_model src/core/clams/prog/visualize_model.cpp)
target_link_libraries(visualize_model ${Boost_LIBRARIES} clams boost_program_options)

add_executable(convert_trajectory src/core/clams/prog/convert_trajectory.cpp)
target_link_libraries(convert_trajectory ${Boost_LIBRARIES} clams boost_program_options)

add_executable(visualize_trajectory src/core/clams/prog/visualize_trajectory.cpp)
target_link_libraries(visualize_trajectory ${Boost_LIBRARIES} clams boost_program_options)

add_executable(generate_map src/core/clams/prog/generate_map.cpp)
target_link_libraries(generate_map ${Boost_LIBRARIES} clams boost_program_options)

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
# exe
install(TARGETS test_gzip test_gunzip cat convert inspect calibrate visualize_model convert_trajectory visualize_trajectory generate_map
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}/${PROJECT_NAME}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}/${PROJECT_NAME}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}/${PROJECT_NAME}
)
# lib
install(TARGETS gzstream bag_of_tricks eigen_extensions serializable timer stream_sequence clams
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}/${PROJECT_NAME}
)

## Mark cpp header files for installation
install(FILES
  scripts/slam
  scripts/visualize_trajectories
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_ros_clams.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
