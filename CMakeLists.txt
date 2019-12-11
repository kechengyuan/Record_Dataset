cmake_minimum_required(VERSION 2.8)
project(Kinect_test)

MESSAGE(${PROJECT_SOURCE_DIR})
SET(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)

include_directories(
    "C:\\Program Files\\Microsoft SDKs\\Kinect\\v2.0_1409\\inc"
    "C:\\Program Files (x86)\\Pangolin\\include"
    "C:\\Users\\ke\\Documents\\glew-2.1.0\\include"
    "C:\\Users\\ke\\Documents\\opencv\\include"
    "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\include"
)
link_directories(
    "C:\\Program Files\\Microsoft SDKs\\Kinect\\v2.0_1409\\Lib\\x64"
    "C:\\Program Files (x86)\\Pangolin\\lib"
    "C:\\Users\\ke\\Documents\\glew-2.1.0\\lib\\Release\\x64"
    "C:\\Program Files (x86)\\Microsoft SDKs\\Windows\\v7.1A\\Lib\\x64"
    "C:\\Users\\ke\\Documents\\opencv\\x64\\mingw\\lib"
    "C:\\Program Files (x86)\\Intel RealSense SDK 2.0\\lib\\x64"
)

set(Opencv_LIB
    opencv_calib3d331.dll
    opencv_core331.dll
    opencv_dnn331.dll
    opencv_features2d331.dll
    opencv_flann331.dll
    opencv_highgui331.dll
    opencv_imgcodecs331.dll
    opencv_imgproc331.dll
    opencv_ml331.dll
    opencv_objdetect331.dll
    opencv_photo331.dll
    opencv_shape331.dll
    opencv_stitching331.dll
    opencv_superres331.dll
    opencv_video331.dll
    opencv_videoio331.dll
    opencv_videostab331.dll
)

add_executable(recordDataset main.cpp Settings.cpp Viewer.cpp Sensor.cpp)
add_executable(checkSkeleton pangolin_main.cpp)

target_link_libraries(recordDataset 
    Kinect20
    ${Opencv_LIB}
    pangolin
    glew32
    OpenGL32
    realsense2
)

target_link_libraries(checkSkeleton
    pangolin
    glew32
    OpenGL32
    ${Opencv_LIB}
)