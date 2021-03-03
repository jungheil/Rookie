cmake_minimum_required(VERSION 3.5)
project(Rookie)
SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin/")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_PREFIX_PATH ~/install/pytorch/torch/)
find_package(Torch REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(ZBAR REQUIRED zbar)

set(CMAKE_CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)
list(REMOVE_ITEM SRCS ${PROJECT_SOURCE_DIR}/src/pyshare.cpp)
message(${SRCS})
add_executable(Rookie ${SRCS} src/ImgFeature.cpp)
add_library(pyshare SHARED src/pyshare.cpp src/proshare.cpp include/ImgFeature.h)

target_link_libraries(Rookie
        -pthread
        ${OpenCV_LIBS}
        ${TORCH_LIBRARIES}
        realsense2
        ${ZBAR_LIBRARIES}
        )

target_link_libraries(pyshare
        ${OpenCV_LIBS}
        )
