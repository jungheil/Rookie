cmake_minimum_required(VERSION 3.15)
project(Rookie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_PREFIX_PATH ~/install/pytorch/torch/)
find_package(Torch REQUIRED)
find_package(OpenCV 4 REQUIRED)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)
add_executable(Rookie ${SRCS})

target_link_libraries(Rookie
        -pthread
        ${OpenCV_LIBS}
        ${TORCH_LIBRARIES}
        realsense2
        )