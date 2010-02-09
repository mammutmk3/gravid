/*
 * blendPipeline.h
 *
 *  Created on: Feb 9, 2010
 *      Author: lars
 */

#ifndef BLENDPIPELINE_H_
#define BLENDPIPELINE_H_

#include "common_gravid.h"
#include "types.h"

#include "CL/cl.h"

namespace GRAVID{
  class BlendPipeline : public Common{
  private:
    // for creating the buffers
    cl_context ctx;
    // for executing the reads/writes
    cl_command_queue cmdQ;

    // height and width of the videos; restrictions made here: both videos have to be of the same dimension
    size_t width, height;

    /**
    * on the host side
    */
    // input image
    cl_mem host3D_in;
    // output image
    cl_mem host2D_out;

    /**
    * on the device
    */
    // input image
    cl_mem dev3D_in;
    cl_mem dev2D_out;

    /**
    * mapped pointers for writing and reading on the host side
    */
    // the area for the first of the two input videos
    RGBA* inputImage_first;
    // the area for the second of the two input videos
    RGBA* inputImage_second;
    // the area where the output frame can be read
    RGBA* outputImage;
  };
}

#endif /* BLENDPIPELINE_H_ */