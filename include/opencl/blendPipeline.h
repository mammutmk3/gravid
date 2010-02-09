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

    // event to track the copy process to the device
    cl_event copyToDevice_event;
    // event to track the copy process from the device
    cl_event copyFromDevice_event;
  public:
    BlendPipeline(cl_context ctx, cl_command_queue cmdQ, size_t width, size_t height);
    ~BlendPipeline();

    /**
    * returns the address of the area where the decoder can write the frame of the first video
    */
    RGBA* getFirstImage_forDecoder(){return this->inputImage_first;}

    /**
    * returns the address of the area where the decoder can write the frame of the second video
    */
    RGBA* getSecondImage_forDecoder(){return this->inputImage_second;}

    /**
    * returns the address of the output image memory area
    */
    RGBA* getImage_forEncoder(){return this->outputImage;}

    /**
    * returns the 3D image object on the device to be used as a parameter in kernel launches
    */
    cl_mem get3DInputImage(){return this->dev3D_in;}

    /**
    * returns the 2D image object on the device to be used as a parameter in kernel launches
    */
    cl_mem get2DOutputImage(){return this->dev2D_out;}

    /**
    * copies the host input areas to the 3D image on the device
    *
    * @param waitFor the event that has to finish before the copy process can be started
    *
    * @return an event to keep track of the copy process
    */
    cl_event copyToDevice(cl_event waitFor);

    /**
    * copies the result image back from the device to the host
    *
    * @param waitFor the event that has to finish before the copy process can be started
    *
    * @return an event to keep track of the copy process
    */
    cl_event copyFromDevice(cl_event waitFor);
  };
}

#endif /* BLENDPIPELINE_H_ */