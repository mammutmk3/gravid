/*
 * videoPipeline.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: lars
 */

#include "opencl/videoPipeline.h"

#include <CL/cl.h>
#include <iostream>

using namespace GRAVID;

VideoPipeline::VideoPipeline(cl_context ctx, cl_command_queue cmdQ, const unsigned int fifo_length,
								size_t image_width, size_t image_height){
	// initialize class members
	this->cmdQ = cmdQ;
	this->ctx = ctx;
	this->fifo_length = fifo_length;
	this->image_width = image_width;
	this->image_height = image_height;
	this->devImage2D_out = NULL;
	this->devImage3D_in = NULL;
	this->hostImage2D_in = NULL;
	this->hostImage2D_out = NULL;
	this->hostPixelData_in = NULL;
	this->hostImage2D_out = NULL;
	this->frame_pos = 0;

	// specify the image format that the images a represented with
	cl_image_format imgFmt;
	imgFmt.image_channel_data_type = CL_UNSIGNED_INT8;
	imgFmt.image_channel_order = CL_RGBA;

	this->hostImage2D_in = clCreateImage2D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D input image on the host");

	this->devImage3D_in = clCreateImage3D(this->ctx, CL_MEM_READ_ONLY, &imgFmt,
											this->image_width, this->image_height, this->fifo_length,
											0,0, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 3D input image on the device");

	this->devImage2D_out = clCreateImage2D(this->ctx, CL_MEM_WRITE_ONLY, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D output image on the device");

	this->hostImage2D_out = clCreateImage2D(this->ctx, CL_MEM_ALLOC_HOST_PTR, &imgFmt,
										this->image_width, this->image_height, 0, NULL,
										&this->errorCode);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't allocate 2D output image on the host");

	/**
	 * map the images on the host to standard pointers, so the decoder and encoder can access them
	 */
	size_t origin[3] = {0,0,0};
	size_t region[3] = {this->image_width, this->image_height, 1};
	size_t *dummy;
	this->hostPixelData_in = (RGBA*) clEnqueueMapImage(this->cmdQ, this->hostImage2D_in, CL_TRUE, CL_MAP_WRITE,
														origin, region, dummy, NULL, 0, NULL, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
			this->errorHappened("couldn't map to 2D input image");

	this->hostPixelData_out = (RGBA*) clEnqueueMapImage(this->cmdQ, this->hostImage2D_out, CL_TRUE, CL_MAP_READ,
														origin, region, dummy, NULL, 0, NULL, NULL, &this->errorCode);
	if(CL_SUCCESS != this->errorCode)
				this->errorHappened("couldn't map to 2D output image");
}

VideoPipeline::~VideoPipeline(){
	if(NULL != this->hostImage2D_in)
		clReleaseMemObject(this->hostImage2D_in);

	if(NULL != this->devImage3D_in)
			clReleaseMemObject(this->devImage3D_in);

	if(NULL != this->devImage2D_out)
			clReleaseMemObject(this->devImage2D_out);

	if(NULL != this->hostImage2D_out)
			clReleaseMemObject(this->hostImage2D_out);

	cl_event unmapping[2];
	if(NULL != this->hostPixelData_in)
		clEnqueueUnmapMemObject(this->cmdQ, this->hostImage2D_in, (void*)this->hostPixelData_in,
								0, NULL, &unmapping[0]);
	if(NULL != this->hostPixelData_out)
		clEnqueueUnmapMemObject(this->cmdQ, this->hostImage2D_out, (void*)this->hostPixelData_out,
								0, NULL, &unmapping[1]);
	// wait for the unmapping to finish
	clWaitForEvents(2, unmapping);
}

cl_event VideoPipeline::copyToDevice(cl_event last_kernel_launch){
	// calculate the image to replace
	size_t origin[3] = {0,0, this->frame_pos % this->fifo_length};
	size_t region[3] = {this->image_width, this->image_height, 1};
	// increase the counter
	this->frame_pos++;

	// do an asynchronous copy to the device
	if(NULL != last_kernel_launch)
			clWaitForEvents(1,&last_kernel_launch);
	this->errorCode = clEnqueueWriteImage(this->cmdQ, this->devImage3D_in, CL_FALSE, origin, region, 0, 0,
											(void*)this->hostPixelData_in, 0, NULL,
											&this->copyToDevice_event);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't enqueue copy process to the 3D image on the device");

	return this->copyToDevice_event;
}

void VideoPipeline::copyFromDevice(cl_event last_kernel_launch){
	size_t origin[3] = {0,0,0};
	size_t region[3] = {this->image_width, this->image_height, 1};
	// do an synchronous copy from the device
	if(NULL != last_kernel_launch)
		clWaitForEvents(1,&last_kernel_launch);
	this->errorCode = clEnqueueReadImage(this->cmdQ, this->devImage2D_out, CL_TRUE,
											origin, region, 0, 0,
											(void *)this->getResultFrame(),
											0, NULL, /*&this->copyFromDevice_event*/NULL);
	if(CL_SUCCESS != this->errorCode)
		this->errorHappened("couldn't enqueue copy process from the device");
}
