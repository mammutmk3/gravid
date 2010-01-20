#cmake -DCUDA_SDK_ROOT_DIR:PATH=/home/kama06/NVIDIA_GPU_Computing_SDK/ .

cmake_minimum_required(VERSION 2.8.0)

# for finding local cmake scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIR})

find_package(FFMPEG REQUIRED)
include_directories(${FFMPEG_INCLUDE_DIR})

message("ffmpeg include path=${FFMPEG_INCLUDE_DIR}")
#find_package(CUDA REQUIRED)

#find_package(CUDASDK REQUIRED)
#include_directories(${CUDA_CUT_INCLUDE_DIR})

# setting default target for building
if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
message("Building for CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

message("libs:=${FFMPEG_LIBRARIES}")
# common flags
SET(CMAKE_CXX_FLAGS "-Wall -Wno-unknown-pragmas -Wno-long-long -Wno-variadic-macros")
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -DGOOGLE_STRIP_LOG=1 -O2 -g0")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-inline")

include_directories(src)
include_directories(include)
#include_directories(/home/kama06/include/)
#include_directories(/home/kama06/include/libavcodec)
#include_directories(/home/kama06/include/libavformat)
#include_directories(/home/kama06/include/libavswscale)
#include_directories(/home/kama06/include/libavutil)

#link_directories(/home/kama06/include)

#find_library(LIBAVCODEC "/home/kama06/include/libavcodec/avcodec.h")

# build a library
#add_subdirectory(src/codec)
#add_subdirectory(src/cpu_effects)
#add_subdirectory(src/opencl)
#add_subdirectory(src/visual)
#add_subdirectory(include)
#add_subdirectory(/home/kama06/include)
add_library(gravid-h SHARED src/codec/videoReader.cpp src/codec/videoWriter.cpp src/cpu_effects/edgeDetection.cpp src/cpu_effects/colorFilter.cpp src/opencl/openCLProgram.cpp src/visual/writePPM.cpp)

add_executable(gravid src/main.cpp)
target_link_libraries(gravid gravid-h  ${OPENCL_LIBRARIES} ${FFMPEG_LIBRARIES})
