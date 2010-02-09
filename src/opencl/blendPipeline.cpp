/*
 * blendPipeline.cpp
 *
 *  Created on: Feb 09, 2010
 *      Author: lars
 */

#include "opencl/blendPipeline.h"

#include <CL/cl.h>

using namespace GRAVID;

BlendPipeline::BlendPipeline(cl_context ctx, cl_command_queue cmdQ, size_t width, size_t height){
  // initialize members
  this->ctx = ctx;
  this->cmdQ = cmdQ;
  this->width = width;
  this->height = height;

  this->host3D_in = NULL;
  this->host2D_out = NULL;
  this->dev3D_in = NULL;
  this->dev2D_out = NULL;

  this->inputImage_first = NULL;
  this->inputImage_second = NULL;
  this->outputImage = NULL;

  // specify the image format that the images a represented with
  cl_image_format imgFmt;
  imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
  imgFmt.image_channel_order = CL_RGBA;

  // create the 3D image on the host
  this->host3D_in = clCreateImage3D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt, this->width, this->height, 
				    2, 0,0, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't create 3D image on the host");
  // create the 3D image on the device
  this->host3D_in = clCreateImage3D(this->ctx, CL_MEM_READ_ONLY, &imgFmt, this->width, this->height, 
				    2, 0,0, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't create 3D image on the device");
  // create the 2D output image on the host
  this->host2D_out = clCreateImage2D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt, this->width, this->height, 
				     0, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't create 2D image on the host");
  // create the 2D output image on the device
  this->dev2D_out = clCreateImage2D(this->ctx, CL_MEM_WRITE_ONLY, &imgFmt, this->width, this->height, 
				     0, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't create 2D image on the device");

  size_t origin[3]={0,0,0}, region[3]={this->width,this->height,1};
  size_t dummy;
  // map the first slice of the 3D image to a pointer for a frame of the first input video
  this->inputImage_first = (RGBA*) clEnqueueMapImage(this->cmdQ, this->host3D_in, CL_TRUE, CL_MAP_WRITE,
						     origin, region, &dummy, &dummy, 0, NULL, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't map to 3D host image");

  origin[2] = 1;
  // map the second slice of the 3D image to a pointer for a frame of the second input video
  this->inputImage_second = (RGBA*) clEnqueueMapImage(this->cmdQ, this->host3D_in, CL_TRUE, CL_MAP_WRITE,
						      origin, region, &dummy, &dummy, 0, NULL, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't map to 3D host image");
  origin[2] = 0;
  // map the output image on the host to an RGBA pointer accessable by the host
  this->outputImage = (RGBA*) clEnqueueMapImage(this->cmdQ, this->host2D_out, CL_TRUE, CL_MAP_READ,
						origin, region, &dummy, NULL, 0, NULL, NULL, &this->errorCode);
  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't map to 2D host image"); 
}

BlendPipeline::~BlendPipeline(){
  cl_event unmapping[3];
  if(NULL != this->outputImage)
    clEnqueueUnmapMemObject(this->cmdQ, this->host2D_out, this->outputImage, 0, NULL, &unmapping[0]);
  if(NULL != this->inputImage_second)
    clEnqueueUnmapMemObject(this->cmdQ, this->host3D_in, this->inputImage_second, 0, NULL, &unmapping[1]);
  if(NULL != this->inputImage_first)
    clEnqueueUnmapMemObject(this->cmdQ, this->host3D_in, this->inputImage_first, 0, NULL, &unmapping[2]);
  // wait for the umapping to finish
  clWaitForEvents(3,unmapping);

  if(NULL != this->host2D_out)
    clReleaseMemObject(this->host2D_out);
  if(NULL != this->dev2D_out)
    clReleaseMemObject(this->dev2D_out);
  if(NULL != this->dev3D_in)
    clReleaseMemObject(this->dev3D_in);
  if(NULL != this->host3D_in)
    clReleaseMemObject(this->host3D_in);
}

cl_event BlendPipeline::copyToDevice(cl_event waitFor){
  if(NULL != waitFor)
    clWaitForEvents(1,&waitFor);

  // copy all 2 frames to the device
  size_t origin[3]={0,0,0},region[3]={this->width, this->height,2};
  this->errorCode = clEnqueueCopyImage(this->cmdQ, this->host3D_in, this->dev3D_in, 
					origin, origin, region,
					0, NULL, &this->copyToDevice_event);

  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't enqueue the copy process to the device");

  return this->copyToDevice_event;
}

cl_event BlendPipeline::copyFromDevice(cl_event waitFor){
  if(NULL != waitFor)
    clWaitForEvents(1,&waitFor);

  // copy all 2 frames to the device
  size_t origin[3]={0,0,0},region[3]={this->width, this->height,1};
  this->errorCode = clEnqueueCopyImage(this->cmdQ, this->dev2D_out, this->host2D_out, 
					origin, origin, region,
					0, NULL, &this->copyFromDevice_event);

  if(CL_SUCCESS != this->errorCode)
    this->errorHappened("couldn't enqueue the copy process from the device");

  return this->copyFromDevice_event;
}