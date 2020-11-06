cmake_minimum_required(VERSION 3.10)
project(Rookie)

SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_PREFIX_PATH ~/install/pytorch/torch/)
find_package(Torch REQUIRED)
find_package(OpenCV 4 REQUIRED)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)
add_executable(Rookie ${SRCS})
add_library(imgshare SHARED src/imgshare.cpp include/imgshare.h)

target_link_libraries(Rookie
        -pthread
        ${OpenCV_LIBS}
        ${TORCH_LIBRARIES}
        realsense2
        )

target_link_libraries(imgshare
        ${OpenCV_LIBS}
        )
